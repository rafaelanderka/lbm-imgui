#include <cmath>
#include "lbm.h"

LBM::LBM(const unsigned int width, const unsigned int height) :
  appState(AppState::getInstance()),
  fluid(width, height, appState.fluidViscosity),
  solutes{Solute(width, height, appState.soluteDiffusivities[0], appState.soluteColors[0]),
          Solute(width, height, appState.soluteDiffusivities[1], appState.soluteColors[1]),
          Solute(width, height, appState.soluteDiffusivities[2], appState.soluteColors[2])},
  reaction(width, height, REACTION_MOLAR_MASSES, REACTION_STOICHIOMETRIC_COEFFS, appState.reactionRate)
{
  createTriangles();

  outputFBO = std::make_unique<Framebuffer>(width, height, 1);
  nodeIdFBO = std::make_unique<ReadWriteFramebuffer>(width, height, 1);

  fs::path executablePath = getExecutablePath();
  fs::path shadersDir = executablePath.parent_path() / "shaders";
  fs::path vertexShaderPath = shadersDir / "vs_base.glsl";

  fs::path fluidInitShaderPath = shadersDir / "fs_fluid_init.glsl";
  fluidInitShader = std::make_unique<ShaderProgram>(vertexShaderPath, fluidInitShaderPath);
  fluidInitShader->validate(vertexArray);

  fs::path soluteInitShaderPath = shadersDir / "fs_solute_init.glsl";
  soluteInitShader = std::make_unique<ShaderProgram>(vertexShaderPath, soluteInitShaderPath);
  soluteInitShader->validate(vertexArray);

  fs::path fluidCollisionShaderPath = shadersDir / "fs_fluid_collision.glsl";
  fluidCollisionShader = std::make_unique<ShaderProgram>(vertexShaderPath, fluidCollisionShaderPath);
  fluidCollisionShader->validate(vertexArray);

  fs::path soluteCollisionShaderPath = shadersDir / "fs_solute_collision.glsl";
  soluteCollisionShader = std::make_unique<ShaderProgram>(vertexShaderPath, soluteCollisionShaderPath);
  soluteCollisionShader->validate(vertexArray);

  fs::path fluidStreamingShaderPath = shadersDir / "fs_fluid_streaming.glsl";
  fluidStreamingShader = std::make_unique<ShaderProgram>(vertexShaderPath, fluidStreamingShaderPath);
  fluidStreamingShader->validate(vertexArray);

  fs::path soluteStreamingShaderPath = shadersDir / "fs_solute_streaming.glsl";
  soluteStreamingShader = std::make_unique<ShaderProgram>(vertexShaderPath, soluteStreamingShaderPath);
  soluteStreamingShader->validate(vertexArray);

  fs::path reactionShaderPath = shadersDir / "fs_reaction.glsl";
  reactionShader = std::make_unique<ShaderProgram>(vertexShaderPath, reactionShaderPath);
  reactionShader->validate(vertexArray);

  fs::path nodeIDShaderPath = shadersDir / "fs_nodeid_update.glsl";
  nodeIDShader = std::make_unique<ShaderProgram>(vertexShaderPath, nodeIDShaderPath);
  nodeIDShader->validate(vertexArray);

  fs::path outputShaderPath = shadersDir / "fs_output.glsl";
  outputShader = std::make_unique<ShaderProgram>(vertexShaderPath, outputShaderPath);
  outputShader->validate(vertexArray);

  initFluid();
  initSolute(0, INIT_SOLUTE_CENTER_0, INIT_SOLUTE_RADIUS_0);
  initSolute(1, INIT_SOLUTE_CENTER_1, INIT_SOLUTE_RADIUS_1);
  initSolute(2, INIT_SOLUTE_CENTER_2, INIT_SOLUTE_RADIUS_2);
}

void LBM::setViscosity(GLfloat viscosity) {
  fluid.setViscosity(viscosity);
}

void LBM::setSoluteDiffusivity(unsigned int soluteID, GLfloat diffusivity) {
  solutes[soluteID].setDiffusivity(diffusivity);
}

void LBM::setSoluteColor(unsigned int soluteID, const glm::vec3& color) {
  solutes[soluteID].setColor(color);
}

void LBM::setReactionRate(GLfloat rate) {
  reaction.setReactionRate(rate);
}

void LBM::updateSimulation() {
  // Perform all simulation updates in turn
  updateNodeIDs();
  updateFluid();
  react();
  for (int i = 0; i < solutes.size(); i++) {
    updateSolute(i);
  }

  // Render output image
  outputFBO->bind();
  outputShader->use();
  outputShader->setTextureUniform("uNodeIds", nodeIdFBO->getTexture(0));
  outputShader->setTextureUniform("uFluidData", fluid.fbo.getTexture(0));
  outputShader->setTextureUniform("uSolute0Data", solutes[0].fbo.getTexture(0));
  outputShader->setTextureUniform("uSolute1Data", solutes[1].fbo.getTexture(0));
  outputShader->setTextureUniform("uSolute2Data", solutes[2].fbo.getTexture(0));
  outputShader->setUniform("uSolute0Col", solutes[0].color);
  outputShader->setUniform("uSolute1Col", solutes[1].color);
  outputShader->setUniform("uSolute2Col", solutes[2].color);
  outputShader->setUniform("uAspect", appState.aspectRatio);
  outputShader->setUniform("uCursorPos", appState.cursorPos);
  outputShader->setUniform("uAnimationPhase", wallAnimationPhase);
  outputShader->setUniform("uToolSize", TOOL_SIZE_MULTIPLIER * appState.toolSize);
  outputShader->setUniform("uViewportSize", appState.viewportSize);
  outputShader->setUniform("uViewportScale", appState.viewportScale);
  outputShader->setUniform("uDrawIndicatorLines", appState.activeOverlay == OverlayType::Lines);
  outputShader->setUniform("uDrawIndicatorArrows", appState.activeOverlay == OverlayType::Arrows);
  outputShader->setUniform("uDrawCursor", appState.isSimulationFocussed);
  glBindVertexArray(vertexArray);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
  glUseProgram(0);
  outputFBO->unbind();
}

void LBM::updateAnimationPhase() {
  wallAnimationPhase = fmod(wallAnimationPhase - 0.1, 2 * M_PI);
}

GLuint LBM::getOutputTexture() const {
  return outputFBO->getTexture(0);
  // return nodeIdFBO->getTexture(0);
}

void LBM::resize() {
  outputFBO->resize(appState.viewportSize);
}

void LBM::createTriangles() {
  GLfloat vertices[] = {
      -1.0f, -1.0f, 0.0f,  // Bottom Left
      3.0f, -1.0f, 0.0f,   // Far right, beyond viewport
      -1.0f,  3.0f, 0.0f   // Far top, beyond viewport
  };

  // Generate and bind the VAO
  glGenVertexArrays(1, &vertexArray);
  glBindVertexArray(vertexArray);

  // Generate and bind the VBO
  glGenBuffers(1, &vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Set the vertex attributes pointers
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
  glEnableVertexAttribArray(0);

  // Unbind the VBO and VAO to make sure they're not accidentally modified
  glBindBuffer(GL_ARRAY_BUFFER, 0); 
  glBindVertexArray(0);
}

void LBM::initFluid() {
  fluid.fbo.bind();
  fluidInitShader->use();
  fluidInitShader->setTextureUniform("uNodeIds", nodeIdFBO->getTexture(0));
  fluidInitShader->setTextureUniform("uFluidData", fluid.fbo.getTextures());
  fluidInitShader->setUniform("uInitVelocity", INIT_FLUID_VELOCITY);
  fluidInitShader->setUniform("uInitDensity", INIT_FLUID_DENSITY);
  fluidInitShader->setUniform("uTau", fluid.tau);
  glBindVertexArray(vertexArray);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
  glUseProgram(0);
  fluid.fbo.unbind();
  fluid.fbo.swap();
}

void LBM::initSolute(unsigned int soluteID, glm::vec2 center, GLfloat radius) {
  solutes[soluteID].fbo.bind();
  soluteInitShader->use();
  soluteInitShader->setTextureUniform("uNodeIds", nodeIdFBO->getTexture(0));
  soluteInitShader->setTextureUniform("uFluidData", fluid.fbo.getTextures());
  soluteInitShader->setTextureUniform("uSoluteData", solutes[soluteID].fbo.getTextures());
  soluteInitShader->setUniform("uCenter", center);
  soluteInitShader->setUniform("uAspect", appState.aspectRatio);
  soluteInitShader->setUniform("uInitDensity", INIT_FLUID_DENSITY);
  soluteInitShader->setUniform("uTau", solutes[soluteID].tau);
  soluteInitShader->setUniform("uRadius", radius);
  glBindVertexArray(vertexArray);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
  glUseProgram(0);
  solutes[soluteID].fbo.unbind();
  solutes[soluteID].fbo.swap();
}

void LBM::updateNodeIDs() {
  bool isAddingWalls = appState.isSimulationFocussed && appState.isCursorActive && (appState.activeTool == ToolType::AddWall);
  bool isRemovingWalls = appState.isSimulationFocussed && appState.isCursorActive && (appState.activeTool == ToolType::RemoveWall);

  nodeIdFBO->bind();
  nodeIDShader->use();
  nodeIDShader->setTextureUniform("uNodeIds", nodeIdFBO->getTexture(0));
  nodeIDShader->setUniform("uIsAddingWalls", isAddingWalls);
  nodeIDShader->setUniform("uIsRemovingWalls", isRemovingWalls);
  nodeIDShader->setUniform("uHasVerticalWalls", appState.hasVerticalWalls);
  nodeIDShader->setUniform("uHasHorizontalWalls", appState.hasHorizontalWalls);
  nodeIDShader->setUniform("uToolSize", TOOL_SIZE_MULTIPLIER * appState.toolSize);
  nodeIDShader->setUniform("uCursorPos", appState.cursorPos);
  nodeIDShader->setUniform("uAspect", appState.aspectRatio);
  nodeIDShader->setUniform("uTexelSize", nodeIdFBO->getTexelSize());
  glBindVertexArray(vertexArray);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
  glUseProgram(0);
  nodeIdFBO->unbind();
  nodeIdFBO->swap();
}

void LBM::updateFluid() {
  bool isApplyingForce = appState.isSimulationFocussed && appState.isCursorActive && (appState.activeTool == ToolType::Force);

  // Perform TRT collision
  fluid.fbo.bind();
  fluidCollisionShader->use();
  fluidCollisionShader->setTextureUniform("uNodeIds", nodeIdFBO->getTexture(0));
  fluidCollisionShader->setTextureUniform("uFluidData", fluid.fbo.getTextures());
  fluidCollisionShader->setUniform("uCursorPos", appState.cursorPos);
  fluidCollisionShader->setUniform("uCursorVel", appState.cursorVel);
  fluidCollisionShader->setUniform("uAspect", appState.aspectRatio);
  fluidCollisionShader->setUniform("uToolSize", TOOL_SIZE_MULTIPLIER * appState.toolSize);
  fluidCollisionShader->setUniform("uInitDensity", INIT_FLUID_DENSITY);
  fluidCollisionShader->setUniform("uPlusOmega", fluid.plusOmega);
  fluidCollisionShader->setUniform("uMinusOmega", fluid.minusOmega);
  fluidCollisionShader->setUniform("uIsApplyingForce", isApplyingForce);
  glBindVertexArray(vertexArray);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
  glUseProgram(0);
  fluid.fbo.unbind();
  fluid.fbo.swap();

  // Perform streaming
  fluid.fbo.bind();
  fluidStreamingShader->use();
  fluidStreamingShader->setTextureUniform("uNodeIds", nodeIdFBO->getTexture(0));
  fluidStreamingShader->setTextureUniform("uFluidData", fluid.fbo.getTextures());
  fluidStreamingShader->setUniform("uTexelSize", fluid.fbo.getTexelSize());
  fluidStreamingShader->setUniform("uInitDensity", INIT_FLUID_DENSITY);
  fluidStreamingShader->setUniform("uSpeedOfSound", SPEED_OF_SOUND);
  glBindVertexArray(vertexArray);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
  glUseProgram(0);
  fluid.fbo.unbind();
  fluid.fbo.swap();
}

void LBM::updateSolute(unsigned int soluteID) {
  bool isSoluteSelected = appState.activeSolute == soluteID;
  bool isAddingConcentration = appState.isSimulationFocussed && appState.isCursorActive && isSoluteSelected && (appState.activeTool == ToolType::AddSolute);
  bool isRemovingConcentration = appState.isSimulationFocussed && appState.isCursorActive && isSoluteSelected && (appState.activeTool == ToolType::RemoveSolute);
  GLfloat concentrationSourcePolarity = (isAddingConcentration ? 1.f : 0.f) - (isRemovingConcentration ? 1.f : 0.f);

  // Perform TRT collision
  solutes[soluteID].fbo.bind();
  soluteCollisionShader->use();
  soluteCollisionShader->setTextureUniform("uNodeIds", nodeIdFBO->getTexture(0));
  soluteCollisionShader->setTextureUniform("uFluidData", fluid.fbo.getTextures());
  soluteCollisionShader->setTextureUniform("uSoluteData", solutes[soluteID].fbo.getTextures());
  soluteCollisionShader->setTextureUniform("uNodalReactionRate", reaction.fbo.getTexture(0));
  soluteCollisionShader->setUniform("uCursorPos", appState.cursorPos);
  soluteCollisionShader->setUniform("uAspect", appState.aspectRatio);
  soluteCollisionShader->setUniform("uToolSize", TOOL_SIZE_MULTIPLIER * appState.toolSize);
  soluteCollisionShader->setUniform("uConcentrationSourcePolarity", concentrationSourcePolarity);
  soluteCollisionShader->setUniform("uInitDensity", INIT_FLUID_DENSITY);
  soluteCollisionShader->setUniform("uInitConcentration", INIT_SOLUTE_CONCENTRATION);
  soluteCollisionShader->setUniform("uPlusOmega", solutes[soluteID].plusOmega);
  soluteCollisionShader->setUniform("uMinusOmega", solutes[soluteID].minusOmega);
  soluteCollisionShader->setUniform("uOneMinusInvTwoTau", solutes[soluteID].oneMinusInvTwoTau);
  soluteCollisionShader->setUniform("uMolMassTimesCoeff", reaction.molMassTimesCoeffs[soluteID]);
  glBindVertexArray(vertexArray);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
  glUseProgram(0);
  solutes[soluteID].fbo.unbind();
  solutes[soluteID].fbo.swap();

  // Perform streaming
  solutes[soluteID].fbo.bind();
  soluteStreamingShader->use();
  soluteStreamingShader->setTextureUniform("uNodeIds", nodeIdFBO->getTexture(0));
  soluteStreamingShader->setTextureUniform("uSoluteData", solutes[soluteID].fbo.getTextures());
  soluteStreamingShader->setUniform("uTexelSize", solutes[soluteID].fbo.getTexelSize());
  soluteStreamingShader->setUniform("uInitConcentration", INIT_SOLUTE_CONCENTRATION);
  glBindVertexArray(vertexArray);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
  glUseProgram(0);
  solutes[soluteID].fbo.unbind();
  solutes[soluteID].fbo.swap();
}

void LBM::react() {
  reaction.fbo.bind();
  reactionShader->use();
  reactionShader->setTextureUniform("uNodalReactionRate", reaction.fbo.getTexture(0));
  reactionShader->setTextureUniform("uSolute0Data", solutes[0].fbo.getTexture(0));
  reactionShader->setTextureUniform("uSolute1Data", solutes[1].fbo.getTexture(0));
  reactionShader->setTextureUniform("uSolute2Data", solutes[2].fbo.getTexture(0));
  reactionShader->setUniform("uReactionRate", appState.isReactionEnabled ? reaction.reactionRate : 0.f);
  reactionShader->setUniform("uStoichiometricCoeff0", reaction.stoichiometricCoeffs[0]);
  reactionShader->setUniform("uStoichiometricCoeff1", reaction.stoichiometricCoeffs[1]);
  reactionShader->setUniform("uStoichiometricCoeff2", reaction.stoichiometricCoeffs[2]);
  glBindVertexArray(vertexArray);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
  glUseProgram(0);
  reaction.fbo.unbind();
  reaction.fbo.swap();
}

void LBM::resetNodeIDs() {
  nodeIdFBO->clear(0.0, 0.0, 0.0, 0.0);
}

void LBM::resetFluid() {
  fluid.fbo.clear(0.0, 0.0, 0.0, 0.0);
  initFluid();
}

void LBM::resetSolute(unsigned int soluteID) {
  initSolute(soluteID, {0, 0}, 0);
}

void LBM::resetAll() {
  // Reset everything
  resetNodeIDs();
  setViscosity(appState.fluidViscosity);
  setReactionRate(appState.reactionRate);
  resetFluid();
  for (int i = 0; i < 3; i++) {
    setSoluteDiffusivity(i, appState.soluteDiffusivities[i]);
    setSoluteColor(i, appState.soluteColors[i]);
    resetSolute(i);
  }

  // Re-initialise everything
  initFluid();
  initSolute(0, INIT_SOLUTE_CENTER_0, INIT_SOLUTE_RADIUS_0);
  initSolute(1, INIT_SOLUTE_CENTER_1, INIT_SOLUTE_RADIUS_1);
  initSolute(2, INIT_SOLUTE_CENTER_2, INIT_SOLUTE_RADIUS_2);
}
