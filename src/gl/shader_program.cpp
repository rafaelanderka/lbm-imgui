#include "shader_program.h"

#include <iostream>
#include <vector>

ShaderProgram::ShaderProgram(const fs::path& vertexShaderPath, const fs::path& fragmentShaderPath) {
  programId = glCreateProgram();
  if(!programId) {
    std::cout << "Error creating shader program!\n"; 
    exit(1);
  }

  // Create shader program
  std::string vertexShaderSource = loadShaderCode(vertexShaderPath);
  std::string fragmentShaderSource = loadShaderCode(fragmentShaderPath);
  GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
  GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
  linkProgram(vertexShader, fragmentShader);

  // Delete shaders after linking
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  cacheUniformLocations();
}

ShaderProgram::~ShaderProgram() {
  glDeleteProgram(programId);
}

std::string ShaderProgram::loadShaderCode(const fs::path& shaderPath) {
  std::ifstream shaderFile(shaderPath);
  if (!shaderFile.is_open()) {
    std::cerr << "Failed to open shader file: " << shaderPath << std::endl;
    return "";
  }
  std::stringstream buffer;
  buffer << shaderFile.rdbuf();
  return buffer.str();
}

GLuint ShaderProgram::compileShader(GLenum type, const std::string& source) {
  GLuint shader = glCreateShader(type);
  const char* sourceCStr = source.c_str();
  glShaderSource(shader, 1, &sourceCStr, nullptr);
  glCompileShader(shader);

  // Check for compilation errors
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLchar infoLog[512];
    glGetShaderInfoLog(shader, 512, nullptr, infoLog);
    std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  return shader;
}

void ShaderProgram::linkProgram(GLuint vertexShader, GLuint fragmentShader) {
  glAttachShader(programId, vertexShader);
  glAttachShader(programId, fragmentShader);
  glLinkProgram(programId);

  // Check for linking errors
  int success;
  glGetProgramiv(programId, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(programId, 512, nullptr, infoLog);
    std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
  }

  // Detach shaders after linking
  glDetachShader(programId, vertexShader);
  glDetachShader(programId, fragmentShader);
}

void ShaderProgram::validate(GLuint VAO) {
  // Validate shader program
  int success;
  glBindVertexArray(VAO);
  glValidateProgram(programId);
  glBindVertexArray(0);
  glGetProgramiv(programId, GL_VALIDATE_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(programId, 512, nullptr, infoLog);
    std::cerr << "ERROR::PROGRAM::VALIDATION_FAILED\n" << infoLog << std::endl;
    return;
  }
}

void ShaderProgram::cacheUniformLocations() {
  GLint numUniforms = 0;
  glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &numUniforms);

  std::vector<GLchar> uniformNameData(256); // Adjust size as needed
  GLsizei nameLength;
  GLint size;
  GLenum type;

  for (GLint i = 0; i < numUniforms; ++i) {
    glGetActiveUniform(programId, i, uniformNameData.size(), &nameLength, &size, &type, uniformNameData.data());
    std::string name(uniformNameData.begin(), uniformNameData.begin() + nameLength);

    // Check and trim [0] for array uniforms
    if (name.ends_with("[0]")) {
      name = name.substr(0, name.length() - 3);
    }

    GLint location = glGetUniformLocation(programId, name.c_str());
    uniformLocations[name] = location;
  }
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec2& value) {
  glUniform2fv(uniformLocations[name], 1, &value[0]);
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec3& value) {
  glUniform3fv(uniformLocations[name], 1, &value[0]);
}

void ShaderProgram::setUniform(const std::string& name, const glm::mat4& value) {
  glUniformMatrix4fv(uniformLocations[name], 1, GL_FALSE, &value[0][0]);
}

void ShaderProgram::setUniform(const std::string& name, GLfloat value) {
  glUniform1f(uniformLocations[name], value);
}

void ShaderProgram::setUniform(const std::string& name, GLint value) {
  glUniform1i(uniformLocations[name], value);
}

void ShaderProgram::setTextureUniform(const std::string& name, GLuint textureID) {
  glActiveTexture(GL_TEXTURE0 + boundTextureCount); // Activate texture unit i
  glBindTexture(GL_TEXTURE_2D, textureID);
  glUniform1i(uniformLocations[name], boundTextureCount);
  boundTextureCount++;
}

void ShaderProgram::setTextureUniform(const std::string& name, const std::vector<GLuint>& textureIDs) {
  // Array to store texture units corresponding to each texture
  std::vector<GLint> textureUnits(textureIDs.size());

  for (size_t i = 0; i < textureIDs.size(); ++i) {
    glActiveTexture(GL_TEXTURE0 + boundTextureCount); // Activate the next texture unit
    glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
    textureUnits[i] = boundTextureCount;
    boundTextureCount++;
  }

  // Set the uniform to the texture units. This tells the shader where to find each texture.
  glUniform1iv(uniformLocations[name], textureIDs.size(), &textureUnits[0]);
}

void ShaderProgram::use() {
  glUseProgram(programId);
  boundTextureCount = 0;
}
