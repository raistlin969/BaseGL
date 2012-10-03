#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>

using glm::vec4;

class GLSLProgram;

struct Point
{
  float x;
  float y;
  float s;
  float t;
};

class Text
{
protected:
  unsigned int _vao;
  FT_Library _ft;
  FT_Face _face;
  GLSLProgram* _program;

public:
  Text(GLSLProgram* p);
  virtual ~Text();
  void Render(const char* text, float x, float y, float sx, float sy, vec4& color);
};