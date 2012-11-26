/*
Copyright 2012, Bas Fagginger Auer.

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
#include <algorithm>

#include <tiny/draw/camerarenderer.h>

using namespace tiny::draw;

CameraRenderer::CameraRenderer(const float &aspectRatio) :
    Renderer(),
    cameraToScreen(mat4::frustumMatrix(vec3(-0.07*aspectRatio, -0.07, 1.0e-1), vec3(0.07*aspectRatio, 0.07, 1.0e6))),
    worldToCamera(mat4::identityMatrix()),
    worldToScreen(mat4::identityMatrix()),
    cameraPosition(0.0f, 0.0f, 0.0f)
{
    updateCameraUniforms();
}

CameraRenderer::~CameraRenderer()
{

}

void CameraRenderer::setProjectionMatrix(const mat4 &matrix)
{
    cameraToScreen = matrix;
    updateCameraUniforms();
}

void CameraRenderer::setCamera(const vec3 &position, const vec4 &orientation)
{
    worldToCamera = mat4(orientation, position).inverted();
    cameraPosition = position;
    updateCameraUniforms();
}

void CameraRenderer::updateCameraUniforms()
{
    worldToScreen = cameraToScreen*worldToCamera;
    uniformMap.setVec3Uniform(cameraPosition, "cameraPosition");
    uniformMap.setMat4Uniform(worldToScreen, "worldToScreen");
}

