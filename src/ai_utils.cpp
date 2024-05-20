//
// Created by AC on 2024/5/14.
//

#include "ai_utils.h"

extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
}

cv::Mat av_frame_to_mat(AVFrame *yuv_frame, SwsContext *sws_context) {

    // 如果是硬解，那么格式为NV12
    //得到AVFrame信息
    int src_width = yuv_frame->width;
    int src_height = yuv_frame->height;
    cv::Mat mat;
    if (src_height == 0 || src_width == 0) {
        return mat;
    }
    //生成Mat对象
    mat.create(cv::Size(src_width, src_height), CV_8UC3);
    //格式转换，直接填充Mat的数据data
    AVFrame *bgr24Frame = av_frame_alloc();
    av_image_fill_arrays(bgr24Frame->data, bgr24Frame->linesize, (uint8_t *) mat.data, (AVPixelFormat) AV_PIX_FMT_BGR24,
                         src_width, src_height, 1);
    sws_scale(sws_context, (const uint8_t *const *) yuv_frame->data, yuv_frame->linesize, 0, src_height,
              bgr24Frame->data,
              bgr24Frame->linesize);
    //释放
    av_frame_free(&bgr24Frame);
    return mat;
}


AVFrame *mat_to_av_frame(cv::Mat &image, AVPixelFormat pix_format) {
    //得到Mat信息
    int width = image.cols;
    int height = image.rows;


    //创建AVFrame填充参数 注：调用者释放该frame
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        std::cerr << "Failed to allocate AVFrame" << std::endl;
        return nullptr;
    }
    frame->width = width;
    frame->height = height;
    frame->format = pix_format;
    //初始化AVFrame内部空间
    int ret = av_frame_get_buffer(frame, 32);
    if (ret < 0) {
        std::cout << "Could not allocate the video frame data" << std::endl;
        av_frame_free(&frame);
        return nullptr;
    }
    ret = av_frame_make_writable(frame);
    if (ret < 0) {
        std::cout << "Av frame make writable failed." << std::endl;
        av_frame_free(&frame);
        return nullptr;
    }
    //转换颜色空间为YUV420//如果颜色格式很多，那么需要switch case
    cv::cvtColor(image, image, cv::COLOR_BGR2YUV_I420);
    //按YUV420格式，设置数据地址
    int frame_size = width * height;
    unsigned char *data = image.data;
    memcpy(frame->data[0], data, frame_size);
    memcpy(frame->data[1], data + frame_size, frame_size / 4);
    memcpy(frame->data[2], data + frame_size * 5 / 4, frame_size / 4);
    return frame;
}

/// 获取format上下文中视频流的索引
/// \param format_context
/// \return 视频流的索引
int get_video_stream_index(AVFormatContext *format_context) {
    auto ret = avformat_find_stream_info(format_context, nullptr);
    if (ret != 0) {
        std::cout << "Failed to get stream info" << std::endl;
        exit(ret);
    }
    int video_stream_index = -1;
    for (int i = 0; i < format_context->nb_streams; i++) {
        if (format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }
    return video_stream_index;
}

bool is_network_media(const std::string &media_file_str) {
    return start_with(media_file_str, "rtsp://") || start_with(media_file_str, "rtmp://") ||
           start_with(media_file_str, "http://") || start_with(media_file_str, "ws://") ||
           start_with(media_file_str, "rtc://");
}

bool start_with(const std::string &str, const std::string &sub) {
    return str.find(sub) == 0;
}

bool end_with(const std::string &str, std::string &sub) {
    return str.rfind(sub) == (str.length() - sub.length());
}

bool is_contain(const std::string &str, const std::string &substr) {
    return str.find(substr) != std::string::npos;
}

