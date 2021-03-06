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
#include <exception>
#include <string>
#include <climits>
#include <ctime>

#include <GL/glew.h>
#include <GL/gl.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_net.h>

#include <tiny/os/sdlapplication.h>

using namespace tiny::os;

SDLApplication::SDLApplication(const int &a_screenWidth,
                               const int &a_screenHeight,
                               const bool &fullScreen,
                               const int &a_screenBPP,
                               const int &a_screenDepthBPP) :
    Application(),
    screenWidth(a_screenWidth),
    screenHeight(a_screenHeight),
    screenBPP(a_screenBPP),
    screenDepthBPP(a_screenDepthBPP),
    screenFlags(0),
    screen(0),
    wireframe(false),
    alDevice(0),
    alContext(0)
#ifdef ENABLE_OPENVR
    , vrHMD(0)
#endif
{
    //Initialise SDL.
    std::cerr << "Initialising SDL..." << std::endl;
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        std::cerr << "Unable to initialise SDL: " << SDL_GetError() << "!" << std::endl;
        throw std::exception();
    }

#ifdef ENABLE_OPENVR
    //Initialise OpenVR.
    std::cerr << "Initialising OpenVR..." << std::endl;

    vr::EVRInitError openvrError = vr::VRInitError_None;

    vrHMD = vr::VR_Init(&openvrError, vr::VRApplication_Scene);

    if (openvrError != vr::VRInitError_None)
    {
        std::cerr << "Unable to initialise OpenVR: " << vr::VR_GetVRInitErrorAsEnglishDescription(openvrError) << "!" << std::endl;
        throw std::exception();
    }
#endif
    
    //Create window.
    std::cerr << "Creating a " << screenWidth << "x" << screenHeight << " window at " << screenBPP << " bits per pixel..." << std::endl;
    
    screenFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
    
    if (fullScreen)
    {
        screenFlags |= SDL_WINDOW_FULLSCREEN;
    }
    
    screen = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, screenFlags);
    
    if (!screen)
    {
        std::cerr << "Unable to create window: " << SDL_GetError() << "!" << std::endl;
        throw std::exception();
    }
    
#ifndef ENABLE_OPENVR
    //Disable deprecated OpenGL functions and request version >= 3.2.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#else
    //Disable deprecated OpenGL functions < 4.1.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
#endif
    
    //Create rendering context.
    glContext = SDL_GL_CreateContext(screen);
    
    if (!glContext)
    {
        std::cerr << "Unable to create OpenGL context: " << SDL_GetError() << "!" << std::endl;
        throw std::exception();
    }

#ifndef ENABLE_OPENVR
    //Enable v-sync.
    SDL_GL_SetSwapInterval(1);
#endif
    
    if (fullScreen)
    {
        SDL_ShowCursor(SDL_DISABLE);
    }
    
    //Initialise OpenGL and GLEW.
    std::cerr << "Initialising OpenGL..." << std::endl;
    
    if (glewInit() == GLEW_OK)
    {
        initOpenGL();
    }
    else
    {
        std::cerr << "Unable to initialise GLEW!" << std::endl;
        throw std::exception();
    }
    
    //Clear screen.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SDL_GL_SwapWindow(screen);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SDL_GL_SwapWindow(screen);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SDL_GL_SwapWindow(screen);
    
    //Initialise image loading.
    std::cerr << "Initialising image loading..." << std::endl;
    
    if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) != (IMG_INIT_PNG | IMG_INIT_JPG))
    {
        std::cerr << "Unable to initialise SDL_image: " << IMG_GetError() << "!" << std::endl;
        throw std::exception();
    }
    
    //Initialise font loading.
    std::cerr << "Initialising font loading..." << std::endl;
    
    if (TTF_Init() != 0)
    {
        std::cerr << "Unable to initialise SDL_ttf: " << TTF_GetError() << "!" << std::endl;
        throw std::exception();
    }
    
    //Initialise networking.
    std::cerr << "Initialising networking..." << std::endl;
    
    if (SDLNet_Init() != 0)
    {
        std::cerr << "Unable to initialise SDL_net: " << SDLNet_GetError() << "!" << std::endl;
        throw std::exception();
    }
    
    //Initialise sound.
    std::cerr << "Initialising OpenAL..." << std::endl;
    
    initOpenAL();

#ifdef ENABLE_OPENVR
    std::cerr << "Initialising OpenVR compositor..." << std::endl;

    if (!vr::VRCompositor())
    {
        std::cerr << "Unable to initialise OpenVR compositor!" << std::endl;
        throw std::exception();
    }

    vrHMD->GetRecommendedRenderTargetSize(&screenWidthVR, &screenHeightVR);
#endif

    //Start main loop.
    std::cerr << "Initialisation complete." << std::endl;
    
    lastCount = SDL_GetTicks();
    curCount = SDL_GetTicks();
}

SDLApplication::~SDLApplication()
{
    //Shut down everything.
    std::cerr << "Shutting down SDL..." << std::endl;
    
#ifdef ENABLE_OPENVR
    vr::VR_Shutdown();
#endif

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(screen);
    
    exitOpenAL();
    SDLNet_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void SDLApplication::exitOpenAL()
{
    //Shut down OpenAL.
    alcMakeContextCurrent(0);
    alcCloseDevice(alDevice);
    alcDestroyContext(alContext);
}

void SDLApplication::initOpenAL()
{
    if (alGetString(AL_VERSION)) std::cerr << "Using OpenAL version " << alGetString(AL_VERSION) << "." << std::endl;
    else std::cerr << "Cannot determine OpenAL version!" << std::endl;
    
    //Use the default audio device.
    alDevice = alcOpenDevice(0);
    
    if (!alDevice)
    {
        std::cerr << "Unable to create default OpenAL device!" << std::endl;
        throw std::exception();
    }
    
    /* List all available audio devices. */
    /*
    if (alcIsExtensionPresent(0, "ALC_ENUMERATION_EXT") == AL_TRUE)
    {
        std::cerr << "Available OpenAL devices:" << std::endl << alcGetString(0, ALC_DEVICE_SPECIFIER) << std::endl;
    }
    */
    
    /* Create context and make it default. */
    alContext = alcCreateContext(alDevice, 0);
    
    if (!alContext)
    {
        std::cerr << "Unable to create OpenAL context!" << std::endl;
        throw std::exception();
    }
    
    alcMakeContextCurrent(alContext);
    
    const ALfloat ori[] = {0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f};
    
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alListenerfv(AL_ORIENTATION, ori);
    alListenerf(AL_GAIN, 1.0);
    
    alDopplerFactor(1.0);
    alDopplerVelocity(343.0f);
    
    //alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
    
    //Clear error state.
    alGetError();
}

void SDLApplication::initOpenGL()
{
    if (glGetString(GL_VERSION)) std::cerr << "Using OpenGL version " << glGetString(GL_VERSION) << "." << std::endl;
    else std::cerr << "Cannot determine OpenGL version!" << std::endl;

    if (glGetString(GL_SHADING_LANGUAGE_VERSION)) std::cerr << "Using GLSL version " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "." << std::endl;
    else std::cerr << "Cannot determine GLSL version!" << std::endl;

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearDepth(1.0);
    
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    
    glEnable(GL_DEPTH_TEST);    
    glDepthMask(GL_TRUE);
    
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(UINT_MAX);

    glDisable(GL_MULTISAMPLE);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    //Clear OpenGL error state.
    glGetError();
}

void SDLApplication::keyDownCallback(const int &keyIndex)
{
    //Default callback for keypresses.
    const SDL_Keycode k = static_cast<SDL_Keycode>(keyIndex);
    
    if (k == SDLK_ESCAPE)
    {
        stopRunning();
    }
    else if (k == SDLK_F1)
    {
        //Enable wireframe view.
        wireframe = !wireframe;
        
        if (wireframe)
        {
            glPolygonMode(GL_FRONT, GL_LINE);
            glPolygonMode(GL_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT, GL_FILL);
            glPolygonMode(GL_BACK, GL_FILL);
        }
    }
    else if (k == SDLK_PRINTSCREEN)
    {
        //Save a screenshot.
        SDL_Surface *image = SDL_CreateRGBSurface(SDL_SWSURFACE, screenWidth, screenHeight, 24, 255 << 0, 255 << 8, 255 << 16, 0);

        if (image)
        {
            //Save screenshot with timestamp.
            time_t rawTime;
            struct tm *timeInfo;
            char timeBuffer[80];
            
            time(&rawTime);
            timeInfo = localtime(&rawTime);
            strftime(timeBuffer, 80, "%Y%m%dT%H%M%S", timeInfo);
            
            const std::string outputFile = std::string("screenshot_") + std::string(timeBuffer) + std::string(".bmp");
            
            glFlush();
            glReadBuffer(GL_BACK);
            glReadPixels(0, 0, screenWidth, screenHeight, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
            SDL_SaveBMP(image, outputFile.c_str());
            SDL_FreeSurface(image);
        }
    }
}

void SDLApplication::keyUpCallback(const int &)
{
    
}

double SDLApplication::pollEvents()
{
    //Poll pending events.
    SDL_Event event;
    
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_KEYDOWN)
        {
            const SDL_Keycode k = event.key.keysym.sym;
            
            if (k >= 0 && k < 256)
            {
                pressedKeys[k] = true;
            }
            
            this->keyDownCallback(static_cast<int>(k));
        }
        else if (event.type == SDL_KEYUP)
        {
            const SDL_Keycode k = event.key.keysym.sym;
            
            if (k >= 0 && k < 256)
            {
                pressedKeys[k] = false;
            }
            
            this->keyUpCallback(static_cast<int>(k));
        }
        else if (event.type == SDL_QUIT)
        {
            stopRunning();
        }
    }
    
    //Find out the update time.
    const double dt = 1.0e-3*static_cast<double>((curCount = SDL_GetTicks()) - lastCount);
    
    lastCount = curCount;
    
    return dt;
}

void SDLApplication::paint()
{
    SDL_GL_SwapWindow(screen);
}

int SDLApplication::getScreenWidth() const
{
    return screenWidth;
}

int SDLApplication::getScreenHeight() const
{
    return screenHeight;
}

#ifdef ENABLE_OPENVR
int SDLApplication::getScreenWidthVR() const
{
    return screenWidthVR;
}

int SDLApplication::getScreenHeightVR() const
{
    return screenHeightVR;
}

vr::IVRSystem *SDLApplication::getHMDVR() const
{
    return vrHMD;
}
#endif

MouseState SDLApplication::getMouseState(const bool &reposition)
{
    int mouseX = 0, mouseY = 0;
    int mouseButtons = 0;
   
    if (reposition)
    {
        if (SDL_GetRelativeMouseMode() == SDL_FALSE)
        {
            SDL_SetRelativeMouseMode(SDL_TRUE);
        }
        
        mouseButtons = SDL_GetRelativeMouseState(&mouseX, &mouseY);
    }
    else
    {
        if (SDL_GetRelativeMouseMode() == SDL_TRUE)
        {
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }
        
        mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);
    }
    
    return MouseState(static_cast<float>(2*mouseX)/static_cast<float>(screenWidth),
                      static_cast<float>(2*mouseY)/static_cast<float>(screenHeight),
                      (mouseButtons & SDL_BUTTON(1) ? 1 : 0) | (mouseButtons & SDL_BUTTON(3) ? 2 : 0) | (mouseButtons & SDL_BUTTON(2) ? 4 : 0));
}

