#pragma once

#include <opencv2/opencv.hpp>

/// HSV thresholds used for colour isolation.
struct HSVThresholds {
    int hMin = 0;
    int hMax = 179;
    int sMin = 0;
    int sMax = 255;
    int vMin = 0;
    int vMax = 255;
};

/// Captures live frames from a camera and applies HSV colour isolation.
class VideoProcessor {
public:
    VideoProcessor();
    ~VideoProcessor();

    /// Open a camera by index (e.g. 0 for default webcam).
    bool openCamera(int cameraIndex = 0);

    /// Whether the camera is currently open.
    bool isOpened() const;

    /// Release the camera device.
    void release();

    /// Grab the next frame into `frame`. Returns false on failure.
    bool captureFrame(cv::Mat& frame);

    /// Apply HSV colour isolation to `input`, writing the binary mask and the
    /// masked output image. `thresholds` selects the colour band to keep.
    void processColorIsolation(const cv::Mat& input,
                               cv::Mat& mask,
                               cv::Mat& output,
                               const HSVThresholds& thresholds);

    /// Live-updatable thresholds (bound to ImGui sliders in the UI).
    HSVThresholds thresholds;

private:
    cv::VideoCapture cap;
};
