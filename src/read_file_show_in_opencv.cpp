//
// Created by AC on 2024/5/14.
//


#include "src/read_file_show_in_opencv.h"
#include "src/ai_utils.h"

extern "C" {
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}


int read_file_show(const char *input_filename) {
    AVDictionary *options = nullptr;
    if (is_network_media(input_filename)) {
        av_dict_set(&options, "buffer_size", "1024000", 0);
        av_dict_set(&options, "max_delay", "500000", 0);
        av_dict_set(&options, "rtsp_transport", "tcp", 0);
        //如果没有设置stimeout，那么流地址错误，av_read_frame会阻塞（时间单位是微妙）
        av_dict_set(&options, "stimeout", "2000000", 0);
    }
    // 打开输入文件
    AVFormatContext *input_format_context = avformat_alloc_context();
    if (avformat_open_input(&input_format_context, input_filename, nullptr, &options) != 0) {
        std::cout << "cant not open video file" << std::endl;
        return -1;
    }

    int video_stream_index = get_video_stream_index(input_format_context);
    if (video_stream_index == -1) {
        std::cout << "cant not find video stream" << std::endl;
        return -1;
    }
    // 获取编解码器参数
    AVCodecParameters *video_codec_parameters = input_format_context->streams[video_stream_index]->codecpar;
    // 查找解码器
    const AVCodec *video_codec = avcodec_find_decoder(video_codec_parameters->codec_id);
    if (video_codec == nullptr) {
        std::cout << "cant note find video decoder" << std::endl;
        return -1;
    }

    // 创建解码器上下文
    AVCodecContext *video_codec_context = avcodec_alloc_context3(nullptr);
    if (avcodec_parameters_to_context(video_codec_context, video_codec_parameters) != 0) {
        std::cout << "con not create videoCodecContext" << std::endl;
        return -1;
    }
    // 打开解码器
    if (avcodec_open2(video_codec_context, video_codec, nullptr) != 0) {
        std::cout << "cant open video codec" << std::endl;
        return -1;
    }
    AVFrame *bgr24_frame = av_frame_alloc();
    cv::Mat mat(video_codec_context->height, video_codec_context->width, CV_8UC3);
    av_image_fill_arrays(bgr24_frame->data, bgr24_frame->linesize, (uint8_t *) mat.data,
                         (AVPixelFormat) AV_PIX_FMT_BGR24,
                         video_codec_context->width, video_codec_context->height, 1);
    SwsContext *sws_context = sws_getContext(video_codec_context->width, video_codec_context->height,
                                             video_codec_context->pix_fmt, video_codec_context->width,
                                             video_codec_context->height,
                                             (AVPixelFormat) AV_PIX_FMT_BGR24, SWS_BICUBIC,
                                             nullptr, nullptr, nullptr);

    AVPacket *packet = av_packet_alloc();
    AVFrame *yuv_frame = av_frame_alloc();
    while (av_read_frame(input_format_context, packet) >= 0) {
        if (packet->stream_index == video_stream_index) {
            if (avcodec_send_packet(video_codec_context, packet) != 0) continue;
            while (avcodec_receive_frame(video_codec_context, yuv_frame) == 0) {
                sws_scale(sws_context, yuv_frame->data,
                          yuv_frame->linesize, 0,
                          yuv_frame->height,
                          bgr24_frame->data,
                          bgr24_frame->linesize);
                if (mat.empty()) continue;
                cv::imshow("", mat);
                if (cv::waitKey(30) == char('q')) break;
            }
        }
        av_packet_unref(packet);
    }

    av_packet_free(&packet);
    av_frame_free(&yuv_frame);
    av_frame_free(&bgr24_frame);
    avcodec_free_context(&video_codec_context);
    avformat_close_input(&input_format_context);
    avformat_free_context(input_format_context);
    return 0;
}
