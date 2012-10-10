#pragma once

#include "Drawable.h"
#include <GL/glew.h>
#include <GL/GL.h>

class SkyBox : public Drawable
{
public:
  SkyBox();
  virtual void Render();
};