//
// Created by AC on 2024/5/14.
//

#pragma once

#include "log.h"

#include <opencv2/opencv.hpp>

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavutil/error.h>
#include <libavutil/ffversion.h>
}

#ifdef _WIN32
#ifdef VP
#define DECL_VP __declspec(dllexport)
#else
#define DECL_VP __declspec(dllimport)
#endif
#else
#define DECL_VP __attribute__((visibility("default")))
#endif


void av_frame_to_mat(AVFrame *yuv_frame, AVFrame *bgr24_frame, SwsContext *sws_context);

AVFrame *mat_to_av_frame(cv::Mat &image, AVPixelFormat pix_format = AV_PIX_FMT_YUV420P);

/// 获取format上下文中视频流的索引
/// \param format_context
/// \return 视频流的索引
int get_video_stream_index(AVFormatContext *format_context);

std::string get_av_error(int err_num);

bool start_with(const std::string &str, const std::string &sub);

bool end_with(const std::string &str, const std::string &sub);

bool is_network_media(const std::string &media_file_str);

bool is_contain(const std::string &str, const std::string &substr);


