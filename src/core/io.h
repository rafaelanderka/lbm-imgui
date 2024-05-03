#include <filesystem>
#if defined(_WIN32)
  #define NOMINMAX
  #include <windows.h>
#elif defined(__linux__)
  #include <unistd.h>
  #include <limits.h>
#elif defined(__APPLE__)
  #include <mach-o/dyld.h>
  #include <limits.h>
#endif

#include "glad/glad.h"
#include "imgui.h"

namespace fs = std::filesystem;

fs::path getExecutablePath();
ImTextureID loadPNG(const fs::path& imagePath);
