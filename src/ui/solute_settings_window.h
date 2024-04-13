#ifndef SOLUTE_SETTINGS_WINDOW_H
#define SOLUTE_SETTINGS_WINDOW_H

#include <string>
#include "imgui.h"
#include "imgui_toggle.h"

#include "lbm/lbm.h"

class SoluteSettingsWindow {
public:
  SoluteSettingsWindow(unsigned int soluteID) : soluteID(soluteID) {}

  void render(LBM& lbm) {
    AppState& appState = AppState::getInstance();
    ImGui::Begin(("Solute " + std::to_string(soluteID + 1)).c_str(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar);

    if (ImGui::IsWindowFocused()) {
      appState.activeSolute = soluteID;
    }

    // Solute diffusivity slider
    ImGui::Text("Solute Diffusivity");
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::SliderFloat(("##diffusivity" + std::to_string(soluteID)).c_str(), &appState.soluteDiffusivities[soluteID], 0.01f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
      lbm.setSoluteDiffusivity(soluteID, appState.soluteDiffusivities[soluteID]);
    }

    // Spacing for aesthetics
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // // Solute color picker
    unsigned int colorPickerWidth = std::min(static_cast<unsigned int>(ImGui::GetContentRegionAvail().x), MAX_COLOR_PICKER_WIDTH);
    ImGui::Text("Solute Color");
    // ImGui::ColorButton("Color Preview", ImVec4(appState.soluteColors[soluteID].x, appState.soluteColors[soluteID].y, appState.soluteColors[soluteID].z, 1.f),
    //                    ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_NoTooltip, ImVec2(colorPickerWidth, COLOR_PREVIEW_HEIGHT));
    ImGui::Spacing();
    ImGui::SetNextItemWidth(colorPickerWidth);
    if (ImGui::ColorPicker3(("##color" + std::to_string(soluteID)).c_str(), &appState.soluteColors[soluteID][0], ImGuiColorEditFlags_NoSidePreview)) {
      lbm.setSoluteColor(soluteID, appState.soluteColors[soluteID]);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Reset section
    ImGui::Text("Reset");
    if (ImGui::Button("Clear Solute")) {
      lbm.resetSolute(soluteID);
    }

    ImGui::End(); // End of the solute settings window
  }

private:
  unsigned int soluteID;
};

#endif // SOLUTE_SETTINGS_WINDOW_H
