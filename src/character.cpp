/// @file   character.cpp
/// @brief  Implements all the methods need to manipulate a character.
/// @author Enrico Fraccaroli
/// @date   Aug 23 2014
/// @copyright
/// Copyright (c) 2014, 2015, 2016 Enrico Fraccaroli <enrico.fraccaroli@gmail.com>
/// Permission to use, copy, modify, and distribute this software for any
/// purpose with or without fee is hereby granted, provided that the above
/// copyright notice and this permission notice appear in all copies.
///
/// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
/// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
/// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
/// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
/// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
/// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
/// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

// Basic Include.
#include "character.hpp"

#include <cstdlib>
#include <typeinfo>
#include <algorithm>
#include <list>
#include <sstream>

// Other Include.
#include "mud.hpp"
#include "logger.hpp"
#include "constants.hpp"
#include "lua/lua_script.hpp"
#include "sqlite/SQLiteDbms.hpp"
#include "luabridge/LuaBridge.h"
#include "command/command.hpp"

#include "model/toolModel.hpp"
#include "model/armorModel.hpp"
#include "model/shieldModel.hpp"
#include "model/resourceModel.hpp"

using namespace std;

Character::Character() :
        name(),
        description(),
        gender(),
        weight(),
        level(),
        flags(),
        race(),
        faction(),
        health(),
        stamina(),
        hunger(100),
        thirst(100),
        room(),
        inventory(),
        equipment(),
        posture(CharacterPosture::Stand),
        effects(),
        L(luaL_newstate()),
        opponents(this),
        action(std::make_shared<GeneralAction>(this))
{
    // Nothing to do.
}

Character::~Character()
{
    // Nothing to do.
}

bool Character::check() const
{
    bool safe = true;
    safe &= CorrectAssert(!name.empty());
    safe &= CorrectAssert(!description.empty());
    safe &= CorrectAssert(weight > 0);
    safe &= CorrectAssert(race != nullptr);
    safe &= CorrectAssert(faction != nullptr);
    safe &= CorrectAssert(this->getAbility(Ability::Strength, false) > 0);
    safe &= CorrectAssert(this->getAbility(Ability::Strength, false) <= 60);
    safe &= CorrectAssert(this->getAbility(Ability::Agility, false) > 0);
    safe &= CorrectAssert(this->getAbility(Ability::Agility, false) <= 60);
    safe &= CorrectAssert(this->getAbility(Ability::Perception, false) > 0);
    safe &= CorrectAssert(this->getAbility(Ability::Perception, false) <= 60);
    safe &= CorrectAssert(this->getAbility(Ability::Constitution, false) > 0);
    safe &= CorrectAssert(this->getAbility(Ability::Constitution, false) <= 60);
    safe &= CorrectAssert(this->getAbility(Ability::Intelligence, false) > 0);
    safe &= CorrectAssert(this->getAbility(Ability::Intelligence, false) <= 60);
    safe &= CorrectAssert(thirst > 0);
    safe &= CorrectAssert(room != nullptr);
    safe &= CorrectAssert(L != nullptr);
    return safe;
}

bool Character::isMobile() const
{
    return false;
}

bool Character::isPlayer() const
{
    return false;
}

void Character::getSheet(Table & sheet) const
{
    // Add the columns.
    sheet.addColumn("Attribute", StringAlign::Left);
    sheet.addColumn("Value", StringAlign::Left);
    // Set the values.
    sheet.addRow( { "Proper Noun", this->name });
    sheet.addRow( { "Description", this->description });
    sheet.addRow( { "Gender", GetGenderTypeName(this->gender) });
    sheet.addRow( { "Weight", ToString(this->weight) });
    sheet.addRow( { "Level", ToString(this->level) });
    sheet.addRow( { "Flags", GetCharacterFlagString(this->flags) });
    if (CorrectAssert(this->race != nullptr))
    {
        sheet.addRow( { "Race", this->race->name });
    }
    else
    {
        sheet.addRow( { "Race", "NONE" });
    }
    if (CorrectAssert(this->faction != nullptr))
    {
        sheet.addRow( { "Faction", this->faction->name });
    }
    else
    {
        sheet.addRow( { "Faction", "NONE" });
    }
    sheet.addRow( { "Health", ToString(this->getHealth()) });
    sheet.addRow( { "Stamina", ToString(this->getStamina()) });
    sheet.addRow( { "Hunger", ToString(this->getHunger()) });
    sheet.addRow( { "Thirst", ToString(this->getThirst()) });
    sheet.addRow(
        { "Strength", ToString(this->getAbility(Ability::Strength, false)) + " ["
            + ToString(this->effects.getAbilityModifier(Ability::Strength)) + "]" });
    sheet.addRow(
        { "Agility", ToString(this->getAbility(Ability::Agility, false)) + " ["
            + ToString(this->effects.getAbilityModifier(Ability::Agility)) + "]" });
    sheet.addRow(
        { "Perception", ToString(this->getAbility(Ability::Perception, false)) + " ["
            + ToString(this->effects.getAbilityModifier(Ability::Perception)) + "]" });
    sheet.addRow(
        { "Constitution", ToString(this->getAbility(Ability::Constitution, false)) + " ["
            + ToString(this->effects.getAbilityModifier(Ability::Constitution)) + "]" });
    sheet.addRow(
        { "Intelligence", ToString(this->getAbility(Ability::Intelligence, false)) + " ["
            + ToString(this->effects.getAbilityModifier(Ability::Intelligence)) + "]" });
    if (CorrectAssert(this->room != nullptr))
    {
        sheet.addRow( { "Room", this->room->name + " [" + ToString(this->room->vnum) + "]" });
    }
    else
    {
        sheet.addRow( { "Room", "NONE" });
    }
    sheet.addRow( { "Posture", GetPostureName(this->posture) });
    sheet.addRow( { "Action", action->getDescription() });
    sheet.addDivider();
    sheet.addRow( { "## Equipment", "## Inventory" });
    for (size_t it = 0; it < std::max(this->inventory.size(), this->equipment.size()); ++it)
    {
        std::string equipmentItem, inventoryItem;
        if (it < this->equipment.size())
        {
            equipmentItem = this->equipment.at(it)->getName();
        }
        if (it < this->inventory.size())
        {
            inventoryItem = this->inventory.at(it)->getName();
        }
        sheet.addRow( { equipmentItem, inventoryItem });
    }
    sheet.addDivider();
    sheet.addRow( { "## Effect Name", "## Expires In" });
    for (EffectList::const_iterator it = this->effects.begin(); it != this->effects.end(); ++it)
    {
        sheet.addRow( { it->name, ToString(it->expires) });
    }
}

std::string Character::getName() const
{
    if (this->isPlayer())
    {
        return ToLower(this->name);
    }
    else if (this->isMobile())
    {
        const Mobile * mobile = dynamic_cast<const Mobile *>(this);
        return ToLower(mobile->staticdesc);
    }
    else
    {
        return "No name";
    }
}

std::string Character::getNameCapital() const
{
    return ToCapitals(this->getName());
}

std::string Character::getStaticDesc() const
{
    // Name
    std::string desc(this->getNameCapital());
    // Posture
    desc += " is";
    if (posture != CharacterPosture::Stand)
    {
        desc += " " + GetPostureName(posture);
    }
    desc += " here";
    if (this->action->getType() != ActionType::Wait)
    {
        desc += ", " + this->getSubjectPronoun() + " is ";
        desc += this->action->getDescription();
    }
    desc += ".";
    return desc;
}

string Character::getSubjectPronoun() const
{
    if (gender == GenderType::Male) return "he";
    if (gender == GenderType::Female) return "she";
    return "it";
}

string Character::getPossessivePronoun() const
{
    if (gender == GenderType::Male) return "his";
    if (gender == GenderType::Female) return "her";
    return "its";
}

std::string Character::getObjectPronoun() const
{
    if (gender == GenderType::Male) return "him";
    if (gender == GenderType::Female) return "her";
    return "it";
}

bool Character::setAbility(const Ability & ability, const unsigned int & value)
{
    if (value <= 60)
    {
        abilities[ability] = value;
        return true;
    }
    else
    {
        return false;
    }
}

unsigned int Character::getAbility(const Ability & ability, bool withEffects) const
{
    auto overall = 0;
    // Try to find the ability value.
    auto it = abilities.find(ability);
    if (it != abilities.end())
    {
        overall = static_cast<int>(it->second);
    }
    // Add the effects if needed.
    if (withEffects)
    {
        overall += effects.getAbilityModifier(ability);
        // Prune the value if exceed the boundaries.
        if (overall <= 0)
        {
            overall = 0;
        }
        else if (overall > 60)
        {
            overall = 60;
        }
    }
    return static_cast<unsigned int>(overall);
}

unsigned int Character::getAbilityModifier(const Ability & ability, bool withEffects) const
{
    return GetAbilityModifier(this->getAbility(ability, withEffects));
}

unsigned int Character::getAbilityLog(
    const Ability & ability,
    const double & base,
    const double & multiplier,
    const bool & withEffects) const
{
    double result = base;
    double modifier = static_cast<double>(this->getAbilityModifier(ability, withEffects));
    if (modifier > 0)
    {
        if (modifier > 25)
        {
            modifier = 25;
        }
        result += multiplier * log10(modifier);
    }
    return static_cast<unsigned int>(result);
}

bool Character::setHealth(const unsigned int & value, const bool & force)
{
    unsigned int maximum = this->getMaxHealth();
    if (value > maximum)
    {
        if (force)
        {
            this->health = maximum;
            return true;
        }
        else
        {
            return false;
        }
    }
    this->health = value;
    return true;
}

bool Character::addHealth(const unsigned int & value, const bool & force)
{
    unsigned int maximum = this->getMaxHealth();
    unsigned int result = this->health + value;
    if (result > maximum)
    {
        if (force)
        {
            result = maximum;
        }
        else
        {
            return false;
        }
    }
    this->health = result;
    return true;
}

bool Character::remHealth(const unsigned int & value, const bool & force)
{
    int result = static_cast<int>(this->health) - static_cast<int>(value);
    if (result < 0)
    {
        if (force)
        {
            result = 0;
        }
        else
        {
            return false;
        }
    }
    this->health = static_cast<unsigned int>(result);
    return true;
}

unsigned int Character::getHealth() const
{
    return this->health;
}

unsigned int Character::getMaxHealth(bool withEffects) const
{
    // Value = 40 + (10 * AbilityModifier(Constitution))
    unsigned int BASE = 40 + (10 * this->getAbilityModifier(Ability::Constitution));
    if (!withEffects)
    {
        return BASE;
    }
    int overall = static_cast<int>(BASE) + effects.getHealthMod();
    if (overall <= 0)
    {
        return 0;
    }
    else
    {
        return static_cast<unsigned int>(overall);
    }
}

string Character::getHealthCondition(Character * character)
{
    string sent_be, sent_have;
    // Determine who is the examined character.
    if (character != nullptr && character != this)
    {
        sent_be = "is";
        sent_have = "has";
    }
    else
    {
        character = this;
        sent_be = "are";
        sent_have = "have";
    }
    // Determine the percentage of current health.
    auto percent = (100 * character->health) / character->getMaxHealth();
    // Determine the correct description.
    if (percent >= 100) return sent_be + " in perfect health";
    else if (percent >= 90) return sent_be + " slightly scratched";
    else if (percent >= 80) return sent_have + " a few bruises";
    else if (percent >= 70) return sent_have + " some cuts";
    else if (percent >= 60) return sent_have + " several wounds";
    else if (percent >= 50) return sent_have + " many nasty wounds";
    else if (percent >= 40) return sent_be + " bleeding freely";
    else if (percent >= 30) return sent_be + " covered in blood";
    else if (percent >= 20) return sent_be + " leaking guts";
    else if (percent >= 10) return sent_be + " almost dead";
    else return sent_be + " DYING";
}

bool Character::setStamina(const unsigned int & value, const bool & force)
{
    unsigned int maximum = this->getMaxStamina();
    if (value > maximum)
    {
        if (force)
        {
            this->stamina = maximum;
            return true;
        }
        else
        {
            return false;
        }
    }
    this->stamina = value;
    return true;
}

bool Character::addStamina(const unsigned int & value, const bool & force)
{
    unsigned int maximum = this->getMaxStamina();
    unsigned int result = this->stamina + value;
    if (result > maximum)
    {
        if (force)
        {
            result = maximum;
        }
        else
        {
            return false;
        }
    }
    this->stamina = result;
    return true;
}

bool Character::remStamina(const unsigned int & value, const bool & force)
{
    int result = static_cast<int>(this->stamina) - static_cast<int>(value);
    if (result < 0)
    {
        if (force)
        {
            result = 0;
        }
        else
        {
            return false;
        }
    }
    this->stamina = static_cast<unsigned int>(result);
    return true;
}

unsigned int Character::getStamina() const
{
    return this->stamina;
}

unsigned int Character::getMaxStamina(bool withEffects) const
{
    // Value = 50 + (15 * AbilityModifier(Constitution))
    unsigned int BASE = 50 + (15 * this->getAbilityModifier(Ability::Constitution));
    if (!withEffects)
    {
        return BASE;
    }
    int overall = static_cast<int>(BASE) + effects.getStaminaMod();
    if (overall <= 0)
    {
        return 0;
    }
    else
    {
        return static_cast<unsigned int>(overall);
    }
}

std::string Character::getStaminaCondition()
{
    // Determine the percentage of current health.
    auto percent = (100 * this->stamina) / this->getMaxStamina(true);
    // Determine the correct description.
    if (percent >= 100) return "look fresh as a daisy";
    else if (percent >= 80) return "are slightly weary";
    else if (percent >= 60) return "are quite tired";
    else if (percent >= 40) return "need to rest";
    else if (percent >= 20) return "need a good night of sleep";
    else return "are too tired, you can't even blink your eyes";
}

int Character::getViewDistance() const
{
    // Value = 3 + LogMod(PER)
    // MIN   = 3.0
    // MAX   = 7.2
    return 3 + static_cast<int>(this->getAbilityLog(Ability::Perception, 0.0, 1.0));
}

void Character::setAction(std::shared_ptr<GeneralAction> _action)
{
    this->action = _action;
}

std::shared_ptr<GeneralAction> Character::getAction()
{
    return this->action;
}

bool Character::canMoveTo(Direction direction, std::string & error) const
{
    // Check if the character is in a no-walk position.
    if (posture == CharacterPosture::Rest || posture == CharacterPosture::Sit)
    {
        error = "You first need to stand up.";
        return false;
    }
    // Find the exit to the destination.
    std::shared_ptr<Exit> destExit = room->findExit(direction);
    if (destExit == nullptr)
    {
        error = "You cannot go that way.";
        return false;
    }
    // Check if the actor has enough stamina to execute the action.
    if (this->getConsumedStaminaFor(ActionType::Move) > this->getStamina())
    {
        error = "You are too tired to move.\n";
        return false;
    }
    // If the direction is upstairs, check if there is a stair.
    if (direction == Direction::Up)
    {
        Logger::log(LogLevel::Debug, "Flags :" + ToString(destExit->flags));
        if (!HasFlag(destExit->flags, ExitFlag::Stairs))
        {
            error = "You can't go upstairs, there are no stairs.";
            return false;
        }
    }
    // Check if the destination is correct.
    if (destExit->destination == nullptr)
    {
        error = "That direction can't take you anywhere.";
        return false;
    }
    // Check if the destination is bocked by a door.
    Item * door = destExit->destination->findDoor();
    if (door != nullptr)
    {
        if (HasFlag(door->flags, ItemFlag::Closed))
        {
            error = "Maybe you have to open that door first.";
            return false;
        }
    }
    // Check if the destination has a floor.
    std::shared_ptr<Exit> destDown = destExit->destination->findExit(Direction::Down);
    if (destDown != nullptr)
    {
        if (!HasFlag(destDown->flags, ExitFlag::Stairs))
        {
            error = "Do you really want to fall in that pit?";
            return false;
        }
    }
    // Check if the destination is forbidden for mobiles.
    if (this->isMobile())
    {
        if (HasFlag(destExit->flags, ExitFlag::NoMob))
        {
            return false;
        }
    }
    return true;
}

void Character::moveTo(
    Room * destination,
    const std::string & msgDepart,
    const std::string & msgArrive,
    const std::string & msgChar)
{
    // Check if the current room exist.
    if (room == nullptr)
    {
        Logger::log(LogLevel::Error, "Character::moveTo: room is a NULLPTR.");
        return;
    }
    // Check if the destination exist.
    if (destination == nullptr)
    {
        Logger::log(LogLevel::Error, "Character::moveTo: DESTINATION is a NULLPTR.");
        return;
    }

    // Activate the entrance event for every mobile in the room.
    for (auto mobile : room->getAllMobile(this))
    {
        if (mobile == this)
        {
            continue;
        }
        if (mobile->canSee(this))
        {
            mobile->triggerEventExit(this);
        }
    }

    // Show a message to the character, if is set.
    if (!msgChar.empty())
    {
        this->sendMsg(msgChar);
    }

    // Set the list of exceptions.
    CharacterVector exceptions;
    exceptions.push_back(this);
    // Tell others where the character went and remove s/he from the old room.
    room->sendToAll(msgDepart, exceptions);

    // Remove the player from the current room.
    room->removeCharacter(this);

    // Add the character to the destionation room.
    destination->addCharacter(this);

    // Look around new room.
    this->doCommand("look");

    // Tell others s/he has arrived and move the character to the new room.
    destination->sendToAll(msgArrive, exceptions);

    // Activate the entrance event for every mobile in the room.
    for (auto mobile : room->getAllMobile(this))
    {
        if (mobile == this)
        {
            continue;
        }
        if (mobile->canSee(this))
        {
            mobile->triggerEventEnter(this);
        }
    }
}

Item* Character::findInventoryItem(string search_parameter, int & number)
{
    for (auto iterator : inventory)
    {
        if (iterator->hasKey(ToLower(search_parameter)))
        {
            if (number == 1)
            {
                return iterator;
            }
            number -= 1;
        }
    }
    return nullptr;
}

Item* Character::findEquipmentItem(string search_parameter, int & number)
{
    for (auto * iterator : equipment)
    {
        if (iterator->hasKey(ToLower(search_parameter)))
        {
            if (number == 1)
            {
                return iterator;
            }
            number -= 1;
        }
    }
    return nullptr;
}

Item * Character::findEquipmentSlotItem(EquipmentSlot slot) const
{
    for (auto iterator : equipment)
    {
        if (iterator->getCurrentSlot() == slot)
        {
            return iterator;
        }
    }
    return nullptr;
}

Item * Character::findEquipmentSlotTool(EquipmentSlot slot, ToolType type)
{
    Item * tool = findEquipmentSlotItem(slot);
    if (tool != nullptr)
    {
        if (tool->model != nullptr)
        {
            if (tool->model->getType() == ModelType::Tool)
            {
                ToolModel * toolModel = tool->model->toTool();
                if (toolModel->toolType == type)
                {
                    return tool;
                }
            }
        }
    }
    return nullptr;
}

Item * Character::findNearbyItem(std::string itemName, int & number)
{
    Item * item;
    if ((item = findInventoryItem(itemName, number)) == nullptr)
    {
        if ((item = findEquipmentItem(itemName, number)) == nullptr)
        {
            item = room->findItem(itemName, number);
        }
    }
    return item;
}

Item * Character::findNearbyTool(const ToolType & toolType, const ItemVector & exceptions,
bool searchRoom,
bool searchInventory,
bool searchEquipment)
{
    if (searchRoom)
    {
        for (auto iterator : room->items)
        {
            if (iterator->model->getType() == ModelType::Tool)
            {
                ToolModel * toolModel = iterator->model->toTool();
                if (toolModel->toolType == toolType)
                {
                    auto findIt = std::find(exceptions.begin(), exceptions.end(), iterator);
                    if (findIt == exceptions.end())
                    {
                        return iterator;
                    }
                }
            }
        }
    }
    if (searchInventory)
    {
        for (auto iterator : this->inventory)
        {
            if (iterator->model->getType() == ModelType::Tool)
            {
                ToolModel * toolModel = iterator->model->toTool();
                if (toolModel->toolType == toolType)
                {
                    auto findIt = std::find(exceptions.begin(), exceptions.end(), iterator);
                    if (findIt == exceptions.end())
                    {
                        return iterator;
                    }
                }
            }
        }
    }
    if (searchEquipment)
    {
        for (auto iterator : this->equipment)
        {
            if (iterator->model->getType() == ModelType::Tool)
            {
                ToolModel * toolModel = iterator->model->toTool();
                if (toolModel->toolType == toolType)
                {
                    auto findIt = std::find(exceptions.begin(), exceptions.end(), iterator);
                    if (findIt == exceptions.end())
                    {
                        return iterator;
                    }
                }
            }
        }
    }
    return nullptr;
}

bool Character::findNearbyTools(ToolSet tools, ItemVector & foundOnes,
bool searchRoom,
bool searchInventory,
bool searchEquipment)
{
    // TODO: Prepare a map with key the tool type and as value:
    //  Option A: A bool which determine if the tool has been found.
    //  Option B: A pointer to the found tool (more interesting, can pass to
    //              this function as reference the map and then update it with
    //              the found tools inside this function).
    for (auto toolType : tools)
    {
        Item * tool = this->findNearbyTool(
            toolType,
            foundOnes,
            searchRoom,
            searchInventory,
            searchEquipment);
        if (tool == nullptr)
        {
            return false;
        }
        else
        {
            foundOnes.push_back(tool);
        }
    }
    return true;
}

bool Character::findNearbyResouces(IngredientMap ingredients, ItemVector & foundOnes)
{
    for (auto ingredient : ingredients)
    {
        // Quantity of ingredients that has to be found.
        unsigned int quantityNeeded = ingredient.second;
        for (auto iterator : inventory)
        {
            if (iterator->model->getType() == ModelType::Resource)
            {
                ResourceModel * resourceModel = iterator->model->toResource();
                if (resourceModel->resourceType == ingredient.first)
                {
                    foundOnes.push_back(iterator);
                    quantityNeeded--;
                    if (quantityNeeded == 0)
                    {
                        break;
                    }
                }
            }
        }
        // If the ingredients are not enough, search even in the room.
        if (quantityNeeded > 0)
        {
            for (auto iterator : room->items)
            {
                if (iterator->model->getType() == ModelType::Resource)
                {
                    ResourceModel * resourceModel = iterator->model->toResource();
                    if (resourceModel->resourceType == ingredient.first)
                    {
                        foundOnes.push_back(iterator);
                        quantityNeeded--;
                        if (quantityNeeded == 0)
                        {
                            break;
                        }
                    }
                }
            }
        }
        // If the ingredients are still not enough, return false.
        if (quantityNeeded > 0)
        {
            return false;
        }
    }
    return true;
}

bool Character::hasInventoryItem(Item * item)
{
    for (auto it : inventory)
    {
        if (it->vnum == item->vnum)
        {
            return true;
        }
    }
    return false;
}

bool Character::hasEquipmentItem(Item * item)
{
    for (auto it : equipment)
    {
        if (it->vnum == item->vnum)
        {
            return true;
        }
    }
    return false;
}

bool Character::addInventoryItem(Item * item)
{
    if (item == nullptr)
    {
        Logger::log(LogLevel::Error, "[addInventoryItem] Item is a nullptr.");
        return false;
    }
    // Set the owner of the item.
    item->owner = this;
    // Add the item to the inventory.
    inventory.push_back(item);
    Logger::log(
        LogLevel::Debug,
        "Item '" + item->getName() + "' added to '" + this->getName() + "' inventory;");
    return true;
}

bool Character::remInventoryItem(Item *item)
{
    bool removed = false;
    for (ItemVector::iterator it = inventory.begin(); it != inventory.end(); ++it)
    {
        Item * invItem = (*it);
        if (invItem->vnum == item->vnum)
        {
            Logger::log(
                LogLevel::Debug,
                "Item '" + item->getName() + "' removed from '" + this->getName() + "';");
            item->owner = nullptr;
            inventory.erase(it);
            removed = true;
            break;
        }
    }
    return removed;
}

bool Character::canCarry(Item * item) const
{
    return ((this->getCarryingWeight() + item->getTotalWeight()) < this->getMaxCarryingWeight());
}

unsigned int Character::getCarryingWeight() const
{
    unsigned int carrying = 0;
    for (auto iterator : inventory)
    {
        carrying += iterator->getTotalWeight();
    }
    for (auto iterator : equipment)
    {
        carrying += iterator->getTotalWeight();
    }
    return carrying;
}

unsigned int Character::getMaxCarryingWeight() const
{
    // Value = 50 + (AbilMod(STR) * 10)
    // MIN   =  50
    // MAX   = 300
    return 50 + (this->getAbilityModifier(Ability::Strength) * 10);
}

bool Character::addEquipmentItem(Item * item)
{
    // Check if the item exist.
    if (item == nullptr)
    {
        Logger::log(LogLevel::Error, "[addEquipmentItem] Item is a nullptr.");
        return false;
    }
    // Set the owner of the item.
    item->owner = this;
    // Add the item to the equipment.
    equipment.push_back(item);
    Logger::log(
        LogLevel::Debug,
        "Item '" + item->getName() + "' added to '" + this->getName() + "' equipment;");
    return true;
}

bool Character::remEquipmentItem(Item * item)
{
    bool removed = false;
    for (auto it = equipment.begin(); it != equipment.end(); ++it)
    {
        Item * eqpItem = (*it);
        if (eqpItem->vnum == item->vnum)
        {
            Logger::log(
                LogLevel::Debug,
                "Item '" + item->getName() + "' removed from '" + this->getName() + "';");
            item->owner = nullptr;
            equipment.erase(it);
            removed = true;
            break;
        }
    }
    return removed;
}

bool Character::canWield(Item * item, std::string & error, EquipmentSlot & where) const
{
    // Gather the item in the right hand, if there is one.
    Item * rightHand = findEquipmentSlotItem(EquipmentSlot::RightHand);
    // Gather the item in the left hand, if there is one.
    Item * leftHand = findEquipmentSlotItem(EquipmentSlot::LeftHand);
    if (HasFlag(item->model->modelFlags, ModelFlag::TwoHand))
    {
        if ((rightHand != nullptr) || (leftHand != nullptr))
        {
            error = "You must have both your hand free to wield it.\n";
            return false;
        }
        where = EquipmentSlot::RightHand;
    }
    else
    {
        if ((rightHand != nullptr) && (leftHand != nullptr))
        {
            error = "You have both your hand occupied.\n";
            return false;
        }
        else if ((rightHand == nullptr) && (leftHand != nullptr))
        {
            if (HasFlag(leftHand->model->modelFlags, ModelFlag::TwoHand))
            {
                error = "You have both your hand occupied.\n";
                return false;
            }
            where = EquipmentSlot::RightHand;
        }
        else if ((rightHand != nullptr) && (leftHand == nullptr))
        {
            if (HasFlag(rightHand->model->modelFlags, ModelFlag::TwoHand))
            {
                error = "You have both your hand occupied.\n";
                return false;
            }
            where = EquipmentSlot::LeftHand;
        }
        else
        {
            where = EquipmentSlot::RightHand;
        }
    }
    return true;
}

bool Character::canWear(Item * item, std::string & error) const
{
    bool result = false;
    if (item->model->getType() == ModelType::Armor)
    {
        result = true;
    }
    else if (item->model->getType() == ModelType::Container)
    {
        if (item->getCurrentSlot() != EquipmentSlot::None)
        {
            result = true;
        }
    }
    if (!result)
    {
        error = "The item is not meant to be weared.\n";
        return false;
    }
    if (findEquipmentSlotItem(item->getCurrentSlot()) != nullptr)
    {
        error = "There is already something in that location.\n";
        return false;
    }
    return true;
}

bool Character::setThirst(int value)
{
    int result = static_cast<int>(this->thirst) + value;
    if (result < 0)
    {
        return false;
    }
    else if (result > 100)
    {
        this->thirst = 100;
    }
    else
    {
        this->thirst = static_cast<unsigned int>(result);
    }
    return true;
}

unsigned int Character::getThirst() const
{
    return this->thirst;
}

std::string Character::getThirstCondition() const
{
    // Determine the correct description.
    if (this->thirst >= 90) return "are not thirsty";
    else if (this->thirst >= 60) return "are quite thirsty";
    else if (this->thirst >= 30) return "are thirsty";
    else return "are dying of thirst";
}

bool Character::setHunger(int value)
{
    int result = static_cast<int>(this->hunger) + value;
    if (result < 0)
    {
        return false;
    }
    else if (result > 100)
    {
        this->hunger = 100;
    }
    else
    {
        this->hunger = static_cast<unsigned int>(result);
    }
    return true;
}

unsigned int Character::getHunger() const
{
    return this->hunger;
}

std::string Character::getHungerCondition() const
{
    // Determine the correct description.
    if (this->hunger >= 90) return "are not hungry";
    else if (this->hunger >= 60) return "are quite hungry";
    else if (this->hunger >= 30) return "are hungry";
    else return "are dying of hunger";
}

void Character::updateHealth()
{
    unsigned int posModifier = 0;
    unsigned int logModifier = this->getAbilityLog(Ability::Constitution, 0.0, 1.0);
    if (posture == CharacterPosture::Sit)
    {
        posModifier = 2;
    }
    else if (posture == CharacterPosture::Rest)
    {
        posModifier = 4;
    }
    if (this->health < this->getMaxHealth())
    {
        this->addHealth((1 + 3 * logModifier) * (1 + 2 * posModifier), true);
    }
}

void Character::updateStamina()
{
    unsigned int posModifier = 0;
    unsigned int logModifier = this->getAbilityLog(Ability::Constitution, 0.0, 1.0);
    if (posture == CharacterPosture::Sit)
    {
        posModifier = 3;
    }
    else if (posture == CharacterPosture::Rest)
    {
        posModifier = 5;
    }
    if (this->stamina < this->getMaxStamina())
    {
        this->addStamina((1 + 4 * logModifier) * (1 + 3 * posModifier), true);
    }
}

void Character::updateHunger()
{
    unsigned int reduction = 10;
    if (this->hunger < reduction)
    {
        this->hunger = 0;
    }
    else
    {
        this->hunger -= reduction;
    }
}

void Character::updateThirst()
{
    unsigned int reduction = 10;
    if (this->thirst < reduction)
    {
        this->thirst = 0;
    }
    else
    {
        this->thirst -= reduction;
    }
}

void Character::updateExpiredEffects()
{
    std::vector<std::string> messages;
    if (this->effects.effectUpdate(messages))
    {
        for (auto message : messages)
        {
            this->sendMsg(message + "\n");
        }
    }
}

void Character::updateActivatedEffects()
{
    std::vector<std::string> messages;
    if (this->effects.effectActivate(messages))
    {
        for (auto message : messages)
        {
            this->sendMsg(message + "\n");
        }
    }
}

string Character::getLook(Character * character)
{
    string sent_be, sent_pronoun;
    string output;

    // Determine who is the examined character.
    if (character != nullptr && character != this)
    {
        output = "You look at " + character->getName() + ".\n";
        sent_be = "is";
        sent_pronoun = character->getSubjectPronoun();
    }
    else
    {
        character = this;
        output = "You give a look at yourself.\n";
        sent_be = "are";
        sent_pronoun = "you";
    }

    Item * head = character->findEquipmentSlotItem(EquipmentSlot::Head);
    Item * back = character->findEquipmentSlotItem(EquipmentSlot::Back);
    Item * torso = character->findEquipmentSlotItem(EquipmentSlot::Torso);
    Item * legs = character->findEquipmentSlotItem(EquipmentSlot::Legs);
    Item * feet = character->findEquipmentSlotItem(EquipmentSlot::Feet);
    Item * right = character->findEquipmentSlotItem(EquipmentSlot::RightHand);
    Item * left = character->findEquipmentSlotItem(EquipmentSlot::LeftHand);

    // Add the condition.
    output += ToCapitals(sent_pronoun) + getHealthCondition(character);
    // Add the description.
    output += character->description + "\n";
    // Add what the target is wearing.
    output += ToCapitals(sent_pronoun) + " " + sent_be + " wearing:\n";

    // Equipment Slot : HEAD
    output += "    " + Formatter::yellow() + "Head" + Formatter::reset() + "       : ";
    output +=
        (head != nullptr) ?
            Formatter::cyan() + head->getNameCapital() : Formatter::gray() + "Nothing";
    output += Formatter::reset() + ".\n";
    // Equipment Slot : BACK
    output += "    " + Formatter::yellow() + "Back" + Formatter::reset() + "       : ";
    output +=
        (back != nullptr) ?
            Formatter::cyan() + back->getNameCapital() : Formatter::gray() + "Nothing";
    output += Formatter::reset() + ".\n";
    // Equipment Slot : TORSO
    output += "    " + Formatter::yellow() + "Torso" + Formatter::reset() + "      : ";
    output +=
        (torso != nullptr) ?
            Formatter::cyan() + torso->getNameCapital() : Formatter::gray() + "Nothing";
    output += Formatter::reset() + ".\n";
    // Equipment Slot : LEGS
    output += "    " + Formatter::yellow() + "Legs" + Formatter::reset() + "       : ";
    output +=
        (legs != nullptr) ?
            Formatter::cyan() + legs->getNameCapital() : Formatter::gray() + "Nothing";
    output += Formatter::reset() + ".\n";
    // Equipment Slot : FEET
    output += "    " + Formatter::yellow() + "Feet" + Formatter::reset() + "       : ";
    output +=
        (feet != nullptr) ?
            Formatter::cyan() + feet->getNameCapital() : Formatter::gray() + "Nothing";
    output += Formatter::reset() + ".\n";

    // Print what is wielding.
    if (right != nullptr)
    {
        if (HasFlag(right->model->modelFlags, ModelFlag::TwoHand))
        {
            output += "    " + Formatter::yellow() + "Both Hands" + Formatter::reset() + " : ";
        }
        else
        {
            output += "    " + Formatter::yellow() + "Right Hand" + Formatter::reset() + " : ";
        }
        output += Formatter::cyan() + right->getNameCapital() + Formatter::reset() + ".\n";
    }
    else
    {
        output += "    " + Formatter::yellow() + "Right Hand" + Formatter::reset() + " : "
            + "Nothing";
        output += ".\n";
    }

    if (left != nullptr)
    {
        output += "    " + Formatter::yellow() + "Left Hand" + Formatter::reset() + "  : ";
        output += Formatter::cyan() + left->getNameCapital() + Formatter::reset() + ".\n";
    }
    return output;
}

bool Character::canSee(Character * target) const
{
    if (HasFlag(target->flags, CharacterFlag::Invisible))
    {
        return false;
    }
    return true;
}

unsigned int Character::getArmorClass() const
{
    // 10
    unsigned int result = 10;
    // + ARMOR BONUS
    for (auto it : equipment)
    {
        if (it->model->getType() == ModelType::Armor)
        {
            result += it->model->toArmor()->damageAbs;
        }
    }
    // + SHIELD BONUS
    Item * rh = this->findEquipmentSlotItem(EquipmentSlot::RightHand);
    if (rh != nullptr)
    {
        if (rh->model->getType() == ModelType::Shield)
        {
            result += rh->model->toShield()->parryChance;
        }
    }
    Item * lh = this->findEquipmentSlotItem(EquipmentSlot::LeftHand);
    if (lh != nullptr)
    {
        if (lh->model->getType() == ModelType::Shield)
        {
            result += lh->model->toShield()->parryChance;
        }
    }
    // + AGILITY MODIFIER
    result += this->getAbilityModifier(Ability::Agility);
    return result;
}

bool Character::canAttackWith(const EquipmentSlot & slot) const
{
    if ((slot == EquipmentSlot::RightHand) || (slot == EquipmentSlot::LeftHand))
    {
        Item * weapon = this->findEquipmentSlotItem(slot);
        if (weapon != nullptr)
        {
            // Check if there is actually a weapon equiped.
            if (weapon->model->getType() == ModelType::Weapon)
            {
                return true;
            }
        }
    }
    return false;
}

bool Character::isAtRange(Character * target, const unsigned int & range) const
{
    if (WrongAssert(target == nullptr)) return false;
    if (WrongAssert(this->room == nullptr)) return false;
    if (WrongAssert(this->room->area == nullptr)) return false;
    if (WrongAssert(target->room == nullptr)) return false;
    if (WrongAssert(target->room->area == nullptr)) return false;
    return this->room->area->fastInSight(this->room->coord, target->room->coord, range);
}

Character * Character::getNextOpponentAtRange(const unsigned int & range) const
{
    if (opponents.hasOpponents())
    {
        for (auto iterator : opponents.aggressionList)
        {
            if (this->isAtRange(iterator.aggressor, range))
            {
                return iterator.aggressor;
            }
        }
    }
    return nullptr;
}

ItemVector Character::getActiveWeapons()
{
    ItemVector ret;
    if (this->canAttackWith(EquipmentSlot::RightHand))
    {
        Item * weapon = this->findEquipmentSlotItem(EquipmentSlot::RightHand);
        if (weapon != nullptr)
        {
            ret.push_back(weapon);
        }
    }
    if (this->canAttackWith(EquipmentSlot::LeftHand))
    {
        Item * weapon = this->findEquipmentSlotItem(EquipmentSlot::LeftHand);
        if (weapon != nullptr)
        {
            ret.push_back(weapon);
        }
    }
    return ret;
}

unsigned int Character::getCooldown(CombatActionType combatAction)
{
    double BASE = 5.0;
    // The strength modifier.
    // MIN = 0.00
    // MAX = 1.40
    double AGI = this->getAbilityLog(Ability::Agility, 0.0, 1.0);
    // The agility modifier.
    // MIN = 0.00
    // MAX = 1.40
    double STR = this->getAbilityLog(Ability::Strength, 0.0, 1.0);
    // The weight modifier.
    // MIN =  0.80
    // MAX =  2.51
    double WGT = 0.8;
    double WGTmod = this->weight;
    if (WGTmod > 0)
    {
        if (WGTmod > 320)
        {
            WGTmod = 320;
        }
        WGT = log10(WGTmod);
    }
    // The carried weight.
    // MIN =  0.00
    // MAX =  2.48
    double CAR = 0.0;
    double CARmod = this->getCarryingWeight();
    if (CARmod > 0)
    {
        if (CARmod > 300)
        {
            CARmod = 300;
        }
        CAR = log10(CARmod);
    }
    if (combatAction == CombatActionType::BasicAttack)
    {
        double RHD = 0;
        double LHD = 0;
        if (this->canAttackWith(EquipmentSlot::RightHand))
        {
            // Right Hand Weapon
            // MIN =  0.00
            // MAX =  1.60
            Item * weapon = this->findEquipmentSlotItem(EquipmentSlot::RightHand);
            double RHDmod = weapon->getTotalWeight();
            if (RHDmod > 0)
            {
                if (RHDmod > 40)
                {
                    RHDmod = 40;
                }
                RHD = log10(RHDmod);
            }
        }
        if (this->canAttackWith(EquipmentSlot::LeftHand))
        {
            // Left Hand Weapon
            // MIN =  0.00
            // MAX =  1.60
            Item * weapon = this->findEquipmentSlotItem(EquipmentSlot::LeftHand);
            double LHDmod = weapon->getTotalWeight();
            if (LHDmod > 0)
            {
                if (LHDmod > 40)
                {
                    LHDmod = 40;
                }
                LHD = log10(LHDmod);
            }
        }
        BASE += -STR - AGI + WGT + CAR + max(RHD, LHD);
    }
    else if (combatAction == CombatActionType::Flee)
    {
        BASE += -STR - AGI + WGT + CAR;
    }
    return static_cast<unsigned int>(BASE);
}

unsigned int Character::getConsumedStaminaFor(
    const ActionType & actionType,
    const CombatActionType & combatAction,
    const EquipmentSlot & slot) const
{
    // BASE     [+1.0]
    // STRENGTH [-0.0 to -2.80]
    // WEIGHT   [+1.6 to +2.51]
    // CARRIED  [+0.0 to +2.48]
    // WEAPON   [+0.0 to +1.60]
    // The base value.
    double BASE = 1.0;
    // The strength modifier.
    double STR = this->getAbilityLog(Ability::Strength, 0.0, 1.0);
    // The weight modifier.
    double WGT = (this->weight == 0) ? 0 : log10(this->weight);
    // The carried weight.
    unsigned int carried = this->getCarryingWeight();
    double CAR = (carried == 0) ? 0 : log10(carried);
    // Partial result;
    double RSLT = BASE - STR + WGT + CAR;
    if (actionType == ActionType::Move)
    {
        // Do something.
    }
    else if (actionType == ActionType::Building)
    {
        // Do something.
    }
    else if (actionType == ActionType::Crafting)
    {
        // Do something.
    }
    else if (actionType == ActionType::Combat)
    {
        if (combatAction == CombatActionType::BasicAttack)
        {
            if (this->canAttackWith(slot))
            {
                Item * weapon = this->findEquipmentSlotItem(slot);
                unsigned int wpnWeight = weapon->getTotalWeight();
                double WPN = (wpnWeight == 0) ? 0 : log10(wpnWeight);
                RSLT += WPN;
            }
        }
        else if (combatAction == CombatActionType::Flee)
        {
            // Do something.
        }
    }
    return static_cast<unsigned int>(RSLT);
}

void Character::kill()
{
    // Create a corpse at the current position.
    Item * corpse = this->createCorpse();
    // Transfer all the items from the character to the corpse.
    auto tempInventory = this->inventory;
    for (auto it = tempInventory.begin(); it != tempInventory.end(); ++it)
    {
        Item * item = (*it);
        // Remove the item from the inventory.
        this->remInventoryItem(item);
        // Add the item to the corpse.
        corpse->content.push_back(item);
        // Set the corpse as container of the item.
        item->container = corpse;
        // Update the item on the database.
        item->updateOnDB();
    }
    auto tempEquipment = this->equipment;
    for (auto it = tempEquipment.begin(); it != tempEquipment.end(); ++it)
    {
        Item * item = (*it);
        // Remove the item from the inventory.
        this->remEquipmentItem(item);
        // Add the item to the corpse.
        corpse->content.push_back(item);
        // Set the corpse as container of the item.
        item->container = corpse;
        // Update the item on the database.
        item->updateOnDB();
    }
    // Reset the action of the character.
    this->setAction(std::make_shared<GeneralAction>(this));
    // Reset the list of opponents.
    this->opponents.resetList();
    // Remove the character from the current room.
    if (room != nullptr)
    {
        room->removeCharacter(this);
    }
}

Item * Character::createCorpse()
{
    // Create the corpse.
    Item * corpse = race->corpse.createItem(this->name, race->material, ItemQuality::Normal);
    // Set the weight of the new corpse.
    corpse->customWeight = this->weight;
    // Add the corpse to the room.
    room->addItem(corpse);
    // Return the corpse.
    return corpse;
}

void Character::doCommand(const string & command)
{
    std::istringstream is(command);
    ProcessCommand(this, is);
}

Player * Character::toPlayer()
{
    return static_cast<Player *>(this);
}

Mobile * Character::toMobile()
{
    return static_cast<Mobile *>(this);
}

VectorHelper<Character *> Character::luaGetTargets()
{
    if (room != nullptr)
    {
        return VectorHelper<Character *>(room->characters);
    }
    return VectorHelper<Character *>();
}

void Character::loadScript(const std::string & scriptFilename)
{
    Logger::log(LogLevel::Debug, "Loading script '%s'...", scriptFilename);

    // Open lua libraries.
    luaL_openlibs(L);

    // Register utilities functions.
    LuaRegisterUtils(L);

    // Register all the classes.
    Character::luaRegister(L);
    Area::luaRegister(L);
    Faction::luaRegister(L);
    ItemModel::luaRegister(L);
    Item::luaRegister(L);
    Material::luaRegister(L);
    Race::luaRegister(L);
    Coordinates<int>::luaRegister(L);
    Exit::luaRegister(L);
    Room::luaRegister(L);

    if (luaL_dofile(L, scriptFilename.c_str()) != LUABRIDGE_LUA_OK)
    {
        Logger::log(LogLevel::Error, "Can't open script+" + scriptFilename + ".");
        Logger::log(LogLevel::Error, "Error :" + std::string(lua_tostring(L, -1)));
    }
}

void Character::luaRegister(lua_State * L)
{
    luabridge::getGlobalNamespace(L) //
    .beginClass<Character>("Character") //
    .addData("name", &Character::name) //
    .addData("race", &Character::race) //
    .addData("faction", &Character::faction) //
    .addData("room", &Character::room) //
    .addFunction("inventoryAdd", &Character::addInventoryItem) //
    .addFunction("inventoryRemove", &Character::remInventoryItem) //
    .addFunction("equipmentAdd", &Character::addEquipmentItem) //
    .addFunction("equipmentRemove", &Character::remEquipmentItem) //
    .addFunction("doCommand", &Character::doCommand) //
    .addFunction("getTargets", &Character::luaGetTargets) //
    .addFunction("isMobile", &Character::isMobile) //
    .endClass() //
    .deriveClass<Mobile, Character>("Mobile") //
    .addData("id", &Mobile::id) //
    .addData("spawnRoom", &Mobile::respawnRoom) //
    .addData("shortdesc", &Mobile::shortdesc) //
    .addData("staticdesc", &Mobile::staticdesc) //
    .addData("message_buffer", &Mobile::message_buffer) //
    .addData("alive", &Mobile::alive) //
    .addData("controller", &Mobile::controller) //
    .addFunction("isMobile", &Mobile::isMobile) //
    .endClass() //
    .deriveClass<Player, Character>("Player") //
    .addData("age", &Player::age, false) //
    .addData("experience", &Player::experience, false) //
    .addData("prompt", &Player::prompt, false) //
    .addData("rent_room", &Player::rent_room, false) //
    .addData("remaining_points", &Player::remaining_points, false) //
    .addData("rent_room", &Player::rent_room, false) //
    //std::map<int, unsigned int> skills;
    .endClass();
}

bool Character::operator<(const class Character & source) const
{
    return name < source.name;
}

bool Character::operator==(const class Character & source) const
{
    return name == source.name;
}

void Character::sendMsg(const std::string & msg)
{
    Logger::log(LogLevel::Error, "[SEND_MESSAGE] Msg :" + msg);
}

std::string GetCharacterFlagString(unsigned int flags)
{
    std::string flagList;
    if (HasFlag(flags, CharacterFlag::IsGod)) flagList += "|IsGod";
    if (HasFlag(flags, CharacterFlag::Invisible)) flagList += "|Invisible";
    flagList += "|";
    return flagList;
}
