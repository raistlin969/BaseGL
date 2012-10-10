#version 400

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec2 vertex_tex_coord;


out vec3 reflect_dir;

uniform bool draw_sky_box;
uniform vec3 world_camera_pos;

uniform mat4 model_view_matrix;
uniform mat4 model_matrix;
uniform mat3 normal_matrix;
uniform mat4 mvp;

void main()
{
  if(draw_sky_box)
  {
    reflect_dir = vertex_position;
  }
  else
  {
    vec3 world_pos = vec3(model_matrix * vec4(vertex_position, 1.0));
    vec3 world_norm = vec3(model_matrix * vec4(vertex_normal, 0.0));
    vec3 world_view = normalize(world_camera_pos - world_pos);
    reflect_dir = reflect(-world_view, world_norm);
  }
  gl_Position = mvp * vec4(vertex_position, 1.0);
}