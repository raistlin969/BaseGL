#include "FullscreenQuad.h"
#include "GLSLProgram.h"
#include <glm/glm.hpp>

using glm::vec3;

FullscreenQuad::FullscreenQuad()
{
  GLfloat v[] = 
  {
    -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f
  };

  GLfloat tc[] = 
  {
    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
  };

  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);

  unsigned int vbo[2];
  glGenBuffers(2, vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(GLfloat), v, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(GLfloat), tc, GL_STATIC_DRAW);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);
}

void FullscreenQuad::Render()
{
  glBindVertexArray(_vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}