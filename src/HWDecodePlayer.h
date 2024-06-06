//
// Created by AC on 2024/5/15.
//

#pragma once

#include "src/ai_utils.h"

extern "C" {
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

class HWDecodePlayer { ;

public:
    void play(const std::string& name);

    explicit HWDecodePlayer(const char *media_file_path, const char *hw_accel_device);

    ~HWDecodePlayer();

    bool init_parameters();

private:
    bool check_and_set_hw_accel();

    void init_ffmpeg_variables();

    void decode_and_show(const std::string& name);

    static int hw_decoder_init(AVCodecContext *ctx, enum AVHWDeviceType type);

    static AVPixelFormat get_hw_format(AVCodecContext *ctx, const AVPixelFormat *pix_fmts);

private:
    cv::Mat image_;
    bool exit_flag_ = false;
    bool is_hw_accel_ = false;
    int video_stream_index_ = -1;
    std::string hw_accel_device_;
    std::string media_file_path_;
    AVDictionary *options_ = nullptr;
    AVHWDeviceType hw_device_type_ = AV_HWDEVICE_TYPE_NONE;
    AVFormatContext *input_format_context_ = nullptr;
    AVCodec *video_codec_ = nullptr;
    AVCodecContext *video_codec_context_ = nullptr;
    AVPacket *packet_ = nullptr;
    AVFrame *yuv_frame_ = nullptr;
    AVFrame *bgr24_frame_ = nullptr;
    AVFrame *temp_frame_ = nullptr;
    AVFrame *nv12_frame_ = nullptr;
    SwsContext *sws_context_ = nullptr;
    inline static AVPixelFormat hw_pix_fmt_ = AV_PIX_FMT_NONE;
    inline static AVBufferRef *hw_device_ctx_ = nullptr;
};



