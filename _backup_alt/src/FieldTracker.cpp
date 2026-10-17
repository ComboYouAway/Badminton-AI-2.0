#include "FieldTracker.hpp"
#include <algorithm>

FieldTracker::FieldTracker() {
    defaultWorldPoints = {
        cv::Point2f(0.0f,        fieldHeightM),  // TL
        cv::Point2f(fieldWidthM, fieldHeightM),  // TR
        cv::Point2f(fieldWidthM, 0.0f),          // BR
        cv::Point2f(0.0f,        0.0f)           // BL
    };
}

void FieldTracker::setFieldDimensions(float w, float h) {
    fieldWidthM = w;
    fieldHeightM = h;
    defaultWorldPoints = {
        cv::Point2f(0.0f,        h),
        cv::Point2f(w,           h),
        cv::Point2f(w,           0.0f),
        cv::Point2f(0.0f,        0.0f)
    };
}

void FieldTracker::orderCorners(std::vector<cv::Point2f>& pts) {
    if (pts.size() < 4) return;
    // Simple heuristic: sort by y, then x to approximate TL/TR/BR/BL
    std::sort(pts.begin(), pts.end(), [](const cv::Point2f& a, const cv::Point2f& b) {
        return a.y < b.y;
    });
    std::vector<cv::Point2f> top(pts.begin(), pts.begin() + 2);
    std::vector<cv::Point2f> bot(pts.begin() + 2, pts.end());
    std::sort(top.begin(), top.end(), [](const cv::Point2f& a, const cv::Point2f& b) { return a.x < b.x; });
    std::sort(bot.begin(), bot.end(), [](const cv::Point2f& a, const cv::Point2f& b) { return a.x < b.x; });
    pts = { top[0], top[1], bot[1], bot[0] }; // TL, TR, BR, BL
}

bool FieldTracker::detectField(const cv::Mat& mask, FieldCorners& corners) {
    // Find contours in the binary mask
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Pick the largest contour (the field)
    size_t best = 0;
    double bestArea = 0;
    for (size_t i = 0; i < contours.size(); ++i) {
        double area = cv::contourArea(contours[i]);
        if (area > bestArea) { bestArea = area; best = i; }
    }
    if (best == 0 && contours.size() == 1) best = 0;
    if (bestArea < 100) return false; // too small

    std::vector<cv::Point> approx;
    cv::approxPolyDP(contours[best], approx, cv::arcLength(contours[best], true) * 0.02, true);

    if (approx.size() < 4) return false;

    std::vector<cv::Point2f> pts;
    for (auto& p : approx) pts.emplace_back(static_cast<float>(p.x), static_cast<float>(p.y));
    orderCorners(pts);

    if (pts.size() < 4) return false;
    pts.resize(4);

    corners.imagePoints = pts;
    corners.worldPoints = defaultWorldPoints;
    corners.valid = true;

    return computeHomography(pts, defaultWorldPoints, corners.homography);
}

bool FieldTracker::computeHomography(const std::vector<cv::Point2f>& imagePts,
                                     const std::vector<cv::Point2f>& worldPts,
                                     cv::Mat& H) {
    if (imagePts.size() < 4 || worldPts.size() < 4) return false;
    H = cv::findHomography(imagePts, worldPts, cv::RANSAC, 5.0);
    return !H.empty() && H.rows == 3 && H.cols == 3;
}

std::optional<cv::Point2f> FieldTracker::mapToWorld(const cv::Point2f& imagePt,
                                                     const cv::Mat& H) const {
    if (H.empty()) return std::nullopt;
    std::vector<cv::Point2f> src = { imagePt };
    std::vector<cv::Point2f> dst;
    cv::perspectiveTransform(src, dst, H);
    if (dst.empty()) return std::nullopt;
    return dst[0];
}