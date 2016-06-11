/// @file   model.cpp
/// @brief  Implements item model methods.
/// @author Enrico Fraccaroli
/// @date   Mar 31 2015
/// @copyright
/// Copyright (c) 2015, 2016 Enrico Fraccaroli <enrico.fraccaroli@gmail.com>
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
#include "model.hpp"

// Local Includes.

// Other Include.
#include "mud.hpp"
#include "logger.hpp"
#include "material.hpp"
#include "constants.hpp"
#include "luabridge/LuaBridge.h"

using namespace std;

Model::Model() :
    vnum(),
    name(),
    article(),
    shortdesc(),
    keys(),
    description(),
    type(ModelType::NoType),
    slot(EquipmentSlot::None),
    flags(),
    weight(),
    price(),
    condition(),
    decay(),
    material(MaterialType::NoType),
    tileSet(),
    tileId(),
    functions()
{
}

Model::Model(const Model & source) :
    vnum(source.vnum),
    name(source.name),
    article(source.article),
    shortdesc(source.shortdesc),
    keys(source.keys),
    description(source.description),
    type(source.type),
    slot(source.slot),
    flags(source.flags),
    weight(source.weight),
    price(source.price),
    condition(source.condition),
    decay(source.decay),
    material(source.material),
    tileSet(source.tileSet),
    tileId(source.tileId),
    functions(source.functions)
{
    // Nothing to do.
}

Model::~Model()
{
    // Nothing to do.
}

///////////////////////////////////////////////////////////
// CHECKER ////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool Model::setFunctions(std::string source)
{
    if (source.empty())
    {
        Logger::log(LogLevel::Error, "Function list is empty.");
        return false;
    }
    std::vector<std::string> functionList = SplitString(source, " ");
    if (functionList.size() != 6)
    {
        Logger::log(LogLevel::Error, "Function list contains the wrong number of parameters.");
        return false;
    }
    for (auto it : functionList)
    {
        int value = ToNumber<int>(it);
        if (value < 0)
        {
            Logger::log(LogLevel::Error, "Function list contains a negative value.");
            return false;
        }
        functions.push_back(static_cast<unsigned int>(value));
    }
    return true;
}

bool Model::check()
{
    assert(vnum > 0);
    assert(!name.empty());
    assert(!article.empty());
    assert(!shortdesc.empty());
    assert(!keys.empty());
    assert(!description.empty());
    assert(type != ModelType::NoType);
    if ((type == ModelType::Armor) || (type == ModelType::Weapon))
    {
        assert(slot != EquipmentSlot::None);
    }
    assert(weight > 0);
    assert(price >= 0);
    assert(condition > 0);
    assert(decay > 0);
    assert(this->material != MaterialType::NoType);
    assert(tileSet >= 0);
    assert(tileId >= 0);
    assert(!functions.empty());
    return true;
}

bool Model::replaceSymbols(std::string & source, Material * itemMaterial, ItemQuality itemQuality)
{
    bool modified = false;
    if (itemMaterial)
    {
        modified = true;
        FindAndReplace(source, "&m", ToLower(itemMaterial->name));
        FindAndReplace(source, "&M", ToLower(itemMaterial->article + ' ' + itemMaterial->name));
    }
    else
    {
        FindAndReplace(source, "&m", "");
        FindAndReplace(source, "&M", "");
    }
    if (itemQuality != ItemQuality::Normal)
    {
        modified = true;
        FindAndReplace(source, "&q", " " + ToLower(GetItemQualityName(itemQuality)));
    }
    else
    {
        FindAndReplace(source, "&q", "");
    }
    return modified;
}

std::string Model::getName(Material * itemMaterial, ItemQuality itemQuality)
{
    // Make a copy of the short description.
    std::string output = shortdesc;
    // Try to replace the symbols inside the short description.
    if (!replaceSymbols(output, itemMaterial, itemQuality))
    {
        output = article + " " + name;
    }
    return output;
}

std::string Model::getDescription(Material * itemMaterial, ItemQuality itemQuality)
{
    // Make a copy of the description.
    std::string output = description;
    replaceSymbols(output, itemMaterial, itemQuality);
    return output;
}

///////////////////////////////////////////////////////////
// ITEM CREATION //////////////////////////////////////////
///////////////////////////////////////////////////////////

Item * Model::createItem(std::string maker, Material * composition, ItemQuality itemQuality)
{
    // Instantiate the new item.
    Item * newItem = new Item();

    // Set the values of the new item.
    newItem->vnum = Mud::instance().getMaxVnumItem() + 1;
    newItem->model = this;
    newItem->maker = maker;
    newItem->condition = condition;
    newItem->composition = composition;
    newItem->quality = itemQuality;
    newItem->flags = 0;
    newItem->room = nullptr;
    newItem->owner = nullptr;
    newItem->container = nullptr;
    newItem->currentSlot = slot;
    newItem->content = std::vector<Item *>();
    newItem->contentLiq = LiquidContent();

    if (!newItem->check())
    {
        Logger::log(LogLevel::Error, "Cannot create the new item.");
        // Delete the item.
        delete (newItem);
        // Return pointer to nothing.
        return nullptr;
    }

    // Prepare the vector used to insert into the database.
    std::vector<std::string> arguments;
    arguments.push_back(ToString(newItem->vnum));
    arguments.push_back(ToString(vnum));
    arguments.push_back(maker);
    arguments.push_back(ToString(condition));
    arguments.push_back(ToString(composition->vnum));
    arguments.push_back(EnumToString(itemQuality));
    arguments.push_back(ToString(flags));

    if (SQLiteDbms::instance().insertInto("Item", arguments))
    {
        // Insert into the item_list the new item.
        Mud::instance().addItem(newItem);
    }
    else
    {
        SQLiteDbms::instance().rollbackTransection();
    }

    return newItem;
}

bool Model::mustBeWielded()
{
    return ((slot == EquipmentSlot::RightHand) || (slot == EquipmentSlot::LeftHand));
}

std::string Model::getSpecificTypeName()
{
    std::string output;
    if (type == ModelType::Weapon)
    {
        output = GetWeaponTypeName(getWeaponFunc().type);
    }
    else if (type == ModelType::Armor)
    {
        output = GetArmorSizeName(getArmorFunc().size);
    }
    else if (type == ModelType::Shield)
    {
        output = GetShieldSizeName(getShieldFunc().size);
    }
    else if (type == ModelType::Tool)
    {
        output = GetToolTypeName(getToolFunc().type);
    }
    else if (type == ModelType::Node)
    {
        output = GetNodeTypeName(getNodeFunc().type);
    }
    else if (type == ModelType::Resource)
    {
        output = GetResourceTypeName(getResourceFunc().type);
    }
    else if (type == ModelType::Seed)
    {
        output = GetSeedTypeName(getSeedFunc().type);
    }
    else if (type == ModelType::Mechanism)
    {
        output = GetMechanismTypeName(getMechanismFunc().type);
    }
    return output;
}

WeaponFunc Model::getWeaponFunc()
{
    WeaponFunc func;
    func.type = static_cast<WeaponType>(functions[0]);
    func.minDamage = functions[1];
    func.maxDamage = functions[2];
    func.range = functions[3];
    return func;
}

ArmorFunc Model::getArmorFunc()
{
    ArmorFunc func;
    func.size = static_cast<ArmorSize>(functions[0]);
    func.damageAbs = functions[1];
    func.allowedAnatomy = functions[2];
    return func;
}

ShieldFunc Model::getShieldFunc()
{
    ShieldFunc func;
    func.size = static_cast<ShieldSize>(functions[0]);
    func.parryChance = functions[1];
    return func;
}

ProjectileFunc Model::getProjectileFunc()
{
    ProjectileFunc func;
    func.damageBonus = functions[0];
    func.rangeBonus = functions[1];
    return func;
}

ContainerFunc Model::getContainerFunc()
{
    ContainerFunc func;
    func.maxWeight = functions[0];
    func.flags = functions[1];
    func.keyVnum = functions[2];
    func.difficulty = functions[3];
    return func;
}

LiqContainerFunc Model::getLiqContainerFunc()
{
    LiqContainerFunc func;
    func.maxWeight = functions[0];
    func.flags = functions[1];
    return func;
}

ToolFunc Model::getToolFunc()
{
    ToolFunc func;
    func.type = static_cast<ToolType>(functions[0]);
    return func;
}

NodeFunc Model::getNodeFunc()
{
    NodeFunc func;
    func.type = static_cast<NodeType>(functions[0]);
    func.provides = functions[1];
    return func;
}

ResourceFunc Model::getResourceFunc()
{
    ResourceFunc func;
    func.type = static_cast<ResourceType>(functions[0]);
    return func;
}

SeedFunc Model::getSeedFunc()
{
    SeedFunc func;
    func.type = static_cast<SeedType>(functions[0]);
    return func;
}

FoodFunc Model::getFoodFunc()
{
    FoodFunc func;
    func.hours = functions[0];
    func.flags = functions[1];
    return func;
}

LightFunc Model::getLightFunc()
{
    LightFunc func;
    func.maxHours = functions[0];
    func.policy = functions[1];
    return func;
}

BookFunc Model::getBookFunc()
{
    BookFunc func;
    func.maxParchments = functions[0];
    return func;
}

RopeFunc Model::getRopeFunc()
{
    RopeFunc func;
    func.type = functions[0];
    func.difficulty = functions[1];
    return func;
}

MechanismFunc Model::getMechanismFunc()
{
    MechanismFunc func;
    func.type = static_cast<MechanismType>(functions[0]);
    if ((func.type == MechanismType::Door) || (func.type == MechanismType::Lock))
    {
        func.key = functions[1];
        func.difficulty = functions[2];
    }
    else if (func.type == MechanismType::Picklock)
    {
        func.efficency = functions[1];
    }
    else if (func.type == MechanismType::Lever)
    {
        func.command = functions[1];
        func.target = functions[2];
    }
    return func;
}

void Model::luaRegister(lua_State * L)
{
    luabridge::getGlobalNamespace(L) //
    .beginClass<Model>("Model") //
    .addData("vnum", &Model::vnum) //
    .addData("type", &Model::type) //
    .addData("weight", &Model::weight) //
    .addData("price", &Model::price) //
    .addData("condition", &Model::condition) //
    .addData("decay", &Model::decay) //
    .endClass();
}

std::string Model::getTile(int offset)
{
    if (Formatter::getFormat() == Formatter::TELNET)
    {
        return ToString(tileSet) + ":" + ToString(tileId + offset);
    }
    else
    {
        // TODO: Too easy this way.
        if (type == ModelType::Armor)
        {
            return "a";
        }
        else if (type == ModelType::Weapon)
        {
            return "w";
        }
        else
        {
            return "i";
        }
    }
}
