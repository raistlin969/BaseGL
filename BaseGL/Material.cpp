#include "Material.h"
#include "GLSLProgram.h"
#include <glm/gtc/type_ptr.hpp>

Material::Material()
{
  _ambient = vec3(1.0f, 1.0f, 1.0f);
  _specular = vec3(1.0f, 1.0f, 1.0f);
  _diffuse = vec3(1.0f, 1.0f, 1.0f);
  _shininess = 1.0f;
  _program = 0;
}

Material::Material(GLSLProgram* p)
{
  _ambient = vec3(1.0f, 1.0f, 1.0f);
  _specular = vec3(1.0f, 1.0f, 1.0f);
  _diffuse = vec3(1.0f, 1.0f, 1.0f);
  _shininess = 1.0f;
  _program = p;
}

Material::~Material()
{
}

void Material::Ambient(vec3& v)
{
  _ambient = v;
}

void Material::Ambient(float r, float g, float b)
{
  _ambient = vec3(r, g, b);
}

vec3& Material::Ambient()
{
  return _ambient;
}

void Material::Diffuse(vec3& v)
{
  _diffuse = v;
}

void Material::Diffuse(float r, float g, float b)
{
  _diffuse = vec3(r, g, b);
}

vec3& Material::Diffuse()
{
  return _diffuse;
}

void Material::Specular(vec3& v)
{
  _specular = v;
}

void Material::Specular(float r, float g, float b)
{
  _specular = vec3(r, g, b);
}

vec3& Material::Specular()
{
  return _specular;
}

void Material::Shininess(float s)
{
  _shininess = s;
}

float Material::Shininess()
{
  return _shininess;
}

void Material::Program(GLSLProgram* p)
{
  _program = p;
}

GLSLProgram* Material::Program()
{
  return _program;
}

void Material::Bind()
{
  _program->Use();
  _program->SetUniform("material.kd", _diffuse);
  _program->SetUniform("material.ka", _ambient);
  _program->SetUniform("material.ks", _specular);
  _program->SetUniform("material.shininess", _shininess);
}