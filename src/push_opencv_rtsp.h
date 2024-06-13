//
// Created by AC on 2024/5/16.
//

#pragma once

#include "src/ai_utils.h"
#include <thread>
#include <queue>
#include "video_analysis.h"


struct PushStreamParameter {
    std::string out_url = "rtsp://172.168.1.112:8554/live/test";
    // hevc_nvenc h264_nvenc,h264_videotoolbox
    std::string hw_accel = "none";
    int thread_nums = 1;
    // 50 * 1024 * 8
    int bit_rate = 409600;
    int width = 640;
    int height = 480;
    int frame_rate = 25;
    int gop_size = 30;
    // 想要支持webrtc不能使用b帧
    int max_b_frame = 0;
    int q_min = 10;
    int q_max = 51;
    AVPixelFormat pix_format = AV_PIX_FMT_YUV420P;
};


class push_opencv_rtsp {
public:
    explicit push_opencv_rtsp(PushStreamParameter parameter);

    int open_codec();

    void push_src_frame(cv::Mat &&frame);

    void push_dst_frame(cv::Mat &&frame);

    void start();

    void stop();

    int set_encoder(const AVCodec **encoder) const;

    void set_hw_accel(const std::string &hw_accel_name);

    void set_resolution(int width, int height);

    void set_frame_rate(int frame_rate);

    void initial_av_options(const AVCodec *encoder, AVDictionary *options);

    static void initial_lib();

    void initial_models(const std::vector<ModelType> &model_types);

    void stop_analysis();

    cv::Mat predict(cv::Mat &image);

    ~push_opencv_rtsp();

private:

    void analysis();

    int push();

    cv::Mat pop_dst_frame();

    cv::Mat pop_src_frame();

private:

    PushStreamParameter parameter_;
    bool stop_signal_ = true;
    std::thread analysis_thread_;
    std::thread push_thread_;
    std::mutex src_queue_mutex_;
    std::mutex dst_queue_mutex_;
    std::condition_variable src_queue_cv_;
    std::condition_variable dst_queue_cv_;
    std::queue<cv::Mat> src_images_;
    std::queue<cv::Mat> dst_images_;
    AVStream *out_av_stream_ = nullptr;
    AVFormatContext *output_format_context_ = nullptr;
    AVCodecContext *video_codec_context_ = nullptr;
    inline static bool library_initialed_ = false;
    bool stop_analysis_ = false;
    std::vector<video_analysis *> analysis_;
};

