#pragma once

#include "drawable.h"
#include <glm/glm.hpp>
using glm::vec3;
using glm::mat3;
using glm::mat4;

class VBOTeapot : public Drawable
{
private:
    unsigned int _faces;

    void GeneratePatches(float * v, float * n, float *tc, unsigned int* el, int grid);
    void BuildPatchReflect(int patchNum,
                           float *B, float *dB,
                           float *v, float *n, float *, unsigned int *el,
                           int &index, int &elIndex, int &, int grid,
                           bool reflectX, bool reflectY);
    void BuildPatch(vec3 patch[][4],
                    float *B, float *dB,
                    float *v, float *n,float *, unsigned int *el,
                    int &index, int &elIndex, int &, int grid, mat3 reflect, bool invertNormal);
    void GetPatch( int patchNum, vec3 patch[][4], bool reverseV );

    void ComputeBasisFunctions( float * B, float * dB, int grid );
    vec3 Evaluate( int gridU, int gridV, float *B, vec3 patch[][4] );
    vec3 EvaluateNormal( int gridU, int gridV, float *B, float *dB, vec3 patch[][4] );
    void MoveLid(int,float *,mat4);

public:
    VBOTeapot(int grid, mat4 lidTransform);

    void Render();
};
