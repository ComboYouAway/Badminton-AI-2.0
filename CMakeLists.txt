cmake_minimum_required(VERSION 3.18)
project(BadmintonAI2 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(BUILD_WITH_CUDA "Enable CUDA support for YOLO inference" OFF)
option(BUILD_WITH_TENSORRT "Enable TensorRT backend" OFF)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
# OpenCV (must be installed on the system)
find_package(OpenCV REQUIRED COMPONENTS core videoio imgproc imgcodecs)

# OpenGL (system-provided on Windows)
find_package(OpenGL REQUIRED)

# GLFW (windowing + input)
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS   OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS    OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLM (header-only 3D math)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        0.9.9.8
)
FetchContent_MakeAvailable(glm)

# Glad (OpenGL function loader)
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        v2.0.6
)
set(GLAD_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(GLAD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glad)

# ImGui (immediate-mode GUI)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.90.6
)
FetchContent_MakeAvailable(imgui)

# ---------------------------------------------------------------------------
# Include directories
# ---------------------------------------------------------------------------
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${glad_SOURCE_DIR}/include
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# ---------------------------------------------------------------------------
# Source files
# ---------------------------------------------------------------------------
set(SOURCES
    src/main.cpp
    src/VideoProcessor.cpp
    src/FieldTracker.cpp
    src/Renderer3D.cpp
    src/YoloTracker.cpp
)

set(HEADERS
    include/VideoProcessor.hpp
    include/FieldTracker.hpp
    include/Renderer3D.hpp
    include/YoloTracker.hpp
)

set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# ---------------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------------
add_executable(BadmintonAI2 ${SOURCES} ${HEADERS} ${IMGUI_SOURCES})

target_include_directories(BadmintonAI2 PRIVATE include)

target_link_libraries(BadmintonAI2 PRIVATE
    OpenCV::core
    OpenCV::videoio
    OpenCV::imgproc
    OpenCV::imgcodecs
    OpenGL::GL
    glad
    glfw
    glm::glm
)

# Windows: link OpenGL explicitly (already via OpenGL::GL but harmless)
if(WIN32)
    target_link_libraries(BadmintonAI2 PRIVATE opengl32.lib)
endif()

# ---------------------------------------------------------------------------
# Post-build: copy OpenCV DLLs next to the executable on Windows (optional)
# ---------------------------------------------------------------------------
if(WIN32 AND OpenCV_INSTALL_PATH)
    set(OPENCV_BIN_DIR "${OpenCV_INSTALL_PATH}/x64/vc16/bin")
    if(EXISTS "${OPENCV_BIN_DIR}")
        add_custom_command(TARGET BadmintonAI2 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${OPENCV_BIN_DIR}" "$<TARGET_FILE_DIR:BadmintonAI2>"
        )
    endif()
endif()