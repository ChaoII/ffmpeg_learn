//
// Created by AC on 2024/6/13.
//

#pragma once

#include <cstdlib>

#ifdef _WIN32
#ifdef CAPI
#define DECL_CAPI __declspec(dllexport)
#else
#define DECL_CAPI __declspec(dllimport)
#endif
#else
#define DECL_CAPI __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif
/*
 * 该文件是c++的c封装，主要将c++的PushStreamParameter
 * 1. 封装CPushStreamParameter其意义与PushStreamParameter相同，只是采用的纯C的数据结构
 * 2. 封装了PushOpenCVRtsp为C结构体PushOpenCVRtspCWrapper，PushOpenCVRtspCWrapper内部一个成员变量用智能指针包裹了原始的c++ 类，
 * 当然也可以不用智能指针，只是后面释放的时候需要多处理一步
 * 3. 在外部调用时底层其实是调用的c++ 所以需要将CPushStreamParameter转化为PushStreamParameter，然后调用实例化PushOpenCVRtsp
 * all: PushOpenCVRtspCWrapper的成员变量调用PushStreamParameter（需要实现互转）初始化，然后其内部PushOpenCVRtsp实例调用成员方法
 */

typedef enum CModelType {
    C_FACE_DETECT,
    C_PERSON_DETECT
} CModelType;

typedef struct CPushStreamParameter {
    char *out_url;
    char *hw_accel;
    int ffmpeg_thread_nums;
    int bit_rate;
    int width;
    int height;
    int frame_rate;
    int gop_size;
    int max_b_frame;
    int q_min;
    int q_max;
    CModelType *model_type;
    size_t model_num;
    int model_thread_num;
    int use_gpu;
} CPushStreamParameter;


typedef struct CPushStreamContext CPushStreamContext;


DECL_CAPI CPushStreamContext *vp_create_push_stream_context(CPushStreamParameter *param);

DECL_CAPI void vp_free_push_stream_context(CPushStreamContext *push_stream_context);

DECL_CAPI CPushStreamParameter *
vp_create_push_stream_parameter(const char *dst_url, const char *hw_accel, int ffmpeg_thread_num,
                                int bit_rate, int width, int height, int frame_rate, int gop_size,
                                int max_b_frame, int q_min, int q_max, CModelType *model_type,
                                size_t model_num, int model_thread_num, int use_gpu);

DECL_CAPI CPushStreamParameter *vp_init_push_stream_parameter();

DECL_CAPI void vp_free_push_stream_parameter(CPushStreamParameter *param);

DECL_CAPI void vp_start_push_stream_thread(CPushStreamContext *push_stream_context);

DECL_CAPI void
vp_enqueue_push_stream_frame(CPushStreamContext *push_stream_context, unsigned char *buffer, int w, int h,
                             int channel);

DECL_CAPI void vp_set_push_stream_hw_accel(CPushStreamContext *push_stream_context, const char *hw_accel);

#ifdef __cplusplus
}
#endif