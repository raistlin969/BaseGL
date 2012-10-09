#version 400

in vec3 position;
in vec3 normal;
in vec2 tex_coord;

uniform sampler2D brick_tex;
uniform sampler2D moss_tex;

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

void Phong(vec3 pos, vec3 norm, out vec3 ambient_diffuse, out vec3 spec)
{
  vec3 s = normalize(vec3(light.position) - pos);
  vec3 v = normalize(vec3(-position.xyz));
  vec3 r = reflect(-s, norm);
  vec3 I = light.intensity;
  vec3 ambient = I * material.ka;
  float sdotn = max(dot(s,norm), 0.0);
  vec3 diffuse = I * material.kd * sdotn;
  spec = vec3(0.0);
  if(sdotn > 0.0)
  {
    spec = I * material.ks * pow(max(dot(r,v), 0.0), material.shininess);
  }

  ambient_diffuse = ambient + diffuse;
}

void main()
{
  vec3 ambient_diffuse, spec;
//  vec4 brick_color = texture(brick_tex, tex_coord);
//  vec4 moss_color = texture(moss_tex, tex_coord);
//  Phong(position, normal, ambient_diffuse, spec);
//  vec4 tex_color = mix(brick_color, moss_color, moss_color.a);
//  frag_color = (vec4(ambient_diffuse, 1.0) * tex_color) + vec4(spec, 1.0);
  vec4 base_color = texture(brick_tex, tex_coord);
  vec4 alpha_map = texture(moss_tex, tex_coord);
  if(alpha_map.a < 0.15)
    discard;
  else
  {
    if(gl_FrontFacing)
    {
      Phong(position, normal, ambient_diffuse, spec);
    }
    else
    {
      Phong(position, -normal, ambient_diffuse, spec);
    }
    frag_color = vec4(ambient_diffuse + spec, 1.0) * base_color;
  }
}