#include "GLWidget.h"
#include <QtCore/qcoreapplication.h>
#include <iostream>
#include <math.h>
#include <gli/gli.hpp>
#include <gli/gtx/gl_texture2d.hpp>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

#include "VboTorus.h"
#include "Material.h"

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
  p->CompileAndLinkShaders("diffuse_vert.glsl", "diffuse_frag.glsl");
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glEnable(GL_DEPTH_TEST);

  VboTorus* torus = new VboTorus(0.7f, 0.3f, 50, 50);
  torus->SetPosition(vec3(-1.0f, -1.0f, 0.0f));
  Material* m = new Material(p);
  m->Ambient(0.9f, 0.5f, 0.3f);
  m->Diffuse(0.9f, 0.5f, 0.3f);
  m->Specular(0.8f, 0.8f, 0.8f);
  m->Shininess(100.0f);
  torus->SetMaterial(m);
  _objs.push_back(torus);
  VboTorus* t = new VboTorus(0.5f, 0.3f, 50, 50);
  t->SetPosition(vec3(2.0f, 0.0f, 0.0f));
  Material* mm = new Material(p);
  mm->Ambient(0.1f, 0.8f, 0.3f);
  mm->Diffuse(0.1f, 0.8f, 0.3f);
  mm->Specular(0.8f, 0.8f, 0.8f);
  mm->Shininess(100.0f);
  t->SetMaterial(mm);
  _objs.push_back(t);
  vec4 world_light = vec4(5.0f, 5.0f, 2.0f, 1.0f);
  p->SetUniform("light.ld", 1.0f, 1.0f, 1.0f);
  p->SetUniform("light.position", _camera.View() * world_light);
  p->SetUniform("light.la", 0.4f, 0.4f, 0.4f);
  p->SetUniform("light.ls", 1.0f, 1.0f, 1.0f);
}

void GLWidget::resizeGL( int w, int h )
{
  // Set the viewport to window dimensions
  glViewport( 0, 0, w, qMax( h, 1 ) );
  _width = w;
  _height = h;
  //_proj = glm::perspective(70.0f, (float)w/h, 0.3f, 100.0f);
  _camera.Projection(70.f, (float)w/h, 0.3f, 100.0f);
}

void GLWidget::paintGL()
{
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  mat4 view = glm::lookAt(vec3(0.0f, 0.0f, 2.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
  for(SceneObjects::iterator it = _objs.begin(); it != _objs.end(); ++it)
  {
    GLSLProgram* p = (*it)->Program();
    mat4 mv = _camera.View() * (*it)->Model();
    p->SetUniform("model_view_matrix", mv);
    p->SetUniform("normal_matrix", mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
    p->SetUniform("mvp", _camera.Projection() * mv);
    (*it)->Render();
  }
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
      _camera.DeltaBearing(-0.1f);
      break;
    case Qt::Key_E:
      _camera.DeltaBearing(0.1f);
      break;
    case Qt::Key_Z:
      _camera.DeltaPitch(-0.1f);
      break;
    case Qt::Key_C:
      _camera.DeltaPitch(0.1f);
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
//  Drawable* d = _objs.front();
//  d->Move(vec3(0.1, 0.0, 0.0));
  updateGL();
}
