#ifndef REACTION_H
#define REACTION_H

#include <vector>
#include "imgui.h"
#include "glm.hpp"
#include "gl/framebuffers.h"

struct Reaction {
  Reaction(const unsigned int width, const unsigned int height,
           const std::vector<GLfloat>& molarMasses,
           const std::vector<GLint>& stoichiometricCoeffs,
           const GLfloat reactionRate)
  : fbo(width, height, 1), stoichiometricCoeffs(stoichiometricCoeffs),
    reactionRate(reactionRate)
  {
    // Store premultiplied coeffs.
    molMassTimesCoeffs.reserve(molarMasses.size());
    for (int i = 0; i < molarMasses.size(); i++) {
      molMassTimesCoeffs.push_back(molarMasses[i] * stoichiometricCoeffs[i]);
    }
  }
  ~Reaction() = default;

  void setReactionRate(const GLfloat r) {
    reactionRate = r;
  }

  ReadWriteFramebuffer fbo;
  GLfloat reactionRate;
  std::vector<GLfloat> molMassTimesCoeffs;
  std::vector<GLint> stoichiometricCoeffs;
};

#endif // REACTION_H
