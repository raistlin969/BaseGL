#pragma once

#include "Drawable.h"

class Plane : public Drawable
{
protected:
  int _faces;

public:
  Plane(float, float, int, int);
  virtual void Render();
};