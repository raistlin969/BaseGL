#include "VboTorus.h"
#include "Defines.h"
#include "Material.h"
#include <GL/glew.h>
#include <GL/GL.h>
#include <cmath>
#include <cstdio>

VboTorus::VboTorus(float outer_radius, float inner_radius, int num_sides, int num_rings) : _rings(num_rings), _sides(num_sides)
{
  _faces = _sides * _rings;
  int num_verts = _sides * (_rings+1);

  //verts
  float* v = new float[3 * num_verts];
  //normals
  float* n = new float[3 * num_verts];
  //tex coords
  float* tex = new float[2 * num_verts];
  //elements
  unsigned int* el = new unsigned int[6 * _faces];

  //genertate vertex data
  GenerateVerts(v, n, tex, el, outer_radius, inner_radius);

  //create and populate buffers
  unsigned int handle[4];
  glGenBuffers(4, handle);

  glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
  glBufferData(GL_ARRAY_BUFFER, (3 * num_verts) * sizeof(float), v, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
  glBufferData(GL_ARRAY_BUFFER, (3 * num_verts) * sizeof(float), n, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, handle[2]);
  glBufferData(GL_ARRAY_BUFFER, (2 * num_verts) * sizeof(float), tex, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[3]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, (6 * _faces) * sizeof(unsigned int), el, GL_STATIC_DRAW);

  delete[] v;
  delete[] n;
  delete[] tex;
  delete[] el;

  //create vao
  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
  glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, handle[2]);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[3]);

  glBindVertexArray(0);
}

void VboTorus::Render()
{
  glBindVertexArray(_vao);
  _material->Bind();
  glDrawElements(GL_TRIANGLES, 6 * _faces, GL_UNSIGNED_INT, NULL);
}

void VboTorus::GenerateVerts(float* verts, float* norms, float* tex, unsigned int* el, float outer_radius, float inner_radius)
{
  float ring_factor = (float)(TWOPI / _rings);
  float side_factor = (float)(TWOPI / _sides);
  int idx = 0;
  int tidx = 0;
  for(int ring = 0; ring <= _rings; ++ring)
  {
    float u = ring * ring_factor;
    float cu = cos(u);
    float su = sin(u);
    for(int side = 0; side < _sides; ++side)
    {
      float v = side * side_factor;
      float cv = cos(v);
      float sv = sin(v);
      float r = outer_radius + inner_radius * cv;
      verts[idx] = r * cu;
      verts[idx + 1] = r * su;
      verts[idx + 2] = inner_radius * sv;
      norms[idx] = cv * cu * r;
      norms[idx + 1] = cv * su * r;
      norms[idx + 2] = sv * r;
      tex[tidx] = (float)(u / TWOPI);
      tex[tidx] = (float)(v / TWOPI);
      tidx += 2;
      //normalize
      float len = sqrt(norms[idx] * norms[idx] + norms[idx+1] * norms[idx+1] + norms[idx+2] * norms[idx+2]);
      norms[idx] /= len;
      norms[idx+1] /= len;
      norms[idx+2] /= len;
      idx += 3;
    }
  }
  idx = 0;
  for(int ring = 0; ring < _rings; ++ring)
  {
    int ring_start = ring * _sides;
    int next_ring = (ring + 1) * _sides;
    for(int side = 0; side < _sides; ++side)
    {
      int next_side = (side+1) % _sides;
      //the quad
      el[idx] = (ring_start + side);
      el[idx+1] = next_ring + side;
      el[idx+2] = next_ring + next_side;
      el[idx+3] = ring_start + side;
      el[idx+4] = next_ring + next_side;
      el[idx+5] = ring_start + next_side;
      idx += 6;
    }
  }
}