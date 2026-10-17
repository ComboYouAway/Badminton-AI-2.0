#include "Renderer3D.hpp"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>

Renderer3D::Renderer3D() {}
Renderer3D::~Renderer3D() { shutdown(); }

// ----- Helper shaders -------------------------------------------------------
static const char* vertexShaderSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 uView;
uniform mat4 uProj;
void main() {
    gl_Position = uProj * uView * vec4(aPos, 1.0);
}
)";

static const char* fragmentShaderSrc = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 uColor;
void main() {
    FragColor = vec4(uColor, 1.0);
}
)";

GLuint Renderer3D::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compile error:\n" << log << std::endl;
    }
    return shader;
}

GLuint Renderer3D::createProgram(const char* vertSrc, const char* fragSrc) {
    GLuint program = glCreateProgram();
    GLuint vert = compileShader(GL_VERTEX_SHADER, vertSrc);
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc);
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);
    GLint ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        std::cerr << "Program link error:\n" << log << std::endl;
    }
    glDeleteShader(vert);
    glDeleteShader(frag);
    return program;
}

// ----- Initialization -------------------------------------------------------
bool Renderer3D::init(int width, int height, const char* title) {
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    // Set user pointer to this instance for callbacks
    glfwSetWindowUserPointer(window, this);
    glfwSetMouseButtonCallback(window, onMouseButton);
    glfwSetCursorPosCallback(window, onCursorPos);
    glfwSetScrollCallback(window, onScroll);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n";
        return false;
    }

    // Build shader program
    shaderProgram = createProgram(vertexShaderSrc, fragmentShaderSrc);
    if (!shaderProgram) return false;

    // Create a dummy VAO/VBO for immediate-mode drawing (we'll use glBegin/end? No, use VAO/VBO with dynamic data)
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ImGui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleDark(); // dark mode
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    viewportW = width;
    viewportH = height;
    return true;
}

void Renderer3D::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (shaderProgram) glDeleteProgram(shaderProgram);
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);

    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

void Renderer3D::newFrame() {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Renderer3D::endFrame() {
    ImGui::Render();
    int displayW, displayH;
    glfwGetFramebufferSize(window, &displayW, &displayH);
    glViewport(0, 0, displayW, displayH);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}

void Renderer3D::renderField(const std::vector<glm::vec3>& linePoints, const glm::vec3& lineColor) {
    if (linePoints.empty()) return;
    glUseProgram(shaderProgram);
    glm::mat4 view, proj;
    updateMatrices(view, proj);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uView"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProj"), 1, GL_FALSE, &proj[0][0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "uColor"), 1, &lineColor[0]);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, linePoints.size() * sizeof(glm::vec3), linePoints.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(linePoints.size()));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer3D::renderPlayers(const std::vector<PlayerBlob>& players) {
    if (players.empty()) return;
    glUseProgram(shaderProgram);
    glm::mat4 view, proj;
    updateMatrices(view, proj);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uView"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProj"), 1, GL_FALSE, &proj[0][0]);

    for (const auto& p : players) {
        glUniform3fv(glGetUniformLocation(shaderProgram, "uColor"), 1, &p.color[0]);
        drawCylinderDisc(p.position, p.radius, p.color);
    }
}

void Renderer3D::drawCylinderDisc(const glm::vec3& pos, float radius, const glm::vec3& color) {
    // Simple disc: a circle made of line loop for now (placeholder)
    const int segments = 32;
    std::vector<glm::vec3> verts;
    for (int i = 0; i <= segments; ++i) {
        float theta = 2.0f * 3.1415926f * static_cast<float>(i) / static_cast<float>(segments);
        float x = radius * cosf(theta);
        float z = radius * sinf(theta);
        verts.push_back(pos + glm::vec3(x, 0.0f, z));
    }
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3), verts.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINE_LOOP, 0, static_cast<GLsizei>(verts.size()));
    // Could add height extrusion for a capsule, but disc is fine for MVP
}

// ----- UI -------------------------------------------------------------------
void Renderer3D::renderUI(bool& showDemo,
                          bool& topDownView,
                          float& orbitAngle,
                          float& zoom,
                          float& cameraHeight,
                          HSVThresholds& thresholds) {
    // Left panel: video feed + HSV sliders
    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Camera & Color Isolation");
    if (ImGui::CollapsingHeader("HSV Thresholds##slider")) {
        ImGui::SliderInt("H Min", &thresholds.hMin, 0, 179);
        ImGui::SliderInt("H Max", &thresholds.hMax, 0, 179);
        ImGui::SliderInt("S Min", &thresholds.sMin, 0, 255);
        ImGui::SliderInt("S Max", &thresholds.sMax, 0, 255);
        ImGui::SliderInt("V Min", &thresholds.vMin, 0, 255);
        ImGui::SliderInt("V Max", &thresholds.vMax, 0, 255);
    }
    ImGui::Separator();
    if (ImGui::Button("Show Demo Window")) showDemo = true;
    ImGui::End();

    // Right panel: 3D viewport info (just a label; actual rendering done elsewhere)
    ImGui::Begin("3D Viewport", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("3D Field View");
    ImGui::Checkbox("Top-down 2D", &topDownView);
    ImGui::Text("Orbit angle: %.2f rad", orbitAngle);
    ImGui::Text("Zoom: %.2f", zoom);
    ImGui::Text("Camera height: %.2f m", cameraHeight);
    ImGui::End();

    // Status bar (at bottom via a full-width window)
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 20));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 20));
    ImGui::Begin("StatusBar", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::Text("FPS: %.1f | Camera: OK | YOLO: Idle", ImGui::GetIO().Framerate);
    ImGui::End();
}

// ----- Camera math ----------------------------------------------------------
void Renderer3D::updateMatrices(glm::mat4& view, glm::mat4& proj) const {
    glm::vec3 camPos;
    if (topDownView) {
        camPos = glm::vec3(0.0f, cameraHeight, 0.0f);
        view = glm::lookAt(camPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    } else {
        camPos = glm::vec3(
            zoom * sinf(orbitAngle),
            cameraHeight,
            zoom * cosf(orbitAngle)
        );
        view = glm::lookAt(camPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    const float aspect = static_cast<float>(viewportW) / std::max(1, viewportH);
    proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
}

// ----- Input callbacks ------------------------------------------------------
void Renderer3D::onMouseButton(GLFWwindow* w, int button, int action, int mods) {
    auto* self = static_cast<Renderer3D*>(glfwGetWindowUserPointer(w));
    if (!self) return;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        self->mouseDragging = true;
        double x, y;
        glfwGetCursorPos(w, &x, &y);
        self->lastMouseX = x;
        self->lastMouseY = y;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        self->mouseDragging = false;
    }
}

void Renderer3D::onCursorPos(GLFWwindow* w, double x, double y) {
    auto* self = static_cast<Renderer3D*>(glfwGetWindowUserPointer(w));
    if (!self) return;
    if (self->mouseDragging) {
        float dx = static_cast<float>(x - self->lastMouseX) * 0.005f;
        float dy = static_cast<float>(y - self->lastMouseY) * 0.005f;
        self->orbitAngle += dx;
        self->cameraHeight = std::max(0.5f, self->cameraHeight - dy * 2.0f);
        self->lastMouseX = x;
        self->lastMouseY = y;
    }
}

void Renderer3D::onScroll(GLFWwindow* w, double xoff, double yoff) {
    auto* self = static_cast<Renderer3D*>(glfwGetWindowUserPointer(w));
    if (!self) return;
    self->zoom = std::max(0.5f, self->zoom - static_cast<float>(yoff) * 0.5f);
}
