//
// Created by AC on 2024/5/14.
//

#include "src/hw_decode_show_in_opencv.h"
#include "src/ai_utils.h"

extern "C" {
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}


typedef struct DecodeContext {
    AVBufferRef *bw_device_ref;
} DecodeContext;

DecodeContext decode = {};

static enum AVPixelFormat hw_pix_fmt;
static AVBufferRef *hw_device_ctx = nullptr;

AVPixelFormat get_hw_format(AVCodecContext *ctx, const AVPixelFormat *pix_formats) {
    const enum AVPixelFormat *p;
    for (p = pix_formats; *p != -1; p++) {
        if (*p == hw_pix_fmt)
            return *p;
    }
    std::cerr << "Failed to get HW surface format." << std::endl;
    return AV_PIX_FMT_NONE;
}

int hw_decoder_init(AVCodecContext *ctx, const AVHWDeviceType type) {
    int err = 0;
    if ((err = av_hwdevice_ctx_create(&hw_device_ctx, type,
                                      nullptr, nullptr, 0)) < 0) {
        std::cerr << "Failed to create specified HW device." << std::endl;
        return err;
    }
    ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);

    return err;
}


int hw_read_file_show(const char *input_filename) {
    AVDictionary *options = nullptr;
    if (is_network_media(input_filename)) {
        av_dict_set(&options, "buffer_size", "1024000", 0);
        av_dict_set(&options, "max_delay", "500000", 0);
        av_dict_set(&options, "rtsp_transport", "tcp", 0);
        //如果没有设置stimeout，那么流地址错误，av_read_frame会阻塞（时间单位是微妙）
        av_dict_set(&options, "stimeout", "2000000", 0);
    }

    AVFormatContext *input_format_context = avformat_alloc_context();
    if (avformat_open_input(&input_format_context, input_filename, nullptr, &options) != 0) {
        std::cout << "cant not open video file" << std::endl;
        return -1;
    }

    if (avformat_find_stream_info(input_format_context, &options) < 0) {
        std::cerr << "cant find input stream information." << std::endl;
        return -1;
    }
    AVCodec *video_codec = nullptr;
    int video_stream_index = av_find_best_stream(input_format_context, AVMEDIA_TYPE_VIDEO, -1, -1,
                                                 (const AVCodec **) &video_codec, 0);
    if (video_stream_index < 0) {
        std::cerr << "Cannot find a video stream in the input file" << std::endl;
        return -1;
    }

    //获取支持该decoder的hw配置型
    AVHWDeviceType type = av_hwdevice_find_type_by_name("cuda");
    std::cerr << std::endl;
    std::cout << av_hwdevice_get_type_name(type) << std::endl;
    if (type == AV_HWDEVICE_TYPE_NONE) {
        std::cerr << "Device type" << av_hwdevice_get_type_name(type) << "is not supported." << std::endl;
        std::cerr << "Available device types:" << std::endl;
        while ((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE)
            std::cerr << av_hwdevice_get_type_name(type) << std::endl;
        std::cerr << std::endl;
        return -1;
    }

    int i;
    for (i = 0;; i++) {
        const AVCodecHWConfig *config = avcodec_get_hw_config(video_codec, i);
        if (!config) {
            std::cerr << "Decoder " << video_codec->name << " does not support device type "
                      << av_hwdevice_get_type_name(type) << std::endl;
            return 0;
        }
        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
            config->device_type == type) {
            hw_pix_fmt = config->pix_fmt;
            break;
        }
    }


    std::cout << "--------: " << hw_pix_fmt << std::endl;

    // 创建解码器上下文
    AVCodecContext *video_codec_context = avcodec_alloc_context3(video_codec);
    AVStream *video_stream = input_format_context->streams[video_stream_index];
    if (!video_codec_context) return 0;
    if (avcodec_parameters_to_context(video_codec_context, video_stream->codecpar) != 0) {
        std::cout << "con not create videoCodecContext" << std::endl;
        return -1;
    }
    video_codec_context->get_format = get_hw_format;
    if (hw_decoder_init(video_codec_context, type) < 0) {
        return 0;
    }
    // 打开解码器
    if (avcodec_open2(video_codec_context, video_codec, nullptr) != 0) {
        std::cout << "cant open video codec" << std::endl;
        return -1;
    }
    //----------------将bgr24_frame与Mat进行绑定，实现了frame和Mat的内存同步，即bgr24_frame变化，mat也就会变化-------------
    // 那么bgr24_frame的内存将被mat接管，无需手动释放bgr24_frame
    AVFrame *bgr24_frame = av_frame_alloc();
    cv::Mat mat(video_codec_context->height, video_codec_context->width, CV_8UC3);

    SwsContext *sws_context = sws_getContext(video_codec_context->width, video_codec_context->height,
                                             AVPixelFormat::AV_PIX_FMT_NV12, video_codec_context->width,
                                             video_codec_context->height,
                                             AVPixelFormat::AV_PIX_FMT_BGR24, SWS_BICUBIC,
                                             nullptr, nullptr, nullptr);
    av_image_fill_arrays(bgr24_frame->data, bgr24_frame->linesize, mat.data,
                         AVPixelFormat::AV_PIX_FMT_BGR24, video_codec_context->width, video_codec_context->height, 1);

    AVPacket *packet = av_packet_alloc();
    AVFrame *yuv_frame = av_frame_alloc();
    AVFrame *nv12_frame = av_frame_alloc();

    while (av_read_frame(input_format_context, packet) >= 0) {
        if (packet->stream_index == video_stream_index) {
            if (avcodec_send_packet(video_codec_context, packet) != 0) continue;
            while (avcodec_receive_frame(video_codec_context, yuv_frame) == 0) {
                // 如果是硬解码，这个很重要，必须从硬件中转换出来
                if (yuv_frame->format == video_codec_context->pix_fmt) {
                    if (av_hwframe_transfer_data(nv12_frame, yuv_frame, 0) < 0) {
                        continue;
                    }
                }
                // 实现格式转换nv12-bgr24
                sws_scale(sws_context, nv12_frame->data,
                          nv12_frame->linesize, 0,
                          nv12_frame->height,
                          bgr24_frame->data,
                          bgr24_frame->linesize);
                // 如果此处不拷贝，那么不要释放bgr24_frame，因为mat会自动释放内存，连带释放掉bgr24_frame
                auto image = mat.clone();
                if (image.empty()) continue;
                cv::imshow("", image);
                if (cv::waitKey(30) == char('q')) break;
            }
        }
        av_packet_unref(packet);
    }

    av_frame_free(&yuv_frame);
    av_frame_free(&nv12_frame);
    av_frame_free(&bgr24_frame);
    av_packet_free(&packet);
    sws_freeContext(sws_context);
    avcodec_free_context(&video_codec_context);
    avformat_close_input(&input_format_context);
    avformat_free_context(input_format_context);
    return 0;
}
