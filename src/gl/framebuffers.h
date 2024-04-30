#ifndef FRAMEBUFFERS_H
#define FRAMEBUFFERS_H

#include <glad/glad.h>
#include <glm.hpp>
#include <memory>
#include <vector>

class Framebuffer {
public:
  Framebuffer(unsigned int width, unsigned int height, unsigned int textureCount);
  ~Framebuffer();

  // Disallow copy and assignment
  Framebuffer(const Framebuffer&) = delete;
  Framebuffer& operator=(const Framebuffer&) = delete;

  void bind() const;
  static void unbind();
  void clear(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
  void resize(const glm::vec2& size);
  GLuint getTexture(unsigned int index) const;
  std::vector<GLuint>& getTextures();
  glm::vec2 getTexelSize() const;

private:
  GLuint fbo;
  std::vector<GLuint> textures;
  unsigned int width, height, textureCount;
  glm::vec2 texelSize;

  void setupTextures();
  bool checkFramebufferComplete() const;
};


class ReadWriteFramebuffer {
public:
  ReadWriteFramebuffer(unsigned int width, unsigned int height, unsigned int textureCount);
  ~ReadWriteFramebuffer() = default;

  // Disallow copy and assignment
  ReadWriteFramebuffer(const ReadWriteFramebuffer&) = delete;
  ReadWriteFramebuffer& operator=(const ReadWriteFramebuffer&) = delete;

  void bind() const;
  static void unbind();
  void clear(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
  GLuint getTexture(unsigned int index) const;
  std::vector<GLuint>& getTextures();
  glm::vec2 getTexelSize() const;
  void swap();

private:
  std::unique_ptr<Framebuffer> readFramebuffer;
  std::unique_ptr<Framebuffer> writeFramebuffer;
};

#endif // FRAMEBUFFERS_H
