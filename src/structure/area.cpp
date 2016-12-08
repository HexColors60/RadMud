/// @file   area.cpp
/// @brief  This file implements all the methods needed to interact with an area.
/// @author Enrico Fraccaroli
/// @date   Mar 10 2015
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

#include "area.hpp"

#include "logger.hpp"
#include "room.hpp"

Area::Area() :
    vnum(),
    name(),
    builder(),
    map(),
    width(),
    height(),
    elevation(),
    tileSet(),
    type(),
    status()
{
}

Area::~Area()
{
    //Logger::log(LogLevel::Debug, "Deleted area\t\t[%s]\t\t(%s)", ToString(this->vnum), this->name);
}

bool Area::check()
{
    assert(vnum > 0);
    assert(!name.empty());
    assert(!builder.empty());
    assert(width > 0);
    assert(height > 0);
    assert(elevation > 0);
    assert(type != AreaType::NoType);
    return true;
}

bool Area::inBoundaries(const Coordinates & coordinates) const
{
    if ((coordinates.x < 0) || (coordinates.x > width)) return false;
    if ((coordinates.y < 0) || (coordinates.y > height)) return false;
    return !((coordinates.z < 0) || (coordinates.z > elevation));
}

bool Area::isValid(const Coordinates & coordinates)
{
    // Check if out of boundaries.
    if (!this->inBoundaries(coordinates)) return false;
    // Get the room at the coordinates and check if there is a room.
    auto room = this->getRoom(coordinates);
    if (room == nullptr) return false;
    // Check if there is a door.
    auto door = room->findDoor();
    if (door != nullptr)
    {
        // Check if the door is closed.
        if (HasFlag(door->flags, ItemFlag::Closed)) return false;
    }
    return true;
}

int Area::getDistance(const Coordinates & source, const Coordinates & target)
{
    return static_cast<int>(std::sqrt(pow(source.x - target.x, 2) +
                                      pow(source.y - target.y, 2) +
                                      pow(source.z - target.z, 2)));
}

Direction Area::getDirection(const Coordinates & source, const Coordinates & target)
{
    auto dx = std::abs(source.x - target.x), dy = std::abs(source.y - target.y), dz = std::abs(source.z - target.z);
    if ((dx > dy) && (dx > dz))
    {
        if (source.x > target.x) return Direction::West;
        else if (source.x < target.x) return Direction::East;
    }
    else if ((dy > dx) && (dy > dz))
    {
        if (source.y > target.y) return Direction::South;
        else if (source.y < target.y) return Direction::North;
    }
    else if ((dz > dx) && (dz > dy))
    {
        if (source.z > target.z) return Direction::Down;
        else if (source.z < target.z) return Direction::Up;
    }
    return Direction::None;
}

CharacterContainer Area::getCharactersAt(const CharacterContainer & exceptions, const Coordinates & coordinates)
{
    CharacterContainer characterContainer;
    if (this->isValid(coordinates))
    {
        for (auto it : this->getRoom(coordinates)->characters)
        {
            if (!exceptions.containsCharacter(it))
            {
                characterContainer.emplace_back_character(it);
            }
        }
    }
    return characterContainer;
}

ItemContainer Area::getItemsAt(const ItemContainer & exceptions, const Coordinates & coordinates)
{
    ItemContainer itemContainer;
    if (this->isValid(coordinates))
    {
        for (auto it : this->getRoom(coordinates)->items)
        {
            if (!exceptions.findItem(it->vnum))
            {
                itemContainer.emplace_back(it);
            }
        }
    }
    return itemContainer;
}

bool Area::addRoom(Room * room)
{
    if (this->inBoundaries(room->coord))
    {
        if (map.set(room->coord.x, room->coord.y, room->coord.z, room))
        {
            // Set the room area to be this one.
            room->area = this;
            return true;
        }
        else
        {
            Logger::log(LogLevel::Error, "Room's insertion could not be completed %s.", room->coord);
        }
    }
    else
    {
        Logger::log(LogLevel::Error, "Room's coordinates are not inside the boundaries %s.", room->coord);
    }
    return false;
}

bool Area::remRoom(Room * room)
{
    return map.erase(room->coord.x, room->coord.y, room->coord.z);
}

Room * Area::getRoom(int room_vnum)
{
    for (auto it : map)
    {
        for (auto it2: it.second)
        {
            for (auto it3: it2.second)
            {
                if (it3.second != nullptr)
                {
                    if (it3.second->vnum == room_vnum)
                    {
                        return it3.second;
                    }
                }
            }
        }
    }
    return nullptr;
}

Room * Area::getRoom(const Coordinates & coordinates)
{
    if (this->inBoundaries(coordinates))
    {
        return map.get(coordinates.x, coordinates.y, coordinates.z);
    }
    return nullptr;
}

std::vector<std::string> Area::drawFov(Room * centerRoom, const int & radius)
{
    std::vector<std::string> layers(3);
    if (!this->inBoundaries(centerRoom->coord))
    {
        return layers;
    }
    // Retrieve the coordinates of the room.
    int origin_x = centerRoom->coord.x;
    int origin_y = centerRoom->coord.y;
    int origin_z = centerRoom->coord.z;
    // Evaluate the minimum and maximum value for x and y.
    int min_x = (origin_x < radius) ? 0 : (origin_x - radius);
    int max_x = ((origin_x + radius) > this->width) ? this->width : (origin_x + radius);
    int min_y = (origin_y < radius) ? 0 : (origin_y - radius);
    int max_y = ((origin_y + radius - 1) > this->height) ? this->height : (origin_y + radius - 1);
    // Evaluate the field of view.
    auto coordinatesInFov = this->fov(centerRoom->coord, radius);
    // Prepare Environment layer.
    for (int y = max_y; y >= min_y; --y)
    {
        for (int x = min_x; x < max_x; ++x)
        {
            std::string tileCode = " : ";
            Coordinates coordinates(x, y, origin_z);
            bool found = false;
            for (auto it : coordinatesInFov)
            {
                if (coordinates == it)
                {
                    found = true;
                    break;
                }
            }
            if (found)
            {
                Room * room = this->getRoom(Coordinates(x, y, origin_z));
                std::shared_ptr<Exit> up = room->findExit(Direction::Up);
                std::shared_ptr<Exit> down = room->findExit(Direction::Down);

                // By default set it to walkable tile.
                tileCode = ToString(15) + ":" + ToString(this->tileSet + 0);

                if ((up != nullptr) && (down != nullptr))
                {
                    if (HasFlag(up->flags, ExitFlag::Stairs)
                        && HasFlag(down->flags, ExitFlag::Stairs))
                    {
                        tileCode = ToString(18) + ":" + ToString(this->tileSet + 1);
                    }
                }
                else if (up != nullptr)
                {
                    if (HasFlag(up->flags, ExitFlag::Stairs))
                    {
                        tileCode = ToString(18) + ":" + ToString(this->tileSet + 1);
                    }
                }
                else if (down != nullptr)
                {
                    if (HasFlag(down->flags, ExitFlag::Stairs))
                    {
                        tileCode = ToString(18) + ":" + ToString(this->tileSet + 0);
                    }
                    else
                    {
                        tileCode = ToString(18) + ":" + ToString(this->tileSet + 4);
                    }
                }
            }
            layers[0] += tileCode;
            if (x != (origin_x + radius - 1))
            {
                layers[0] += ",";
            }
        }
        layers[0] += ";";
    }

    // Prepare Objects layer.
    for (int y = max_y; y > min_y; --y)
    {
        for (int x = min_x; x < max_x; ++x)
        {
            std::string tileCode = " : ";
            Coordinates coordinates(x, y, origin_z);
            bool found = false;
            for (auto it : coordinatesInFov)
            {
                if (coordinates == it)
                {
                    found = true;
                    break;
                }
            }
            if (found)
            {
                Room * room = this->getRoom(coordinates);
                if (room != nullptr)
                {
                    Item * door = room->findDoor();
                    if (!room->items.empty())
                    {
                        tileCode = room->items.back()->model->getTile();
                    }
                    else if (door != nullptr)
                    {
                        if (HasFlag(door->flags, ItemFlag::Closed))
                        {
                            tileCode = door->model->getTile(+1);
                        }
                        else
                        {
                            tileCode = door->model->getTile(+3);
                        }
                    }
                }
            }
            layers[1] += tileCode;
            if (x != (origin_x + radius - 1))
            {
                layers[1] += ",";
            }
        }
        layers[1] += ";";
    }

    // Prepare Living Creatures layer.
    for (int y = max_y; y > min_y; --y)
    {
        for (int x = min_x; x < max_x; ++x)
        {
            std::string tileCode = " : ";
            Coordinates coordinates(x, y, origin_z);
            bool found = false;
            for (auto it : coordinatesInFov)
            {
                if (coordinates == it)
                {
                    found = true;
                    break;
                }
            }
            if ((origin_x == x) && (origin_y == y))
            {
                tileCode = ToString(1) + ":" + ToString(480);
            }
            else if (found)
            {
                Room * room = this->getRoom(Coordinates(x, y, origin_z));
                // Check if there are creatures in the tile.
                if (!room->characters.empty())
                {
                    for (auto iterator : room->characters)
                    {
                        if (!HasFlag(iterator->flags, CharacterFlag::Invisible))
                        {
                            tileCode = iterator->race->getTile();
                            break;
                        }
                    }
                }
            }
            layers[2] += tileCode;
            if (x != (origin_x + radius - 1))
            {
                layers[2] += ",";
            }
        }
        layers[2] += ";";
    }
    return layers;
}

std::string Area::drawASCIIFov(Room * centerRoom, const int & radius)
{
    std::string result;
    if (!this->inBoundaries(centerRoom->coord))
    {
        return result;
    }
    // Retrieve the coordinates of the room.
    int origin_x = centerRoom->coord.x;
    int origin_y = centerRoom->coord.y;
    int origin_z = centerRoom->coord.z;
    (void) origin_z;
    // Evaluate the minimum and maximum value for x and y.
    int min_x = (origin_x - radius);
    int min_y = (origin_y - radius);
    int max_x = (origin_x + radius);
    int max_y = (origin_y + radius);
    // Evaluate the field of view.
    auto validFov = this->fov(centerRoom->coord, radius);
    for (int y = max_y; y >= min_y; --y)
    {
        for (int x = min_x; x <= max_x; ++x)
        {
            std::string tile = " ";
            Coordinates coordinates(x, y, origin_z);
            if (std::find(validFov.begin(), validFov.end(), coordinates) != validFov.end())
            {
                Room * room = this->getRoom(Coordinates(x, y, origin_z));
                if (room != nullptr)
                {
                    std::shared_ptr<Exit> up = room->findExit(Direction::Up);
                    std::shared_ptr<Exit> down = room->findExit(Direction::Down);
                    // VI  - WALKABLE
                    tile = ".";
                    // V   - OPEN DOOR
                    Item * door = room->findDoor();
                    if (door != nullptr)
                    {
                        if (HasFlag(door->flags, ItemFlag::Closed))
                        {
                            tile = 'D';
                        }
                        else
                        {
                            tile = 'O';
                        }
                    }
                    // IV  - STAIRS
                    if ((up != nullptr) && (down != nullptr))
                    {
                        if (HasFlag(up->flags, ExitFlag::Stairs)
                            && HasFlag(down->flags, ExitFlag::Stairs))
                        {
                            tile = "X";
                        }
                    }
                    else if (up != nullptr)
                    {
                        if (HasFlag(up->flags, ExitFlag::Stairs))
                        {
                            tile = ">";
                        }
                    }
                    else if (down != nullptr)
                    {
                        if (HasFlag(down->flags, ExitFlag::Stairs))
                        {
                            tile = "<";
                        }
                        else
                        {
                            tile = " ";
                        }
                    }
                    // III - ITEMS
                    if (room->items.size() > 0)
                    {
                        tile = room->items.back()->model->getTile();
                    }
                    // II  - CHARACTERS
                    if (room->characters.size() > 0)
                    {
                        for (auto iterator : room->characters)
                        {
                            if (!HasFlag(iterator->flags, CharacterFlag::Invisible))
                            {
                                tile = iterator->race->getTile();
                            }
                        }
                    }
                    // I   - PLAYER
                    if ((origin_x == x) && (origin_y == y))
                    {
                        tile = "@";
                    }
                }
            }
            result += tile;
        }
        result += "\n";
    }
    return result;
}

CharacterContainer Area::getCharactersInSight(CharacterContainer & exceptions, Coordinates & origin, const int & radius)
{
    CharacterContainer characterContainer;
    auto validCoordinates = this->fov(origin, radius);
    for (auto coordinates : validCoordinates)
    {
        characterContainer.addUnique(this->getCharactersAt(exceptions, coordinates));
    }
    return characterContainer;
}

ItemContainer Area::getItemsInSight(ItemContainer & exceptions, Coordinates & origin, const int & radius)
{
    ItemContainer foundItems;
    auto validCoordinates = this->fov(origin, radius);
    for (auto coordinates : validCoordinates)
    {
        for (auto it :this->getItemsAt(exceptions, coordinates))
        {
            foundItems.emplace_back(it);
        }
    }
    return foundItems;
}

std::vector<Coordinates> Area::fov(Coordinates & origin, const int & radius)
{
    std::vector<Coordinates> fovCoordinates;
    Coordinates target, point;
    while (point.x <= radius)
    {
        while ((point.y <= point.x) && (point.square() <= pow(radius, 2)))
        {
            target = Coordinates(origin.x + point.x, origin.y + point.y, origin.z);
            if ((target.x == origin.x) && (target.y == origin.y))
            {
                fovCoordinates.emplace_back(target);
            }
            else
            {
                if (this->los(origin, target, radius)) fovCoordinates.emplace_back(target);

                target = Coordinates(origin.x - point.x, origin.y + point.y, origin.z);
                if (this->los(origin, target, radius)) fovCoordinates.emplace_back(target);

                target = Coordinates(origin.x + point.x, origin.y - point.y, origin.z);
                if (this->los(origin, target, radius)) fovCoordinates.emplace_back(target);

                target = Coordinates(origin.x - point.x, origin.y - point.y, origin.z);
                if (this->los(origin, target, radius)) fovCoordinates.emplace_back(target);

                target = Coordinates(origin.x + point.y, origin.y + point.x, origin.z);
                if (this->los(origin, target, radius)) fovCoordinates.emplace_back(target);

                target = Coordinates(origin.x - point.y, origin.y + point.x, origin.z);
                if (this->los(origin, target, radius)) fovCoordinates.emplace_back(target);

                target = Coordinates(origin.x + point.y, origin.y - point.x, origin.z);
                if (this->los(origin, target, radius)) fovCoordinates.emplace_back(target);

                target = Coordinates(origin.x - point.y, origin.y - point.x, origin.z);
                if (this->los(origin, target, radius)) fovCoordinates.emplace_back(target);
            }
            ++point.y;
        }
        point.y = 0;
        ++point.x;
    }
    return fovCoordinates;
}

bool Area::los(const Coordinates & source, const Coordinates & target, const int & radius)
{
    // Deal with the easiest case.
    if (source == target)
    {
        return true;
    }
    // Check if there is a room at the given coordinates.
    if (this->getRoom(target) == nullptr)
    {
        return false;
    }
    // Ensure that the line will not extend too long.
    if (Area::getDistance(source, target) > radius)
    {
        return false;
    }
    // Evaluates the difference.
    double dx = target.x - source.x;
    double dy = target.y - source.y;
    // Evaluate the distance between the
    double distance = std::sqrt((dx * dx) + (dy * dy));
    // Evaluate the unit increment for both X and Y.
    // Decrease the value of precision for a faster execution with a worsening in terms of accuracy (default 6).
    double precision = 10;
    double unitx = dx / (distance * precision);
    double unity = dy / (distance * precision);
    // Evaluate the minimum value of increment.
    double min = std::min(unitx, unity);
    // Set the initial values for X and Y.
    double x = source.x + 0.5;
    double y = source.y + 0.5;
    Coordinates coordinates(source.x, source.y, source.z);
    for (double i = 0; i <= distance; i += min)
    {
        // Evaluate the integer version of the coordinates using the floor value.
        int floor_x = static_cast<int>(std::floor(x));
        int floor_y = static_cast<int>(std::floor(y));
        coordinates = Coordinates(floor_x, floor_y, source.z);
        if (coordinates == target)
        {
            return true;
        }
        if (!this->isValid(coordinates))
        {
            return false;
        }
        // Increment both x and y.
        x += unitx;
        y += unity;
    }
    return false;
}

void Area::luaRegister(lua_State * L)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Area>("Area")
        .addData("vnum", &Area::vnum, false)
        .addData("name", &Area::name, false)
        .addData("builder", &Area::builder, false)
        .addData("width", &Area::width, false)
        .addData("height", &Area::height, false)
        .addData("elevation", &Area::elevation, false)
        .endClass();
}
