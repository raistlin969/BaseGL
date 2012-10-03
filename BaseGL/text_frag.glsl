#version 400

in vec2 texcoord;
out vec4 frag_color;
uniform sampler2D tex;
uniform vec4 color;

void main()
{
  frag_color = vec4(1.0, 1.0, 1.0, texture(tex, texcoord).r) * color;
// frag_color = color;
//  vec4 tex_color = texture(tex, texcoord);
 // frag_color = tex_color;
}