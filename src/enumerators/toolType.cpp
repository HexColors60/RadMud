/// @file   toolType.cpp
/// @brief  Implement the toolType class.
/// @author Enrico Fraccaroli
/// @date   Nov 23 2016
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

#include "toolType.hpp"
#include "LuaBridge.hpp"

ToolType::ToolType() :
    toolType(None)
{
    // Nothing to do.
}

ToolType::ToolType(const unsigned int & _toolType) :
    toolType()
{
    if (_toolType == 1) toolType = Pickaxe;
    else if (_toolType == 10) toolType = WoodcutterAxe;
    else if (_toolType == 11) toolType = Saw;
    else if (_toolType == 12) toolType = PrecisionChisel;
    else if (_toolType == 20) toolType = Hammer;
    else if (_toolType == 21) toolType = PlaneChisel;
    else if (_toolType == 30) toolType = Forge;
    else if (_toolType == 31) toolType = Anvil;
    else if (_toolType == 32) toolType = BlacksmithHammer;
    else if (_toolType == 33) toolType = Bellows;
    else if (_toolType == 34) toolType = Crucible;
    else if (_toolType == 40) toolType = Tinderbox;
    else toolType = None;
}

ToolType::ToolType(const Enum & _toolType) :
    toolType(_toolType)
{
    // Nothing to do.
}

ToolType::ToolType(const std::string & _toolType) :
    toolType()
{
    if (_toolType == "Pickaxe") toolType = Pickaxe;
    else if (_toolType == "WoodcutterAxe") toolType = WoodcutterAxe;
    else if (_toolType == "Saw") toolType = Saw;
    else if (_toolType == "PrecisionChisel") toolType = PrecisionChisel;
    else if (_toolType == "Hammer") toolType = Hammer;
    else if (_toolType == "PlaneChisel") toolType = PlaneChisel;
    else if (_toolType == "Forge") toolType = Forge;
    else if (_toolType == "Anvil") toolType = Anvil;
    else if (_toolType == "BlacksmithHammer") toolType = BlacksmithHammer;
    else if (_toolType == "Bellows") toolType = Bellows;
    else if (_toolType == "Crucible") toolType = Crucible;
    else if (_toolType == "Tinderbox") toolType = Tinderbox;
    else toolType = None;
}

bool ToolType::isValid(const unsigned int & _toolType)
{
    return (_toolType >= 1) && (_toolType <= 40);
}

bool ToolType::isValid(const std::string & _toolType)
{
    if (_toolType == "Pickaxe") return true;
    if (_toolType == "WoodcutterAxe")return true;
    if (_toolType == "Saw")return true;
    if (_toolType == "PrecisionChisel")return true;
    if (_toolType == "Hammer") return true;
    if (_toolType == "PlaneChisel") return true;
    if (_toolType == "Forge") return true;
    if (_toolType == "Anvil") return true;
    if (_toolType == "BlacksmithHammer") return true;
    if (_toolType == "Bellows") return true;
    if (_toolType == "Crucible") return true;
    if (_toolType == "Tinderbox") return true;
    return false;
}

std::string ToolType::toString() const
{
    if (toolType == Pickaxe) return "Pickaxe";
    if (toolType == WoodcutterAxe) return "WoodcutterAxe";
    if (toolType == Saw) return "Saw";
    if (toolType == PrecisionChisel) return "PrecisionChisel";
    if (toolType == Hammer) return "Hammer";
    if (toolType == PlaneChisel) return "PlaneChisel";
    if (toolType == Forge) return "Forge";
    if (toolType == Anvil) return "Anvil";
    if (toolType == BlacksmithHammer) return "BlacksmithHammer";
    if (toolType == Bellows) return "Bellows";
    if (toolType == Crucible) return "Crucible";
    if (toolType == Tinderbox) return "Tinderbox";
    return "None";
}

unsigned int ToolType::toUInt() const
{
    return static_cast<unsigned int>(toolType);
}

bool ToolType::operator==(const ToolType & rhs) const
{
    return toolType == rhs.toolType;
}

bool ToolType::operator==(const ToolType::Enum & rhs) const
{
    return toolType == rhs;
}

bool ToolType::operator!=(const ToolType::Enum & rhs) const
{
    return toolType != rhs;
}

bool ToolType::operator<(const ToolType & rhs) const
{
    return toolType < rhs.toolType;
}

void ToolType::luaRegister(lua_State * L)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<ToolType>("ToolType")
        .addConstructor < void(*)(const std::string &)>()
        .addFunction("toString", &ToolType::toString)
        .endClass();
}