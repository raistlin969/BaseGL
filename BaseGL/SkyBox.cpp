#include "SkyBox.h"
#include "GLSLProgram.h"
#include <glm/glm.hpp>

using glm::vec3;

SkyBox::SkyBox()
{
  float side = 50.0f;
  float side2 = side / 2.0f;

  float v[24*3] = 
  {
    //front
    -side2, -side2,  side2,
     side2, -side2,  side2,
     side2,  side2,  side2,
    -side2,  side2,  side2,

     //right
    side2, -side2,  side2,
    side2, -side2, -side2,
    side2,  side2, -side2,
    side2,  side2,  side2,
    
    //back
    -side2, -side2, -side2,
    -side2,  side2, -side2,
     side2,  side2, -side2,
     side2, -side2, -side2,

     //left
    -side2, -side2,  side2,
    -side2,  side2,  side2,
    -side2,  side2, -side2,
    -side2, -side2, -side2,
     
    //bottom
    -side2, -side2,  side2,
    -side2, -side2, -side2,
     side2, -side2, -side2,
     side2, -side2,  side2,
     //top
    -side2,  side2,  side2,
     side2,  side2,  side2,
     side2,  side2, -side2,
    -side2,  side2, -side2
  };

  GLuint el[] = 
  {
    0,2,1,0,3,2,
    4,6,5,4,7,6,
    8,10,9,8,11,10,
    12,14,13,12,15,14,
    16,18,17,16,19,18,
    20,22,21,20,23,22
  };

  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);

  unsigned int vbo[2];
  glGenBuffers(2, vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, 24 * 3 * sizeof(float), v, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(GLuint), el, GL_STATIC_DRAW);

  glBindVertexArray(0);
}

void SkyBox::Render()
{
  glBindVertexArray(_vao);
  GLSLProgram* p = _material->Program();
  p->SetUniform("draw_sky_box", true);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
  p->SetUniform("draw_sky_box", false);
}