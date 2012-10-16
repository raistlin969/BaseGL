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

GLuint fbo;

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
  p->CompileAndLinkShaders("RenderToTexVert.glsl", "RenderToTexFrag.glsl");
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glEnable(GL_DEPTH_TEST);

  VBOTeapot* tea = new VBOTeapot(14, mat4(1.0f));
  Cube* cube = new Cube;

  Material* m = new Material(p);
  m->Diffuse(0.9f, 0.9f, 0.9f);
  m->Specular(0.95f, 0.95f, 0.95f);
  m->Ambient(0.1f, 0.1f, 0.1f);
  m->Shininess(100.0f);

  tea->SetMaterial(m);
  _objs.push_back(tea);

  tea->SetPosition(vec3(0.0f, -1.5f, 0.0f));
  tea->Rotate(-90.0f, vec3(1.0f, 0.0f, 0.0f));
  p->SetUniform("light.intensity", vec3(1.0f, 1.0f, 1.0f));

  Material* mm = new Material(p);
  mm->Diffuse(0.9f, 0.9f, 0.9f);
  mm->Specular(0.0f, 0.0f, 0.0f);
  mm->Ambient(0.1f, 0.1f, 0.1f);
  mm->Shininess(0.0f);

  cube->SetMaterial(mm);
  _objs.push_back(cube);

  ///////////////////////SETUP FBO/////////////////////////////////
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  GLuint render_tex;
  glGenTextures(1, &render_tex);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, render_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_tex, 0);

  //create depth buffer
  GLuint depth_buffer;
  glGenRenderbuffers(1, &depth_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 512, 512);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
  GLenum draw_buffers[] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, draw_buffers);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  ///////////////////////////////////////////////////////////////////

  //one pixel white texture
  GLuint white_tex_handle;
  GLubyte white_tex[] = {255, 255, 255, 255};
  glActiveTexture(GL_TEXTURE1);
  glGenTextures(1, &white_tex_handle);
  glBindTexture(GL_TEXTURE_2D, white_tex_handle);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_tex);
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
  glClearColor(0.5, 0.5, 0.5, 1.0);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  Drawable* tea = _objs[0];
  Drawable* cube = _objs[1];
  GLSLProgram* p = tea->Program();

  p->SetUniform("render_tex", 1);
  glViewport(0, 0, 512, 512);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  mat4 view = glm::lookAt(vec3(0.0f, 0.0f, 7.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
  mat4 projection = glm::perspective(60.0f, 1.0f, 0.3f, 100.0f);
  p->SetUniform("light.position", vec4(0.0f, 0.0f, 0.0f, 1.0f));
  mat4 mv = view * tea->Model();
  p->SetUniform("model_view_matrix", mv);
  p->SetUniform("normal_matrix", mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
  p->SetUniform("mvp", projection * mv);
  tea->Render();
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  p = cube->Program();
  p->SetUniform("render_tex", 0);
  glViewport(0, 0, _width, _height);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  
  view = _camera.View();
  _camera.Projection(45.0f, (float)_width/_height, 0.3f, 100.0f);
  mv = view * cube->Model();
  p->SetUniform("model_view_matrix", mv);
  p->SetUniform("normal_matrix", mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
  p->SetUniform("mvp", _camera.Projection() * mv);
  cube->Render();
  //for(SceneObjects::iterator it = _objs.begin(); it != _objs.end(); ++it)
  //{
  //  GLSLProgram* p = (*it)->Program();
  //  mat4 mv = view * (*it)->Model();
//    p->SetUniform("light.position", view * vec4(10.0f * cos(_angle), 1.0f, 10.0f * sin(_angle), 1.0f));
//    p->SetUniform("model_view_matrix", mv);
//    p->SetUniform("normal_matrix", mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
  //  p->SetUniform("world_camera_pos", camera_pos);
  //  p->SetUniform("material_color", vec4(0.5f, 0.5f, 0.5f, 1.0f));
  //  p->SetUniform("reflect_factor", 0.1f);
  //  p->SetUniform("eta", 0.94f);
  //  p->SetUniform("mvp", _camera.Projection() * mv);
  //  p->SetUniform("model_matrix", (*it)->Model());
  //  (*it)->Render();
  //}
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
  _angle += 0.0001f;
  if(_angle > TWOPI)
    _angle -= TWOPI;
  updateGL();
}
