#include "GLWidget.h"
#include <QtCore/qcoreapplication.h>
#include <qimage.h>
#include <iostream>
#include <math.h>
#include <gli/gli.hpp>
#include <gli/gtx/gl_texture2d.hpp>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <cstdlib>
#include <cstdio>
#include "VboTorus.h"
#include "VboTeapot.h"
#include "Material.h"
#include "Plane.h"
#include "Cube.h"
#include "Mesh.h"
#include "SkyBox.h"
#include "Defines.h"


GLWidget::GLWidget(const QGLFormat& format, QWidget* parent)
  : QGLWidget(format, parent), _log(get_global_log())
{
  makeCurrent();
  glewExperimental= GL_TRUE;
  GLenum err = glewInit();
  if(err != GLEW_OK)
    exit(1);
  setAutoBufferSwap(true);
  _timer = new QTimer(this);
  _elapsed.start();
  _frames = 0;
  _width = width();
  _height = height();
}

void GLWidget::DumpGLInfo(bool dump_extentions)
{
  const GLubyte* renderer = glGetString(GL_RENDERER);
  const GLubyte* vendor = glGetString(GL_VENDOR);
  const GLubyte* version = glGetString(GL_VERSION);
  const GLubyte* glsl_version = glGetString(GL_SHADING_LANGUAGE_VERSION);

  GLint major, minor;
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  glGetIntegerv(GL_MINOR_VERSION, &minor);

  BOOST_LOG_SEV(_log, info) << "GL Vendor:             " << vendor;
  BOOST_LOG_SEV(_log, info) << "GL Renderer:           " << renderer;
  BOOST_LOG_SEV(_log, info) << "GL Version (string):   " << version;
  BOOST_LOG_SEV(_log, info) << "GL Version (integer):  " << major << "." << minor;
  BOOST_LOG_SEV(_log, info) << "GLSL Version:          " << glsl_version;

  if(dump_extentions)
  {
    GLint num;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num);
    for(int i = 0; i < num; ++i)
      BOOST_LOG_SEV(_log, info) << glGetStringi(GL_EXTENSIONS, i);
  }
}

void GLWidget::initializeGL()
{
  glGenVertexArrays = (_glGenVertexArrays) QGLWidget::context()->getProcAddress("glGenVertexArrays");
  glBindVertexArray = (_glBindVertexArray) QGLWidget::context()->getProcAddress("glBindVertexArray");
  glEnable(GL_DEPTH_TEST);

  DumpGLInfo(false);

  connect(_timer, SIGNAL(timeout()), this, SLOT(Idle()));
  _timer->start(0);

  GLSLProgram* p = new GLSLProgram;
  p->CompileAndLinkShaders("SobelVert.glsl", "SobelFrag.glsl");
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  Material* tea_material = new Material(p);
  tea_material->Diffuse(0.9f, 0.9f, 0.9f);
  tea_material->Specular(0.95f, 0.95f, 0.95f);
  tea_material->Ambient(0.1f, 0.1f, 0.1f);
  tea_material->Shininess(100.0f);

  Material* plane_material = new Material(p);
  plane_material->Diffuse(0.4f, 0.4f, 0.4f);
  plane_material->Specular(0.0f, 0.0f, 0.0f);
  plane_material->Ambient(0.1f, 0.1f, 0.1f);
  plane_material->Shininess(1.0f);

  Material* torus_material = new Material(p);
  torus_material->Diffuse(0.9f, 0.5f, 0.2f);
  torus_material->Specular(0.95f, 0.95f, 0.95f);
  torus_material->Ambient(0.1f, 0.1f, 0.1f);
  torus_material->Shininess(100.0f);

  VBOTeapot* tea = new VBOTeapot(14, mat4(1.0f));
  Plane* plane = new Plane(50.0f, 50.0f, 1, 1);
  float c = 1.5f;
  VboTorus* torus = new VboTorus(0.7f * c, 0.3f * c, 50, 50);

  tea->SetMaterial(tea_material);
  tea->Rotate(-90.0f, vec3(1.0f, 0.0f, 0.0f));
  _objs.push_back(tea);

  plane->SetMaterial(plane_material);
  plane->SetPosition(vec3(0.0f, -0.75f, 0.0f));
  _objs.push_back(plane);

  torus->SetMaterial(torus_material);
  torus->SetPosition(vec3(1.0f, 1.0f, 3.0f));
  torus->Rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
  _objs.push_back(torus);

  SetupFBO();

  _pass_index_1 = glGetSubroutineIndex(p->Handle(), GL_FRAGMENT_SHADER, "Pass1");
  _pass_index_2 = glGetSubroutineIndex(p->Handle(), GL_FRAGMENT_SHADER, "Pass2");
  p->SetUniform("width", _width);
  p->SetUniform("height", _height);
  p->SetUniform("edge_threshold", 0.1f);
  p->SetUniform("render_tex", 0);
  p->SetUniform("light.intensity", vec3(1.0f, 1.0f, 1.0f));
  _angle = PI / 4.0;
  _quad = new FullscreenQuad;
  _quad->SetMaterial(plane_material);  //doesnt really use a material, just ned a GLSLProgram ref
}

void GLWidget::resizeGL( int w, int h )
{
  // Set the viewport to window dimensions
  glViewport( 0, 0, w, qMax( h, 1 ) );
  _width = w;
  _height = h;
  //_proj = glm::perspective(70.0f, (float)w/h, 0.3f, 100.0f);
  _camera.Projection(60.f, (float)w/h, 0.3f, 100.0f);
}

void GLWidget::paintGL()
{
  Pass1();
  Pass2();
}

void GLWidget::keyPressEvent( QKeyEvent* e )
{
  switch ( e->key() )
  {
    case Qt::Key_Escape:
      QCoreApplication::instance()->quit();
      break;
    case Qt::Key_A:
      _camera.StrafeLeft();
      break;
    case Qt::Key_D:
      _camera.StrafeRight();
      break;
    case Qt::Key_W:
      _camera.MoveForward();
      break;
    case Qt::Key_S:
      _camera.MoveBack();
      break;
    case Qt::Key_Q:
      _camera.DeltaBearing(0.1f);
      break;
    case Qt::Key_E:
      _camera.DeltaBearing(-0.1f);
      break;
    case Qt::Key_Z:
      _camera.DeltaPitch(-0.1f);
      break;
    case Qt::Key_C:
      _camera.DeltaPitch(0.1f);
      break;
    case Qt::Key_F:
      _camera.Up();
      break;
    case Qt::Key_G:
      _camera.Down();
      break;
    default:
      QGLWidget::keyPressEvent( e );
  }
}

void GLWidget::FreeResources()
{
}

GLWidget::~GLWidget()
{
  FreeResources();
}

void GLWidget::Idle()
{
  _angle += 0.001f;
  if(_angle > TWOPI)
    _angle -= TWOPI;
  updateGL();
}

void GLWidget::SetupFBO()
{
  ///////////////////////SETUP FBO/////////////////////////////////
  glGenFramebuffers(1, &_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
  GLuint render_tex;
  glGenTextures(1, &render_tex);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, render_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_tex, 0);

  //create depth buffer
  GLuint depth_buffer;
  glGenRenderbuffers(1, &depth_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _width, _height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
  GLenum draw_buffers[] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, draw_buffers);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  ///////////////////////////////////////////////////////////////////
}

void GLWidget::Pass1()
{
  glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &_pass_index_1);
  //mat4 view = glm::lookAt(vec3(7.0f * cos(_angle), 4.0f, 7.0f * sin(_angle)), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
  //mat4 projection = glm::perspective(60.0f, (float)_width/_height, 0.3f, 100.0f);
  mat4 view = _camera.View();
  mat4 projection = _camera.Projection();
  for(SceneObjects::iterator it = _objs.begin(); it != _objs.end(); ++it)
  {
    GLSLProgram* p = (*it)->Program();
    mat4 mv = view * (*it)->Model();
    p->SetUniform("light.position", vec4(0.0f, 0.0f, 0.0f, 1.0f));
    p->SetUniform("model_view_matrix", mv);
    p->SetUniform("normal_matrix", mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
    p->SetUniform("mvp", projection * mv);
    (*it)->Render();
  }
}

void GLWidget::Pass2()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &_pass_index_2);
  GLSLProgram* p = _quad->Program();
  mat4 model = mat4(1.0f);
  mat4 view = mat4(1.0f);
  mat4 projection = mat4(1.0f);
  mat4 mv = view * model;
  p->SetUniform("model_view_matrix", mv);
  p->SetUniform("normal_matrix", mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
  p->SetUniform("mvp", projection * mv);
  _quad->Render();
}