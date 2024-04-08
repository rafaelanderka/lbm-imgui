#include "app.h"

#include <cstdlib>
#include "imgui_internal.h"

#include "util.h"

const GLint WIDTH = 800;
const GLint HEIGHT = 600;

GLuint VAO;
GLuint VBO;

static void glfw_error_callback(int error, const char* description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void create_triangles() {
  GLfloat vertices[] = {
    -1.0f, -1.0f, 0.0f, // 1. vertex x, y, z
     1.0f, -1.0f, 0.0f, // 2. vertex ...
     0.0f,  1.0f, 0.0f  // etc... 
  };

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

App::App() {
  // Set up app window
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    exit(1);

  // Decide GL+GLSL versions
#if defined(__APPLE__)
  // GL 3.2 + GLSL 150
  const char* glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
  // GL 3.0 + GLSL 130
  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

  // Create window with graphics context
  this->window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
  if (this->window == nullptr)
    exit(1);
  glfwMakeContextCurrent(this->window);
  glfwSwapInterval(1); // Enable vsync

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    exit(1);
  }

  // Check required features are supported
  // if (!GLAD_GL_ARB_texture_float) {
  //   std::cout << "Floating point textures not supported!" << std::endl;
  //   exit(1);
  // }

  GLint maxColorAttachments;
  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
  if(maxColorAttachments < 4) {
    std::cout << "Your hardware supports only "
              << maxColorAttachments
              << " color attachments per framebuffer, but 4 are required."
              << std::endl;
    exit(1);
  }

  // Set up viewport
  int bufferWidth, bufferHeight;
  glfwGetFramebufferSize(this->window, &bufferWidth, &bufferHeight);
  glViewport(0, 0, bufferWidth, bufferHeight);

  create_triangles();
  fbo = std::make_unique<Framebuffer>(WIDTH, HEIGHT, 1);

  fs::path executablePath = getExecutablePath();
  fs::path shadersDir = executablePath.parent_path() / "shaders";
  fs::path vertexShaderPath = shadersDir / "vert.glsl";
  fs::path fragmentShaderPath = shadersDir / "frag.glsl";

  shader = std::make_unique<ShaderProgram>(vertexShaderPath, fragmentShaderPath);
  shader->validate(VAO);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  io = &(ImGui::GetIO()); (void)io;
  this->io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  this->io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  this->io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable multiple viewports support
  this->io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(this->window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
  // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
  // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
  // - Read 'docs/FONTS.md' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
  // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
  //io->Fonts->AddFontDefault();
  //io->Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
  //io->Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  //io->Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  //io->Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  //ImFont* font = io->Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io->Fonts->GetGlyphRangesJapanese());
  //IM_ASSERT(font != nullptr);
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
    update();

    // Update viewport
    fbo->bind();
    shader->use();
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glUseProgram(0);
    fbo->unbind();

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
  this->clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
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
    reinterpret_cast<void*>(fbo->getTexture(0)),
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
