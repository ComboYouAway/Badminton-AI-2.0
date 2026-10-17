#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>

/// A single tracked player rendered as a 3D object on the field.
struct PlayerBlob {
    glm::vec3 position = glm::vec3(0.0f); // world coordinates (m)
    float radius = 0.3f;                  // visual radius (m)
    glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f);
};

/// OpenGL + ImGui renderer: window, 3D field viewport, mouse orbit/zoom,
/// and the dark-mode UI overlay.
class Renderer3D {
public:
    Renderer3D();
    ~Renderer3D();

    /// Create the GLFW window, OpenGL context, and ImGui backends.
    bool init(int width, int height, const char* title);

    /// Shut down and release all resources.
    void shutdown();

    /// Begin a new frame (poll events, start ImGui).
    void newFrame();

    /// Render the 3D field grid and lines into the viewport.
    void renderField(const std::vector<glm::vec3>& linePoints,
                     const glm::vec3& lineColor = glm::vec3(1.0f, 1.0f, 1.0f));

    /// Render player blobs as coloured cylinders/discs on the field.
    void renderPlayers(const std::vector<PlayerBlob>& players);

    /// Render the ImGui dark-mode UI overlay (left sliders, right viewport,
    /// status bar). Returns the updated camera/orbit state via out params.
    void renderUI(bool& showDemo,
                  bool& topDownView,
                  float& orbitAngle,
                  float& zoom,
                  float& cameraHeight,
                  HSVThresholds& thresholds);

    /// End the frame: render ImGui draw data and swap buffers.
    void endFrame();

    GLFWwindow* window() const { return window; }
    bool shouldClose() const { return glfwWindowShouldClose(window); }

    // ---- Input callbacks (static, routed to instance via user pointer) ----
    static void onMouseButton(GLFWwindow* w, int button, int action, int mods);
    static void onCursorPos(GLFWwindow* w, double x, double y);
    static void onScroll(GLFWwindow* w, double xoff, double yoff);

    void setViewportSize(int w, int h) { viewportW = w; viewportH = h; }

private:
    GLFWwindow* window = nullptr;
    int viewportW = 1280;
    int viewportH = 720;

    // OpenGL resources
    GLuint shaderProgram = 0;
    GLuint vao = 0;
    GLuint vbo = 0;

    // Camera state
    float orbitAngle = 0.0f;     // radians around Y
    float zoom = 4.0f;           // distance from origin
    float cameraHeight = 2.5f;   // height above field
    bool topDownView = false;

    // Mouse interaction state
    bool mouseDragging = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    // Shader helpers
    GLuint compileShader(GLenum type, const char* source);
    GLuint createProgram(const char* vertSrc, const char* fragSrc);

    void updateMatrices(glm::mat4& view, glm::mat4& proj) const;
    void drawLines(const std::vector<glm::vec3>& points,
                   const glm::vec3& color);
    void drawCylinderDisc(const glm::vec3& pos,
                          float radius,
                          const glm::vec3& color);
};
