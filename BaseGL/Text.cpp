#include "Text.h"
#include <boost/log/trivial.hpp>
#include <GL/glew.h>
#include "Logger.h"
#include <glm/glm.hpp>
#include "GLSLProgram.h"

using glm::vec4;

Text::Text(GLSLProgram* p) : _program(p)
{
  if(FT_Init_FreeType(&_ft))
  {
    BOOST_LOG_SEV(get_global_log(), error) << "Could not init freetype lib";
  }
  if(FT_New_Face(_ft, "c:\\windows\\fonts\\lucon.ttf", 0, &_face))
  {
    BOOST_LOG_SEV(get_global_log(), error) << "Could not open font";
  }

  FT_Set_Pixel_Sizes(_face, 0, 24);


  glGenVertexArrays(1, &_vao);
  //glBindVertexArray(_vao);
}

Text::~Text()
{
}

void Text::Render(const char* text, float x, float y, float sx, float sy, vec4& color)
{
  const char* p;
  FT_GlyphSlot g = _face->glyph;
  GLuint tex;
  glBindVertexArray(_vao);
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  GLint loc = glGetUniformLocation(_program->Handle(), "tex");
  if(loc >= 0)
    glUniform1i(loc, 0);
  else
    BOOST_LOG_SEV(get_global_log(), error) << "Uniform tex not found";
  _program->SetUniform("color", color);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, _vao);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
  for(p = text; *p; p++)
  {
    if(FT_Load_Char(_face, *p, FT_LOAD_RENDER))
      continue;
    //int width = NextP2(g->bitmap.width);
    //int height = NextP2(g->bitmap.rows);
    //GLubyte* expanded_data = new GLubyte[2 * width * height];
    //for(int j = 0; j < height; ++j)
    //{
    //  for(int i = 0; i < width; ++i)
    //  {
    //    expanded_data[2*(i+j*width)] = expanded_data[2*(i+j*width)+1] = (i>=g->bitmap.width || j>=g->bitmap.rows) ? 0 : g->bitmap.buffer[i + g->bitmap.width*j];
    //  }
    //}
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, g->bitmap.width, g->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, expanded_data);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, g->bitmap.width, g->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);
    float x2 = x + g->bitmap_left * sx;
    float y2 = -y - g->bitmap_top * sy;
    float w = g->bitmap.width * sx;
    float h = g->bitmap.rows * sy;

    Point box[4] = {
      {x2,     -y2,     0, 0},
      {x2 + w, -y2,     1, 0},
      {x2,     -y2 - h, 0, 1},
      {x2 + w, -y2 - h, 1, 1},
    };
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * 4, box, GL_DYNAMIC_DRAW);
  
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    x += (g->advance.x >> 6) * sx;
    y += (g->advance.y >> 6) * sy;
  }
//  glDisableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glDeleteTextures(1, &tex);
}