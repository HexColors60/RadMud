/// @file   processNewConfirm.cpp
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

#include "input/initialization/processNewConfirm.hpp"

#include "input/initialization/processNewWeight.hpp"
#include "utilities/formatter.hpp"
#include "character/player.hpp"
#include "mud.hpp"

bool ProcessNewConfirm::process(ArgumentHandler &args)
{
	auto player = character->toPlayer();
	auto input = args.getOriginal();
	if (ToLower(input) == "back") {
		// Create a shared pointer to the previous step.
		auto newStep = std::make_shared<ProcessNewWeight>(character);
		// Set the handler.
		player->inputProcessor = newStep;
		// Advance to the next step.
		newStep->rollBack();
		return true;
	} else if (ToLower(input) == "confirm") {
		// Initialize the player.
		player->initialize();
		// Set the base variables for the player.
		player->level = 0;
		player->experience = 0;
		player->flags = 0;
		player->rent_room = 1000;
		for (const auto &skillData : player->race->skills) {
			player->skillManager.addSkill(skillData->skill,
										  skillData->skillLevel);
		}
		// Update the player on the database.
		SQLiteDbms::instance().beginTransaction();
		if (player->updateOnDB()) {
			// Set the handler.
			player->inputProcessor = std::make_shared<ProcessInput>(character);
			// Entered the MUD.
			player->enterGame();
			// Set the connection state to playing.
			player->connectionState = ConnectionState::Playing;
			return true;
		} else {
			SQLiteDbms::instance().rollbackTransection();
		}
		player->closeConnection();
		SQLiteDbms::instance().endTransaction();
	} else {
		error = "You must write 'confirm' if you agree.";
		this->advance();
	}
	return false;
}

void ProcessNewConfirm::advance()
{
	// Print the choices.
	this->printChoices();

	std::stringstream ss;
	ss << "# Give a look to the information you have provided, now it's the right time";
	ss << " to decide if you want to change something.\n";
	ss << "# Type [" << Formatter::magenta("confirm")
	   << "] to conclude the character creation.\n";
	ss << "# Type [" << Formatter::magenta("back")
	   << "]    to return to the previous step.\n";
	ss << Formatter::green("Do you confirm?") << "\n";
	character->sendMsg(ss.str());
	this->printError();
}

void ProcessNewConfirm::rollBack()
{
}
