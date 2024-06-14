//
// Created by AC on 2024/5/15.
//

#include "video_decode_play.h"


video_decode_play::video_decode_play(const char *media_file_path, const char *hw_accel_device) {
    media_file_path_ = media_file_path;
    hw_accel_device_ = hw_accel_device;
    init_ffmpeg_variables();
}

video_decode_play::~video_decode_play() {
    av_frame_free(&yuv_frame_);
    av_frame_free(&nv12_frame_);
    av_frame_free(&bgr24_frame_);
    av_packet_free(&packet_);
    avcodec_free_context(&video_codec_context_);
    avformat_close_input(&input_format_context_);
    av_buffer_unref(&hw_device_ctx_);
}

int video_decode_play::hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type) {
    int err = 0;
    if ((err = av_hwdevice_ctx_create(&hw_device_ctx_, type,
                                      nullptr, nullptr, 0)) < 0) {
        std::cerr << "Failed to create specified HW device." << std::endl;
        return err;
    }
    ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx_);
    return err;
}

AVPixelFormat video_decode_play::get_hw_format(AVCodecContext *ctx,
                                               const enum AVPixelFormat *pix_fmts) {
    const enum AVPixelFormat *p;
    for (p = pix_fmts; *p != -1; p++) {
        if (*p == hw_pix_fmt_)
            return *p;
    }
    std::cerr << "Failed to get HW surface format." << std::endl;
    return AV_PIX_FMT_NONE;
}


void video_decode_play::init_ffmpeg_variables() {
    if (is_network_media(media_file_path_)) {
        av_dict_set(&options_, "buffer_size", "1024000", 0);
        av_dict_set(&options_, "max_delay", "500", 0);
        av_dict_set(&options_, "rtsp_transport", "tcp", 0);
        //如果没有设置stimeout，那么流地址错误，av_read_frame会阻塞（时间单位是微妙）
        av_dict_set(&options_, "stimeout", "2000000", 0);
    }

    input_format_context_ = avformat_alloc_context();
    packet_ = av_packet_alloc();
    yuv_frame_ = av_frame_alloc();
    bgr24_frame_ = av_frame_alloc();
}

bool video_decode_play::init_parameters() {

    if (avformat_open_input(&input_format_context_, media_file_path_.c_str(), nullptr, &options_) != 0) {
        std::cout << "cant not open video file" << std::endl;
        return false;
    }
    if (avformat_find_stream_info(input_format_context_, nullptr) < 0) {
        std::cerr << "cant find input stream information." << std::endl;
        return false;
    }

    video_stream_index_ = av_find_best_stream(input_format_context_, AVMEDIA_TYPE_VIDEO, -1, -1,
                                              (const AVCodec **) &video_codec_, 0);

    if (video_stream_index_ < 0) {
        std::cerr << "Cannot find a video stream in the input file" << std::endl;
        return false;
    }


    if (!(video_codec_context_ = avcodec_alloc_context3(video_codec_))) {
        return AVERROR(ENOMEM);
    }

    auto video_stream = input_format_context_->streams[video_stream_index_];
    if (avcodec_parameters_to_context(video_codec_context_, video_stream->codecpar) < 0) {
        std::cout << "con not create videoCodecContext" << std::endl;
        return false;
    }


    AVPixelFormat src_video_pix_format = video_codec_context_->pix_fmt;
    is_hw_accel_ = check_and_set_hw_accel();
    if (is_hw_accel_) {
        std::cout << "use hardware accelerate, hardware: [" << hw_accel_device_ << "]" << std::endl;
        src_video_pix_format = AV_PIX_FMT_NV12;
        nv12_frame_ = av_frame_alloc();
    }


    if (avcodec_open2(video_codec_context_, video_codec_, &options_) < 0) {
        std::cout << "Failed to open codec for stream :" << video_stream_index_ << std::endl;;
        return false;
    }
    image_ = cv::Mat(video_codec_context_->height, video_codec_context_->width, CV_8UC3);
    av_image_fill_arrays(bgr24_frame_->data, bgr24_frame_->linesize, image_.data,
                         AVPixelFormat::AV_PIX_FMT_BGR24, video_codec_context_->width, video_codec_context_->height, 1);
    sws_context_ = sws_getContext(video_codec_context_->width, video_codec_context_->height,
                                  src_video_pix_format, video_codec_context_->width,
                                  video_codec_context_->height,
                                  AV_PIX_FMT_BGR24, SWS_BICUBIC,
                                  nullptr, nullptr, nullptr);
    return true;
}

void video_decode_play::play(const std::string &name) {
    while (av_read_frame(input_format_context_, packet_) >= 0 && !exit_flag_) {
        if (packet_->stream_index == video_stream_index_) {
            if (packet_->size > 0) {
                decode_and_show(name);
            }
        }
        av_packet_unref(packet_);
    }
}

void video_decode_play::decode_and_show(const std::string &name) {

    int ret;
    ret = avcodec_send_packet(video_codec_context_, packet_);
    if (ret < 0) {
        std::cerr << "Error sending a packet for decoding" << std::endl;
        exit_flag_ = true;
    }
    while (ret >= 0) {
        std::cout << "d---------:" << name << std::endl;
        ret = avcodec_receive_frame(video_codec_context_, yuv_frame_);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            std::cerr << "Error during decoding" << std::endl;
            exit_flag_ = true;
        }
        if (is_hw_accel_) {
            // 如果是硬解码，这个很重要，必须从硬件中转换出来
            if (av_hwframe_transfer_data(nv12_frame_, yuv_frame_, 0) < 0) {
                return;
            }
            temp_frame_ = nv12_frame_;
        } else {
            temp_frame_ = yuv_frame_;
        }
        // 实现格式转换nv12-bgr24
        sws_scale(sws_context_, temp_frame_->data,
                  temp_frame_->linesize, 0,
                  temp_frame_->height,
                  bgr24_frame_->data,
                  bgr24_frame_->linesize);
        if (image_.empty()) return;
        cv::imshow(name, image_);
        if (cv::waitKey(2) == char('q')) {
            exit_flag_ = true;
        }
    }
}


bool video_decode_play::check_and_set_hw_accel() {
    //获取支持该decoder的hw配置型
    hw_device_type_ = av_hwdevice_find_type_by_name(hw_accel_device_.c_str());
    if (hw_device_type_ == AV_HWDEVICE_TYPE_NONE) {
        std::cerr << "Device type [" << hw_accel_device_ << "] is not supported." << std::endl;
        std::cerr << "Available device types: ";
        hw_device_type_ = av_hwdevice_iterate_types(AV_HWDEVICE_TYPE_NONE);
        while (hw_device_type_ != AV_HWDEVICE_TYPE_NONE) {
            std::cerr << av_hwdevice_get_type_name(hw_device_type_) << " ";
            hw_device_type_ = av_hwdevice_iterate_types(hw_device_type_);
        }
        std::cerr << std::endl;
        return false;
    }
    for (int i = 0;; i++) {
        const AVCodecHWConfig *config = avcodec_get_hw_config(video_codec_, i);
        if (!config) {
            std::cerr << "Decoder [" << video_codec_->name << "] does not support device type "
                      << av_hwdevice_get_type_name(hw_device_type_) << std::endl;
            return false;
        }
        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
            config->device_type == hw_device_type_) {
            hw_pix_fmt_ = config->pix_fmt;
            break;
        }
    }
    video_codec_context_->get_format = get_hw_format;
    if (hw_decoder_init(video_codec_context_, hw_device_type_) < 0) {
        return false;
    }
    return true;
}



