#pragma once

#include <glm/glm.hpp>

using glm::vec3;
using glm::mat4;

class Camera
{
protected:
  vec3 _direction;
  vec3 _position;
  vec3 _up;
  vec3 _right;
  mat4 _view;
  mat4 _projection;
  float _rotation_speed;
  float _bearing;
  float _pitch;
  float _fov;
  float _aspect_ratio;
  float _near;
  float _far;

public:
  Camera();
  
  void DeltaPitch(float dp);
  void DeltaBearing(float db);
  void MoveForward();
  void MoveBack();
  void StrafeLeft();
  void StrafeRight();
  void Up();
  void Down();
  mat4 Projection();
  mat4 View();
  void Projection(float fov, float aspect, float near, float far);
};