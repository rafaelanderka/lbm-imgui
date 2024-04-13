# Add executable
file(GLOB SOURCES *.cpp core/*.cpp gl/*.cpp lbm/*.cpp ui/*.cpp)
add_executable(lbm ${SOURCES})

# Set include directories
target_include_directories(lbm PRIVATE ${PROJECT_SOURCE_DIR}/src)

# Link libraries
find_package(OpenGL REQUIRED)
target_link_libraries(lbm PRIVATE glad glfw glm::glm-header-only imgui imgui_toggle OpenGL::GL stb)

# Set executable directory
set_target_properties(lbm PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
# Copy shaders directory to binary dir
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/src/shaders
            ${CMAKE_BINARY_DIR}/bin/shaders
    COMMENT "Copying shaders to binary directory")

# Copy resources directory to binary dir
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources
            ${CMAKE_BINARY_DIR}/bin/resources
    COMMENT "Copying resources to binary directory")
