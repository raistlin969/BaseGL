#pragma once

#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

using std::string;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

namespace GLSLShader
{
  enum GLSLShaderType
  {
    VERTEX,
    FRAGMENT,
    GEOMETRY,
    TESS_CONTROL,
    TESS_EVALUATION
  };
};

class GLSLProgram
{
private:
  int _handle;
  bool _linked;
  string _log_string;
  
private:
  int GetUniformLocation(const char* name);
  bool FileExists(const char* file_name);

public:
  GLSLProgram();
  bool CompileShaderFromFile(const char* file_name, GLSLShader::GLSLShaderType type);
  bool CompileShaderFromString(const string& source, GLSLShader::GLSLShaderType type);
  bool CompileAndLinkShaders(const string& v, const string& f, const string& g = "", const string& tc = "", const string& te = "");
  bool Link();
  void Use();
  string Log();
  int Handle();
  bool IsLinked();
  void BindAttribLocation(GLuint location, const char* name);
  void BindFragDataLocation(GLuint location, const char* name);
  void SetUniform(const char* name, float x, float y, float z);
  void SetUniform(const char* name, const vec3& v);
  void SetUniform(const char* name, const vec4& v);
  void SetUniform(const char* name, const mat4& m);
  void SetUniform(const char* name, const mat3& m);
  void SetUniform(const char* name, float value);
  void SetUniform(const char* name, int value);
  void SetUniform(const char* name, bool value);
  void PrintActiveUniforms();
  void PrintActiveAttribs();
};
