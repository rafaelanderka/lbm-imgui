#ifndef LBM_H
#define LBM_H

#include <iostream>

#include <glad/glad.h>
#include <glm.hpp>
#include "gl/framebuffers.h"
#include "gl/shader_program.h"
#include "io.h"
#include "lbm/fluid.h"
#include "lbm/solute.h"
#include "lbm/reaction.h"

class LBM {
public:
  LBM(const unsigned int width, const unsigned int height);

  // Disallow copy and assignment to avoid multiple deletions of OpenGL objects
  LBM(const LBM&) = delete;
  LBM& operator=(const LBM&) = delete;

  void update();
  void setViscosity(GLfloat viscosity);
  void setSoluteDiffusivity(unsigned int soluteID, GLfloat diffusivity);
  void setSoluteColor(unsigned int soluteID, GLfloat r, GLfloat g, GLfloat b);
  void setReactionRate(GLfloat rate);
  GLuint getOutputTexture() const;

private:
  // LBM data structures
  Fluid fluid;
  std::array<Solute, 3> solutes;
  Reaction reaction;

  // Vertex data
  GLuint vertexArray;
  GLuint vertexBuffer;

  // Frame buffer objects
  std::unique_ptr<ReadWriteFramebuffer> outputFBO;
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

  // Additional state
  glm::vec2 aspectRatio = {1., 1.};
  glm::vec2 cursorPos = {0.5, 0.5};
  glm::vec2 cursorVel = {0.3, 0.};
  GLfloat animationPhase = 0.;
  GLfloat toolSize = 0.1;
  unsigned int overlayType = 2;
  unsigned int selectedSolute = 0;
  unsigned int selectedTool = 0;
  bool isWindowFocussed = true;
  bool isCursorActive = true;
  bool hasVerticalWalls = false;
  bool hasHorizontalWalls = false;
  bool isReactionEnabled = true;

  void createTriangles();
  void initFluid();
  void initSolute(unsigned int soluteID, glm::vec2 center, GLfloat radius);
  void updateNodeIDs();
  void updateFluid();
  void updateSolute(unsigned int soluteID);
  void react();
  void resetNodeIDs();
  void resetFluid();
  void resetSolute(unsigned int soluteID);
};

#endif // LBM_H
