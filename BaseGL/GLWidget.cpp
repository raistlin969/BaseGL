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
  p->CompileAndLinkShaders("NormalMapVert.glsl", "NormalMapFrag.glsl");
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glEnable(GL_DEPTH_TEST);

  Mesh* ogre = new Mesh("bs_ears.obj");
  _objs.push_back(ogre);
  Material* m = new Material(p);
//  m->Diffuse(0.9f, 0.9f, 0.9f);
  m->Specular(0.2f, 0.2f, 0.2f);
  m->Ambient(0.1f, 0.1f, 0.1f);
  m->Shininess(1.0f);
  ogre->SetMaterial(m);
  p->SetUniform("light.intensity", vec3(0.9f, 0.9f, 0.9f));

  QImage brick_img = QGLWidget::convertToGLFormat(QImage("Textures/ogre_diffuse.png", "PNG"));
  QImage moss_img = QGLWidget::convertToGLFormat(QImage("Textures/ogre_normalmap.png", "PNG"));

  GLuint tid[2];
  glGenTextures(2, tid);

  glActiveTexture(GL_TEXTURE0);

  glBindTexture(GL_TEXTURE_2D, tid[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, brick_img.width(), brick_img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, brick_img.bits());
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  p->SetUniform("color_tex", 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, tid[1]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, moss_img.width(), moss_img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, moss_img.bits());
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  p->SetUniform("normal_map_tex", 1);

  _angle = (float)(TO_RADIANS(100.0));
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
  glClearColor(0.5, 0.5, 0.5, 1.0);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  vec4 light_pos = vec4(10.0f * cos(_angle), 10.0f, 10.0f * sin(_angle), 1.0f);
  mat4 view = _camera.View();
  for(SceneObjects::iterator it = _objs.begin(); it != _objs.end(); ++it)
  {
    GLSLProgram* p = (*it)->Program();
    mat4 mv = view * (*it)->Model();
    p->SetUniform("light.position", view * vec4(10.0f * cos(_angle), 1.0f, 10.0f * sin(_angle), 1.0f));
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
