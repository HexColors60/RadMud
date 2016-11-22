/// @file   lua_script.hpp
/// @brief  Define all the methods used by lua to interract and communicate with the mud.
/// @author Enrico Fraccaroli
/// @date   Aug 25 2014
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

#include <string>
#include <vector>
#include <cassert>

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

class Item;

class Character;

class LuaCharacter;

/// @brief Lua wrapper for C++ vectors.
template<class T>
class VectorHelper
{
private:
    /// Wrapped vector.
    std::vector<T> vector;

public:
    /// @brief Constructor.
    VectorHelper() :
        vector()
    {
    }

    /// @brief Constructor.
    /// @param _vector The vector which has to be wrapped.
    VectorHelper(const std::vector<T> & _vector) :
        vector(_vector)
    {
    }

    /// @brief Provide the size of the vector.
    /// @return The size of the vector.
    size_t size() const
    {
        return (vector.size());
    }

    /// @brief Return a constant pointer to the last element of the vector.
    /// @return The last element of the vector.
    T back() const
    {
        return vector.back();
    }

    /// @brief Return a constant pointer to the first element of the vector.
    /// @return The first element of the vector.
    T front() const
    {
        return vector.front();
    }

    /// @brief Provides access to the data contained in the vector.
    /// @param index The index of the element for which data should be accessed.
    /// @return  Read-only (constant) reference to data.
    T at(const unsigned int index)
    {
        assert((index >= 0) && (index < vector.size()));
        return vector.at(index);
    }

    /// @brief  Add data to the end of the vector.
    /// @param  element  Data to be added.
    void push_back(T element)
    {
        vector.push_back(element);
    }

    /// @brief  Removes last element.
    void pop_back()
    {
        vector.pop_back();
    }

    /// @brief Returns if the vector is empty.
    bool empty()
    {
        return vector.empty();
    }
};

/// @brief Allow from lua code, to log a string.
/// @param message The message to log.
void LuaLog(std::string message);

/// @brief Allow from lua to call the sleep function.
/// @param sleepTime The amount of seconds.
void LuaSleep(int sleepTime);

/// @brief Allow from lua coden to generate a random integer.
/// @param min Lower bound.
/// @param max Upper bound.
/// @return The generated random value.
int LuaRandom(int min, int max);

/// @brief Allow from lua to load an item.
/// @param maker        The maker of the new item.
/// @param vnumModel    The vnum of the model.
/// @param vnumMaterial The vnum of the material.
/// @param qualityValue The initial quality of the item.
/// @return The newly created item.
Item * LuaLoadItem(Character * maker, int vnumModel, int vnumMaterial, unsigned int qualityValue);

/// @brief Register in Lua all the functions.
/// @param L The lua state.
void LuaRegisterUtils(lua_State * L);
