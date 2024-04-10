#include "app.h"

#include <cstdlib>
#include "imgui_internal.h"

#include "io.h"

const GLint SIMULATION_WIDTH = 256;
const GLint SIMULATION_HEIGHT = 256;

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
  this->window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
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
  lbm = std::make_unique<LBM>(SIMULATION_WIDTH, SIMULATION_HEIGHT);
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
    update();

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
  this->isInitialised = false;
}

void App::update() {
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
    first_iteration = false; // Ensure we only do this once

    // Reset layout
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);

    // Ensure dockspace covers entire window
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetWindowSize());

    // Split the dockspace to create left, right and top panes
    auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.9f, nullptr, &dockspace_id);
    auto dock_id_right = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Right, 0.3f, nullptr, &dock_id_left);
    auto dock_id_top = dockspace_id; // Top pane uses the remaining space

    // Assign default windows to the dock IDs
    ImGui::DockBuilderDockWindow("Toolbar", dock_id_top);
    ImGui::DockBuilderDockWindow("Viewport", dock_id_left);
    ImGui::DockBuilderDockWindow("Settings", dock_id_right);
    ImGui::DockBuilderFinish(dockspace_id);
  }

  // Top Pane - Toolbar
  const float toolbarFixedHeight = 30.0f; // Example fixed height for the toolbar
  ImGui::SetNextWindowSizeConstraints(ImVec2(-1, toolbarFixedHeight), ImVec2(-FLT_MIN, toolbarFixedHeight)); // Width is flexible, height is fixed
  ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
  // Content for the toolbar
  ImGui::End();

  // Left Pane - Viewport
  ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

  // we get the screen position of the window
  ImVec2 pos = ImGui::GetCursorScreenPos();

  // and here we can add our created texture as image to ImGui
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

  // Right Pane - Settings
  ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
  // Content for the settings pane
  ImGui::End();

  // Finish up the dockspace
  ImGui::End(); // End DockSpace window
}
