cmake_minimum_required(VERSION 3.15)
project(lbm VERSION 1.0 LANGUAGES CXX)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add glfw
add_subdirectory(extern/glfw)

# Build ImGui
file(GLOB IMGUI_SOURCES
     extern/imgui/*.cpp
     extern/imgui/backends/imgui_impl_glfw.cpp
     extern/imgui/backends/imgui_impl_opengl3.cpp)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC extern/imgui extern/imgui/backends)
target_link_libraries(imgui PUBLIC glfw)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)

# Add your application's source directory
add_subdirectory(src)
