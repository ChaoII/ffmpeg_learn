//
// Created by AC on 2024/5/16.
//


#include "PushOpenCVRtsp.h"

#include <utility>

PushOpenCVRtsp::PushOpenCVRtsp(PushStreamParameter parameter) : parameter_(std::move(parameter)) {
    PushOpenCVRtsp::initial_lib();
}

int PushOpenCVRtsp::push() {
    cv::Mat frame;
    AVFrame *yuv = nullptr;
    long pts = 0;
    AVPacket *pack = av_packet_alloc();
    if (!pack) {
        std::cerr << "Failed to allocate AVPacket" << std::endl;
        return AVERROR(ENOMEM);
    }
    if (!(output_format_context_->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&output_format_context_->pb, parameter_.out_url.c_str(), AVIO_FLAG_WRITE) < 0) {
            std::cerr << "Could not open output file" << std::endl;
            return -1;
        }
    }
    int ret = avformat_write_header(output_format_context_, nullptr);
    if (ret < 0) {
        std::cerr << "Error occurred when writing header: " << get_av_error(ret) << std::endl;
        av_packet_free(&pack);
        return ret;
    }
    while (!stop_signal_) {
        frame = pop_dst_frame();
        if (frame.empty()) {
            continue;
        }
        yuv = mat_to_av_frame(frame);
        if (!yuv) {
            std::cerr << "Failed to convert Mat to AVFrame" << std::endl;
            continue;
        }
        yuv->pts = pts;
        pts += 1;
        ret = avcodec_send_frame(video_codec_context_, yuv);
        if (ret != 0) {
            std::cerr << "Error sending frame to encoder: " << get_av_error(ret) << std::endl;
            av_frame_free(&yuv);
            continue;
        }
        ret = avcodec_receive_packet(video_codec_context_, pack);
        if (ret != 0 || pack->size <= 0) {
            av_frame_free(&yuv);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                continue;
            } else {
                std::cerr << "Error receiving packet from encoder: " << get_av_error(ret) << std::endl;
                continue;
            }
        }

        if (pack->dts < 0 || pack->pts < 0 || pack->dts > pack->pts) {
            pack->dts = pack->pts = pack->duration = 0;
        }
        pack->pts = av_rescale_q(pack->pts, video_codec_context_->time_base, out_av_stream_->time_base); // 显示时间
        pack->dts = av_rescale_q(pack->dts, video_codec_context_->time_base, out_av_stream_->time_base); // 解码时间
        pack->duration = av_rescale_q(pack->duration, video_codec_context_->time_base,
                                      out_av_stream_->time_base); // 数据时长
        ret = av_interleaved_write_frame(output_format_context_, pack);
        if (ret < 0) {
            std::cerr << "Error sending packet to output: " << get_av_error(ret) << std::endl;
            av_frame_free(&yuv);
            av_packet_unref(pack);
            continue;
        }
        av_frame_free(&yuv);
        av_packet_unref(pack);
    }
    av_packet_free(&pack);
    return ret;
}

int PushOpenCVRtsp::open_codec() {
    // 硬编码器
    const AVCodec *encoder = nullptr;
    int ret = set_encoder(&encoder);
    if (ret != 0) {
        return AVERROR(ENOMEM);
    }
    // 创建编码器上下文
    if (!(video_codec_context_ = avcodec_alloc_context3(encoder))) {
        return AVERROR(ENOMEM);
    }
    // 配置编码器参数
    video_codec_context_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER; // 全局参数
    video_codec_context_->flags |= AV_CODEC_FLAG2_LOCAL_HEADER;
    video_codec_context_->flags |= AV_CODEC_FLAG_LOW_DELAY;
    video_codec_context_->codec_id = encoder->id;
    video_codec_context_->codec_type = AVMEDIA_TYPE_VIDEO;
    video_codec_context_->thread_count = parameter_.thread_nums;
    video_codec_context_->bit_rate = parameter_.bit_rate;// 压缩后每秒视频的bit位大小为50kb
    video_codec_context_->width = parameter_.width;
    video_codec_context_->height = parameter_.height;
    video_codec_context_->time_base = {1, parameter_.frame_rate};
    video_codec_context_->framerate = {parameter_.frame_rate, 1};
    video_codec_context_->gop_size = parameter_.gop_size;
    video_codec_context_->max_b_frames = parameter_.max_b_frame;
    video_codec_context_->qmax = parameter_.q_max;
    video_codec_context_->qmin = parameter_.q_min;
    video_codec_context_->pix_fmt = AV_PIX_FMT_YUV420P;
    AVDictionary *options = nullptr;


    initial_av_options(encoder, options);
    // 打开编码器上下文
    if ((ret = avcodec_open2(video_codec_context_, encoder, &options)) < 0) {
        std::cout << "avcodec_open2 failed: " << get_av_error(ret) << std::endl;
        return ret;
    }
    // 创建输出包装
    ret = avformat_alloc_output_context2(&output_format_context_, nullptr, "rtsp", parameter_.out_url.c_str());
    // 创建输出流
    out_av_stream_ = avformat_new_stream(output_format_context_, encoder);
    out_av_stream_->codecpar->codec_tag = 0;
    // 从编码器复制参数
    avcodec_parameters_from_context(out_av_stream_->codecpar, video_codec_context_);
    return ret;
}

cv::Mat PushOpenCVRtsp::pop_src_frame() {
    std::unique_lock<std::mutex> lock(src_queue_mutex_);
    src_queue_cv_.wait(lock, [this]() { return !src_images_.empty(); });
    // 转移所有权避免拷贝
    cv::Mat tmp = std::move(src_images_.front());
    src_images_.pop();
    return tmp;
}

cv::Mat PushOpenCVRtsp::pop_dst_frame() {
    std::unique_lock<std::mutex> lock(dst_queue_mutex_);
    dst_queue_cv_.wait(lock, [this]() { return !dst_images_.empty(); });
    // 转移所有权避免拷贝
    std::cout << "dst_images size: " << dst_images_.size() << std::endl;
    cv::Mat tmp = std::move(dst_images_.front());
    dst_images_.pop();
    return tmp;
}

void PushOpenCVRtsp::push_src_frame(cv::Mat &&frame) {
    {
        std::unique_lock<std::mutex> lock(src_queue_mutex_);
        if (src_images_.size() < 256) {
            // 转移所有权避免复制
            src_images_.push(std::move(frame));
        }
    }
    src_queue_cv_.notify_all();
}

void PushOpenCVRtsp::push_dst_frame(cv::Mat &&frame) {
    {
        std::unique_lock<std::mutex> lock(dst_queue_mutex_);
        if (dst_images_.size() < 256) {
            // 转移所有权避免复制
            dst_images_.push(std::move(frame));
        }
    }
    dst_queue_cv_.notify_all();
}

void PushOpenCVRtsp::start() {
    push_thread_ = std::thread(&PushOpenCVRtsp::push, this);
    stop_signal_ = false;
    push_thread_.detach();
    initial_models({ModelType::FACE_DETECT});

    start_video_analysis();
}


void PushOpenCVRtsp::stop() {
    stop_signal_ = true;
}

int PushOpenCVRtsp::set_encoder(const AVCodec **encoder) const {
    if (parameter_.hw_accel == "none") {
        // 默认使用h264软编码
        *encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
    } else {
        *encoder = avcodec_find_encoder_by_name(parameter_.hw_accel.c_str());
    }
    if (!*encoder) {
        std::cerr << "can't find encoder, please chose other encoders!" << std::endl;
        return -1;
    }
    std::cout << "find encoder: " << (*encoder)->long_name << std::endl;
    return 0;
}

void PushOpenCVRtsp::initial_lib() {
    if (!library_initialed_) {
        //初始化网络流格相关(使用网络流时必须先执行)
        avformat_network_init();
        //设置日志级别
        //如果不想看到烦人的打印信息可以设置成 AV_LOG_QUIET 表示不打印日志
        //有时候发现使用不正常比如打开了没法播放视频则需要打开日志看下报错提示
        av_log_set_level(AV_LOG_QUIET);
        std::cout << "initial ffmpeg success, ffmpeg version: " << FFMPEG_VERSION << std::endl;
        library_initialed_ = true;
    }
}

void PushOpenCVRtsp::set_hw_accel(const std::string &hw_accel_name) {
    parameter_.hw_accel = hw_accel_name;
}

void PushOpenCVRtsp::initial_av_options(const AVCodec *encoder, AVDictionary *options) {
    av_dict_set(&options, "rc", "cbr", 0);
    av_dict_set(&options, "rc-lookahead", "0", 0);
    av_dict_set(&options, "delay", "0", 0);
    av_dict_set(&options, "zerolatency", "0", 0);
    av_dict_set(&options, "strict_gop", "1", 0);
    if (is_contain(encoder->long_name, "VideoToolbox")) {
        av_dict_set(&options, "profile", "baseline", 0);
    }
    // 硬件加速选项
    av_dict_set(&options, "preset", "fast", 0); // 使用硬件加速器
    av_dict_set(&options, "gpu", "0", 0); // 指定 GPU
}

void PushOpenCVRtsp::set_resolution(int width, int height) {
    parameter_.width = width;
    parameter_.height = height;
}

void PushOpenCVRtsp::set_frame_rate(int frame_rate) {
    parameter_.frame_rate = frame_rate;
}


void PushOpenCVRtsp::initial_models(const std::vector<ModelType> &model_types) {
    for (auto &model_type: model_types) {
        analysis_.emplace_back(new VideoAnalysis(model_type));
    }
}

cv::Mat PushOpenCVRtsp::predict(cv::Mat &image) {
    for (auto &analysis: analysis_) {
        image = analysis->predict(image);
    }
    return image;
}

void PushOpenCVRtsp::start_video_analysis() {
    std::thread([=]() {
        while (!stop_analysis_) {
            auto image = pop_src_frame();
            auto im = predict(image);
            push_dst_frame(std::move(im));
        }
    }).detach();
}

void PushOpenCVRtsp::stop_analysis() {
    stop_analysis_ = true;
}

PushOpenCVRtsp::~PushOpenCVRtsp() {
    avcodec_free_context(&video_codec_context_);
    avformat_close_input(&output_format_context_);
    for (auto &analysis: analysis_) {
        delete analysis;
    }
    analysis_.clear();
}






