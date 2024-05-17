//
// Created by AC on 2024/5/16.
//

#include "PushOpenCVRtsp.h"
//#include <chrono>

PushOpenCVRtsp::PushOpenCVRtsp(const char *dst_url) {
    dst_url_ = dst_url;
}

int PushOpenCVRtsp::push() {
    int ret = 0;
    cv::Mat frame;
    AVFrame *yuv;
    long pts = 0;
    AVPacket pack;

    ret = avformat_write_header(output_format_context_, nullptr);

    while (!stop_signal_) {
        frame = pop_one_frame();
        yuv = mat_to_av_frame(frame);
        yuv->pts = pts;
        pts += 1;
        ret = avcodec_send_frame(video_codec_context_, yuv);
        if (ret != 0) {
            av_free(&pack);
            continue;
        }
        ret = avcodec_receive_packet(video_codec_context_, &pack);
        if (ret != 0 || pack.size > 0) {
        } else {
            av_frame_free(&yuv);
            av_free(&pack);
            continue;
        }
        int firstFrame = 0;
        if (pack.dts < 0 || pack.pts < 0 || pack.dts > pack.pts || firstFrame) {
            firstFrame = 0;
            pack.dts = pack.pts = pack.duration = 0;
        }
        pack.pts = av_rescale_q(pack.pts, video_codec_context_->time_base, out_av_stream_->time_base); // 显示时间
        pack.dts = av_rescale_q(pack.dts, video_codec_context_->time_base, out_av_stream_->time_base); // 解码时间
        pack.duration = av_rescale_q(pack.duration, video_codec_context_->time_base, out_av_stream_->time_base); // 数据时长
        ret = av_interleaved_write_frame(output_format_context_, &pack);
        if (ret < 0) {
            std::cerr << "send packet error" << std::endl;
            av_frame_free(&yuv);
            av_free(&pack);
            continue;
        }
        av_frame_unref(yuv);
        av_packet_unref(&pack);
    }
    return ret;
}


int PushOpenCVRtsp::open_codec(int width, int height, int den) {
    int ret = 0;
    avformat_network_init();
//    auto h264_codec = avcodec_find_encoder_by_name("hevc_nvenc");

    const AVCodec *h264_codec = avcodec_find_encoder(AV_CODEC_ID_H265);

    std::cout << h264_codec->long_name << std::endl;
    std::cout << *h264_codec->pix_fmts << std::endl;


    if (!h264_codec) {
        throw std::logic_error("Can't find h264 encoder!"); // 找不到264编码器
    }
    // 创建编码器上下文
    if (!(video_codec_context_ = avcodec_alloc_context3(h264_codec))) {
        return AVERROR(ENOMEM);
    }
    // 配置编码器参数
    video_codec_context_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER; // 全局参数
    video_codec_context_->flags |= AV_CODEC_FLAG2_LOCAL_HEADER;
    video_codec_context_->flags |= AV_CODEC_FLAG_LOW_DELAY;
    video_codec_context_->codec_id = h264_codec->id;
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
    video_codec_context_->pix_fmt = *h264_codec->pix_fmts;

    AVDictionary *options = nullptr;
    av_dict_set(&options, "rc", "cbr", 0);
    av_dict_set(&options, "rc-lookahead", "0", 0);
    av_dict_set(&options, "delay", "0", 0);
    av_dict_set(&options, "zerolatency", "1", 0);
    av_dict_set(&options, "strict_gop", "1", 0);
//    av_dict_set(&options, "profile", "high", 0);


    // 打开编码器上下文
    if ((ret = avcodec_open2(video_codec_context_, h264_codec, &options)) < 0) {
        std::cout << "avcodec_open2 failed!" << std::endl;
        return ret;
    }
    // 创建输出包装
    ret = avformat_alloc_output_context2(&output_format_context_, nullptr, "rtsp", dst_url_.c_str());
    // 创建输出流
    out_av_stream_ = avformat_new_stream(output_format_context_, h264_codec);
    out_av_stream_->codecpar->codec_tag = 0;
    // 从编码器复制参数
    avcodec_parameters_from_context(out_av_stream_->codecpar, video_codec_context_);
    return ret;
}

cv::Mat PushOpenCVRtsp::pop_one_frame() {
    while (true) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        cv_.wait(lock, [this]() { return !pic_buffer_.empty(); });
        std::cout << "=====" << pic_buffer_.size() << "=====" << std::endl;
        cv::Mat tmp = pic_buffer_.front().clone();
        pic_buffer_.pop();
        return tmp;
    }
}

void PushOpenCVRtsp::push_frame(cv::Mat &frame) {
    if (pic_buffer_.size() < 256) {
        std::cout << "---" << pic_buffer_.size() << "-----" << std::endl;
        std::unique_lock<std::mutex> lock(queue_mutex_);
        pic_buffer_.push(frame);
        cv_.notify_all();
    }
}

void PushOpenCVRtsp::start() {
    push_thread_ = std::thread(&PushOpenCVRtsp::push, this);
    stop_signal_ = false;
    push_thread_.detach();
}

void PushOpenCVRtsp::stop() {
    stop_signal_ = true;
}
