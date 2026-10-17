#pragma once

#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

/// A single detection returned by the YOLO tracker.
struct Detection {
    cv::Rect box;           // bounding box in image coordinates
    int classId = 0;        // class index (0 = player)
    float confidence = 0.0f;
    glm::vec3 worldPos = glm::vec3(0.0f); // projected world position (m)
};

/// YOLO player tracking interface.
/// This boilerplate stub returns an empty detection list. Wire in ONNX
/// Runtime / LibTorch / TensorRT inference in a later milestone.
class YoloTracker {
public:
    YoloTracker();
    ~YoloTracker();

    /// Load a model from `modelPath`. Returns false if the backend is not yet
    /// wired up (stub).
    bool loadModel(const std::string& modelPath);

    /// Run inference on `frame`, returning player detections.
    std::vector<Detection> detect(const cv::Mat& frame);

    /// Whether the model is loaded and ready.
    bool isReady() const { return ready; }

    /// Confidence threshold for detections.
    float confidenceThreshold = 0.5f;

    /// NMS threshold.
    float nmsThreshold = 0.4f;

private:
    bool ready = false;
    std::string modelPath;
};
