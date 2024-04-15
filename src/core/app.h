#ifndef APP_H
#define APP_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <memory>
#include <stdio.h>
#include <iostream>
#define GL_SILENCE_DEPRECATION
#include "glad/glad.h"
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include "lbm/lbm.h"
#include "core/app_state.h"
#include "ui/toolbar_window.h"
#include "ui/viewport_window.h"
#include "ui/fluid_settings_window.h"
#include "ui/reaction_settings_window.h"
#include "ui/solute_settings_window.h"

class App {
public:
  App();
  ~App();

  void run();

private:
  AppState& state;
  GLFWwindow* window;
  ImGuiIO* io;
  ImVec4 clearColor;
  bool isInitialised = false;
  std::unique_ptr<LBM> lbm;

  // UI elements
  ToolbarWindow toolbarWindow;
  ViewportWindow viewportWindow;
  FluidSettingsWindow fluidSettingsWindow;
  ReactionSettingsWindow reactionSettingsWindow;
  std::vector<SoluteSettingsWindow> soluteSettingsWindows = {SoluteSettingsWindow(0), SoluteSettingsWindow(1), SoluteSettingsWindow(2)};

  void init();
  void updateUI();
  void updateCursorData();
  void checkFeatureSupport() const;
};

#endif // APP_H
