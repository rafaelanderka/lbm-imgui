#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <glad/glad.h>
#include <glm.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

class ShaderProgram {
public:
  ShaderProgram(const fs::path& vertexShaderPath, const fs::path& fragmentShaderPath);
  ~ShaderProgram();

  // Disallow copy and assignment to avoid multiple deletions of OpenGL objects
  ShaderProgram(const ShaderProgram&) = delete;
  ShaderProgram& operator=(const ShaderProgram&) = delete;

  void use();
  void validate(GLuint VAO);

  // Uniform utility methods
  void setUniform(const std::string& name, const glm::vec2& value);
  void setUniform(const std::string& name, const glm::vec3& value);
  void setUniform(const std::string& name, const glm::mat4& value);
  void setUniform(const std::string& name, GLfloat value);
  void setUniform(const std::string& name, GLint value);
  void setTextureUniform(const std::string& name, GLuint textureID);
  void setTextureUniform(const std::string& name, const std::vector<GLuint>& textureIDs);

private:
  unsigned int boundTextureCount = 0;
  GLuint programId;
  std::unordered_map<std::string, GLint> uniformLocations;

  std::string loadShaderCode(const fs::path& shaderPath);
  GLuint compileShader(GLenum type, const std::string& source);
  void linkProgram(GLuint vertexShader, GLuint fragmentShader);
  void cacheUniformLocations();
};

#endif // SHADER_PROGRAM_H
