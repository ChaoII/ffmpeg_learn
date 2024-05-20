//
// Created by AC on 2024/5/16.
//

#include "PushOpenCVRtsp.h"
//#include <chrono>


char av_error[AV_ERROR_MAX_STRING_SIZE] = {0};
#define av_err2str(err_num) av_make_error_string(av_error, AV_ERROR_MAX_STRING_SIZE, err_num)

PushOpenCVRtsp::PushOpenCVRtsp(const char *dst_url, const char *hw_accel) {
    dst_url_ = dst_url;
    hw_accel_ = hw_accel;
}

int PushOpenCVRtsp::push() {
    int ret = 0;
    cv::Mat frame;
    AVFrame *yuv = nullptr;
    long pts = 0;
    AVPacket *pack = av_packet_alloc();
    if (!pack) {
        std::cerr << "Failed to allocate AVPacket" << std::endl;
        return AVERROR(ENOMEM);
    }
    if (!(output_format_context_->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&output_format_context_->pb, dst_url_.c_str(), AVIO_FLAG_WRITE) < 0) {
            std::cerr << "Could not open output file" << std::endl;
            return -1;
        }
    }
    ret = avformat_write_header(output_format_context_, nullptr);
    if (ret < 0) {
        std::cerr << "Error occurred when writing header: " << av_err2str(ret) << std::endl;
        av_packet_free(&pack);
        return ret;
    }
    while (!stop_signal_) {
        frame = pop_one_frame();
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
            std::cerr << "Error sending frame to encoder: " << av_err2str(ret) << std::endl;
            av_frame_free(&yuv);
            continue;
        }
        ret = avcodec_receive_packet(video_codec_context_, pack);
        if (ret != 0 || pack->size <= 0) {
            av_frame_free(&yuv);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                continue;
            } else {
                std::cerr << "Error receiving packet from encoder: " << av_err2str(ret) << std::endl;
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
            std::cerr << "Error sending packet to output: " << av_err2str(ret) << std::endl;
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


int PushOpenCVRtsp::open_codec(int width, int height, int den) {
    int ret = 0;
    avformat_network_init();
    av_log_set_level(AV_LOG_DEBUG); //启用日志
    // 硬编码器
    const AVCodec *encoder = nullptr;
    // hevc_nvenc h264_nvenc,h264_videotoolbox
    ret = set_encoder(&encoder, hw_accel_.c_str());
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
    video_codec_context_->thread_count = 10;
    video_codec_context_->bit_rate = 50 * 1024 * 8; // 压缩后每秒视频的bit位大小为50kb
    video_codec_context_->width = width;
    video_codec_context_->height = height;
    video_codec_context_->time_base = {1, den};
    video_codec_context_->framerate = {den, 1};
    video_codec_context_->gop_size = 30;
    video_codec_context_->max_b_frames = 2;
    video_codec_context_->qmax = 51;
    video_codec_context_->qmin = 10;
    video_codec_context_->pix_fmt = AV_PIX_FMT_YUV420P;


    AVDictionary *options = nullptr;
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

    // 打开编码器上下文
    if ((ret = avcodec_open2(video_codec_context_, encoder, &options)) < 0) {
        std::cout << "avcodec_open2 failed!" << std::endl;
        return ret;
    }
    // 创建输出包装
    ret = avformat_alloc_output_context2(&output_format_context_, nullptr, "rtsp", dst_url_.c_str());
    // 创建输出流
    out_av_stream_ = avformat_new_stream(output_format_context_, encoder);
    out_av_stream_->codecpar->codec_tag = 0;
    // 从编码器复制参数
    avcodec_parameters_from_context(out_av_stream_->codecpar, video_codec_context_);
    return ret;
}

cv::Mat PushOpenCVRtsp::pop_one_frame() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    cv_.wait(lock, [this]() { return !pic_buffer_.empty(); });
    // 转移所有权避免拷贝
    cv::Mat tmp = std::move(pic_buffer_.front());
    pic_buffer_.pop();
    return tmp;
}

void PushOpenCVRtsp::push_frame(cv::Mat &&frame) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        if (pic_buffer_.size() < 256) {
            // 转移所有权避免复制
            pic_buffer_.push(std::move(frame));
        }
    }
    cv_.notify_all();
}

void PushOpenCVRtsp::start() {
    push_thread_ = std::thread(&PushOpenCVRtsp::push, this);
    stop_signal_ = false;
    push_thread_.detach();
}

void PushOpenCVRtsp::stop() {
    stop_signal_ = true;
}

// hevc_nvenc h264_nvenc
int PushOpenCVRtsp::set_encoder(const AVCodec **encoder, const char *hw_encoder_name) {
    if (std::string(hw_encoder_name).empty()) {
        *encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
        std::cerr << "use cpu encode" << std::endl;
    } else {
        *encoder = avcodec_find_encoder_by_name(hw_encoder_name);
        std::cerr << "use hw accelerate encode" << std::endl;
    }
    if (!*encoder) {
        std::cerr << "Can't find encoder!" << std::endl;
        return -1;
    }
    std::cout << "find encoder: " << (*encoder)->long_name << std::endl;
    return 0;
}
