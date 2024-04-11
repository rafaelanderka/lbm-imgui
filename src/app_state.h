#ifndef APP_STATE_H
#define APP_STATE_H

#include <vector>
#include "glm.hpp"
#include "imgui.h"

const GLfloat SPEED_OF_SOUND = 0.3;

const GLfloat INIT_FLUID_DENSITY = 1.0;
const glm::vec2 INIT_FLUID_VELOCITY = {0., 0.};
const GLfloat INIT_FLUID_VISCOSITY = 0.1;

const GLfloat INIT_SOLUTE_CONCENTRATION = 0.0;
const GLfloat INIT_SOLUTE_DIFFUSIVITY_0 = 0.02;
const GLfloat INIT_SOLUTE_DIFFUSIVITY_1 = 0.02;
const GLfloat INIT_SOLUTE_DIFFUSIVITY_2 = 0.02;
const glm::vec3 INIT_SOLUTE_COLOR_0 = {1., 0.78, 0.};
const glm::vec3 INIT_SOLUTE_COLOR_1 = {0.39, 0., 1.};
const glm::vec3 INIT_SOLUTE_COLOR_2 = {0.39, 1., 0.78};
const glm::vec2 INIT_SOLUTE_CENTER_0 = {0.4, 0.4};
const glm::vec2 INIT_SOLUTE_CENTER_1 = {0.5, 0.6};
const glm::vec2 INIT_SOLUTE_CENTER_2 = {0.6, 0.4};
const GLfloat INIT_SOLUTE_RADIUS_0 = 0.2;
const GLfloat INIT_SOLUTE_RADIUS_1 = 0.2;
const GLfloat INIT_SOLUTE_RADIUS_2 = 0.2;

const std::vector<GLfloat> REACTION_MOLAR_MASSES = {1, 1, 1};
const std::vector<GLint> REACTION_STOICHIOMETRIC_COEFFS = {-1, -1, 1};
const GLfloat INIT_REACTION_RATE = 0.01;

enum class ToolType {
  Force,
  AddWall,
  RemoveWall,
  AddSolute,
  RemoveSolute,
};

enum class OverlayType {
  None,
  Lines,
  Arrows,
};

struct AppState {
  // Cursor input
  glm::vec2 cursorPos;       // Cursor position relative to interactive simulation area
  glm::vec2 cursorVel;       // Cursor's drag velocity, normalized to simulation context dimensions
  bool isSimulationFocussed; // Is the cursor hovering the interactive simulation area
  bool isCursorActive ;      // Is the cursor currently clicking

  // Tool state
  ToolType activeTool;       // Currently active tool
  unsigned int activeSolute; // Currently active solute
  GLfloat toolSize;          // Size of the tool

  // Simulation state
  bool hasVerticalWalls;     // Does the simulation have vertical boundary walls
  bool hasHorizontalWalls;   // Does the simulation have horizontal boundary walls

  // Visualization state
  glm::vec2 viewportScale;   // Content scale of interactive viewport
  glm::vec2 viewportSize;       // Size of the interactive viewport
  glm::vec2 aspectRatio;     // Aspect ratio of the interactive simulation viewport
  OverlayType activeOverlay; // Type of flow field visualisation (0: off, 1: lines, 2: arrows)

  // Fluid params
  GLfloat fluidViscosity;    // Fluid viscosity

  // Reaction params
  bool isReactionEnabled;    // Is the reaction between solutes enabled
  GLfloat reactionRate;      // Rate of reaction between solutes

  // Solutes params
  std::vector<GLfloat> soluteDiffusivities; // Solute diffusivities
  std::vector<glm::vec3> soluteColors;      // Solute colors

  AppState() {
    cursorPos = {0.5f, 0.5f};
    cursorVel = {0.0f, 0.f};
    isSimulationFocussed = false;
    isCursorActive = false;
    activeTool = ToolType::Force;
    activeSolute = 0;
    toolSize = 0.1f;
    hasVerticalWalls = false;
    hasHorizontalWalls = false;
    viewportScale = {1.f, 1.f};
    viewportSize = {0.f, 0.f};
    aspectRatio = {1.f, 1.f};
    activeOverlay = OverlayType::Lines;
    fluidViscosity = INIT_FLUID_VISCOSITY;
    isReactionEnabled = false;
    reactionRate = INIT_REACTION_RATE;
    soluteDiffusivities = {INIT_SOLUTE_DIFFUSIVITY_0, INIT_SOLUTE_DIFFUSIVITY_1, INIT_SOLUTE_DIFFUSIVITY_2};
    soluteColors = {INIT_SOLUTE_COLOR_0, INIT_SOLUTE_COLOR_1, INIT_SOLUTE_COLOR_2};
  }
};

#endif // APP_STATE_H
