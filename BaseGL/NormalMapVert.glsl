#version 400

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec2 vertex_tex_coord;
layout (location = 3) in vec4 vertex_tangent;

struct LightInfo
{
  vec4 position;
  vec3 intensity;
};

out vec3 light_dir;
out vec3 view_dir;
out vec2 tex_coord;

uniform LightInfo light;
uniform mat4 model_view_matrix;
uniform mat3 normal_matrix;
uniform mat4 mvp;

void main()
{
  //transform normal and tangent to eye space
  vec3 norm = normalize(normal_matrix * vertex_normal);
  vec3 tang = normalize(normal_matrix * vec3(vertex_tangent));
  //compute binormal
  vec3 binormal = normalize(cross(norm, tang)) * vertex_tangent.w;

  //matrix for transformation to tangent space
  mat3 to_obj_local = mat3(tang.x, binormal.x, norm.x,
                           tang.y, binormal.y, norm.y,
                           tang.z, binormal.z, norm.z);

  //get position in eye coords
  vec3 pos = vec3(model_view_matrix * vec4(vertex_position, 1.0));
  //transform light dir and view dir to tangent space
  light_dir = normalize(to_obj_local * (light.position.xyz - pos));
  view_dir = to_obj_local * normalize(-pos);
  
  tex_coord = vertex_tex_coord;
  gl_Position = mvp * vec4(vertex_position, 1.0);
}