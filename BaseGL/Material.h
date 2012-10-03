#pragma once

#include <glm/glm.hpp>

using glm::vec3;

class GLSLProgram;

class Material
{
protected:
  vec3 _ambient;
  vec3 _diffuse;
  vec3 _specular;
  float _shininess;
  GLSLProgram* _program;

public:
  Material();
  Material(GLSLProgram* p);
  virtual ~Material();
  
  void Ambient(vec3& a);
  void Ambient(float r, float g, float b);
  vec3& Ambient();

  void Diffuse(vec3& a);
  void Diffuse(float r, float g, float b);
  vec3& Diffuse();

  void Specular(vec3& a);
  void Specular(float r, float g, float b);
  vec3& Specular();

  void Shininess(float s);
  float Shininess();

  void Bind();
  void Program(GLSLProgram* p);
  GLSLProgram* Program();
};