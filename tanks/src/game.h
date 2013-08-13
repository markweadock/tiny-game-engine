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
#pragma once

#include <string>
#include <map>

#include <tinyxml.h>

#include <tiny/os/application.h>

#include <tiny/draw/staticmesh.h>
#include <tiny/draw/icontexture2d.h>
#include <tiny/draw/iconhorde.h>
#include <tiny/draw/texture2d.h>
#include <tiny/draw/effects/sunsky.h>
#include <tiny/draw/worldrenderer.h>

#include "network.h"
#include "terrain.h"
#include "tank.h"

namespace tanks
{

class Player
{
    public:
        Player();
        ~Player();
        
        unsigned int tankIndex;
};

class TanksGame
{
    public:
        TanksGame(const tiny::os::Application *, const std::string &);
        ~TanksGame();
        
        void update(tiny::os::Application *, const float &);
        void render();
        
        TanksMessageTranslator *getTranslator() const;
        bool applyMessage(const unsigned int &, const Message &);
        void clear();
        void updateConsole() const;
        
    private:
        bool msgHost(const unsigned int &, std::ostream &, bool &, const unsigned int &);
        bool msgJoin(const unsigned int &, std::ostream &, bool &, const unsigned int &, const unsigned int &, const unsigned int &);
        bool msgDisconnect(const unsigned int &, std::ostream &, bool &);
        bool msgAddPlayer(const unsigned int &, std::ostream &, bool &, const unsigned int &);
        bool msgRemovePlayer(const unsigned int &, std::ostream &, bool &, const unsigned int &);
        bool msgWelcomePlayer(const unsigned int &, std::ostream &, bool &, const unsigned int &);
        bool msgTerrainOffset(const unsigned int &, std::ostream &, bool &, const tiny::vec2 &);
        bool msgAddTank(const unsigned int &, std::ostream &, bool &, const unsigned int &, const unsigned int &, const tiny::vec2 &);
        bool msgRemoveTank(const unsigned int &, std::ostream &, bool &, const unsigned int &);
        bool msgUpdateTank(const unsigned int &, std::ostream &, bool &, const unsigned int &, const unsigned int &, const tiny::vec3 &, const tiny::vec4 &, const tiny::vec3 &, const tiny::vec3 &);
        bool msgSetPlayerTank(const unsigned int &, std::ostream &, bool &, const unsigned int &, const unsigned int &);
        bool msgPlayerSpawnRequest(const unsigned int &, std::ostream &, bool &, const unsigned int &);
        
        void readResources(const std::string &);
        void readConsoleResources(const std::string &, TiXmlElement *);
        void readSkyResources(const std::string &, TiXmlElement *);
        
        //Renderer.
        const double aspectRatio;
        tiny::draw::WorldRenderer *renderer;
        
        tiny::vec3 cameraPosition;
        tiny::vec4 cameraOrientation;
        
        //Console and font.
        bool consoleMode;
        tiny::draw::ScreenIconHorde *font;
        tiny::draw::IconTexture2D *fontTexture;
        
        //Sky and atmosphere.
        tiny::draw::StaticMesh *skyBoxMesh;
        tiny::draw::RGBTexture2D *skyBoxDiffuseTexture;
        tiny::draw::RGBTexture2D *skyGradientTexture;
        tiny::draw::effects::SunSky *skyEffect;
        
        TanksTerrain *terrain;
        
        std::map<unsigned int, TankType *> tankTypes;
        std::map<unsigned int, TankInstance> tanks;
        unsigned int lastTankIndex;
        
        //Networking.
        TanksMessageTranslator * const translator;
        TanksConsole * const console;
        TanksHost *host;
        TanksClient *client;
        unsigned int ownPlayerIndex;
        
        std::map<unsigned int, Player> players;
};

} //namespace tanks

