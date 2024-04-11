#include "app.h"

#include <algorithm>
#include <cstdlib>
#include <string>
#include "imgui_internal.h"
#include "imgui_toggle.h"
#include "imgui_toggle_presets.h"

#include "io.h"

const GLint SIMULATION_WIDTH = 256;
const GLint SIMULATION_HEIGHT = 256;
const GLfloat CURSOR_FORCE_MULTIPLIER = 8.f;
const unsigned int MAX_COLOR_PICKER_WIDTH = 200;
const unsigned int COLOR_PREVIEW_HEIGHT = 40;

static void glfw_error_callback(int error, const char* description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

App::App() {
  // Set up app window
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    exit(1);

  // Request GL 3.3 + GLSL 330
  const char* glsl_version = "#version 330";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
#if defined(__APPLE__)
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // Required on Mac
#endif
  printf("Requested OpenGL version: 3.3\n");
  printf("Requested GLSL version: 330\n");

  // Create window with graphics context
  this->window = glfwCreateWindow(1280, 900, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
  if (this->window == nullptr)
    exit(1);
  glfwMakeContextCurrent(this->window);
  glfwSwapInterval(1); // Enable vsync

  // Load GLAD bindings
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    exit(1);
  }

  // Log active GPU and OpenGL version
  printf("GPU: %s\n", glGetString(GL_RENDERER));
  printf("Active OpenGL version: %s\n", glGetString(GL_VERSION));

  // Check all required features are supported
  checkFeatureSupport();

  // Set up viewport
  int bufferWidth, bufferHeight;
  glfwGetFramebufferSize(this->window, &bufferWidth, &bufferHeight);
  glViewport(0, 0, bufferWidth, bufferHeight);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  io = &(ImGui::GetIO()); (void)io;
  this->io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  this->io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  this->io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable multiple viewports support
  this->io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

  // Setup Dear ImGui style
  // ImGui::StyleColorsDark();
  ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(this->window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Setup LBM simulation
  lbm = std::make_unique<LBM>(SIMULATION_WIDTH, SIMULATION_HEIGHT, state);
}

App::~App() {
  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  this->io = nullptr;

  glfwDestroyWindow(this->window);
  glfwTerminate();
}

void App::checkFeatureSupport() const {
  // Check sufficient texture units are supported (>=9)
  GLint maxTextureUnits;
  glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
  if(maxTextureUnits < 9) {
    std::cout << "Your hardware supports only "
              << maxTextureUnits
              << " texture units, but 9 are required."
              << std::endl;
    exit(1);
  }

  // Check sufficient colour attachments are supported (>=4)
  GLint maxColorAttachments;
  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
  if(maxColorAttachments < 4) {
    std::cout << "Your hardware supports only "
              << maxColorAttachments
              << " color attachments per framebuffer, but 4 are required."
              << std::endl;
    exit(1);
  }
}

void App::run() {
  // Initialise app state
  init();

  // Main loop
  while(!glfwWindowShouldClose(this->window)) {
    // Poll and handle events (inputs, window resize, etc.)
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Update GUI and process user input
    updateUI();

    // Update LBM simulation
    lbm->update();

    // Render GUI + viewport
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(this->window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(this->clearColor.x, this->clearColor.y, this->clearColor.z, this->clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }

    glfwSwapBuffers(this->window);
  }
}

void App::init() {
  this->clearColor = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
  this->isInitialised = true;
}

void App::updateUI() {
  // 1. Create the fullscreen window for the dockspace
  auto window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
  const auto viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->Pos);
  ImGui::SetNextWindowSize(viewport->Size);
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

  ImGui::Begin("Root", nullptr, window_flags);
  ImGui::PopStyleVar(3);

  // 2. Create the dockspace
  auto dockspace_id = ImGui::GetID("Dockspace");
  auto dockspace_flags = ImGuiDockNodeFlags_NoDockingInCentralNode | ImGuiDockNodeFlags_PassthruCentralNode;
  ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

  // 3. Splitting the dockspace into two vertical panes
  static auto first_iteration = true;
  if (first_iteration) {

    // Reset layout
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);

    // Ensure dockspace covers entire window
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetWindowSize());

    // Split the dockspace to create left, right and top panes
    auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.9f, nullptr, &dockspace_id);
    auto dock_id_right_top = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Right, 0.25f, nullptr, &dock_id_left);             // Top pane of right-hand split
    auto dock_id_right_bottom = ImGui::DockBuilderSplitNode(dock_id_right_top, ImGuiDir_Down, 0.7f, nullptr, &dock_id_right_top); // Bottom pane of right-hand split
    auto dock_id_top = dockspace_id; // Top pane uses the remaining space

    // Assign default windows to the dock IDs
    ImGui::DockBuilderDockWindow("Toolbar", dock_id_top);
    ImGui::DockBuilderDockWindow("Viewport", dock_id_left);
    ImGui::DockBuilderDockWindow("Fluid Settings", dock_id_right_top);
    ImGui::DockBuilderDockWindow("Reaction Settings", dock_id_right_top);


    // For Solute settings, we dock multiple windows to the same ID to create tabs
    for (int i = 1; i < 4; i ++) {
      ImGui::DockBuilderDockWindow(("Solute " + std::to_string(i)).c_str(), dock_id_right_bottom);
    }
    ImGui::DockBuilderFinish(dockspace_id);
  }

  // Top Pane - Toolbar
  ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
  // Content for the toolbar
  ImGui::End();

  // Left Pane - Viewport
  ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

  // Update window data
  glfwGetWindowContentScale(window, &state.viewportScale.x, &state.viewportScale.y);
  ImVec2 viewportSize = ImGui::GetContentRegionAvail();
  GLfloat aspect = viewportSize.x / viewportSize.y;
  state.aspectRatio = aspect > 1. ? glm::vec2(aspect, 1.) : glm::vec2(1., 1. / aspect);
  if (state.viewportSize.x != state.viewportScale.x * viewportSize.x || state.viewportSize.y != state.viewportScale.y * viewportSize.y) {
    state.viewportSize = glm::vec2(state.viewportScale.x * viewportSize.x, state.viewportScale.y * viewportSize.y);
    lbm->resize();
  }

  // Poll the cursor state over the interactive simulation viewport
  updateCursorData();

  // Add simulation output texture as image to ImGui
  ImVec2 pos = ImGui::GetCursorScreenPos();
  const float window_width = ImGui::GetContentRegionAvail().x;
  const float window_height = ImGui::GetContentRegionAvail().y;
  ImGui::GetWindowDrawList()->AddImage(
    reinterpret_cast<void*>(lbm->getOutputTexture()),
    ImVec2(pos.x, pos.y),
    ImVec2(pos.x + window_width, pos.y + window_height),
    ImVec2(0, 1),
    ImVec2(1, 0)
  );

  ImGui::End();

  // Right Top Pane - Fluid and Reaction Settings
  insertFluidSettings();
  insertReactionSettings();

  // Right Bottom Pane - Solute Settings
  for (int i = 0; i < 3; i ++) {
    insertSoluteSettings(i);
  }

  ImGui::End(); // End of the DockSpace window

  if (first_iteration) {
    ImGui::SetWindowFocus("Fluid Settings");
    ImGui::SetWindowFocus("Solute 1");
    first_iteration = false; // Ensure we only do the setup steps once
  }
}

void App::insertFluidSettings() {
  ImGui::Begin("Fluid Settings", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar);

  // Fluid viscosity slider
  ImGui::Text("Fluid Viscosity");
  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
  if (ImGui::SliderFloat("##viscosity", &state.fluidViscosity, 0.01f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
    lbm->setViscosity(state.fluidViscosity);
  }

  // Spacing for aesthetics
  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  // Boundary walls toggle
  ImGui::Text("Boundary Walls");
  static int boundaryWalls = 0;
  ImGui::RadioButton("Off##1", &boundaryWalls, 0); ImGui::SameLine();
  ImGui::RadioButton("Vertical", &boundaryWalls, 1); ImGui::SameLine();
  ImGui::RadioButton("Horizontal", &boundaryWalls, 2); ImGui::SameLine();
  ImGui::RadioButton("All", &boundaryWalls, 3);
  state.hasVerticalWalls = boundaryWalls == 1 || boundaryWalls == 3;
  state.hasHorizontalWalls = boundaryWalls == 2 || boundaryWalls == 3;

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  // Fluid flow overlay toggle
  ImGui::Text("Flow Field Visualization");
  ImGui::RadioButton("Off##2", reinterpret_cast<int*>(&state.activeOverlay), static_cast<int>(OverlayType::None)); ImGui::SameLine();
  ImGui::RadioButton("Lines", reinterpret_cast<int*>(&state.activeOverlay), static_cast<int>(OverlayType::Lines)); ImGui::SameLine();
  ImGui::RadioButton("Arrows", reinterpret_cast<int*>(&state.activeOverlay), static_cast<int>(OverlayType::Arrows));

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  // Reset section
  ImGui::Text("Reset");
  if (ImGui::Button("Reset Fluid")) {
    lbm->resetFluid();
  }
  ImGui::SameLine();
  if (ImGui::Button("Reset Walls")) {
    lbm->resetNodeIDs();
  }

  ImGui::End(); // End of the fluid settings window
}

void App::insertReactionSettings() {
  ImGui::Begin("Reaction Settings", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar);

  ImGui::Text("Enable Reaction");
  ImGui::Toggle("##enableReaction", &state.isReactionEnabled);

  // Spacing for aesthetics
  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  // Enable/disable the remaining settings depending on if reaction is active
  if (!state.isReactionEnabled) {
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
  }

  // Reaction rate slider
  ImGui::Text("Reaction Rate");
  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
  if (ImGui::SliderFloat("##reactionRate", &state.reactionRate, 0.f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
    lbm->setReactionRate(state.reactionRate);
  }

  if (!state.isReactionEnabled) {
    ImGui::PopItemFlag();
    ImGui::PopStyleVar();
  }

  // Add note explaining the stoichiometry
  ImGui::Spacing();
  ImGui::TextWrapped("Note: This simulation adheres to a fixed stoichiometry, where one unit each of solutes 1 and 2 combine at the set rate to form one unit of solute 3.");
  ImGui::End(); // End of the reaction settings window
}


void App::insertSoluteSettings(unsigned int soluteID) {
  ImGui::Begin(("Solute " + std::to_string(soluteID + 1)).c_str(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar);

  // Solute diffusivity slider
  ImGui::Text("Solute Diffusivity");
  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
  if (ImGui::SliderFloat(("##diffusivity" + std::to_string(soluteID)).c_str(), &state.soluteDiffusivities[soluteID], 0.01f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
    lbm->setSoluteDiffusivity(soluteID, state.soluteDiffusivities[soluteID]);
  }

  // Spacing for aesthetics
  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  // // Solute color picker
  unsigned int colorPickerWidth = std::min(static_cast<unsigned int>(ImGui::GetContentRegionAvail().x), MAX_COLOR_PICKER_WIDTH);
  ImGui::Text("Solute Color");
  // ImGui::ColorButton("Color Preview", ImVec4(state.soluteColors[soluteID].x, state.soluteColors[soluteID].y, state.soluteColors[soluteID].z, 1.f),
  //                    ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_NoTooltip, ImVec2(colorPickerWidth, COLOR_PREVIEW_HEIGHT));
  ImGui::Spacing();
  ImGui::SetNextItemWidth(colorPickerWidth);
  if (ImGui::ColorPicker3(("##color" + std::to_string(soluteID)).c_str(), &state.soluteColors[soluteID][0], ImGuiColorEditFlags_NoSidePreview)) {
    lbm->setSoluteColor(soluteID, state.soluteColors[soluteID]);
  }

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  // Reset section
  ImGui::Text("Reset");
  if (ImGui::Button("Reset Solute")) {
    lbm->resetSolute(soluteID);
  }

  ImGui::End(); // End of the solute settings window
}

void App::updateCursorData() {
  ImVec2 viewportPos = ImGui::GetCursorScreenPos();     // Top-left corner of the window

  // Calculate the cursor's relative position within the window
  ImVec2 absoluteCursorPos = io->MousePos;
  state.cursorPos = glm::vec2((absoluteCursorPos.x - viewportPos.x) / state.viewportSize.x * state.viewportScale.x,
                              1.f + (viewportPos.y - absoluteCursorPos.y) / state.viewportSize.y * state.viewportScale.y);

  // Update the data structure
  state.isSimulationFocussed = ImGui::IsWindowHovered();
  auto isCursorDragging = ImGui::IsMouseDragging(0);
  state.isCursorActive = ImGui::IsMouseClicked(0) || isCursorDragging;

  // Calculate drag velocity if dragging
  if (isCursorDragging) {
    ImVec2 dragDelta = ImGui::GetMouseDragDelta(0); // Assuming left-click
    ImGui::ResetMouseDragDelta(0);
    state.cursorVel = glm::vec2(CURSOR_FORCE_MULTIPLIER * dragDelta.x / state.viewportSize.x * state.viewportScale.x,
                                -CURSOR_FORCE_MULTIPLIER * dragDelta.y / state.viewportSize.y * state.viewportScale.y);
  } else {
    state.cursorVel = glm::vec2(0., 0.);
  }
}
