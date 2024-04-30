#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <glm.hpp>

class Window {
public:
  virtual ~Window() = default;

  // Abstract methods
  virtual void render() = 0;
};

#endif // WINDOW_H
