#include "GLSLProgram.h"
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>
#include <boost/log/trivial.hpp>
#include "Logger.h"

using std::ifstream;
using std::ios;
using std::ostringstream;

GLSLProgram::GLSLProgram()
{
  _handle = 0;
  _linked = false;
  _log_string = "";
}

int GLSLProgram::GetUniformLocation(const char* name)
{
  return glGetUniformLocation(_handle, name);
}

bool GLSLProgram::FileExists(const char* file_name)
{
  struct stat info;
  int ret = -1;

  ret = stat(file_name, &info);
  return 0 == ret;
}

bool GLSLProgram::CompileShaderFromFile(const char* file_name, GLSLShader::GLSLShaderType type)
{
  if(!FileExists(file_name))
  {
    _log_string = "File not found";
    return false;
  }
  if(_handle <= 0)
  {
    _handle = glCreateProgram();
    if(_handle == 0)
    {
      _log_string = "Unable to create shader program";
      return false;
    }
  }
  ifstream in(file_name, ios::in);
  if(!in)
    return false;

  ostringstream code;
  while(in.good())
  {
    int c = in.get();
    if(!in.eof())
      code << (char) c;
  }
  in.close();
  return CompileShaderFromString(code.str(), type);
}

bool GLSLProgram::CompileShaderFromString(const string& source, GLSLShader::GLSLShaderType type)
{
  if(_handle <= 0)
  {
    _handle = glCreateProgram();
    if(_handle == 0)
    {
      _log_string = "Unable to create shader program";
      return false;
    }
  }

  GLuint shader_handle = 0;

  switch(type)
  {
  case GLSLShader::VERTEX:
    shader_handle = glCreateShader(GL_VERTEX_SHADER);
    break;
  case GLSLShader::FRAGMENT:
    shader_handle = glCreateShader(GL_FRAGMENT_SHADER);
    break;
  case GLSLShader::GEOMETRY:
    shader_handle = glCreateShader(GL_GEOMETRY_SHADER);
    break;
  case GLSLShader::TESS_CONTROL:
    shader_handle = glCreateShader(GL_TESS_CONTROL_SHADER);
    break;
  case GLSLShader::TESS_EVALUATION:
    shader_handle = glCreateShader(GL_TESS_EVALUATION_SHADER);
    break;
  default:
    return false;
  }

  const char* c_code = source.c_str();
  glShaderSource(shader_handle, 1, &c_code, NULL);

  //compile the shader
  glCompileShader(shader_handle);

  //check for errors
  int result;
  glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &result);
  if(GL_FALSE == result)
  {
    int length = 0;
    _log_string = "";
    glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &length);
    if(length > 0)
    {
      char* c_log = new char[length];
      int written = 0;
      glGetShaderInfoLog(shader_handle, length, &written, c_log);
      _log_string = c_log;
      delete[] c_log;
    }
    return false;
  }
  else
  {
    glAttachShader(_handle, shader_handle);
    return true;
  }
}

bool GLSLProgram::Link()
{
  if(_linked)
    return true;
  if(_handle <= 0)
    return false;

  glLinkProgram(_handle);
  int status;
  glGetProgramiv(_handle, GL_LINK_STATUS, &status);
  if(GL_FALSE == status)
  {
    int length = 0;
    _log_string = "";
    glGetProgramiv(_handle, GL_INFO_LOG_LENGTH, &length);
    if(length > 0)
    {
      char* c_log = new char[length];
      int written = 0;
      glGetProgramInfoLog(_handle, length, &written, c_log);
      _log_string = c_log;
      delete[] c_log;
    }
    return false;
  }
  else
  {
    _linked = true;
    return _linked;
  }
}

void GLSLProgram::Use()
{
  if(_handle <= 0 || (!_linked))
    return;
  glUseProgram(_handle);
}

string GLSLProgram::Log()
{
  return _log_string;
}

int GLSLProgram::Handle()
{
  return _handle;
}

bool GLSLProgram::IsLinked()
{
  return _linked;
}

void GLSLProgram::BindAttribLocation(GLuint location, const char* name)
{
  glBindAttribLocation(_handle, location, name);
}

void GLSLProgram::BindFragDataLocation(GLuint location, const char* name)
{
  glBindFragDataLocation(_handle, location, name);
}

void GLSLProgram::SetUniform(const char* name, float x, float y, float z)
{
  int loc = GetUniformLocation(name);
  if(loc >= 0)
    glUniform3f(loc, x, y, z);
}

void GLSLProgram::SetUniform(const char* name, const vec3 & v)
{
  SetUniform(name, v.x, v.y, v.z);
}

void GLSLProgram::SetUniform(const char* name, const vec4& v)
{
  int loc = GetUniformLocation(name);
  if(loc >= 0)
    glUniform4f(loc, v.x, v.y, v.z, v.w);
}

void GLSLProgram::SetUniform(const char* name, const mat4& m)
{
  int loc = GetUniformLocation(name);
  if(loc >= 0)
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
}

void GLSLProgram::SetUniform(const char* name, const mat3& m)
{
  int loc = GetUniformLocation(name);
  if(loc >= 0)
    glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(m));
}

void GLSLProgram::SetUniform(const char* name, float val)
{
  int loc = GetUniformLocation(name);
  if(loc >= 0)
    glUniform1f(loc, val);
}

void GLSLProgram::SetUniform(const char* name, int val)
{
  int loc = GetUniformLocation(name);
  if(loc >= 0)
    glUniform1i(loc, val);
}

void GLSLProgram::SetUniform(const char* name, bool val)
{
  int loc = GetUniformLocation(name);
  if(loc >= 0)
    glUniform1i(loc, val);
}

void GLSLProgram::PrintActiveUniforms()
{
  GLint uniforms, size, location, max_len;
  GLchar* name;
  GLsizei written;
  GLenum type;

  glGetProgramiv(_handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_len);
  glGetProgramiv(_handle, GL_ACTIVE_UNIFORMS, &uniforms);

  name = new GLchar[max_len];

  ostringstream out;
  out << " Location | Name\n";
  out << "-----------------------------------------------\n";
  for(int i = 0; i < uniforms; ++i)
  {
    glGetActiveUniform(_handle, i, max_len, &written, &size, &type, name);
    location = glGetUniformLocation(_handle, name);
    out.width(8);
    out.fill(' ');
    out << location << "   " << name << "\n";
  }
  boost::log::sources::severity_logger<severity_level>& lg = get_global_log();
  BOOST_LOG_SEV(lg, info) << out.str();
  delete[] name;
}

void GLSLProgram::PrintActiveAttribs()
{
  GLint written, size, location, max_len, attribs;
  GLchar* name;
  GLenum type;

  glGetProgramiv(_handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_len);
  glGetProgramiv(_handle, GL_ACTIVE_ATTRIBUTES, &attribs);

  name = new GLchar[max_len];

  ostringstream out;
  out << " Index | Name\n";
  out << "-----------------------------------------------\n";
  for(int i = 0; i < attribs; ++i)
  {
    glGetActiveAttrib(_handle, i, max_len, &written, &size, &type, name);
    location = glGetAttribLocation(_handle, name);
    out.width(5);
    out.fill(' ');
    out << location << "   " << name << "\n";
  }
  boost::log::sources::severity_logger<severity_level>& lg = get_global_log();
  BOOST_LOG_SEV(lg, info) << out.str();

  delete[] name;
}

bool GLSLProgram::CompileAndLinkShaders(const string& v, const string& f, const string& g, const string& tc, const string& te)
{
  if(!CompileShaderFromFile(v.c_str(), GLSLShader::VERTEX))
  {
    BOOST_LOG_SEV(get_global_log(), error) << "Vertex Shader " << v.c_str() << " failed\n" << Log() << std::endl << "Hit any key to exit";
    char c;
    std::cin >> c;
    exit(1);
  }

//fragment
  if(!CompileShaderFromFile(f.c_str(), GLSLShader::FRAGMENT))
  {
    BOOST_LOG_SEV(get_global_log(), error) << "Fragment Shader " << f.c_str() << " failed\n" << Log() << std::endl << "Hit any key to exit";
    char c;
    std::cin >> c;
    exit(1);
  }

  if(!Link())
  {
    BOOST_LOG_SEV(get_global_log(), error) << "Program failed to link\n" << Log() << std::endl << "Hit any key to exit";
    char c;
    std::cin >> c;
    exit(1);
  }
  Use();
  PrintActiveAttribs();
  PrintActiveUniforms();
  return true;
}
