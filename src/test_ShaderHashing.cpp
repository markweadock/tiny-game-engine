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
#include <iostream>
#include <vector>
#include <string>
#include <exception>

#include <config.h>

#include <tiny/os/application.h>
#include <tiny/os/sdlapplication.h>

#include <tiny/img/image.h>
#include <tiny/img/io/image.h>
#include <tiny/mesh/staticmesh.h>
#include <tiny/mesh/io/staticmesh.h>

#include <tiny/draw/staticmesh.h>
#include <tiny/draw/effects/lambert.h>
#include <tiny/draw/worldrenderer.h>

using namespace std;
using namespace tiny;

os::Application *application = 0;

draw::WorldRenderer *worldRenderer = 0;

const unsigned int N = 3;
draw::StaticMesh *testMeshes[N];
draw::RGBTexture2D *testDiffuseTextures[N];
draw::RGBTexture2D *testNormalTextures[N];

draw::Renderable *screenEffect = 0;

vec3 cameraPosition = vec3(0.0f, 0.0f, 3.0f);
vec4 cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);

void setup()
{
    //Create a test meshes.
    testMeshes[0] = new draw::StaticMesh(mesh::io::readStaticMesh(DATA_DIRECTORY + "mesh/tree0_trunk.obj"));
    testDiffuseTextures[0] = new draw::RGBTexture2D(img::io::readImage(DATA_DIRECTORY + "img/tree0_trunk.png"));
    testNormalTextures[0] = new draw::RGBTexture2D(img::io::readImage(DATA_DIRECTORY + "img/tree0_trunk_normal.png"));
    
    testMeshes[1] = new draw::StaticMesh(mesh::io::readStaticMesh(DATA_DIRECTORY + "mesh/tank1.dae"));
    testDiffuseTextures[1] = new draw::RGBTexture2D(img::Image::createTestImage());
    testNormalTextures[1] = new draw::RGBTexture2D(img::Image::createUpNormalImage());
    
    testMeshes[2] = new draw::StaticMesh(mesh::io::readStaticMesh(DATA_DIRECTORY + "mesh/cubes.dae"));
    testDiffuseTextures[2] = new draw::RGBTexture2D(img::Image::createSolidImage());
    testNormalTextures[2] = new draw::RGBTexture2D(img::Image::createUpNormalImage());
    
    for (unsigned int i = 0; i < N; ++i)
    {
        testMeshes[i]->setDiffuseTexture(*testDiffuseTextures[i]);
        testMeshes[i]->setNormalTexture(*testNormalTextures[i]);
    }
    
    //Render only diffuse colours to the screen.
    screenEffect = new draw::effects::Lambert();
    
    //Create a renderer and add the test and the diffuse rendering effect to it.
    worldRenderer = new draw::WorldRenderer(application->getScreenWidth(), application->getScreenHeight());
    
    for (unsigned int i = 0; i < N; ++i)
    {
        worldRenderer->addWorldRenderable(i, testMeshes[i]);
    }
    
    worldRenderer->addScreenRenderable(0, screenEffect, false, false);
}

void cleanup()
{
    delete worldRenderer;
    
    delete screenEffect;
    
    for (unsigned int i = 0; i < N; ++i)
    {
        delete testMeshes[i];
        delete testDiffuseTextures[i];
        delete testNormalTextures[i];
    }
}

void update(const double &dt)
{
    //Move the camera around.
    application->updateSimpleCamera(dt, cameraPosition, cameraOrientation);
    
    //Tell the world renderer that the camera has changed.
    worldRenderer->setCamera(cameraPosition, cameraOrientation);
}

void render()
{
    worldRenderer->clearTargets();
    worldRenderer->render();
}

int main(int, char **)
{
    try
    {
        application = new os::SDLApplication(SCREEN_WIDTH, SCREEN_HEIGHT);
        setup();
    }
    catch (std::exception &e)
    {
        cerr << "Unable to start application!" << endl;
        return -1;
    }
    
    while (application->isRunning())
    {
        update(application->pollEvents());
        render();
        application->paint();
    }
    
    cleanup();
    delete application;
    
    cerr << "Goodbye." << endl;
    
    return 0;
}

