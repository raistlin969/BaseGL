#version 400

in vec3 light_dir;
in vec3 view_dir;
in vec2 tex_coord;

uniform sampler2D color_tex;
uniform sampler2D normal_map_tex;

struct LightInfo
{
  vec4 position;
  vec3 intensity;
};

struct MaterialInfo
{
  vec3 ka;
  vec3 ks;
  float shininess;
};

uniform LightInfo light;
uniform MaterialInfo material;

layout (location = 0) out vec4 frag_color;

vec3 Phong(vec3 norm, vec3 diffr)
{
  vec3 r = reflect(-light_dir, norm);
  vec3 ambient = light.intensity * material.ka;
  float sdotn = max(dot(light_dir, norm), 0.0);
  vec3 diffuse = light.intensity * diffr * sdotn;
  vec3 spec = vec3(0.0);
  if(sdotn > 0.0)
  {
    spec = light.intensity * material.ks * pow(max(dot(r, view_dir), 0.0), material.shininess);
  }
  return ambient + diffuse + spec;
}

void main()
{
  //lookup the normal from the normal map
  vec4 normal = texture(normal_map_tex, tex_coord);
  //the color texxture is used as the diffuse reflectivity
  vec4 tex_color = texture(color_tex, tex_coord);
  frag_color = vec4(Phong(normal.xyz, tex_color.rgb), 1.0);
}