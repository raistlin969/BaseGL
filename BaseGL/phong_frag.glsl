#version 400

in vec3 position;
in vec3 normal;

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

layout (location = 0) out vec4 frag_color;

vec3 ads()
{
  vec3 n = normalize(normal);
  vec3 s = normalize(vec3(light.position) - position);

  vec3 v = normalize(vec3(-position));
  vec3 h = normalize(v + s);
  vec3 I = light.intensity;
  return I * (material.ka + material.kd * max(dot(s, n), 0.0) + material.ks * pow(max(dot(h,n),0.0), material.shininess));
}

void main()
{
  frag_color = vec4(ads(), 1.0);
}