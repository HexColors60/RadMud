/// @file   processNewName.cpp
/// @author Enrico Fraccaroli
/// @date   Nov 14, 2016
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

#include "processNewName.hpp"
#include "player.hpp"
#include "mud.hpp"
#include "processPlayerName.hpp"
#include "processNewPassword.hpp"

void ProcessNewName::process(Character * character, ArgumentHandler & args)
{
    auto player = character->toPlayer();
    auto input = args.getOriginal();
    // Player_password can't be blank.
    if (input.empty())
    {
        this->advance(character, "Invalid input.");
    }
    else if (ToLower(input) == "back")
    {
        // Create a shared pointer to the previous step.
        std::shared_ptr<ProcessPlayerName> newStep = std::make_shared<ProcessPlayerName>();
        // Set the handler.
        player->inputHandler = newStep;
        // Advance to the next step.
        newStep->rollBack(character);
    }
    else if (input.find_first_not_of(VALID_CHARACTERS_NAME) != std::string::npos)
    {
        this->advance(character, "That player name contains disallowed characters.");
    }
        // Check for bad names here.
    else if (Mud::instance().badNames.find(input) != Mud::instance().badNames.end())
    {
        this->advance(character, "That name is not permitted.");
    }
        // Check if the player name has already been used.
    else if (SQLiteDbms::instance().searchPlayer(ToCapitals(input)))
    {
        this->advance(character, "That player already exists, please choose another name.");
    }
    else
    {
        Room * spawnRoom = Mud::instance().findRoom(player->rent_room);
        if (spawnRoom == nullptr)
        {
            spawnRoom = Mud::instance().findRoom(1000);
        }
        player->name = ToCapitals(input);
        player->room = spawnRoom;
        player->faction = Mud::instance().findFaction(1);
        player->prompt_save = "[" + player->name + "]";
        player->password_attempts = 0;
        // Create a shared pointer to the next step.
        std::shared_ptr<ProcessNewPassword> newStep = std::make_shared<ProcessNewPassword>();
        // Set the handler.
        player->inputHandler = newStep;
        // Advance to the next step.
        newStep->advance(character);
    }
}

void ProcessNewName::advance(Character * character, const std::string & error)
{
    // Change the connection state to awaiting age.
    character->toPlayer()->connectionState = ConnectionState::AwaitingNewName;
    // Print the choices.
    this->printChices(character);
    std::string msg;
    msg += "# " + Formatter::bold() + "Character's Name." + Formatter::reset() + "\n";
    msg += "# Choose carefully, because this it's the only chance you have";
    msg +=
        " to pick a legendary name, maybe one day it will be whispered all over the lands.\n";
    msg += "# Type [" + Formatter::magenta() + "back" + Formatter::reset()
           + "] to return to the login.\n";
    character->sendMsg(msg);
    if (!error.empty())
    {
        character->sendMsg("# " + error + "\n");
    }
}

void ProcessNewName::rollBack(Character * character)
{
    auto player = character->toPlayer();
    player->name = "";
    this->advance(character);
}
