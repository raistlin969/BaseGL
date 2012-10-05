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

uniform LightInfo lights[5];
uniform MaterialInfo material;

uniform mat4 model_view_matrix;
uniform mat3 normal_matrix;
uniform mat4 mvp;

void GetEyeSpace(out vec3 norm, out vec4 position)
{
  norm = normalize(normal_matrix * vertex_normal);
  position = model_view_matrix * vec4(vertex_position, 1.0);
}

vec3 ads(int light_index, vec4 position, vec3 norm)
{
  vec3 s = normalize(vec3(lights[light_index].position - position));
  vec3 v = normalize(vec3(-position));
  vec3 r = reflect(-s, norm);
  vec3 I = lights[light_index].intensity;
  return I * (material.ka + material.kd * max(dot(s, norm), 0.0) + material.ks * pow(max(dot(r,v),0.0), material.shininess));
}

void main()
{
  vec3 eye_norm;
  vec4 eye_position;
  GetEyeSpace(eye_norm, eye_position);

  color = vec3(0.0);
  for(int i = 0; i < 5; i++)
    color += ads(i, eye_position, eye_norm);

  gl_Position = mvp * vec4(vertex_position, 1.0);
}