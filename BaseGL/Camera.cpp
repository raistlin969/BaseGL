#include "Camera.h"
#include "Defines.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <math.h>

Camera::Camera()
{
//  _direction = vec3(0.0f, 0.0f, 0.0f);
  _position = vec3(0.0f, 0.0f, 2.0f);
  //_up = vec3(0.0f, 1.0f, 0.0f);
  _rotation_speed = 0.05f;
  _view = glm::lookAt(_position, _direction, _up);
  _projection = mat4(1.0f);
  _bearing = -PI;
  _pitch = 0.0f;
  _fov = 45.0f;
  _near = 0.3f;
  _far = 100.0f;
  _aspect_ratio = 800.0f / 600.0f;
  _right = vec3(sin(_pitch - PI/2.0f), 0.0f, cos(_pitch - PI/2.0f));
  _direction = vec3(cos(_bearing) * sin(_pitch), sin(_bearing), cos(_bearing) * cos(_pitch));
  _up = glm::cross(_right, _direction);
}

void Camera::DeltaBearing(float db)
{
  _bearing += db;
  _right = vec3(sin(_bearing - PI/2.0f), 0.0f, cos(_bearing - PI/2.0f));
  _direction = vec3(cos(_pitch) * sin(_bearing), sin(_pitch), cos(_pitch) * cos(_bearing));
}

void Camera::DeltaPitch(float dp)
{
  _pitch += dp;
  _direction = vec3(cos(_pitch) * sin(_bearing), sin(_pitch), cos(_pitch) * cos(_bearing));
}

void Camera::MoveForward()
{
  _position += _direction * 0.1f;
}

void Camera::MoveBack()
{
  _position -= _direction * 0.1f;
}

void Camera::StrafeLeft()
{
  _position -= _right * 0.1f;
}

void Camera::StrafeRight()
{
  _position += _right * 0.1f;
}

mat4 Camera::Projection()
{
  return _projection;
}

mat4 Camera::View()
{
  _up = glm::cross(_right, _direction);
  _view = glm::lookAt(_position, _position + _direction, _up);
  return _view;
}

void Camera::Projection(float fov, float aspect, float near, float far)
{
  _fov = fov;
  _aspect_ratio = aspect;
  _near = near;
  _far = far;
  _projection = glm::perspective(_fov, _aspect_ratio, _near, _far);
}