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

struct FogInfo
{
  float max_dist;
  float min_dist;
  vec3 color;
};

uniform LightInfo light;
uniform MaterialInfo material;
uniform FogInfo fog;

layout (location = 0) out vec4 frag_color;

vec3 ads()
{
  vec3 s = normalize(vec3(light.position.xyz) - position.xyz);
  vec3 v = normalize(vec3(-position));
  vec3 h = normalize(v + s);
  vec3 ambient = material.ka * light.intensity;
  vec3 diffuse = light.intensity * material.kd * max(0.0, dot(s, normal));
  vec3 spec = light.intensity * material.ks * pow(max(0.0, dot(h, normal)), material.shininess);

  return ambient + diffuse + spec;
}

void main()
{
  float dist = abs(position.z);
  float fog_factor = (fog.max_dist - dist) / (fog.max_dist - fog.min_dist);
  fog_factor = clamp(fog_factor, 0.0, 1.0);
  vec3 shade_color = ads();
  vec3 color = mix(fog.color, shade_color, fog_factor);
  frag_color = vec4(color, 1.0);
}