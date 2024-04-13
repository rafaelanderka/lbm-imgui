#ifndef APP_STATE_H
#define APP_STATE_H

#include <vector>
#include "glm.hpp"
#include "imgui.h"

// Simulation constants
const GLint SIMULATION_WIDTH = 256;
const GLint SIMULATION_HEIGHT = 256;
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

// GUI and interaction constants
const unsigned int MIN_APP_WIDTH = 800;
const unsigned int MIN_APP_HEIGHT = 400;
const unsigned int MAX_COLOR_PICKER_WIDTH = 200;
const unsigned int TOOLBAR_HEIGHT = 66;
const GLfloat TOOLBAR_CONTENT_SHIFT_Y = 6.f;
const unsigned int TOOL_SIZE_SLIDER_WIDTH = 130;
const GLfloat CURSOR_FORCE_MULTIPLIER = 6.f;
const GLfloat TOOL_SIZE_MULTIPLIER = 0.5f;

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
  glm::vec2 cursorPos;        // Cursor position relative to interactive simulation area
  glm::vec2 cursorVel;        // Cursor's drag velocity, normalized to simulation context dimensions
  bool isSimulationFocussed;  // Is the cursor hovering the interactive simulation area
  bool isCursorActive ;       // Is the cursor currently clicking

  // Tool state
  ToolType activeTool;        // Currently active tool
  unsigned int activeSolute;  // Currently active solute
  GLfloat toolSize;           // Size of the tool

  // Simulation state
  bool hasVerticalWalls;      // Does the simulation have vertical boundary walls
  bool hasHorizontalWalls;    // Does the simulation have horizontal boundary walls
  unsigned int stepsPerFrame; // Number of simulation steps per rendered frame

  // Visualization state
  glm::vec2 viewportScale;    // Content scale of interactive viewport
  glm::vec2 viewportSize;     // Size of the interactive viewport
  glm::vec2 aspectRatio;      // Aspect ratio of the interactive simulation viewport
  OverlayType activeOverlay;  // Type of flow field visualisation (0: off, 1: lines, 2: arrows)

  // Fluid params
  GLfloat fluidViscosity;     // Fluid viscosity

  // Reaction params
  bool isReactionEnabled;     // Is the reaction between solutes enabled
  GLfloat reactionRate;       // Rate of reaction between solutes

  // Solutes params
  std::vector<GLfloat> soluteDiffusivities; // Solute diffusivities
  std::vector<glm::vec3> soluteColors;      // Solute colors

  // AppState access method
  static AppState& getInstance() {
    static AppState instance;
    return instance;
  }

  // Prevent copying or moving
  AppState(const AppState&) = delete;
  AppState& operator=(const AppState&) = delete;
  AppState(AppState&&) = delete;
  AppState& operator=(AppState&&) = delete;

  void reset() {
    // Resets all resettable state
    activeTool = ToolType::Force;
    toolSize = 0.2f;
    hasVerticalWalls = false;
    hasHorizontalWalls = false;
    stepsPerFrame = 1;
    activeOverlay = OverlayType::Lines;
    fluidViscosity = INIT_FLUID_VISCOSITY;
    isReactionEnabled = false;
    reactionRate = INIT_REACTION_RATE;
    soluteDiffusivities = {INIT_SOLUTE_DIFFUSIVITY_0, INIT_SOLUTE_DIFFUSIVITY_1, INIT_SOLUTE_DIFFUSIVITY_2};
    soluteColors = {INIT_SOLUTE_COLOR_0, INIT_SOLUTE_COLOR_1, INIT_SOLUTE_COLOR_2};
  }

private:
  // Constructor is private to prevent external instantiation
  AppState() {
    reset();

    // Non-resettable state
    cursorPos = {0.5f, 0.5f};
    cursorVel = {0.f, 0.f};
    isSimulationFocussed = false;
    isCursorActive = false;
    activeSolute = 0;
    viewportScale = {1.f, 1.f};
    viewportSize = {0.f, 0.f};
    aspectRatio = {1.f, 1.f};
  }
};

#endif // APP_STATE_H
