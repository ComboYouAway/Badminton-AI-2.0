#include "YoloTracker.hpp"
#include <iostream>

// TODO: Integrate ONNX Runtime / LibTorch / TensorRT
// This is a minimal stub for the first milestone.

YoloTracker::YoloTracker() = default;
YoloTracker::~YoloTracker() = default;

bool YoloTracker::loadModel(const std::string& path) {
    modelPath = path;
    ready = true; // Stub: always true
    return ready;
}

std::vector<Detection> YoloTracker::detect(const cv::Mat& /* frame */) {
    // Stub: return empty. In a later milestone, run inference here.
    return {};
}