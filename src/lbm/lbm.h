#ifndef LBM_H
#define LBM_H

#include <array>
#include <iostream>

#include <glad/glad.h>
#include <glm.hpp>

#include "core/io.h"
#include "core/app_state.h"
#include "gl/framebuffers.h"
#include "gl/shader_program.h"
#include "lbm/fluid.h"
#include "lbm/reaction.h"
#include "lbm/solute.h"

class LBM {
public:
  LBM(const unsigned int width, const unsigned int height);

  // Disallow copy and assignment to avoid multiple deletions of OpenGL objects
  LBM(const LBM&) = delete;
  LBM& operator=(const LBM&) = delete;

  void updateSimulation();
  void updateAnimationPhase();
  void setViscosity(GLfloat viscosity);
  void setSoluteDiffusivity(unsigned int soluteID, GLfloat diffusivity);
  void setSoluteColor(unsigned int soluteID, const glm::vec3& color);
  void setReactionRate(GLfloat rate);
  void resize();
  void resetNodeIDs();
  void resetFluid();
  void resetSolute(unsigned int soluteID);
  void resetAll();
  GLuint getOutputTexture() const;

private:
  // Simulation parameters
  const AppState& appState;
  GLfloat wallAnimationPhase = 0.;

  // LBM data structures
  Fluid fluid;
  std::array<Solute, 3> solutes;
  Reaction reaction;

  // Vertex data
  GLuint vertexArray;
  GLuint vertexBuffer;

  // Frame buffer objects
  std::unique_ptr<Framebuffer> outputFBO;
  std::unique_ptr<ReadWriteFramebuffer> nodeIdFBO;

  // Shader programs
  std::unique_ptr<ShaderProgram> fluidInitShader;
  std::unique_ptr<ShaderProgram> soluteInitShader;
  std::unique_ptr<ShaderProgram> fluidCollisionShader;
  std::unique_ptr<ShaderProgram> soluteCollisionShader;
  std::unique_ptr<ShaderProgram> fluidStreamingShader;
  std::unique_ptr<ShaderProgram> soluteStreamingShader;
  std::unique_ptr<ShaderProgram> reactionShader;
  std::unique_ptr<ShaderProgram> nodeIDShader;
  std::unique_ptr<ShaderProgram> outputShader;

  void createTriangles();
  void createFBOs(const unsigned int width, const unsigned int height);
  void createShaderPrograms();
  void initFluid();
  void initSolute(unsigned int soluteID, glm::vec2 center, GLfloat radius);
  void updateNodeIDs();
  void updateFluid();
  void updateSolute(unsigned int soluteID);
  void react();
};

#endif // LBM_H
