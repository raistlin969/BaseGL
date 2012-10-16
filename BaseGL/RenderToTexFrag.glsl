#version 400

in vec3 position;
in vec3 normal;
in vec2 tex_coord;

uniform sampler2D render_tex;

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

vec3 Phong(vec3 pos, vec3 norm)
{
  vec3 s = normalize(vec3(light.position) - pos);
  vec3 v = normalize(vec3(-position.xyz));
  vec3 r = reflect(-s, norm);
  vec3 I = light.intensity;
  vec3 ambient = I * material.ka;
  float sdotn = max(dot(s,norm), 0.0);
  vec3 diffuse = I * material.kd * sdotn;
  vec3 spec = vec3(0.0);
  if(sdotn > 0.0)
  {
    spec = I * material.ks * pow(max(dot(r,v), 0.0), material.shininess);
  }

  return ambient + diffuse + spec;
}

void main()
{
  vec4 tex_color = texture(render_tex, tex_coord);
  frag_color = vec4(Phong(position, normal), 1.0) * tex_color;
}