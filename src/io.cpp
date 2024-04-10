#include "io.h"

fs::path getExecutablePath() {
#if defined(_WIN32)
  char path[MAX_PATH];
  GetModuleFileNameA(NULL, path, MAX_PATH);
  return fs::path(path);
#elif defined(__linux__)
  char path[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
  return fs::path(std::string(path, (count > 0) ? count : 0));
#elif defined(__APPLE__)
  char path[PATH_MAX];
  uint32_t size = sizeof(path);
  if (_NSGetExecutablePath(path, &size) != 0) {
    // Buffer size is too small.
    return fs::path();
  }
  return fs::path(path);
#else
  #error "Unsupported platform."
#endif
}
