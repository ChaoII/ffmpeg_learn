//
// Created by AC on 2024/6/6.
//

#include "VideoAnalysis.h"


VideoAnalysis::VideoAnalysis(const ModelType &model_type, bool use_gpu) {
    fastdeploy::FDLogger::enable_info = false;
    fastdeploy::RuntimeOption option;
    option.SetCpuThreadNum(1);
    if (use_gpu) option.UseGpu();
    modelType_ = model_type;
    switch (model_type) {
        case ModelType::FACE_DETECT: {
            std::string model_file = "version-RFB-320-sim.onnx";
            model_ = new fastdeploy::vision::facedet::UltraFace(model_file, "", option);
            break;
        }
        case ModelType::PERSON_DETECT: {
            std::string model_file = "2";
            std::string param_file = "3";
            std::string config_file = "4";
            model_ = new fastdeploy::vision::detection::PPYOLOE(model_file, param_file, config_file, option);
            break;
        }
        default: {
            model_ = nullptr;
        }
    }
}

VideoAnalysis::~VideoAnalysis() {
    switch (modelType_) {
        case ModelType::FACE_DETECT: {
            delete static_cast<fastdeploy::vision::facedet::UltraFace *>(model_);
            model_ = nullptr;
            break;
        }
        case ModelType::PERSON_DETECT: {
            delete static_cast<fastdeploy::vision::detection::PPYOLOE *>(model_);
            model_ = nullptr;
            break;
        }
        default: {

        }
    }
    std::cout << "delete" << std::endl;
}


cv::Mat VideoAnalysis::predict(cv::Mat &image) {
    switch (modelType_) {
        case ModelType::FACE_DETECT: {
            fastdeploy::vision::FaceDetectionResult res;
            auto model = static_cast<fastdeploy::vision::facedet::UltraFace *>(model_);
            model->Predict(&image, &res);
            return fastdeploy::vision::VisFaceDetection(image, res);
        }
        case ModelType::PERSON_DETECT: {
            auto model = static_cast<fastdeploy::vision::detection::PPYOLOE *>(model_);
            fastdeploy::vision::DetectionResult res;
            model->Predict(image, &res);
            return fastdeploy::vision::VisDetection(image, res);
        }
        default: {
            return image;
        }
    }
}