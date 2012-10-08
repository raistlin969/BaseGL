#pragma once
#include <GL/glew.h>
#include <GL/wglew.h>
#include <QGLWidget>
#include <QtGui/qevent.h>
#include <QTimer>
#include <QElapsedTimer>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <boost/log/trivial.hpp>
#include <vector>
#include <string>
#include "Logger.h"
#include "GLSLProgram.h"
#include "Text.h"
#include "Camera.h"
#include "Drawable.h"

class GLWidget : public QGLWidget
{
  Q_OBJECT
public:
  GLWidget(const QGLFormat& format, QWidget* parent = 0);
  virtual ~GLWidget();

protected:
  typedef void (APIENTRY *_glGenVertexArrays) (GLsizei, GLuint*);
  typedef void (APIENTRY *_glBindVertexArray) (GLuint);
  typedef std::vector<Drawable*> SceneObjects;

protected:
  //Qt functions
  virtual void initializeGL();
  virtual void resizeGL(int w, int h);
  virtual void paintGL();
  virtual void keyPressEvent(QKeyEvent* e);
  
  virtual void FreeResources();
  
protected:
  void DumpGLInfo(bool dump_extentions = false);

protected slots:
  virtual void Idle();

protected:
  QTimer* _timer;
  QElapsedTimer _elapsed;

  //GLuint _vao;
  Text* _text;
  GLSLProgram _text_program;
  Camera _camera;
  int _width;
  int _height;
  unsigned int _frames;
  float _angle;

  boost::log::sources::severity_logger<severity_level>& _log;
  SceneObjects _objs;

protected:
  _glGenVertexArrays glGenVertexArrays;
  _glBindVertexArray glBindVertexArray;

};
