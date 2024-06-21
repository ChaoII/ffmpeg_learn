# ffmpeg_learn

```c++
ffmpeg -re -stream_loop -1 -i 1.mp4 -vcodec libx264 -acodec acc -f flv rtmp://127.0.0.1/live/test
```

```c++
    AVDictionary *options = nullptr;
// 预设为fast，编码速度较快可选，速度逐渐降低，质量逐渐升高。
// [ultrafast, superfast, veryfast, faster, fast, medium, slow, slower, veryslow, placebo]
av_dict_set(&options, "preset", "fast", 0); // 尝试使用不同的预设
// cbr (Constant Bit Rate): 恒定比特率，适用于需要稳定网络带宽的场景。
// vbr (Variable Bit Rate): 可变比特率，允许比特率根据内容复杂度变化，通常能提供更好的质量。
av_dict_set(&options, "rc", "cbr", 0);
// 参数用于设置编码器在进行比特率控制时的前视帧数。值越大，编码器可以查看更多的未来帧来决定当前帧的编码，
// 但会增加延迟和计算负担。设为 0 通常用于低延迟场景。
av_dict_set(&options, "rc-lookahead", "0", 0);
//delay 参数用于控制编码器的延迟模式。设为 0 以减少编码延迟，通常用于实时通信或直播场景。
av_dict_set(&options, "delay", "0", 0);
//zerolatency 参数启用零延迟模式，优化编码器以最小化延迟。注意，硬编码报错很多大部分是该参数没有设置为0
// 这通常会禁用一些可能增加延迟的功能，如B帧。设为 0 表示关闭零延迟模式。
av_dict_set(&options, "zerolatency", "0", 0);
// strict_gop 参数用于确保编码器严格遵循设置的GOP（Group of Pictures）结构。如果设为 1，
// 编码器将严格按照设定的GOP大小进行编码，通常用于需要严格控制I帧位置的场景。
av_dict_set(&options, "strict_gop", "1", 0);
// rtsp_transport 参数用于设置RTSP（Real-Time Streaming Protocol）传输方式。常见值包括：
// tcp: 使用TCP传输，可靠性高，但延迟可能较大。
// udp: 使用UDP传输，延迟低，但可靠性较差。
// udp_multicast: 使用UDP组播，适用于多客户端接收同一流的场景。
// http: 使用HTTP传输，通常用于穿越防火墙的场景。
// av_dict_set(&options, "rtsp_transport", "tcp", 0);
```
# opencv cmake 编译选项
```c++
-DBUILD_ZLIB=ON
-DBUILD_TIFF=ON
-DBUILD_OPENJPEG=ON
-DBUILD_JASPER=OFF
-DBUILD_JPEG=ON
-DBUILD_PNG=ON
-DBUILD_OPENEXR=OFF
-DBUILD_WEBP=OFF
-DBUILD_TBB=OFF
-DBUILD_IPP_IW=OFF
-DBUILD_ITT=ON
-DWITH_AVFOUNDATION=OFF
-DWITH_CAP_IOS=OFF
-DWITH_CAROTENE=OFF
-DWITH_CPUFEATURES=OFF
-DWITH_EIGEN=OFF
-DWITH_FFMPEG=OFF
-DWITH_GSTREAMER=OFF
-DWITH_GTK=OFF
-DWITH_IPP=OFF
-DWITH_HALIDE=OFF
-DWITH_VULKAN=OFF
-DWITH_INF_ENGINE=OFF
-DWITH_NGRAPH=OFF
-DWITH_JASPER=OFF
-DWITH_OPENJPEG=OFF
-DWITH_JPEG=ON
-DWITH_WEBP=OFF
-DWITH_OPENEXR=OFF
-DWITH_PNG=ON
-DWITH_TIFF=ON
-DWITH_OPENVX=OFF
-DWITH_GDCM=OFF
-DWITH_TBB=OFF
-DWITH_HPX=OFF
-DWITH_OPENMP=ON
-DWITH_PTHREADS_PF=OFF
-DWITH_V4L=OFF
-DWITH_CLP=OFF
-DWITH_OPENCL=OFF
-DWITH_OPENCL_SVM=OFF
-DWITH_VA=OFF
-DWITH_VA_INTEL=OFF
-DWITH_ITT=OFF
-DWITH_PROTOBUF=OFF
-DWITH_IMGCODEC_HDR=OFF
-DWITH_IMGCODEC_SUNRASTER=OFF
-DWITH_IMGCODEC_PXM=OFF
-DWITH_IMGCODEC_PFM=OFF
-DWITH_QUIRC=OFF
-DWITH_ANDROID_MEDIANDK=OFF
-DWITH_TENGINE=OFF
-DWITH_ONNX=OFF
-DWITH_TIMVX=OFF
-DWITH_OBSENSOR=OFF
-DWITH_CANN=OFF
-DWITH_FLATBUFFERS=OFF
-DBUILD_SHARED_LIBS=OFF
-DBUILD_opencv_apps=OFF
-DBUILD_ANDROID_PROJECTS=OFF
-DBUILD_ANDROID_EXAMPLES=OFF
-DBUILD_DOCS=OFF
-DBUILD_EXAMPLES=OFF
-DBUILD_PACKAGE=OFF
-DBUILD_PERF_TESTS=OFF
-DBUILD_TESTS=OFF
-DBUILD_WITH_STATIC_CRT=OFF
-DBUILD_FAT_JAVA_LIB=OFF
-DBUILD_ANDROID_SERVICE=OFF
-DBUILD_JAVA=OFF
-DBUILD_OBJC=OFF
-DBUILD_KOTLIN_EXTENSIONS=OFF
-DENABLE_PRECOMPILED_HEADERS=OFF
-DENABLE_FAST_MATH=ON
-DCV_TRACE=OFF
-DBUILD_opencv_java=OFF
-DBUILD_opencv_gapi=ON
-DBUILD_opencv_objc=OFF
-DBUILD_opencv_js=OFF
-DBUILD_opencv_ts=OFF
-DBUILD_opencv_python2=OFF
-DBUILD_opencv_python3=OFF
-DBUILD_opencv_dnn=OFF
-DBUILD_opencv_imgcodecs=ON
-DBUILD_opencv_videoio=ON
-DBUILD_opencv_calib3d=ON
-DBUILD_opencv_flann=ON
-DBUILD_opencv_objdetect=OFF
-DBUILD_opencv_stitching=OFF
-DBUILD_opencv_ml=OFF
```
