#version 400

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;

out vec3 color;

struct LightInfo
{
  vec4 position;
  vec3 intensity;
};

struct MaterialInfo
{
  vec3 ka;
  vec3 kd;
  vec3 ks;
  float shininess;
};

uniform LightInfo light;
uniform MaterialInfo material;

uniform mat4 model_view_matrix;
uniform mat3 normal_matrix;
uniform mat4 mvp;

void GetEyeSpace(out vec3 norm, out vec4 position)
{
  norm = normalize(normal_matrix * vertex_normal);
  position = model_view_matrix * vec4(vertex_position, 1.0);
}

vec3 ads(vec4 position, vec3 norm)
{
  vec3 s;
  if(light.position.w == 0.0)
    s = normalize(vec3(light.position));
  else
    s = normalize(vec3(light.position - position));

  vec3 v = normalize(vec3(-position));
  vec3 r = reflect(-s, norm);
  vec3 I = light.intensity;
  return I * (material.ka + material.kd * max(dot(s, norm), 0.0) + material.ks * pow(max(dot(r,v),0.0), material.shininess));
}

void main()
{
  vec3 eye_norm;
  vec4 eye_position;
  GetEyeSpace(eye_norm, eye_position);

  color = ads(eye_position, eye_norm);

  gl_Position = mvp * vec4(vertex_position, 1.0);
}