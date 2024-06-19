//
// Created by AC on 2024/5/14.
//

#pragma once


#ifdef _WIN32
#ifdef VP
#define DECL_VP __declspec(dllexport)
#else
#define DECL_VP __declspec(dllimport)
#endif
#else
#define DECL_VP __attribute__((visibility("default")))
#endif

#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else

#include <unistd.h>

#endif

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define RESET "\033[0m"

#include <opencv2/opencv.hpp>

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavutil/error.h>
#include <libavutil/ffversion.h>
}


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

std::string get_current_time_formatted(const std::string &format = "%Y-%m-%d %H:%M:%S");


class VPLogger {
public:
    static bool enable_info;
    static bool enable_warning;

    VPLogger() {
        line_ = "";
        prefix_ = "[FastDeploy]";
        verbose_ = true;
    }

    explicit VPLogger(bool verbose, const std::string &color, const std::string &prefix = "[VP]");

    template<typename T>
    VPLogger &operator<<(const T &val) {
        if (!verbose_) {
            return *this;
        }
        std::stringstream ss;
        ss << val;
        line_ += ss.str();
        return *this;
    }

    VPLogger &operator<<(std::ostream &(*os)(std::ostream &));

    ~VPLogger() {
        if (verbose_ && !line_.empty()) {
            std::cout << color_ << get_current_time_formatted() << " | " << prefix_ << " | " << line_ << color_end_
                      << std::endl;
        }
    }


private:
    std::string line_;
    std::string prefix_;
    std::string color_;
    bool verbose_ = true;
    std::string color_end_ = "\033[0m";
};


#ifndef __REL_FILE__
#define __REL_FILE__ __FILE__
#endif

#define VPERROR                                                                \
  VPLogger(true, RED, "[ERROR]")                                                    \
      << __REL_FILE__ << "(" << __LINE__ << ")::" << __FUNCTION__ << "\t"

#define VPWARNING                                                              \
  VPLogger(VPLogger::enable_warning, YELLOW, "[WARNING]")                  \
      << __REL_FILE__ << "(" << __LINE__ << ")::" << __FUNCTION__ << "\t"

#define VPINFO  VPLogger(VPLogger::enable_info, GREEN ,"[INFO]")



