#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <glad/glad.h>
#include <vector>

class Framebuffer {
public:
  Framebuffer(unsigned int width, unsigned int height, unsigned int textureCount);
  ~Framebuffer();

  Framebuffer(const Framebuffer&) = delete;
  Framebuffer& operator=(const Framebuffer&) = delete;

  void bind() const;
  static void unbind();
  void rescale(unsigned int newWidth, unsigned int newHeight);
  GLuint getTexture(unsigned int index) const;

private:
  GLuint fbo;
  std::vector<GLuint> textures;
  unsigned int width, height;

  void setupTextures(unsigned int textureCount);
  bool checkFramebufferComplete() const;
};

#endif // FRAMEBUFFER_H
