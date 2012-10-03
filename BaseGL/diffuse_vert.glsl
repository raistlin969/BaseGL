#version 400

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;

out vec3 light_intensity;

struct LightInfo
{
  vec4 position;
  vec3 la;
  vec3 ld;
  vec3 ls;
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

vec3 PhongModel(vec4 position, vec3 norm)
{
  vec3 s = normalize(vec3(light.position - position));
  vec3 v  = normalize(-position.xyz);
  vec3 r = reflect(-s, norm);
  vec3 ambient = light.la * material.ka;
  float s_dot_n = max(dot(s, norm), 0.0);
  vec3 diffuse = light.ld * material.kd * s_dot_n;
  vec3 spec = vec3(0.0);
  if(s_dot_n > 0.0)
  {
    spec = light.ls * material.ks * pow(max(dot(r,v), 0.0), material.shininess);
  }
  return ambient + diffuse + spec;
}

void main()
{
  vec3 eye_norm;
  vec4 eye_position;
  GetEyeSpace(eye_norm, eye_position);
  light_intensity = PhongModel(eye_position, eye_norm);

  gl_Position = mvp * vec4(vertex_position, 1.0);
}