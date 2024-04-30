#ifndef REACTION_SETTINGS_WINDOW_H
#define REACTION_SETTINGS_WINDOW_H

#include <memory>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_toggle.h"

#include "core/app_state.h"
#include "ui/window.h"
#include "lbm/lbm.h"

class ReactionSettingsWindow : public Window {
public:
  ReactionSettingsWindow(std::shared_ptr<LBM> lbm) : lbm(lbm) {}

  void render() override {
    AppState& appState = AppState::getInstance();
    ImGui::Begin("Reaction Settings", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar);

    ImGui::Text("Enable Reaction");
    ImGui::Toggle("##enableReaction", &appState.isReactionEnabled);

    // Spacing for aesthetics
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Enable/disable the remaining settings depending on if reaction is active
    if (!appState.isReactionEnabled) {
      ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
      ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    }

    // Reaction rate slider
    ImGui::Text("Reaction Rate");
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::SliderFloat("##reactionRate", &appState.reactionRate, 0.f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
      lbm->setReactionRate(appState.reactionRate);
    }

    if (!appState.isReactionEnabled) {
      ImGui::PopItemFlag();
      ImGui::PopStyleVar();
    }

    // Add note explaining the stoichiometry
    ImGui::Spacing();
    ImGui::TextWrapped("Note: This simulation adheres to a fixed stoichiometry, where one unit each of solutes 1 and 2 combine at the set rate to form one unit of solute 3.");
    ImGui::End(); // End of the reaction settings window
  }

private:
  std::shared_ptr<LBM> lbm;
};

#endif // REACTION_SETTINGS_WINDOW_H
