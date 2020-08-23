/*
Copyright 2020, Bas Fagginger Auer.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <sstream>
#include <tiny/draw/voxelmap.h>

using namespace tiny;
using namespace tiny::draw;
using namespace tiny::draw::detail;

VoxelMap::VoxelMap(const int &a_nrSteps) :
    ScreenFillingSquare(),
    nrSteps(a_nrSteps)
{
    //Setup textures.
    uniformMap.addTexture("voxelTexture");
}

VoxelMap::~VoxelMap()
{

}

std::string VoxelMap::getFragmentShaderCode() const
{
    std::stringstream strm;
    
    strm <<
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform mat4 screenToWorld;\n"
"uniform mat4 worldToScreen;\n"
"uniform vec3 cameraPosition;\n"
"uniform vec2 inverseScreenSize;\n"
"\n"
"uniform sampler3D voxelTexture;\n"
"uniform float voxelSize;\n"
"\n"
"const float epsilon = 0.001f;\n"
"const float C = 1.0f, D = 1.0e8, E = 1.0f;\n"
"\n"
"out vec4 diffuse;\n"
"out vec4 worldNormal;\n"
"out vec4 worldPosition;\n"
"\n"
"void castRay(const vec3 direction, inout vec3 position, out vec3 normal, out vec3 material)\n"
"{\n"
"   //Prevent division by zero.\n"
"   const vec3 invDirection = 1.0f/max(abs(direction), epsilon);\n"
"   const vec3 directionSign = (step(-epsilon, direction) - 1.0f) + step(epsilon, direction);\n"
"   \n"
"   normal = vec3(0.0f, 0.0f, 0.0f);\n"
"   material = vec3(0.0f, 0.0f, 0.0f);\n"
"   \n"
"   ivec3 voxelIndices = ivec3(floor(position/voxelSize));\n"
"   vec4 voxel = texelFetch(voxelTexture, voxelIndices, 0);\n"
"   \n"
"   for (int i = 0; i < 0; ++i)\n"
"   {\n"
"       position += (255.0f*voxel.w*voxelSize - epsilon)*direction;\n"
"       voxelIndices = ivec3(floor(position/voxelSize));\n"
"       voxel = texelFetch(voxelTexture, voxelIndices, 0);\n"
"   }\n"
"   \n"
"   vec3 distances = (step(0.0f, direction) - fract(position/voxelSize))*directionSign*invDirection;\n"
"   bvec3 mask = bvec3(1, 0, 0);\n"
"   \n"
"   for (int i = 0; i < " << nrSteps << "; ++i)\n"
"   {\n"
"       voxel = texelFetch(voxelTexture, voxelIndices, 0);\n"
"       \n"
"       if (voxel.w == 0.0f) {\n"
"           normal = -normalize(directionSign*vec3(mask));\n"
"           material = voxel.xyz;\n"
"           return;\n"
"       }\n"
"       \n"
"       mask = lessThanEqual(distances.xyz, min(distances.yzx, distances.zxy));\n"
"       position += (dot(vec3(mask), distances)*voxelSize - epsilon)*direction;\n"
"       distances += vec3(mask)*invDirection;\n"
"       voxelIndices += ivec3(mask)*ivec3(directionSign);\n"
"   }\n"
"}\n"
"\n"
"void main(void)\n"
"{\n"
"   vec3 rayDir = normalize((screenToWorld*vec4(2.0f*(gl_FragCoord.xy*inverseScreenSize - 1.0f), 0.0f, 1.0f)).xyz - cameraPosition);\n"
"   vec3 position = cameraPosition;\n"
"   vec3 normal;\n"
"   vec3 diffuse;\n"
"   \n"
"   castRay(rayDir, position, normal, diffuse);\n"
"   \n"
"   if (normal == vec3(0.0f, 0.0f, 0.0f))\n"
"   {\n"
"       discard;\n"
"   }\n"
"   \n"
"   float cameraDepth = (worldToScreen*vec4(position, 1.0f)).z;\n"
"   \n"
"   worldNormal = vec4(normal, 0.0f);"
"   worldPosition = vec4(position, cameraDepth);\n"
"   \n"
"   gl_FragDepth = (log(C*cameraDepth + E) / log(C*D + E));\n"
"}\n\0";
    
    return strm.str();
}

