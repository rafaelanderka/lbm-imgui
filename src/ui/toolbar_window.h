#ifndef TOOLBAR_WINDOW_H
#define TOOLBAR_WINDOW_H

#include <map>
#include <unordered_map>
#include <string>
#include "imgui.h"
#include "imgui_internal.h"

#include "core/app_state.h"
#include "core/io.h"

struct ToolInfo {
  std::string description;
  std::string iconFilename;
};

class ToolbarWindow {
public:
  void render(LBM& lbm) {
    if (!isInitialised) init();

    AppState& appState = AppState::getInstance();

    const auto viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, TOOLBAR_HEIGHT));
    ImGui::Begin("Tools", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

    // Tool buttons
    for (auto [k, v] : tools) {
      bool isToolInactive = appState.activeTool != k;
      if (isToolInactive) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
      if (ImGui::ImageButton(toolTextures[k], ImVec2(25, 25))) appState.activeTool = k;
      if (isToolInactive) ImGui::PopStyleColor();
      if (ImGui::IsItemHovered()) {
          ImGui::BeginTooltip();
          ImGui::Text("%s", v.description.c_str());
          ImGui::EndTooltip();
      }
      ImGui::SameLine();
    }

    // Vertical separator
    insertVerticalSeparator();
    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    // Tool size slider
    GLfloat alignedCursorPosY = ImGui::GetCursorPos().y + (ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeight()) / 2;
    ImGui::SetCursorPosY(alignedCursorPosY);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Tool Size:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(TOOL_SIZE_SLIDER_WIDTH);
    ImGui::SetCursorPosY(alignedCursorPosY);
    ImGui::SliderFloat("##toolSize", &appState.toolSize, 0.005f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SameLine();

    if (ImGui::GetWindowSize().x < 900) {
      insertVerticalSeparator();
      ImGui::SameLine();
    }

    // Calculate the starting position for right alignment
    float rightAlignedOffset = ImGui::GetContentRegionAvail().x - rightAlignedItemsWidth;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + rightAlignedOffset);

    // Simulation speed buttons
    ImGui::SetCursorPosY(alignedCursorPosY);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", simSpeedTitle);
    ImGui::SameLine();
    for (int i = 0; i < simSpeeds.size(); i++) {
      bool isSpeedInactive = appState.stepsPerFrame != simSpeeds[i];
      if (isSpeedInactive) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
      ImGui::SetCursorPosY(alignedCursorPosY);
      if (ImGui::Button(simSpeedLabels[i])) appState.stepsPerFrame = simSpeeds[i];
      if (isSpeedInactive) ImGui::PopStyleColor();
      ImGui::SameLine();
    }

    // Vertical separator
    insertVerticalSeparator();
    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    // Reset button
    ImGui::SetCursorPosY(alignedCursorPosY);
    if (ImGui::Button(resetButtonLabel)) {
      appState.reset();
      lbm.resetAll();
    }

    ImGui::End(); // End of the toolbar
  }

private:
  std::unordered_map<ToolType, ImTextureID> toolTextures;
  std::map<ToolType, ToolInfo> tools = {
    {ToolType::Force, {"Drag fluid", "icon_tool_force_4x.png"}},
    {ToolType::AddWall, {"Add walls", "icon_tool_add_wall_4x.png"}},
    {ToolType::RemoveWall, {"Remove walls", "icon_tool_remove_wall_4x.png"}},
    {ToolType::AddSolute, {"Add solute", "icon_tool_add_solute_4x.png"}},
    {ToolType::RemoveSolute, {"Remove solute", "icon_tool_remove_solute_4x.png"}}
  };
  std::vector<int> simSpeeds = {1, 2, 4, 6};
  std::vector<const char*> simSpeedLabels = {"1x", "2x", "4x", "6x"};
  const char* simSpeedTitle = "Simulation Speed:";
  const char* resetButtonLabel = "Reset All";
  float rightAlignedItemsWidth;
  bool isInitialised = false;

  void init() {
    loadIcons();
    calcStylingConstants();
    isInitialised = true;
  }

  void loadIcons() {
    // Load tool icons as textures
    fs::path executablePath = getExecutablePath();
    fs::path resourcesDir = executablePath.parent_path() / "resources";
    for (auto [k, v] : tools) {
      toolTextures[k] = loadPNG(resourcesDir / v.iconFilename);
    }
  }

  void calcStylingConstants() {
    // Width of right-aligned labels
    float simSpeedTitleWidth = ImGui::CalcTextSize(simSpeedTitle).x;
    float labelText2Width = ImGui::CalcTextSize(resetButtonLabel).x;

    // Width of right-aligned buttons
    ImGuiStyle& style = ImGui::GetStyle();
    float simSpeedButtonsWidth = 0.f;
    for (int i = 0; i < simSpeedLabels.size(); i++) {
        simSpeedButtonsWidth += ImGui::CalcTextSize(simSpeedLabels[i]).x + 2 * style.FramePadding.x + style.ItemSpacing.x;  // Adding spacing
    }
    float resetButtonWidth = labelText2Width + 2 * style.FramePadding.x;

    // Calculate the total width of all right-aligned items
    float separatorWidth = 4 * style.ItemSpacing.x;
    rightAlignedItemsWidth = simSpeedTitleWidth + simSpeedButtonsWidth + separatorWidth + resetButtonWidth;
  }

  void insertVerticalSeparator() {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGui::Spacing();
    ImGui::SameLine();

    ImGuiStyle& style = ImGui::GetStyle();
    float available_height = ImGui::GetContentRegionAvail().y;
    float x1 = window->DC.CursorPos.x;
    float y1 = window->DC.CursorPos.y;

    float x2 = x1;
    float y2 = y1 + available_height;

    ImVec4 col = style.Colors[ImGuiCol_Separator];
    ImGui::GetWindowDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImGui::GetColorU32(col));

    ImGui::Spacing();
  }
};

#endif // TOOLBAR_WINDOW_H
