#pragma once

#include "Drawable.h"

class VboTorus : public Drawable
{
private:
  int _faces;
  int _rings;
  int _sides;

private:
  void GenerateVerts(float*, float*, float*, unsigned int*, float, float);

public:
  VboTorus(float, float, int, int);
  void Render();
};