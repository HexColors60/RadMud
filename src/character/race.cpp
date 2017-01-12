/// @file   race.cpp
/// @brief  Implement race methods.
/// @author Enrico Fraccaroli
/// @date   Aug 23 2014
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

#include "race.hpp"

#include "logger.hpp"
#include "mud.hpp"

Race::Race() :
    vnum(),
    name(),
    description(),
    abilities(),
    availableFaction(),
    player_allow(),
    tileSet(),
    tileId(),
    corpse(),
    naturalWeapon(),
    bodyParts()
{
    // Nothing to do.
}

Race::~Race()
{
//    Logger::log(LogLevel::Debug,
//                "Deleted race\t\t[%s]\t\t(%s)",
//                ToString(this->vnum),
//                this->name);
}

bool Race::check()
{
    assert(vnum > 0);
    assert(!name.empty());
    assert(!description.empty());
    assert(!naturalWeapon.empty());
    return true;
}

std::string Race::getShortDescription(bool capital)
{
    std::string shortDescription = this->article + " " + this->name;
    if (capital && !shortDescription.empty())
    {
        shortDescription[0] = static_cast<char>(toupper(shortDescription[0]));
    }
    return shortDescription;
}

bool Race::setAbilities(std::string source)
{
    // Check if it is received an empty string.
    if (source.empty()) return false;
    // Split the string.
    std::vector<std::string> abilityList = SplitString(source, ";");
    // Check if the number of chunks is correct.
    if (abilityList.size() != 5) return false;
    // For each chunk set the ability value.
    for (unsigned int it = 0; it < abilityList.size(); ++it)
    {
        auto ability = Ability(it);
        if (ability == Ability::None) return false;
        abilities[ability] = ToNumber<unsigned int>(abilityList[it]);
    }
    return true;
}

bool Race::setAvailableFactions(const std::string & source)
{
    // Check if it is received an empty string.
    if (source.empty()) return true;
    // Split the string.
    std::vector<std::string> factionList = SplitString(source, ";");
    // Check if the number of chunks is correct.
    if (factionList.empty()) return false;
    // For each chunk add the faction.
    for (auto it : factionList)
    {
        Faction * faction = Mud::instance().findFaction(ToNumber<int>(it));
        if (faction == nullptr)
        {
            Logger::log(LogLevel::Error, "Can't find the faction: %s.", it);
            return false;
        }
        availableFaction.push_back(faction);
    }
    return true;
}

unsigned int Race::getAbility(const Ability & ability) const
{
    for (auto it : abilities)
    {
        if (it.first == ability)
        {
            return it.second;
        }
    }
    return 0;
}

unsigned int Race::getAbilityLua(const unsigned int & abilityNumber)
{
    if (Ability::isValid(abilityNumber))
    {
        return this->getAbility(Ability(abilityNumber));
    }
    return 0;
}

bool Race::factionAllowed(int factionVnum)
{
    for (auto it : availableFaction)
    {
        if (it->vnum == factionVnum)
        {
            return true;
        }
    }
    return false;
}

void Race::luaRegister(lua_State * L)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Race>("Race")
        .addData("vnum", &Race::vnum)
        .addData("name", &Race::name)
        .addFunction("getAbility", &Race::getAbilityLua)
        .addData("available_faction", &Race::availableFaction)
        .endClass();
}

std::string Race::getTile()
{
    if (Formatter::getFormat() == Formatter::CLIENT)
    {
        return ToString(tileSet) + ":" + ToString(tileId);
    }
    return "c";
}
