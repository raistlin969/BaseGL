#version 400

in vec3 position;
in vec3 normal;

struct SpotLightInfo
{
  vec4 position;
  vec3 intensity;
  vec3 direction;
  float exponent;
  float cutoff;
};

struct MaterialInfo
{
  vec3 ka;
  vec3 kd;
  vec3 ks;
  float shininess;
};

uniform SpotLightInfo light;
uniform MaterialInfo material;

layout (location = 0) out vec4 frag_color;

vec3 SpotlightAds()
{
  vec3 s = normalize(vec3(light.position) - position);
  vec3 spot_dir = normalize(light.direction);
  float angle = acos(dot(-s, spot_dir));
  float cutoff = radians(clamp(light.cutoff, 0.0, 90.0));
  vec3 ambient = light.intensity * material.ka;

  if(angle < cutoff)
  {
    float spot_factor = pow(dot(-s, spot_dir), light.exponent);
    vec3 v = normalize(vec3(-position));
    vec3 h = normalize(v + s);
    return ambient + spot_factor * light.intensity * 
      (material.kd * max(dot(s, normal), 0.0) + material.ks * pow(max(dot(h,normal),0.0), material.shininess));
  }
  else
  {
    return ambient;
  }
}

void main()
{
  frag_color = vec4(SpotlightAds(), 1.0);
}