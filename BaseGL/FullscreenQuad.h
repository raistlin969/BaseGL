#pragma once

#include "Drawable.h"
#include <GL/glew.h>
#include <GL/GL.h>

class FullscreenQuad : public Drawable
{
public:
  FullscreenQuad();
  virtual void Render();
};