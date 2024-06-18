#include <iostream>
#include <thread>
#include "src/video_decode_play.h"
#include "src/push_opencv_rtsp.h"
#include <opencv2/core/utils/logger.hpp>
#include "capi/video_push_api.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavfilter/avfilter.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/ffversion.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#include "libpostproc/postprocess.h"
}

int test_push() {

    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cout << "无法打开摄像头！" << std::endl;
        return -1;
    }
    PushStreamParameter parameter;
    auto pushUtils = new PushOpenCVRtsp(std::unique_ptr<PushStreamParameter>(&parameter));
    pushUtils->set_hw_accel("h264_nvenc");
    pushUtils->set_resolution(640, 480);
    pushUtils->set_frame_rate(10);
    pushUtils->start();
    namedWindow("test", cv::WINDOW_AUTOSIZE);
    while (true) {
        cv::Mat frame;
        bool bSuccess = cap.read(frame);
        flip(frame, frame, 1);
        pushUtils->push_src_frame(std::move(frame.clone()));
        if (!bSuccess) {
            std::cout << "" << std::endl;
            break;
        }
        imshow("test", frame);
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }
    cap.release();
    cv::destroyAllWindows();
    delete pushUtils;
    return 0;
}

int test_push_c() {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cout << "无法打开摄像头！" << std::endl;
        return -1;
    }
    // 初始化推流参数
    auto params = vp_init_push_stream_parameter();
    auto handle = vp_create_push_stream_context(params);
    // 创建完成后，推流参数可以释放
    vp_free_push_stream_parameter(params);
    // 设置推流编码硬件加速
    vp_set_push_stream_hw_accel(handle, "h264_nvenc");
    // 开启推流线程
    vp_start_push_stream_thread(handle);
    namedWindow("test", cv::WINDOW_AUTOSIZE);
    while (true) {
        cv::Mat frame;
        bool bSuccess = cap.read(frame);
        flip(frame, frame, 1);
        // 开始往推流队列添加帧
        vp_enqueue_push_stream_frame(handle, frame.data, frame.cols, frame.rows, frame.channels());
        if (!bSuccess) {
            std::cout << "" << std::endl;
            break;
        }
        imshow("test", frame);
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }
    cap.release();
    cv::destroyAllWindows();
    // 释放推流上下文
    vp_free_push_stream_context(handle);
    return 0;
}

void test_play() {
    av_log_set_level(AV_LOG_DEBUG); //启用日志
    //执行ffmpeg -hwaccels 查看硬解码设备
    VideoDecodePlay player("rtsp://localhost/live/test3", "cuda");
    if (!player.init_parameters()) return;
    player.play("cece");
}


int main() {

    // test_play()
     test_push();
//    test_push_c();
    return 0;
}
