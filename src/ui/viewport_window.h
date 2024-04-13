#ifndef VIEWPORT_WINDOW_H
#define VIEWPORT_WINDOW_H

#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_toggle.h"

#include "lbm/lbm.h"

class ViewportWindow {
public:
  void render(LBM& lbm, GLFWwindow* window) {
    AppState& appState = AppState::getInstance();
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    // Update window data
    glfwGetWindowContentScale(window, &appState.viewportScale.x, &appState.viewportScale.y);
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    GLfloat aspect = viewportSize.x / viewportSize.y;
    appState.aspectRatio = aspect > 1. ? glm::vec2(aspect, 1.) : glm::vec2(1., 1. / aspect);
    if (appState.viewportSize.x != appState.viewportScale.x * viewportSize.x || appState.viewportSize.y != appState.viewportScale.y * viewportSize.y) {
      appState.viewportSize = glm::vec2(appState.viewportScale.x * viewportSize.x, appState.viewportScale.y * viewportSize.y);
      lbm.resize();
    }

    // Poll the cursor state over the interactive simulation viewport
    updateCursorData();

    // Add simulation output texture as image to ImGui
    ImVec2 pos = ImGui::GetCursorScreenPos();
    const float window_width = ImGui::GetContentRegionAvail().x;
    const float window_height = ImGui::GetContentRegionAvail().y;
    ImGui::GetWindowDrawList()->AddImage(
      reinterpret_cast<void*>(lbm.getOutputTexture()),
      ImVec2(pos.x, pos.y),
      ImVec2(pos.x + window_width, pos.y + window_height),
      ImVec2(0, 1),
      ImVec2(1, 0)
    );

    ImGui::End(); // End of the Viewport window
  }

private:
  void updateCursorData() {
    ImGuiIO& io = ImGui::GetIO();
    AppState& appState = AppState::getInstance();

    // Calculate the cursor's relative position within the window
    ImVec2 viewportPos = ImGui::GetCursorScreenPos(); // Top-left corner of the window
    ImVec2 absoluteCursorPos = io.MousePos;
    appState.cursorPos = glm::vec2((absoluteCursorPos.x - viewportPos.x) / appState.viewportSize.x * appState.viewportScale.x,
                                1.f + (viewportPos.y - absoluteCursorPos.y) / appState.viewportSize.y * appState.viewportScale.y);

    // Update the data structure
    appState.isSimulationFocussed = ImGui::IsWindowHovered();
    auto isCursorDragging = ImGui::IsMouseDragging(0);
    appState.isCursorActive = ImGui::IsMouseDown(0);

    // Calculate drag velocity if dragging
    if (isCursorDragging) {
      ImVec2 dragDelta = ImGui::GetMouseDragDelta(0); // Assuming left-click
      ImGui::ResetMouseDragDelta(0);
      appState.cursorVel = glm::vec2(CURSOR_FORCE_MULTIPLIER * dragDelta.x / appState.viewportSize.x * appState.viewportScale.x,
                                  -CURSOR_FORCE_MULTIPLIER * dragDelta.y / appState.viewportSize.y * appState.viewportScale.y);
    } else {
      appState.cursorVel = glm::vec2(0., 0.);
    }
  }
};

#endif // VIEWPORT_WINDOW_H