#include <iostream>
#include "src/read_file_show_in_opencv.h"
#include "src/hw_decode_show_in_opencv.h"
#include <iomanip>
#include "src/HWDecodePlayer.h"

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


int main() {

    HWDecodePlayer player("juren.mp4", "cuda");
    if (!player.init_parameters()) return -1;
    player.play();

//    hw_read_file_show("juren.mp4");
//    read_file_show("juren.mp4");
//    read_file_show("rtsp://172.168.1.112:8554/live/test");
//    hw_read_file_show("rtsp://172.168.1.112:8554/live/test");
//    std::cout << av_version_info() << std::endl;
//    auto codecVer = avcodec_version();
//    std::cout << codecVer << std::endl;
//
//    auto ver_major = (codecVer >> 16) & 0xff;
//    auto ver_minor = (codecVer >> 8) & 0xff;
//    auto ver_micro = (codecVer) & 0xff;
//    std::cout << ver_major << "." << ver_minor << "." << ver_micro << std::endl;
    return 0;
}
