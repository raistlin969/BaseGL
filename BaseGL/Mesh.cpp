#include "Mesh.h"

#define uint unsigned int

#include <iostream>
using std::cout;
using std::endl;
#include <fstream>
using std::ifstream;
#include <sstream>
using std::istringstream;

#include <GL/glew.h>
#include <GL/GL.h>
#include <boost/log/trivial.hpp>
#include "Logger.h"

Mesh::Mesh(const char* filename, bool center)
{
  LoadObj(filename, center);
}

void Mesh::Render()
{
  glBindVertexArray(_vao);
  _material->Bind();
  glDrawElements(GL_TRIANGLES, 3 * _faces, GL_UNSIGNED_INT, 0);
}

void Mesh::LoadObj(const char* filename, bool recenter)
{
  vector<vec3> points;
  vector<vec3> normals;
  vector<vec2> tex_coords;
  vector<int> faces;
  int num_faces = 0;
  ifstream obj_stream(filename, std::ios::in);
  boost::log::sources::severity_logger<severity_level>& lg = get_global_log();
  if(!obj_stream)
  {
    BOOST_LOG_SEV(lg, error) << "Unable to open OBJ file: " << filename << endl << "Hit any key to exit";
    char c;
    std::cin >> c;
    exit(1);
  }

  string line, token;
  vector<int> face;
  getline(obj_stream, line);
  while(!obj_stream.eof())
  {
    TrimString(line);
    if(line.length() > 0 && line.at(0) != '#')
    {
      istringstream line_stream(line);
      line_stream >> token;
      if(token == "v")
      {
        float x, y, z;
        line_stream >> x >> y >> z;
        points.push_back(vec3(x,y,z));
      }
      else if(token == "vt")
      {
        float s, t;
        line_stream >> s >> t;
        tex_coords.push_back(vec2(s, t));
      }
      else if(token == "vn")
      {
        float x, y, z;
        line_stream >> x >> y >> z;
        normals.push_back(vec3(x, y, z));
      }
      else if(token == "f")
      {
        num_faces++;
        face.clear();
        size_t slash1, slash2;

        while(line_stream.good())
        {
          string vstring;
          line_stream >> vstring;
          int pi = -1, ni = -1, tci = -1;
          slash1 = vstring.find("/");
          if(slash1 == string::npos)
          {
            pi = atoi(vstring.c_str()) - 1;
          }
          else
          {
            slash2 = vstring.find("/", slash1 + 1);
            pi = atoi(vstring.substr(0, slash1).c_str()) - 1;
            if(slash2 > slash1 + 1)
            {
              tci = atoi(vstring.substr(slash1 + 1, slash2).c_str()) - 1;
            }
            ni = atoi(vstring.substr(slash2 + 1, vstring.length()).c_str()) - 1;
          }
          if(pi == -1)
          {
            BOOST_LOG_SEV(lg, warning) << "Missing point index!!!";
          }
          else
          {
            face.push_back(pi);
          }
          if(tci != -1 && pi != ni)
          {
            BOOST_LOG_SEV(lg, warning) << "Texture and point indices are not consistent";
          }
          if(ni != -1 && ni != pi)
          {
            BOOST_LOG_SEV(lg, warning) << "Normal and point indices are not consistent";
          }
        }
        if(face.size() != 3)
        {
          BOOST_LOG_SEV(lg, warning) << "Found non-triangular face";
        }
        else
        {
          faces.push_back(face[0]);
          faces.push_back(face[1]);
          faces.push_back(face[2]);
        }
      }
    }
    getline(obj_stream, line);
  }

  obj_stream.close();

  if(normals.size() == 0)
    GenerateAveragedNormals(points, normals, faces);

  vector<vec4> tangents;
  
  if(tex_coords.size() > 0)
    GenerateTangents(points, normals, faces, tex_coords, tangents);

  if(recenter)
    Center(points);

  StoreVbo(points, normals, tex_coords, tangents, faces);

  BOOST_LOG_SEV(lg, info) << "Loaded mesh from " << filename << endl <<
    " " << points.size() << " points" << endl <<
    " " << num_faces << " faces" << endl << 
    " " << normals.size() << " normals" << endl <<
    " " << tex_coords.size() << " texture coordinates." << endl;
}

void Mesh::Center(vector<vec3>& points)
{
  if(points.size() < 1)
    return;
  vec3 max_point = points[0];
  vec3 min_point = points[0];

  //find the AABB
  for(uint i = 0; i < points.size(); ++i)
  {
    vec3& point = points[i];
    if(point.x > max_point.x)
      max_point.x = point.x;
    if(point.y > max_point.y)
      max_point.y = point.y;
    if(point.z > max_point.z)
      max_point.z = point.z;
    if(point.x > min_point.x)
      min_point.x = point.x;
    if(point.y > min_point.y)
      min_point.y = point.y;
    if(point.z > min_point.z)
      min_point.z = point.z;
  }

  //center of the AABB
  vec3 center = vec3((max_point.x + min_point.x) / 2.0f, (max_point.y + min_point.y) / 2.0f, (max_point.z + min_point.z) / 2.0f);
  //translate center to origin
  for(uint i = 0; i < points.size(); ++i)
  {
    vec3& point = points[i];
    point = point - center;
  }
}

void Mesh::GenerateAveragedNormals(const vector<vec3>& points, vector<vec3>& normals, const vector<int>& faces)
{
  for(uint i = 0; i < points.size(); ++i)
  {
    normals.push_back(vec3(0.0f));
  }
  for(uint i = 0; i < faces.size(); i += 3)
  {
    const vec3& p1 = points[faces[i]];
    const vec3& p2 = points[faces[i+1]];
    const vec3& p3 = points[faces[i+2]];
    vec3 a = p2 - p1;
    vec3 b = p3 - p1;
    vec3 n = glm::normalize(glm::cross(a,b));
    normals[faces[i]] += n;
    normals[faces[i+1]] += n;
    normals[faces[i+2]] += n;
  }
  for(uint i = 0; i < normals.size(); ++i)
  {
    normals[i] = glm::normalize(normals[i]);
  }
}

void Mesh::GenerateTangents(const vector<vec3>& points, const vector<vec3>& normals, const vector<int>& faces, const vector<vec2>& texc, vector<vec4>& tangents)
{
  vector<vec3> tan1_accum;
  vector<vec3> tan2_accum;

  for(uint i = 0; i < points.size(); ++i)
  {
    tan1_accum.push_back(vec3(0.0f));
    tan2_accum.push_back(vec3(0.0f));
    tangents.push_back(vec4(0.0f));
  }

  for(uint i = 0; i < faces.size(); i+=3)
  {
    const vec3& p1 = points[faces[i]];
    const vec3& p2 = points[faces[i+1]];
    const vec3& p3 = points[faces[i+2]];

    const vec2& tc1 = texc[faces[i]];
    const vec2& tc2 = texc[faces[i+1]];
    const vec2& tc3 = texc[faces[i+2]];

    vec3 q1 = p2 - p1;
    vec3 q2 = p3 - p1;
    float s1 = tc2.x - tc1.x, s2 = tc3.x - tc1.x;
    float t1 = tc2.y - tc1.y, t2 = tc3.y - tc1.y;
    float r = 1.0f / (s1 * t2 - s2 * t1);
    vec3 tan1((t2*q1.x - t1*q2.x) * r, (t2*q1.y - t1*q2.y) * r, (t2*q1.z - t1*q2.z) * r);
    vec3 tan2((s1*q2.x - s2*q1.x) * r, (s1*q2.y - s2*q1.y) * r, (s1*q2.z - s2*q1.z) * r);
    tan1_accum[faces[i]] += tan1;
    tan1_accum[faces[i+1]] += tan1;
    tan1_accum[faces[i+2]] += tan1;
    tan2_accum[faces[i]] += tan2;
    tan2_accum[faces[i+1]] += tan2;
    tan2_accum[faces[i+2]] += tan2;
  }

  for(uint i = 0; i < points.size(); ++i)
  {
    const vec3& n = normals[i];
    vec3& t1 = tan1_accum[i];
    vec3& t2 = tan2_accum[i];
    //gram-schmidt ortho
    tangents[i] = vec4(glm::normalize(t1 - (glm::dot(n,t1) * n)), 0.0f);
    //store handiness in w
    tangents[i].w = (glm::dot(glm::cross(n,t1), t2) < 0.0f) ? -1.0f : 1.0f;
  }
  tan1_accum.clear();
  tan2_accum.clear();
}

void Mesh::StoreVbo(const vector<vec3>& points, const vector<vec3>& normals, const vector<vec2>& texc, const vector<vec4>& tangents, const vector<int>& elements)
{
  int nverts = points.size();
  _faces = elements.size() / 3;

  float* v = new float[3*nverts];
  float* n = new float[3*nverts];
  float* tc = NULL;
  float* tang = NULL;

  if(texc.size() > 0 && tangents.size() > 0)
  {
    tc = new float[2*nverts];
    tang = new float[4*nverts];
  }

  unsigned int* el = new unsigned int[elements.size()];
  int idx = 0, tcidx = 0, tanidx = 0;
  for(int i = 0; i < nverts; ++i)
  {
    v[idx] = points[i].x;
    v[idx+1] = points[i].y;
    v[idx+2] = points[i].z;
    n[idx] = normals[i].x;
    n[idx+1] = normals[i].y;
    n[idx+2] = normals[i].z;
    idx += 3;
    if(tc != NULL)
    {
      tang[tanidx] = tangents[i].x;
      tang[tanidx+1] = tangents[i].y;
      tang[tanidx+2] = tangents[i].z;
      tang[tanidx+3] = tangents[i].w;
      tanidx += 4;
      tc[tcidx] = texc[i].x;
      tc[tcidx+1] = texc[i].y;
      tcidx += 2;
    }
  }
  for(unsigned int i = 0; i < elements.size(); ++i)
  {
    el[i] = elements[i];
  }

  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);

  int nbuffers = 5;
  GLuint element_buffer = 4;
  if(tc == NULL)
  {
    nbuffers = 3;
    element_buffer = 2;
  }

  unsigned int vbo[5];
  glGenBuffers(nbuffers, vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, (3*nverts) * sizeof(float), v, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, (3*nverts) * sizeof(float), n, GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(1);

  if(tc != NULL)
  {
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, (2*nverts) * sizeof(float), tc, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
    glBufferData(GL_ARRAY_BUFFER, (4*nverts) * sizeof(float), tang, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(3);
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[element_buffer]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * _faces * sizeof(unsigned int), el, GL_STATIC_DRAW);

  delete[] v;
  delete[] n;
  if(tc != NULL)
  {
    delete[] tc;
    delete[] tang;
  }
  delete[] el;
}

void Mesh::TrimString(string& s)
{
  const char* white = " \t\n\r";
  size_t location;
  location = s.find_first_not_of(white);
  s.erase(0, location);
  location = s.find_last_not_of(white);
  s.erase(location + 1);
}