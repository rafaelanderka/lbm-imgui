#include "framebuffers.h"

#include <cstdlib>
#include <iostream>

Framebuffer::Framebuffer(unsigned int width, unsigned int height, unsigned int textureCount)
  : width(width), height(height), textureCount(textureCount), texelSize{1.f / width, 1.f / height} {
  glGenFramebuffers(1, &fbo);

  setupTextures();

  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  if (!checkFramebufferComplete()) {
    std::cerr << "Framebuffer not complete!" << std::endl;
    exit(1);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::~Framebuffer() {
  glDeleteTextures(static_cast<GLsizei>(textures.size()), textures.data());
  glDeleteFramebuffers(1, &fbo);
}

void Framebuffer::bind() const {
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glViewport(0, 0, width, height);
}

void Framebuffer::unbind() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::clear() {
  bind();
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  unbind();
}

void Framebuffer::resize(const glm::vec2& size) {
  // Update state
  width = size.x;
  height = size.y;
  texelSize = 1.f / size;

  // Delete old textures
  glDeleteTextures(textures.size(), textures.data());

  // Clear the textures vector to repopulate it
  textures.clear();

  // Re-setup textures with the new dimensions
  setupTextures();
}

GLuint Framebuffer::getTexture(unsigned int index) const {
  if (index < textures.size()) {
    return textures[index];
  }
  std::cerr << "Texture index out of range" << std::endl;
  return 0;
}

std::vector<GLuint>& Framebuffer::getTextures() {
  return textures;
}

glm::vec2 Framebuffer::getTexelSize() const {
  return texelSize;
}

void Framebuffer::setupTextures() {
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  textures.resize(textureCount);
  glGenTextures(textureCount, textures.data());

  std::vector<GLenum> drawBuffers;
  drawBuffers.reserve(textureCount);

  for (unsigned int i = 0; i < textureCount; ++i) {
    glBindTexture(GL_TEXTURE_2D, textures[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i], 0);

    drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
  }

  if (!drawBuffers.empty()) {
    glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool Framebuffer::checkFramebufferComplete() const {
  return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

ReadWriteFramebuffer::ReadWriteFramebuffer(unsigned int width, unsigned int height, unsigned int textureCount)
: readFramebuffer(std::make_unique<Framebuffer>(width, height, textureCount)),
  writeFramebuffer(std::make_unique<Framebuffer>(width, height, textureCount)) {}

void ReadWriteFramebuffer::bind() const {
  writeFramebuffer->bind();
}

void ReadWriteFramebuffer::unbind() {
  Framebuffer::unbind();
}

void ReadWriteFramebuffer::clear() {
  readFramebuffer->clear();
  writeFramebuffer->clear();
}

GLuint ReadWriteFramebuffer::getTexture(unsigned int index) const {
  return readFramebuffer->getTexture(index);
}

std::vector<GLuint>& ReadWriteFramebuffer::getTextures() {
  return readFramebuffer->getTextures();
}

glm::vec2 ReadWriteFramebuffer::getTexelSize() const {
  return readFramebuffer->getTexelSize();
}

void ReadWriteFramebuffer::swap() {
  std::swap(readFramebuffer, writeFramebuffer);
}
