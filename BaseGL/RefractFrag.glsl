#version 400

in vec3 reflect_dir;
in vec3 refract_dir;

uniform samplerCube cube_map_tex;
uniform bool draw_sky_box;
uniform float reflect_factor;
uniform vec4 material_color;

layout (location = 0) out vec4 frag_color;

void main()
{
  vec4 reflect_color = texture(cube_map_tex, reflect_dir);
  vec4 refract_color = texture(cube_map_tex, refract_dir);
  if(draw_sky_box)
    frag_color = reflect_color;
  else
    frag_color = mix(refract_color, reflect_color, reflect_factor);
}