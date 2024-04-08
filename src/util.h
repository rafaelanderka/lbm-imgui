#include <filesystem>
#if defined(_WIN32)
  #include <windows.h>
#elif defined(__linux__)
  #include <unistd.h>
  #include <limits.h>
#elif defined(__APPLE__)
  #include <mach-o/dyld.h>
  #include <limits.h>
#endif

namespace fs = std::filesystem;

fs::path getExecutablePath();
