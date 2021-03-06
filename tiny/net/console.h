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

#include <iostream>
#include <string>
#include <vector>

namespace tiny
{

namespace net
{

class Console
{
    public:
        Console();
        virtual ~Console();
        
        void scrollUp();
        void scrollDown();
        void scrollDownFull();
        void keyDown(const int &);
        void addLine(const std::string &);
        std::string getText(const int &) const;

    protected:
        virtual void execute(const std::string &);
        virtual void update();

    private:
        std::vector<std::string> lines;
        std::string curLine;
        int lineScroll;
};

}

}

