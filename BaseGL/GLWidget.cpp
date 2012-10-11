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
  p->CompileAndLinkShaders("RefractVert.glsl", "RefractFrag.glsl");
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glEnable(GL_DEPTH_TEST);

  VBOTeapot* tea = new VBOTeapot(14, mat4(1.0f));
  SkyBox* sky = new SkyBox;

  Material* m = new Material(p);
  m->Diffuse(0.9f, 0.9f, 0.9f);
  m->Specular(0.2f, 0.2f, 0.2f);
  m->Ambient(0.1f, 0.1f, 0.1f);
  m->Shininess(1.0f);

  tea->SetMaterial(m);
  sky->SetMaterial(m);
  _objs.push_back(sky);
  _objs.push_back(tea);

  tea->SetPosition(vec3(0.0f, -1.0f, 0.0f));
  tea->Rotate(-90.0f, vec3(1.0f, 0.0f, 0.0f));
  p->SetUniform("light.intensity", vec3(0.9f, 0.9f, 0.9f));

  QImage posx = QGLWidget::convertToGLFormat(QImage("Textures/brightday2_positive_x.png", "PNG")).mirrored();
  QImage negx = QGLWidget::convertToGLFormat(QImage("Textures/brightday2_negative_x.png", "PNG")).mirrored();
  QImage posy = QGLWidget::convertToGLFormat(QImage("Textures/brightday2_positive_y.png", "PNG")).mirrored();
  QImage negy = QGLWidget::convertToGLFormat(QImage("Textures/brightday2_negative_y.png", "PNG")).mirrored();
  QImage posz = QGLWidget::convertToGLFormat(QImage("Textures/brightday2_positive_z.png", "PNG")).mirrored();
  QImage negz = QGLWidget::convertToGLFormat(QImage("Textures/brightday2_negative_z.png", "PNG")).mirrored();

  //QImage posx = QGLWidget::convertToGLFormat(QImage("Textures/terrain_positive_x.png", "PNG")).mirrored();
  //QImage negx = QGLWidget::convertToGLFormat(QImage("Textures/terrain_negative_x.png", "PNG")).mirrored();
  //QImage posy = QGLWidget::convertToGLFormat(QImage("Textures/terrain_positive_y.png", "PNG")).mirrored();
  //QImage negy = QGLWidget::convertToGLFormat(QImage("Textures/terrain_negative_y.png", "PNG")).mirrored();
  //QImage posz = QGLWidget::convertToGLFormat(QImage("Textures/terrain_positive_z.png", "PNG")).mirrored();
  //QImage negz = QGLWidget::convertToGLFormat(QImage("Textures/terrain_negative_z.png", "PNG")).mirrored();

  //QImage posx = QGLWidget::convertToGLFormat(QImage("Textures/snow_positive_x.jpg", "JPG")).mirrored();
  //QImage negx = QGLWidget::convertToGLFormat(QImage("Textures/snow_negative_x.jpg", "JPG")).mirrored();
  //QImage posy = QGLWidget::convertToGLFormat(QImage("Textures/snow_positive_y.jpg", "JPG")).mirrored();
  //QImage negy = QGLWidget::convertToGLFormat(QImage("Textures/snow_negative_y.jpg", "JPG")).mirrored();
  //QImage posz = QGLWidget::convertToGLFormat(QImage("Textures/snow_positive_z.jpg", "JPG")).mirrored();
  //QImage negz = QGLWidget::convertToGLFormat(QImage("Textures/snow_negative_z.jpg", "JPG")).mirrored();

  glActiveTexture(GL_TEXTURE0);
  GLuint tid;
  glGenTextures(1, &tid);

  glBindTexture(GL_TEXTURE_CUBE_MAP, tid);

  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, posx.width(), posx.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, posx.bits());
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, negx.width(), negx.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, negx.bits());
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, posy.width(), posy.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, posy.bits());
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, negy.width(), negy.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, negy.bits());
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, posz.width(), posz.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, posz.bits());
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, negz.width(), negz.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, negz.bits());

  glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  p->SetUniform("cube_map_tex", 0);

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
  //vec4 light_pos = vec4(10.0f * cos(_angle), 10.0f, 10.0f * sin(_angle), 1.0f);
  mat4 view = _camera.View();
  vec3 camera_pos = _camera.Position();
  //camera_pos = vec3(7.0f * cos(_angle), 2.0f, 7.0f * sin(_angle));
  //view = glm::lookAt(camera_pos, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
  for(SceneObjects::iterator it = _objs.begin(); it != _objs.end(); ++it)
  {
    GLSLProgram* p = (*it)->Program();
    mat4 mv = view * (*it)->Model();
//    p->SetUniform("light.position", view * vec4(10.0f * cos(_angle), 1.0f, 10.0f * sin(_angle), 1.0f));
//    p->SetUniform("model_view_matrix", mv);
//    p->SetUniform("normal_matrix", mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
    p->SetUniform("world_camera_pos", camera_pos);
    p->SetUniform("material_color", vec4(0.5f, 0.5f, 0.5f, 1.0f));
    p->SetUniform("reflect_factor", 0.1f);
    p->SetUniform("eta", 0.94f);
    p->SetUniform("mvp", _camera.Projection() * mv);
    p->SetUniform("model_matrix", (*it)->Model());
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
