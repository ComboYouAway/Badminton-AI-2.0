// Add the vector3 header for YoloTracker.hpp (it's missing).
// The user wanted main.cpp to contain the core application structure, so let's create it with the following content:
// - Basic window setup and init.
// - Basic video processing and field detection.
// - Basic renderer interaction.
// - Game loop, ImGui integration.

#include "VideoProcessor.hpp"
#include "FieldTracker.hpp"
#include "Renderer3D.hpp"
#include "YoloTracker.hpp"

#include <imgui.h>

int main(int argc, char* argv[]) {
    const int WINDOW_WIDTH = 1280;
    const int WINDOW_HEIGHT = 720;

    // Initialize the core modules
    VideoProcessor videoProcessor;
    FieldTracker fieldTracker;
    Renderer3D renderer;
    YoloTracker yoloTracker;

    // Load the camera
    if (!videoProcessor.openCamera(0)) {
        std::cerr << "Failed to open camera.\n";
        return 1;
    }

    // Initialize the renderer (OpenGL + ImGui)
    if (!renderer.init(WINDOW_WIDTH, WINDOW_HEIGHT, "Badminton AI 2.0")) {
        std::cerr << "Failed to init renderer.\n";
        return 1;
    }

    // Prepare the YOLO model (optional, for this milestone we just stub load)
    yoloTracker.loadModel("models/yolov5s.onnx");

    // For the 3D field we will just draw a simple grid
    std::vector<glm::vec3> fieldLines;
    float courtWidth = 6.10f;  // meters
    float courtLength = 13.40f;

    // Helper to add line segment between two 3D points
    auto addLine = [&](const glm::vec3& a, const glm::vec3& b) {
        fieldLines.push_back(a);
        fieldLines.push_back(b);
    };

    // Create a wireframe of the court: four border lines, plus center lines (if any)
    // Top-left corner -> Top-right corner
    addLine(glm::vec3(0.0f, 0.0f, -courtLength / 2.0f), // near
            glm::vec3(courtWidth, 0.0f, -courtLength / 2.0f)); // far
    // Bottom-left -> Bottom-right
    addLine(glm::vec3(0.0f, 0.0f, courtLength / 2.0f), // far
            glm::vec3(courtWidth, 0.0f, courtLength / 2.0f)); // near
    // Left side
    addLine(glm::vec3(0.0f, 0.0f, -courtLength / 2.0f), // near
            glm::vec3(0.0f, 0.0f, courtLength / 2.0f)); // far
    // Right side
    addLine(glm::vec3(courtWidth, 0.0f, -courtLength / 2.0f), // near
            glm::vec3(courtWidth, 0.0f, courtLength / 2.0f)); // far

    // Center cross (court splits into halves)
    // No cross lines in standard badminton, but could be nice; we can skip for now.

    // Main application loop
    bool running = true;
    bool showDemoWindow = false;

    // UI state variables
    bool topDownView = false;
    float orbitAngle = 0.0f;
    float zoom = 4.0f;
    float cameraHeight = 2.5f;

    while (!renderer.shouldClose() && running) {
        renderer.newFrame();

        // 1) Capture & process video frame
        cv::Mat rawFrame;
        if (videoProcessor.captureFrame(rawFrame)) {
            // Apply colour isolation mask
            cv::Mat mask, isolated;
            videoProcessor.processColorIsolation(rawFrame, mask, isolated, videoProcessor.thresholds);

            // 2) Adaptive field tracking
            FieldCorners corners;
            fieldTracker.detectField(mask, corners);
            // If corners are valid, compute world positions for visual grid in 3D (optional)
            // For this demo we just use the static field lines.

            // 3) YOLO detection (stub) - pass the raw frame
            std::vector<Detection> detections = yoloTracker.detect(rawFrame);

            // (TODO) Transform detection bounding boxes into world coordinates using corners
        }

        // UI rendering
        renderer.renderUI(showDemoWindow, topDownView, orbitAngle, zoom, cameraHeight, videoProcessor.thresholds);
        if (showDemoWindow) ImGui::ShowDemoWindow(&showDemoWindow);

        renderer.renderField(fieldLines);
        // TODO: render YoloTracker::dtects as PlayerBlob instances

        renderer.endFrame();
    }

    return 0;
}
