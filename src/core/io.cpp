#include "io.h"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" 

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

ImTextureID loadPNG(const fs::path& imagePath) {
  int width, height, channels;
  unsigned char* imageData = stbi_load(imagePath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
  if (!imageData) {
      std::cerr << "Failed to load image: " << imagePath << std::endl;
      return 0;
  }

  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

  stbi_image_free(imageData);
  return reinterpret_cast<void*>(static_cast<intptr_t>(textureID));
}
