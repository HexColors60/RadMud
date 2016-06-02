/// @file   comm_combat.cpp
/// @brief  Implements the methods used by the character in order to <b>fight</b>.
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
#include "room.hpp"
#include "commands.hpp"
#include "constants.hpp"

using namespace std;

void DoKill(Character * character, std::istream & sArgs)
{
    // Get the arguments of the command.
    ArgumentList arguments = ParseArgs(sArgs);
    // If there are no arguments, show the room.
    if (arguments.size() != 1)
    {
        character->sendMsg("You have to specify whom to kill.\n");
    }

    // Retrieve the target.
    Character * target = character->room->findCharacter(arguments[0].first, arguments[0].second, character);
    if (!target)
    {
        character->sendMsg("You don't see '" + arguments[0].first + "' anywhere.\n");
        return;
    }

    // Check if the attacker can see the target.
    if (!character->canSee(target))
    {
        character->sendMsg("You don't see '" + arguments[0].first + "' anywhere.\n");
        return;
    }

    // Check if the character is already in combat.
    if (character->getAction()->getType() == ActionType::Combat)
    {
        OpponentsList * opponents = character->getAction()->getOpponentsList();
        // Check if the target is part of the same combat action, thus
        //  check if is one of its opponents.
        if (!opponents->hasOpponent(target))
        {
            character->sendMsg("You are already fighting!\n");
            return;
        }
        if (opponents->moveToTopAggro(character))
        {
            character->sendMsg("You focus your attacks on " + target->getName() + "!\n");
        }
        else
        {
            character->sendMsg("You are already doing your best to kill " + target->getName() + "!\n");
        }
    }
    else
    {
        // So, basically if a player attacks a Target and both are not involved
        //  in any combat action, then we can start their combat action.
        bool attackerRes = character->getAction()->setCombat(target, CombatAction::BasicAttack);
        bool defenderRes = target->getAction()->setCombat(character, CombatAction::BasicAttack);
        if (attackerRes && defenderRes)
        {
            character->sendMsg("You attack " + target->getName() + "!\n");
            target->sendMsg(character->getNameCapital() + " attacks you!\n");
        }
        else
        {
            character->sendMsg("You cannot attack " + target->getName() + ".\n");
            character->getAction()->stop();
            target->getAction()->stop();
        }
    }
}
