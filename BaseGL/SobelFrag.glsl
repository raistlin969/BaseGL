#version 400

in vec3 position;
in vec3 normal;
in vec2 tex_coord;

uniform sampler2D render_tex;
uniform float edge_threshold;
uniform int width;
uniform int height;

subroutine vec4 RenderPassType();
subroutine uniform RenderPassType RenderPass;

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

float luma(vec3 color)
{
  return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

//pass 1
subroutine (RenderPassType)
vec4 Pass1()
{
  return vec4(Phong(position, normal), 1.0);
}

//pass 2
subroutine(RenderPassType)
vec4 Pass2()
{
  float dx = 1.0 / float(width);
  float dy = 1.0 / float(height);
  float s00 = luma(texture(render_tex, tex_coord + vec2(-dx, dy)).rgb);
  float s10 = luma(texture(render_tex, tex_coord + vec2(-dx, 0.0)).rgb);
  float s20 = luma(texture(render_tex, tex_coord + vec2(-dx, -dy)).rgb);
  float s01 = luma(texture(render_tex, tex_coord + vec2(0.0, dy)).rgb);
  float s21 = luma(texture(render_tex, tex_coord + vec2(0.0, -dy)).rgb);
  float s02 = luma(texture(render_tex, tex_coord + vec2(dx, dy)).rgb);
  float s12 = luma(texture(render_tex, tex_coord + vec2(dx, 0.0)).rgb);
  float s22 = luma(texture(render_tex, tex_coord + vec2(dx, -dy)).rgb);

  float sx = s00 + 2 * s10 + s20 - (s02 + 2 * s12 + s22);
  float sy = s00 + 2 * s01 + s02 - (s20 + 2 * s21 + s22);
  float dist = sx * sx + sy * sy;
  if(dist > edge_threshold)
    return vec4(1.0);
  else
    return vec4(0.0, 0.0, 0.0, 1.0);
}

void main()
{
  frag_color = RenderPass();
}