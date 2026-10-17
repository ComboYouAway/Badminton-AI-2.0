#include "VideoProcessor.hpp"
#include <opencv2/imgproc.hpp>

VideoProcessor::VideoProcessor() {}
VideoProcessor::~VideoProcessor() { release(); }

bool VideoProcessor::openCamera(int cameraIndex) {
    cap.open(cameraIndex, cv::CAP_DSHOW); // DirectShow is reliable on Windows
    return cap.isOpened();
}

bool VideoProcessor::isOpened() const { return cap.isOpened(); }

void VideoProcessor::release() { if (cap.isOpened()) cap.release(); }

bool VideoProcessor::captureFrame(cv::Mat& frame) {
    return cap.read(frame);
}

void VideoProcessor::processColorIsolation(const cv::Mat& input,
                                            cv::Mat& mask,
                                            cv::Mat& output,
                                            const HSVThresholds& thresholds) {
    cv::Mat hsv;
    cv::cvtColor(input, hsv, cv::COLOR_BGR2HSV);

    cv::inRange(hsv, cv::Scalar(thresholds.hMin, thresholds.sMin, thresholds.vMin),
               cv::Scalar(thresholds.hMax, thresholds.sMax, thresholds.vMax), mask);

    // Morphological cleanup (close small gaps, remove noise)
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN,  kernel);

    output = cv::Mat::zeros(input.size(), input.type());
    input.copyTo(output, mask);
}
