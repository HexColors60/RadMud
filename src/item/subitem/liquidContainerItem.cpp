/// @file   liquidContainerItem.cpp
/// @author Enrico Fraccaroli
/// @date   Jan 03 2017
/// @copyright
/// Copyright (c) 2017 Enrico Fraccaroli <enrico.fraccaroli@gmail.com>
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

#include "liquidContainerItem.hpp"
#include "liquidContainerModel.hpp"
#include "sqliteDbms.hpp"
#include "formatter.hpp"

LiquidContainerItem::LiquidContainerItem() :
    liquidContent(),
    liquidQuantity()
{
    // Nothing to do.
}

LiquidContainerItem::~LiquidContainerItem()
{
    // Nothing to do.
}

void LiquidContainerItem::getSheet(Table & sheet) const
{
    // Call the function of the father class.
    Item::getSheet(sheet);
    // Add a divider.
    sheet.addDivider();
    sheet.addRow({"Liquid", "Quantity"});
    sheet.addRow({liquidContent->getNameCapital(), ToString(liquidQuantity)});
    sheet.addDivider();
}

double LiquidContainerItem::getWeight(bool) const
{
    // Add the default weight of the model.
    auto totalWeight = this->weight;
    if (!this->isEmpty())
    {
        totalWeight += liquidQuantity;
    }
    return totalWeight;
}

std::string LiquidContainerItem::lookContent()
{
    auto Italic = [](const std::string & s)
    {
        return Formatter::italic() + s + Formatter::reset();
    };
    std::string output;
    if (HasFlag(this->flags, ItemFlag::Closed))
    {
        output += Italic("It is closed.\n");
        if (!HasFlag(this->model->modelFlags, ModelFlag::CanSeeThrough))
        {
            return output + "\n";
        }
    }
    if (liquidContent == nullptr)
    {
        output += Formatter::italic() +
                  "It does not contain any liquid.\n" +
                  Formatter::reset();
    }
    else
    {
        // Cast the model to liquid container.
        auto containerModel =
            std::static_pointer_cast<LiquidContainerModel>(model);
        int percent = 0;
        if (HasFlag(containerModel->liquidFlags, LiqContainerFlag::Endless))
        {
            percent = 100;
        }
        else if (this->getUsedSpace() > 0)
        {
            percent = static_cast<int>((100.0 * this->getUsedSpace()) /
                                       this->getTotalSpace());
        }
        if (percent >= 100) output += "It's full of ";
        else if (percent >= 75) output += "It's half full of ";
        else if (percent >= 50) output += "Contains a discrete amount of ";
        else if (percent >= 25) output += "It contains a little bit of ";
        else if (percent >= 0) output += "It contains some drops of ";
        else output += "It's empty, but you can see some ";
        output += Formatter::cyan() +
                  liquidContent->getName() +
                  Formatter::reset() + ".\n";
    }
    return output;
}

bool LiquidContainerItem::isAContainer() const
{
    return true;
}

bool LiquidContainerItem::isEmpty() const
{
    return liquidContent == nullptr;
}

double LiquidContainerItem::getTotalSpace() const
{
    // Cast the model to liquid container.
    auto containerModel = std::static_pointer_cast<LiquidContainerModel>(model);
    // The base space.
    double spaceBase = containerModel->maxWeight;
    // Evaluate the result.
    return ((spaceBase + (spaceBase * quality.getModifier())) / 2);
}

double LiquidContainerItem::getUsedSpace() const
{
    return liquidQuantity;
}

bool LiquidContainerItem::canContainLiquid(Liquid * newLiquidContent,
                                           const double & newLiquidQuantity) const
{
    if (newLiquidQuantity > this->getFreeSpace())
    {
        return false;
    }
    if (!this->isEmpty())
    {
        return liquidContent->vnum == newLiquidContent->vnum;
    }
    return true;
}

bool LiquidContainerItem::pourIn(Liquid * newLiquidContent,
                                 const double & quantityToPourIn,
                                 bool updateDB)
{
    if (!this->canContainLiquid(newLiquidContent, quantityToPourIn))
    {
        return false;
    }
    if (liquidContent == nullptr)
    {
        // Set the liquid and the amount.
        liquidContent = newLiquidContent;
        liquidQuantity = quantityToPourIn;
    }
    else if (liquidContent->vnum == newLiquidContent->vnum)
    {
        // Increment the liquid amount.
        liquidQuantity += quantityToPourIn;
    }
    // Prepare the query arguments.
    if (updateDB)
    {
        SQLiteDbms::instance().insertInto(
            "ItemContentLiq",
            {
                ToString(vnum),
                ToString(liquidContent->vnum),
                ToString(liquidQuantity)
            }, false, true);
    }
    return true;
}

bool
LiquidContainerItem::pourOut(const double & quantityToPourOut, bool updateDB)
{
    if (model->getType() != ModelType::LiquidContainer)
    {
        return false;
    }
    // Cast the model to liquid container.
    auto containerModel = std::static_pointer_cast<LiquidContainerModel>(model);
    // If the item has an Endless provision of liquid, don't do any check.
    if (HasFlag(containerModel->liquidFlags, LiqContainerFlag::Endless))
    {
        return true;
    }
    // Check if the container has the necessary amount of liquid.
    if (liquidQuantity >= quantityToPourOut)
    {
        // Decrement the liquid amount.
        liquidQuantity -= quantityToPourOut;
        if (updateDB)
        {
            // Check if the quantity has dropped to zero.
            if (liquidQuantity < 0.1)
            {
                QueryList where;
                where.push_back(std::make_pair("container",
                                               ToString(vnum)));
                where.push_back(std::make_pair("content",
                                               ToString(liquidContent->vnum)));
                // If the container is empty, remove the entry from
                //  the liquid contained table.
                SQLiteDbms::instance().deleteFrom("ItemContentLiq", where);
                // Erase the key of the liquid.
                liquidContent = nullptr;
                liquidQuantity = 0.0;
            }
            else
            {
                SQLiteDbms::instance().insertInto(
                    "ItemContentLiq",
                    {
                        ToString(vnum),
                        ToString(liquidContent->vnum),
                        ToString(liquidQuantity)
                    }, false, true);
            }
        }
        return true;
    }
    return false;
}

void LiquidContainerItem::updateTicImpl()
{
    Item::updateTicImpl();
}

void LiquidContainerItem::updateHourImpl()
{
    Item::updateHourImpl();
}
