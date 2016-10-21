/// @file   building.hpp
/// @brief  Define building variables and methods.
/// @author Enrico Fraccaroli
/// @date   Feb 24 2016
/// @copyright
/// Copyright (c) 2016 Enrico Fraccaroli <enrico.fraccaroli@gmail.com>
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

#pragma once

#include <string>
#include <map>

#include "toolModel.hpp"
#include "resourceModel.hpp"

class ItemModel;

/// @brief Holds details about a building.
class Building
{
public:
    /// The virtual number of the building.
    int vnum;
    /// The name of the building.
    std::string name;
    /// The difficulty of the building.
    unsigned int difficulty;
    /// The time required to perform the building.
    unsigned int time;
    /// A flag which indicates if the maker can be assisted by someone.
    bool assisted;
    /// The list of needed type of tools.
    std::set<ToolType> tools;
    /// The model that has to be built.
    ItemModel * buildingModel;
    /// The list of needed type of ingredients and their quantity.
    std::map<ResourceType, unsigned int> ingredients;
    /// Flag which determine if in the same room only one of this building can be present.
    bool unique;

    /// @brief Constructor.
    Building();

    /// @brief Copy Constructor.
    Building(const Building & source);

    /// @brief Destructor.
    ~Building();

    /// @brief Check the correctness of the building.
    /// @return <b>True</b> if the profession has correct values,<br> <b>False</b> otherwise.
    bool check();

    /// @brief Return the name of the building.
    /// @return The name of the building.
    std::string getName();

    /// @brief Return the name of the building with the first letter capitalized.
    /// @return The name of the building capitalized.
    std::string getNameCapital();

    /// @brief Given a source string, this function parse the string
    ///         and sets the tool.
    /// @param source Contains the list of tools.
    /// @return <b>True</b> if the operation succeeded,<br>
    ///         <b>False</b> otherwise.
    bool setTool(const std::string & source);

    /// @brief Given a source string, this function parse the string
    ///         and sets the tool.
    /// @param source Contains the list of tools.
    /// @return <b>True</b> if the operation succeeded,<br>
    ///         <b>False</b> otherwise.
    bool setIngredient(const std::string & source);
};