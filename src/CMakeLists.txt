# Add the executable for your application
file(GLOB SOURCES *.cpp)
add_executable(lbm ${SOURCES})

# Include directories for your application
target_include_directories(lbm PRIVATE ${PROJECT_SOURCE_DIR}/src)

# Link libraries
find_package(OpenGL REQUIRED)
target_link_libraries(lbm PRIVATE imgui glfw OpenGL::GL glad glm::glm-header-only)

# Set executable directory
set_target_properties(lbm PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
# Copy shaders directory to binary dir, adjusting for the new shaders location
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/src/shaders
            ${CMAKE_BINARY_DIR}/bin/shaders
    COMMENT "Copying shaders to binary directory")
