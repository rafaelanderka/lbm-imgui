#include "app.h"

#include <algorithm>
#include <cstdlib>
#include <string>
#include "imgui_internal.h"
#include "imgui_toggle.h"
#include "imgui_toggle_presets.h"

#include "core/io.h"

static void glfw_error_callback(int error, const char* description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

App::App() : state(AppState::getInstance()) {
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
  this->window = glfwCreateWindow(1280, 900, "Lattice Boltzmann Simulator", nullptr, nullptr);
  if (this->window == nullptr)
    exit(1);
  glfwMakeContextCurrent(this->window);
  glfwSwapInterval(1); // Enable vsync

  // Constrain window dimensions
  glfwSetWindowSizeLimits(window, MIN_APP_WIDTH, MIN_APP_HEIGHT, GLFW_DONT_CARE, GLFW_DONT_CARE);

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

  // Set up Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  io = &(ImGui::GetIO()); (void)io;
  this->io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  this->io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  this->io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

  // Set up Dear ImGui style
  ImGui::StyleColorsLight();

  // Set up Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(this->window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);
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
    for (int i = 0; i < state.stepsPerFrame; i++) {
      lbm->updateSimulation();
    }
    lbm->updateAnimationPhase();

    // Render GUI + viewport
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(this->window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(this->clearColor.x, this->clearColor.y, this->clearColor.z, this->clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(this->window);
  }
}

void App::init() {
  this->clearColor = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
  this->isInitialised = true;

  // Set up LBM simulation
  lbm = std::make_unique<LBM>(SIMULATION_WIDTH, SIMULATION_HEIGHT);
}

void App::updateUI() {
  // Create the fullscreen window for the dockspace
  int window_flags = ImGuiWindowFlags_NoDocking;
  const auto viewport = ImGui::GetMainViewport();
  auto dockspaceContainerPos = ImVec2(viewport->Pos.x, viewport->Pos.y + TOOLBAR_HEIGHT);
  auto dockspaceContainerSize = ImVec2(viewport->Size.x, viewport->Size.y - TOOLBAR_HEIGHT);
  ImGui::SetNextWindowPos(dockspaceContainerPos);
  ImGui::SetNextWindowSize(dockspaceContainerSize);
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

  ImGui::Begin("Root", nullptr, window_flags);
  ImGui::PopStyleVar(3);

  // Create the dockspace
  auto dockspace_id = ImGui::GetID("Dockspace");
  auto dockspace_flags = ImGuiDockNodeFlags_NoDockingInCentralNode | ImGuiDockNodeFlags_PassthruCentralNode;
  ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

  // Split the dockspace into panes
  static auto first_iteration = true;
  if (first_iteration) {
    // Reset layout
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);

    // Ensure dockspace covers entire window
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetWindowSize());

    // Split the dockspace to create left, right and top panes
    auto dock_id_right_top = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.25f, nullptr, &dockspace_id);             // Top pane of right-hand split
    auto dock_id_right_bottom = ImGui::DockBuilderSplitNode(dock_id_right_top, ImGuiDir_Down, 0.7f, nullptr, &dock_id_right_top); // Bottom pane of right-hand split
    auto dock_id_left = dockspace_id; // Top pane uses the remaining space

    // Assign default windows to the dock IDs
    ImGui::DockBuilderDockWindow("Viewport", dock_id_left);
    ImGui::DockBuilderDockWindow("Fluid Settings", dock_id_right_top);
    ImGui::DockBuilderDockWindow("Reaction Settings", dock_id_right_top);


    // For Solute settings, we dock multiple windows to the same ID to create tabs
    for (int i = 1; i < 4; i ++) {
      ImGui::DockBuilderDockWindow(("Solute " + std::to_string(i)).c_str(), dock_id_right_bottom);
    }
    ImGui::DockBuilderFinish(dockspace_id);
  }

  // Above the dockspace - Toolbar
  toolbarWindow.render(*lbm);

  // Left Pane - Viewport
  viewportWindow.render(*lbm, window);

  // Right Top Pane - Fluid and Reaction Settings
  fluidSettingsWindow.render(*lbm);
  reactionSettingsWindow.render(*lbm);

  // Right Bottom Pane - Solute Settings
  for (auto soluteSettingsWindow : soluteSettingsWindows) {
    soluteSettingsWindow.render(*lbm);
  }

  ImGui::End(); // End of the DockSpace window

  if (first_iteration) {
    ImGui::SetWindowFocus("Fluid Settings");
    ImGui::SetWindowFocus("Solute 1");
    first_iteration = false; // Ensure we only do the setup once
  }
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
