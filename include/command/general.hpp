/// @file   general.hpp
/// @author Enrico Fraccaroli
/// @date   Aug 23 2016
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

#include "input/argument.hpp"
#include "character/character.hpp"

/// Loads all the general commands.
void LoadGeneralCommands();

/// @defgroup ComInterfaces Commands List Interface.
/// @brief All the functions necessary to handle the commands that a player
///         can execute.
/// @{

/// Disconnect the player from the Mud.
bool DoQuit(Character *character, ArgumentHandler &args);

/// Shows a list of all the online players.
bool DoWho(Character *character, ArgumentHandler &args);

/// Allow to modify the character informations.
bool DoSet(Character *character, ArgumentHandler &args);

/// @brief The character look at: the current room, the near rooms, an
/// object, a player, a character, and so on.
bool DoLook(Character *character, ArgumentHandler &args);

/// Provide a complete help on the mud.
bool DoHelp(Character *character, ArgumentHandler &args);

/// Allow character to modify his prompt.
bool DoPrompt(Character *character, ArgumentHandler &args);

/// Report the current day phase.
bool DoTime(Character *character, ArgumentHandler &args);

/// Show player statistics.
bool DoStatistics(Character *character, ArgumentHandler &args);

/// Show player effects.
bool DoEffects(Character *character, ArgumentHandler &args);

/// Allow player to rent and disconnect.
bool DoRent(Character *character, ArgumentHandler &args);

/// Show player skills.
bool DoSkills(Character *character, ArgumentHandler &args);

/// Show player list of actions.
bool DoActions(Character *character, ArgumentHandler &args);

/// Show server informations and statistics.
bool DoServer(Character *character, ArgumentHandler &args);

/// Generates a random name.
bool DoGenerateName(Character *character, ArgumentHandler &args);

///@}
