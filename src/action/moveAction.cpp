/// @file   moveAction.cpp
/// @brief  Implementation of the class for a move action.
/// @author Enrico Fraccaroli
/// @date   Jul 14 2016
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

#include "moveAction.hpp"
#include "../logger.hpp"
#include "../character.hpp"

using namespace std::chrono;

MoveAction::MoveAction(
    Character * _actor,
    Room * _destination,
    Direction _direction,
    unsigned int _cooldown) :
        GeneralAction(_actor, system_clock::now() + seconds(_cooldown)),
        destination(_destination),
        direction(_direction)
{
    Logger::log(LogLevel::Debug, "Created move action.");
}

MoveAction::~MoveAction()
{
    Logger::log(LogLevel::Debug, "Deleted move action.");
}

bool MoveAction::check() const
{
    bool correct = GeneralAction::check();
    correct &= this->checkDestination();
    correct &= this->checkDirection();
    return correct;
}

ActionType MoveAction::getType() const
{
    return ActionType::Move;
}

std::string MoveAction::getDescription() const
{
    return "moving";
}

std::string MoveAction::stop()
{
    return "You stop moving.";
}

ActionStatus MoveAction::perform()
{
    // Check if the cooldown is ended.
    if (!this->checkElapsed())
    {
        return ActionStatus::Running;
    }
    // Create a variable which will contain the ammount of consumed stamina.
    std::string error;
    // Check if the actor has enough stamina to execute the action.
    if (!actor->canMoveTo(direction, error))
    {
        // Notify that the actor can't move because too tired.
        actor->sendMsg(error + "\n");
        return ActionStatus::Error;
    }
    else
    {
        // Get the amount of required stamina.
        unsigned int consumedStamina = actor->getConsumedStaminaFor(ActionType::Building);
        // Consume the stamina.
        actor->remStamina(consumedStamina, true);
        // Move character.
        actor->moveTo(
            destination,
            actor->getNameCapital() + " goes " + GetDirectionName(direction) + ".\n",
            actor->getNameCapital() + " arives from " + GetDirectionName(InverDirection(direction))
                + ".\n");
    }
    return ActionStatus::Finished;
}

bool MoveAction::checkDestination() const
{
    if (this->destination == nullptr)
    {
        Logger::log(LogLevel::Error, "No destination has been set.");
        return false;
    }
    return true;
}

bool MoveAction::checkDirection() const
{
    if (this->direction == Direction::None)
    {
        Logger::log(LogLevel::Error, "No direction has been set.");
        return false;
    }
    return true;
}