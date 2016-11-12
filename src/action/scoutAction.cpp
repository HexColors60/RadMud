/// @file   scoutAction.cpp
/// @brief  Implementation of the class for scouting an area.
/// @author Enrico Fraccaroli
/// @date   Oct 9 2016
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

#include "scoutAction.hpp"
#include "room.hpp"
#include "area.hpp"
#include "effectFactory.hpp"

ScoutAction::ScoutAction(Character * _actor, unsigned int _cooldown) :
    GeneralAction(_actor, std::chrono::system_clock::now() + std::chrono::seconds(_cooldown))
{
    Logger::log(LogLevel::Debug, "Created scout action.");
}

ScoutAction::~ScoutAction()
{
    Logger::log(LogLevel::Debug, "Deleted scout action.");
}

bool ScoutAction::check(std::string & error) const
{
    if (!GeneralAction::check(error))
    {
        return false;
    }
    if (actor->room == nullptr)
    {
        Logger::log(LogLevel::Error, "The actor's room is a nullptr.");
        return false;
    }
    if (actor->room->area == nullptr)
    {
        Logger::log(LogLevel::Error, "The room's area is a nullptr.");
        return false;
    }
    if (this->getConsumedStamina(actor) > actor->getStamina())
    {
        error = "You are too tired to scout the area.";
        return false;
    }
    return true;
}

ActionType ScoutAction::getType() const
{
    return ActionType::Scout;
}

std::string ScoutAction::getDescription() const
{
    return "scouting";
}

std::string ScoutAction::stop()
{
    return "You stop scouting the area.";
}

ActionStatus ScoutAction::perform()
{
    // Check if the cooldown is ended.
    if (!this->checkElapsed())
    {
        return ActionStatus::Running;
    }
    std::string error;
    if (!this->check(error))
    {
        actor->sendMsg(error + "\n\n");
        return ActionStatus::Error;
    }
    // Get the amount of required stamina and try to consume it.
    actor->remStamina(this->getConsumedStamina(actor));
    // Get the characters in sight.
    CharacterContainer exceptions;
    exceptions.emplace_back(actor);
    actor->charactersInSight = actor->room->area->getCharactersInSight(exceptions,
                                                                       actor->room->coord,
                                                                       actor->getViewDistance());
    if (actor->charactersInSight.empty())
    {
        actor->sendMsg("You have found nothing...\n");
        return ActionStatus::Error;
    }
    actor->sendMsg("Nearby you can see...\n");
    for (auto it : actor->charactersInSight)
    {
        actor->sendMsg("    %s\n", it->getName());
    }
    actor->sendMsg("\n");
    // Add the effect.
    unsigned int modifier = actor->getAbilityModifier(Ability::Perception);
    actor->effects.forceAddEffect(EffectFactory::clearTargets(actor, 2 + modifier));
    return ActionStatus::Finished;
}

unsigned int ScoutAction::getConsumedStamina(Character * character)
{
    // BASE     [+1.0]
    // STRENGTH [-0.0 to -2.80]
    // WEIGHT   [+1.6 to +2.51]
    // CARRIED  [+0.0 to +2.48]
    auto consumed = 1.0
                    - character->getAbilityLog(Ability::Strength, 0.0, 1.0)
                    + SafeLog10(character->weight)
                    + SafeLog10(character->getCarryingWeight());
    return (consumed < 0) ? 0 : static_cast<unsigned int>(consumed);
}
