//
// Created by AC on 2024/5/16.
//

#pragma once

#include "src/ai_utils.h"
#include <thread>

class PushOpenCVRtsp {
public:
    explicit PushOpenCVRtsp(const char *dst_url);

    int open_codec(int width, int height, int den);

    void push_frame(cv::Mat &frame);

    void start();

    void stop();

private:

    int push();

    cv::Mat pop_one_frame();

private:

    bool stop_signal_ = true;
    std::thread push_thread_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;
    std::queue<cv::Mat> pic_buffer_;
    AVStream *out_av_stream_ = nullptr;
    std::string dst_url_;
    AVFormatContext *output_format_context_ = nullptr;
    AVCodec *video_codec_ = nullptr;
    AVCodecContext *video_codec_context_ = nullptr;
};


