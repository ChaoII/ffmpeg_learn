//
// Created by AC on 2024/6/6.
//

#pragma once

#include "fastdeploy/vision.h"

enum class ModelType {
    FACE_DETECT,
    PERSON_DETECT
};

class VideoAnalysis {
public:
    explicit VideoAnalysis(const ModelType &model_type, bool use_gpu = false);

    ~VideoAnalysis();

    cv::Mat predict(cv::Mat &image);

private:
    fastdeploy::FastDeployModel *model_;

    ModelType modelType_;
};


