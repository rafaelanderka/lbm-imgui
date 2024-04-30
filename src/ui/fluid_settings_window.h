#ifndef FLUID_SETTINGS_WINDOW_H
#define FLUID_SETTINGS_WINDOW_H

#include <memory>

#include "imgui.h"

#include "core/app_state.h"
#include "ui/window.h"
#include "lbm/lbm.h"

class FluidSettingsWindow : public Window {
public:
  FluidSettingsWindow(std::shared_ptr<LBM> lbm) : lbm(lbm) {}

  void render() override {
    AppState& appState = AppState::getInstance();
    ImGui::Begin("Fluid Settings", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar);

    // Fluid viscosity slider
    ImGui::Text("Fluid Viscosity");
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::SliderFloat("##viscosity", &appState.fluidViscosity, 0.01f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
      lbm->setViscosity(appState.fluidViscosity);
    }

    // Spacing for aesthetics
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Boundary walls toggle
    // We use a trick here, converting our boolean state into an integer for simplicity
    ImGui::Text("Boundary Walls");
    int boundaryWalls = (appState.hasVerticalWalls ? 1 : 0) + (appState.hasHorizontalWalls ? 2 : 0);
    ImGui::RadioButton("Off##1", &boundaryWalls, 0); ImGui::SameLine();
    ImGui::RadioButton("Vertical", &boundaryWalls, 1); ImGui::SameLine();
    ImGui::RadioButton("Horizontal", &boundaryWalls, 2); ImGui::SameLine();
    ImGui::RadioButton("All", &boundaryWalls, 3);
    appState.hasVerticalWalls = boundaryWalls == 1 || boundaryWalls == 3;
    appState.hasHorizontalWalls = boundaryWalls == 2 || boundaryWalls == 3;

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Fluid flow overlay toggle
    ImGui::Text("Flow Field Visualization");
    ImGui::RadioButton("Off##2", reinterpret_cast<int*>(&appState.activeOverlay), static_cast<int>(OverlayType::None)); ImGui::SameLine();
    ImGui::RadioButton("Lines", reinterpret_cast<int*>(&appState.activeOverlay), static_cast<int>(OverlayType::Lines)); ImGui::SameLine();
    ImGui::RadioButton("Arrows", reinterpret_cast<int*>(&appState.activeOverlay), static_cast<int>(OverlayType::Arrows));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Reset section
    ImGui::Text("Reset");
    if (ImGui::Button("Reset Fluid")) {
      lbm->resetFluid();
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Walls")) {
      lbm->resetNodeIDs();
    }

    ImGui::End(); // End of the fluid settings window
  }

private:
  std::shared_ptr<LBM> lbm;
};

#endif // FLUID_SETTINGS_WINDOW_H
