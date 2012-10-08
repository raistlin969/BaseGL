#version 400

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;

out vec3 position;
out vec3 normal;

uniform mat4 model_view_matrix;
uniform mat3 normal_matrix;
uniform mat4 mvp;

void main()
{
  normal = normalize(normal_matrix * vertex_normal);
  position = vec3(model_view_matrix * vec4(vertex_position, 1.0));
  gl_Position = mvp * vec4(vertex_position, 1.0);
}