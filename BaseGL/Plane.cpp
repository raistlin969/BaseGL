#include "Plane.h"
#include "Defines.h"
#include "Material.h"
#include <GL/glew.h>
#include <GL/GL.h>
#include <math.h>

Plane::Plane(float xsize, float zsize, int xdivs, int zdivs)
{
  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);
  _faces = xdivs * zdivs;
  float* v = new float[3 * (xdivs + 1) * (zdivs + 1)];
  float* tex = new float[2 * (xdivs + 1) * (zdivs + 1)];
  unsigned int* el = new unsigned int[6 * xdivs * zdivs];

  float x2 = xsize / 2.0f;
  float z2 = zsize / 2.0f;
  float ifactor = (float)zsize / zdivs;
  float jfactor = (float)xsize / xdivs;
  float texi = 1.0f / zdivs;
  float texj = 1.0f / xdivs;
  float x, z;
  int vidx = 0, tidx = 0;
  
  for(int i = 0; i <= zdivs; ++i)
  {
    z = ifactor * i - z2;
    for(int j = 0; j <= xdivs; ++j)
    {
      x = jfactor * j - x2;
      v[vidx] = x;
      v[vidx+1] = 0.0f;
      v[vidx+2] = z;
      vidx += 3;
      tex[tidx] = i * texi;
      tex[tidx+1] = j * texj;
      tidx += 2;
    }
  }

  unsigned int row_start, next_row_start;
  int idx = 0;
  for(int i = 0; i < zdivs; ++i)
  {
    row_start = i * (xdivs+1);
    next_row_start = (i+1) * (xdivs+1);
    for(int j = 0; j < xdivs; ++j)
    {
      el[idx] = row_start + j;
      el[idx+1] = next_row_start + j;
      el[idx+2] = next_row_start + j + 1;
      el[idx+3] = row_start + j;
      el[idx+4] = next_row_start + j + 1;
      el[idx+5] = row_start + j + 1;
      idx += 6;
    }
  }

  unsigned int vbo[3];
  glGenBuffers(3, vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, 3 * (xdivs+1) * (zdivs+1) * sizeof(float), v, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, 2 * (xdivs+1) * (zdivs+1) * sizeof(float), tex, GL_STATIC_DRAW);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * xdivs * zdivs * sizeof(unsigned int), el, GL_STATIC_DRAW);

  delete[] v;
  delete[] tex;
  delete[] el;
}

void Plane::Render()
{
  glBindVertexArray(_vao);
  _material->Bind();
  glVertexAttrib3f(1, 0.0f, 1.0f, 0.0f);  //constant normal for all verts
  glDrawElements(GL_TRIANGLES, 6 * _faces, GL_UNSIGNED_INT, 0);
}