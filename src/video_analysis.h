//
// Created by AC on 2024/6/6.
//

#pragma once

#include "fastdeploy/vision.h"

enum class ModelType {
    FACE_DETECT,
    PERSON_DETECT
};

class video_analysis {
public:
    explicit video_analysis(const ModelType &model_type, bool use_gpu = false);

    ~video_analysis();

    cv::Mat predict(cv::Mat &image);

private:
    fastdeploy::FastDeployModel *model_;

    ModelType modelType_;
};


