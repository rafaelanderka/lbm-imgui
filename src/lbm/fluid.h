#ifndef FLUID_H
#define FLUID_H

#include "gl/framebuffers.h"


struct Fluid {
  Fluid(const unsigned int width, const unsigned int height, const GLfloat viscosity)
  : fbo(width, height, 4) {
    setViscosity(viscosity);
  }
  ~Fluid() = default;

  void setViscosity(const GLfloat viscosity) {
    plusOmega = 1.0 / (3.0 * viscosity + 0.5);
    minusOmega = 1.0 / ((TRT_MAGIC / (1.0 / plusOmega - 0.5)) + 0.5);
    tau = 1.0 / plusOmega;
  }
  
  static constexpr GLfloat TRT_MAGIC = 1. / 4.;

  ReadWriteFramebuffer fbo;
  GLfloat plusOmega;
  GLfloat minusOmega;
  GLfloat tau;
};

#endif // FLUID_H
