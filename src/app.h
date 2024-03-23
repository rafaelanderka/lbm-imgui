#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <cstdlib>
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

class App {
public:
  App();
  ~App();

  void run();

private:
  GLFWwindow* window;
  ImGuiIO* io;
  ImVec4 clear_color;
  bool show_demo_window;
  bool show_another_window;

  void init();
  void update();
};
