#version 400

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec2 vertex_tex_coord;

out vec3 position;
out vec3 normal;
out vec2 tex_coord;

uniform mat4 model_view_matrix;
uniform mat3 normal_matrix;
uniform mat4 mvp;

void main()
{
  tex_coord = vertex_tex_coord;
  normal = normalize(normal_matrix * vertex_normal);
  position = vec3(model_view_matrix * vec4(vertex_position, 1.0));
  gl_Position = mvp * vec4(vertex_position, 1.0);
}