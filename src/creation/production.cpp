/// @file   production.cpp
/// @brief  Implmement production variables and methods.
/// @author Enrico Fraccaroli
/// @date   Aug 11 2015
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

#include "production.hpp"

#include "logger.hpp"
#include "mud.hpp"

Production::Production() :
    vnum(-1),
    name(),
    profession(),
    difficulty(),
    time(),
    assisted(),
    outcome(),
    quantity(),
    tools(),
    ingredients(),
    workbench(ToolType::NoType),
    material(ResourceType::NoType)
{
    // Nothing to do.
}

Production::~Production()
{
    Logger::log(
        LogLevel::Debug,
        "Deleted production\t[%s]\t\t(%s)",
        ToString(this->vnum),
        this->name);
}

bool Production::setOutcome(const std::string & source)
{
    if (source.empty())
    {
        Logger::log(LogLevel::Error, "No outcome set.");
        return false;
    }
    std::vector<std::string> outcomeList = SplitString(source, ";");
    // Flag used to determine if all goes well.
    bool correct = true;
    for (auto iterator : outcomeList)
    {
        std::vector<std::string> outcomeInfo = SplitString(iterator, "*");
        if (outcomeInfo.size() != 2)
        {
            correct = false;
        }

        outcome = Mud::instance().findItemModel(ToNumber<int>(outcomeInfo[0]));
        if (outcome == nullptr)
        {
            Logger::log(LogLevel::Error, "Can't find the Outcome :" + outcomeInfo[0]);
            correct = false;
            break;
        }
        int outcomeQuantity = ToNumber<int>(outcomeInfo[1]);
        if (outcomeQuantity <= 0)
        {
            Logger::log(
                LogLevel::Error,
                "Can't find the quantity of the Outcome :" + outcomeInfo[0]);
            correct = false;
            break;
        }
        quantity = static_cast<unsigned int>(outcomeQuantity);
    }
    return correct;
}

bool Production::setTool(const std::string & source)
{
    if (source.empty())
    {
        Logger::log(LogLevel::Error, "No tool set.");
        return false;
    }
    std::vector<std::string> toolList = SplitString(source, ";");
    // Flag used to determine if all goes well.
    bool correct = true;
    for (auto iterator : toolList)
    {
        ToolType toolType = static_cast<ToolType>(ToNumber<int>(iterator));
        if (toolType == ToolType::NoType)
        {
            Logger::log(LogLevel::Error, "Can't find the Tool Type:" + iterator);
            correct = false;
            break;
        }
        this->tools.insert(toolType);
    }
    return (!toolList.empty()) && correct;
}

bool Production::setIngredient(const std::string & source)
{
    if (source.empty())
    {
        Logger::log(LogLevel::Error, "No outcome set.");
        return true;
    }
    // Split the string of ingredients.
    std::vector<std::string> ingredientList = SplitString(source, ";");
    // Flag used to determine if all goes well.
    bool correct = true;
    for (auto iterator : ingredientList)
    {
        // Split the string with the information about the ingredient.
        std::vector<std::string> ingredientInfo = SplitString(iterator, "*");
        if (ingredientInfo.size() != 2)
        {
            correct = false;
            break;
        }
        ResourceType ingredient = static_cast<ResourceType>(ToNumber<int>(ingredientInfo[0]));
        if (ingredient == ResourceType::NoType)
        {
            Logger::log(LogLevel::Error, "Can't find the Ingredient :" + ingredientInfo[0]);
            correct = false;
            break;
        }

        int ingredientQuantity = ToNumber<int>(ingredientInfo[1]);
        if (ingredientQuantity == 0)
        {
            Logger::log(
                LogLevel::Error,
                "Can't find the quantity of the Outcome :" + ingredientInfo[0]);
            correct = false;
            break;
        }
        if (!this->ingredients.insert(std::make_pair(ingredient, ingredientQuantity)).second)
        {
            Logger::log(LogLevel::Error, "Cannot insert the ingredient");
            correct = false;
            break;
        }
    }
    return correct;
}

bool Production::check()
{
    assert(vnum >= 0);
    assert(!name.empty());
    assert(profession != nullptr);
    assert(difficulty > 0);
    assert(time > 0);
    assert(outcome != nullptr);
    assert(quantity > 0);
    assert(!tools.empty());
    for (auto it : tools)
    {
        assert(it != ToolType::NoType);
    }
    for (auto it : ingredients)
    {
        assert(it.first != ResourceType::NoType);
        assert(it.second > 0);
    }
    assert(material != ResourceType::NoType);
    return true;
}

std::string Production::getName()
{
    return ToLower(name);
}

std::string Production::getNameCapital()
{
    return name;
}
