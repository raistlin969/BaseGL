#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "Drawable.h"

using glm::vec3;
using glm::vec2;
using glm::vec4;
using std::string;
using std::vector;

class Mesh : public Drawable
{
protected:
  unsigned int _faces;
  
  void TrimString(string& s);
  void StoreVbo(const vector<vec3>& points, const vector<vec3>& normals, const vector<vec2>& tex_coords, const vector<vec4>& tangents, const vector<int>& elements);
  void GenerateAveragedNormals(const vector<vec3>& points, vector<vec3>& normals, const vector<int>& faces);
  void GenerateTangents(const vector<vec3>& points, const vector<vec3>& normals, const vector<int>& faces, const vector<vec2>& tex_coords, vector<vec4>& tangents);
  void Center(vector<vec3>&);

public:
  Mesh(const char* filename, bool recenter = false);
  virtual void Render();
  void LoadObj(const char* filename, bool);
};