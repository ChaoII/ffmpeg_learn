# ffmpeg_learn

学习ffmpeg

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
