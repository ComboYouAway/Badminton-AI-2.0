#pragma once

#include <opencv2/opencv.hpp>
#include <optional>
#include <vector>

/// Detected field boundary: four corners in image space, their corresponding
/// world coordinates, and the homography mapping between them.
struct FieldCorners {
    std::vector<cv::Point2f> imagePoints;   // 4 corners in image (px)
    std::vector<cv::Point2f> worldPoints;   // 4 corners in world (m)
    cv::Mat homography;                     // 3x3 homography (image -> world)
    bool valid = false;
};

/// Adaptive field tracker: detects the badminton court from a colour-isolated
/// mask and computes a homography to a normalised 3D coordinate system.
class FieldTracker {
public:
    FieldTracker();
    ~FieldTracker() = default;

    /// Detect the four field corners from a binary mask.
    bool detectField(const cv::Mat& mask, FieldCorners& corners);

    /// Compute a homography from 4 image points to 4 world points.
    bool computeHomography(const std::vector<cv::Point2f>& imagePts,
                           const std::vector<cv::Point2f>& worldPts,
                           cv::Mat& H);

    /// Map a single image point into world coordinates using `H`.
    std::optional<cv::Point2f> mapToWorld(const cv::Point2f& imagePt,
                                          const cv::Mat& H) const;

    /// Set physical field dimensions (metres). Defaults to a doubles court.
    void setFieldDimensions(float widthM, float heightM);

private:
    float fieldWidthM  = 6.10f;  // badminton doubles court width  (m)
    float fieldHeightM = 13.40f; // badminton doubles court length (m)

    std::vector<cv::Point2f> defaultWorldPoints;

    // Order four detected corners as TL, TR, BR, BL.
    static void orderCorners(std::vector<cv::Point2f>& pts);
};
