#include <iostream>
#include <thread>
#include "src/HWDecodePlayer.h"
#include "src/PushOpenCVRtsp.h"
#include <opencv2/core/utils/logger.hpp>

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
    auto pushUtils = new PushOpenCVRtsp(parameter);
    pushUtils->set_hw_accel("h264_nvenc");
    pushUtils->set_resolution(640, 480);
    pushUtils->set_frame_rate(30);
    pushUtils->open_codec();
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
    return 0;
}


int main() {
//    av_log_set_level(AV_LOG_DEBUG); //启用日志
    // 执行ffmpeg -hwaccels 查看硬解码设备
//    HWDecodePlayer player("rtsp://localhost/live/test3", "cuda");
//    if (!player.init_parameters()) return -1;
//    player.play();
//    std::vector<std::thread> threads;
////    threads.reserve(2);
//    threads.reserve(10);
//for (int i = 0; i < 10; i++) {
//        threads.emplace_back([=]() {
//            HWDecodePlayer player("rtsp://localhost/live/test3", "cuda");
//            player.init_parameters();
//            std::cout << "----------------------------------------------:" << i << std::endl;
//            player.play(std::to_string(i));
//        });
//    }
//
//    for (auto &th: threads) {
//        th.join();
//    }

    test_push();
    return 0;
}
