#ifndef SOLUTE_H
#define SOLUTE_H

#include "imgui.h"
#include "glm.hpp"
#include "gl/framebuffers.h"

struct Solute {
  Solute(const unsigned int width, const unsigned int height, const GLfloat diffusivity, const glm::vec3& color)
  : fbo(width, height, 3) {
    setDiffusivity(diffusivity);
    setColor(color);
  }
  ~Solute() = default;

  void setDiffusivity(const GLfloat diffusivity) {
    minusOmega =   1. / (3. * diffusivity + 0.5);
    plusOmega = 1. / ((TRT_MAGIC / (1. / minusOmega - 0.5)) + 0.5);
    tau = 1. / minusOmega;
    oneMinusInvTwoTau = (1. - 0.5 / tau);
  }

  void setColor(const glm::vec3& c) {
    color = c;
  }

  static constexpr GLfloat TRT_MAGIC = 1. / 4.;

  ReadWriteFramebuffer fbo;
  GLfloat plusOmega;
  GLfloat minusOmega;
  GLfloat tau;
  GLfloat oneMinusInvTwoTau;
  glm::vec3 color;
};

#endif // SOLUTE_H
