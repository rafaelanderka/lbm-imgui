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
#include "app_state.h"

class App {
public:
  App();
  ~App();

  void run();

private:
  AppState state;
  GLFWwindow* window;
  ImGuiIO* io;
  ImVec4 clearColor;
  bool isInitialised;
  std::unique_ptr<LBM> lbm;

  void init();
  void updateUI();
  void checkFeatureSupport() const;
  void insertToolbar();
  void insertFluidSettings();
  void insertReactionSettings();
  void insertSoluteSettings(unsigned int soluteID);
  void updateCursorData();
};
