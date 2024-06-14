//
// Created by AC on 2024/6/13.
//
#include <cstring>
#include <string.h>
#include "video_push_api.h"
#include "src/push_opencv_rtsp.h"

//***********************************内部使用*******************************************************
typedef struct PushStreamParameterCWrapper {
    std::unique_ptr<PushStreamParameter> parameter;
} PushStreamParameterCWrapper;

PushStreamParameterCWrapper *vp_create_push_stream_parameter_c_wrapper() {
    auto cpp_param = new PushStreamParameterCWrapper();
    cpp_param->parameter = std::make_unique<PushStreamParameter>();
    return cpp_param;
}

void vp_free_push_stream_parameter_c_wrapper(PushStreamParameterCWrapper *push_stream_parameter_c_wrapper) {
    delete push_stream_parameter_c_wrapper;
}

PushStreamParameterCWrapper *vp_c_parameter_to_cpp_parameter(CPushStreamParameter *param) {
    auto cpp_param = vp_create_push_stream_parameter_c_wrapper();
    cpp_param->parameter->out_url = param->out_url;
    cpp_param->parameter->hw_accel = param->hw_accel;
    cpp_param->parameter->ffmpeg_thread_nums = param->ffmpeg_thread_nums;
    cpp_param->parameter->bit_rate = param->bit_rate;
    cpp_param->parameter->width = param->width;;
    cpp_param->parameter->height = param->height;;
    cpp_param->parameter->frame_rate = param->frame_rate;
    cpp_param->parameter->gop_size = param->gop_size;;
    cpp_param->parameter->max_b_frame = param->max_b_frame;;
    cpp_param->parameter->q_min = param->q_min;
    cpp_param->parameter->q_max = param->q_max;

    // 初始化模型类型
    for (int i = 0; i < param->model_num; i++) {
        cpp_param->parameter->model_types.push_back((ModelType) param->model_type[i]);
    }
    cpp_param->parameter->model_thread_num = param->model_thread_num;
    cpp_param->parameter->use_gpu = param->use_gpu;
    return cpp_param;
}

//***********************************内部使用*******************************************************


typedef struct PushOpenCVRtspCWrapper {
    std::unique_ptr<PushOpenCVRtsp> push_opencv_rtsp;
} PushOpenCVRtspCWrapper;


PushOpenCVRtspCWrapper *vp_create_push_stream_context(CPushStreamParameter *parameter) {
    auto push_stream_context = new PushOpenCVRtspCWrapper();
    auto cpp_parameter = vp_c_parameter_to_cpp_parameter(parameter);
    push_stream_context->push_opencv_rtsp = std::make_unique<PushOpenCVRtsp>(std::move(cpp_parameter->parameter));
    vp_free_push_stream_parameter_c_wrapper(cpp_parameter);
    return push_stream_context;
}

void vp_free_push_stream_context(PushOpenCVRtspCWrapper *push_stream_wrapper) {
    delete push_stream_wrapper;
}


CPushStreamParameter *vp_create_push_stream_parameter(const char *dst_url, const char *hw_accel, int ffmpeg_thread_num,
                                                      int bit_rate, int width, int height, int frame_rate, int gop_size,
                                                      int max_b_frame, int q_min, int q_max, CModelType *model_type,
                                                      size_t model_num, int model_thread_num, int use_gpu) {
    auto param = (CPushStreamParameter *) malloc(sizeof(CPushStreamParameter));

    param->out_url = (char *) malloc(strlen(dst_url) + 1);
    strcpy(param->out_url, dst_url);
    param->hw_accel = (char *) malloc(strlen(hw_accel) + 1);
    strcpy(param->hw_accel, hw_accel);
    param->ffmpeg_thread_nums = ffmpeg_thread_num;
    param->bit_rate = bit_rate;
    param->width = width;
    param->height = height;
    param->frame_rate = frame_rate;
    param->gop_size = gop_size;
    param->max_b_frame = max_b_frame;
    param->q_min = q_min;
    param->q_max = q_max;
    // 初始化模型类型
    param->model_num = model_num;
    param->model_type = (CModelType *) malloc(param->model_num * sizeof(CModelType));
    memcpy(param->model_type, model_type, param->model_num * sizeof(CModelType));
    param->model_thread_num = model_thread_num;
    param->use_gpu = use_gpu;
    return param;
}

CPushStreamParameter *vp_init_push_stream_parameter() {
    auto param = (CPushStreamParameter *) malloc(sizeof(CPushStreamParameter));
    param->out_url = strdup("rtsp://172.168.1.112:8554/live/test");
    param->hw_accel = strdup("none");
    param->ffmpeg_thread_nums = 10;
    param->bit_rate = 409600;
    param->width = 640;
    param->height = 480;
    param->frame_rate = 10;
    param->gop_size = 30;
    param->max_b_frame = 0;
    param->q_min = 10;
    param->q_max = 51;

    // 初始化模型类型
    param->model_num = 1;
    param->model_type = (CModelType *) malloc(param->model_num * sizeof(CModelType));
    param->model_type[0] = C_FACE_DETECT;
    param->model_thread_num = 1;
    param->use_gpu = 0;
    return param;
}

void vp_free_push_stream_parameter(CPushStreamParameter *param) {
    if (param->out_url) free(param->out_url);
    if (param->hw_accel) free(param->hw_accel);
    if (param->model_type) free(param->model_type);
    free(param);
}


void vp_start_push_stream_thread(PushOpenCVRtspCWrapper *push_stream_wrapper) {
    push_stream_wrapper->push_opencv_rtsp->start();
}

void
vp_enqueue_push_stream_frame(PushOpenCVRtspCWrapper *push_stream_wrapper, unsigned char *buffer, int w, int h,
                             int channel) {
    push_stream_wrapper->push_opencv_rtsp->push_src_frame(
            std::move(cv::Mat(h, w, CV_MAKE_TYPE(CV_8U, channel), buffer).clone())
    );
}

void vp_set_push_stream_hw_accel(PushOpenCVRtspCWrapper *push_stream_wrapper, const char *hw_accel) {
    push_stream_wrapper->push_opencv_rtsp->set_hw_accel(hw_accel);
}




