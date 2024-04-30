#ifndef APP_H
#define APP_H

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <memory>
#include <stdio.h>
#include <iostream>
#define GL_SILENCE_DEPRECATION
// #include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include "lbm/lbm.h"
#include "core/app_state.h"
#include "ui/toolbar_window.h"
#include "ui/viewport_window.h"
#include "ui/fluid_settings_window.h"
#include "ui/reaction_settings_window.h"
#include "ui/solute_settings_window.h"

class App {
public:
  using WindowList = std::vector<std::shared_ptr<Window>>;

  App();
  ~App();

  void run();

private:
  GLFWwindow* window;
  ImGuiIO* io;
  ImVec4 clearColor;
  bool isInitialised = false;
  std::shared_ptr<LBM> lbm;

  // UI elements
  WindowList windows;

  void init();
  void updateUI();
  void updateCursorData();
  void checkFeatureSupport() const;
};

#endif // APP_H
