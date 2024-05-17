#include <iostream>
#include <iomanip>
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
    auto pushUtils = new PushOpenCVRtsp("rtsp://172.168.1.112:8554/live/test1");
    pushUtils->open_codec(640, 480, 25);
    pushUtils->start();
    namedWindow("test", cv::WINDOW_AUTOSIZE);
    while (true) {
        cv::Mat frame;
        bool bSuccess = cap.read(frame);
        flip(frame, frame, 1);
        pushUtils->push_frame(frame);
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
    // 执行ffmpeg -hwaccels 查看硬解码设备
//    HWDecodePlayer player("rtsp://172.168.1.112/live/test", "d3d12va");
//    if (!player.init_parameters()) return -1;
//    player.play();

    test_push();
    return 0;
}
