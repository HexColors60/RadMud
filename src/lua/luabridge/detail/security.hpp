/// @file   security.hpp
/// @copyright
/// Copyright (c) 2016 Enrico Fraccaroli <enrico.fraccaroli@gmail.com>
/// Permission is hereby granted, free of charge, to any person obtaining a
/// copy of this software and associated documentation files (the "Software"),
/// to deal in the Software without restriction, including without limitation
/// the rights to use, copy, modify, merge, publish, distribute, sublicense,
/// and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///     The above copyright notice and this permission notice shall be included
///     in all copies or substantial portions of the Software.
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
/// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
/// DEALINGS IN THE SOFTWARE.

#pragma once

namespace luabridge
{

/// @brief Class needed for secuirity options.
class Security
{
public:
    static bool hideMetatables()
    {
        return getSettings().hideMetatables;
    }

    static void setHideMetatables(bool shouldHide)
    {
        getSettings().hideMetatables = shouldHide;
    }

private:
    struct Settings
    {
        Settings() :
            hideMetatables(true)
        {
        }

        bool hideMetatables;
    };

    static Settings & getSettings()
    {
        static Settings settings;
        return settings;
    }
};

/// @brief Change whether or not metatables are hidden (on by default).
inline void setHideMetatables(bool shouldHide)
{
    Security::setHideMetatables(shouldHide);
}

}
