#include "framebuffer.h"

#include <cstdlib>
#include <iostream>

Framebuffer::Framebuffer(unsigned int width, unsigned int height, unsigned int textureCount)
  : width(width), height(height) {
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  setupTextures(textureCount);

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
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void Framebuffer::unbind() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint Framebuffer::getTexture(unsigned int index) const {
  if (index < textures.size()) {
    return textures[index];
  }
  std::cerr << "Texture index out of range" << std::endl;
  return 0;
}

void Framebuffer::setupTextures(unsigned int textureCount) {
  textures.resize(textureCount);
  glGenTextures(textureCount, textures.data());

  std::vector<GLenum> drawBuffers;
  drawBuffers.reserve(textureCount);

  for (unsigned int i = 0; i < textureCount; ++i) {
    glBindTexture(GL_TEXTURE_2D, textures[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i], 0);

    drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
  }

  if (!drawBuffers.empty()) {
    glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
  }
}

bool Framebuffer::checkFramebufferComplete() const {
  return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}
