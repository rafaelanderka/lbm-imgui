# Add the executable for your application
file(GLOB SOURCES *.cpp)
add_executable(lbm ${SOURCES})

# Include directories for your application
target_include_directories(lbm PRIVATE ${PROJECT_SOURCE_DIR}/include)

# Link libraries
find_package(OpenGL REQUIRED)
target_link_libraries(lbm PRIVATE imgui glfw OpenGL::GL)

# Set executable directory
set_target_properties(lbm PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
