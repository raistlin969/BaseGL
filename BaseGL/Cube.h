#pragma once

#include "Drawable.h"
#include <GL/glew.h>
#include <GL/GL.h>

class Cube : public Drawable
{
public:
  Cube();
  virtual void Render();
};