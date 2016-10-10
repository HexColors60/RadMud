/// @file   generalAction.hpp
/// @brief  Defines the base class for any kind of action.
/// @author Enrico Fraccaroli
/// @date   Jul 13 2016
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

#include "../production.hpp"
#include "../building.hpp"
#include "../defines.hpp"
#include "../opponent.hpp"
#include "../item/item.hpp"

#include <memory>
#include <chrono>

class BuildAction;

class CraftAction;

class MoveAction;

class CombatAction;

/// The list of possible actions.
using ActionType = enum class ActionType_t
{
    Wait,       ///< The character is doing common action.
    Move,       ///< The character is moving to another location.
    Crafting,   ///< The character is crafting something.
    Building,   ///< The character is crafting something.
    Combat,     ///< The character is fighting.
    Scout,      ///< The character is scouting the area.
    Reload,     ///< The character is reloading a firearm.
    Load        ///< The character is loading something.
};

/// The list of possible actions.
using ActionStatus = enum class ActionStatus_t
{
    Finished,   ///< The action is finished.
    Running,    ///< The action is still running.
    Error       ///< The action has encountered an error.
};

/// @brief A general action. Performing this action will result in doing nothing.
class GeneralAction :
    public std::enable_shared_from_this<GeneralAction>
{
protected:
    /// Actor of the action.
    Character * actor;
    /// The time point in the future needed by the action to complete.
    std::chrono::time_point<std::chrono::system_clock> actionCooldown;

public:
    /// @brief Constructor.
    GeneralAction(Character * _actor);

    /// @brief Constructor.
    GeneralAction(Character * _actor, std::chrono::time_point<std::chrono::system_clock> _actionCooldown);

    /// @brief Destructor.
    virtual ~GeneralAction();

    /// @brief Check if the cooldown of the action is elapsed.
    /// @return <b>True</b> if the time has passed,<br>
    ///         <b>False</b> otherwise.
    bool checkElapsed() const;

    /// @brief Get the elapsed time.
    long int getElapsed() const;

    /// @brief Checks the correctness of the action's values.
    /// @return <b>True</b> if it has correct values,<br>
    ///         <b>False</b> otherwise.
    virtual bool check() const;

    /// @brief Provides the type of the action.
    /// @return The type of action.
    virtual ActionType getType() const;

    /// @brief Provides the description of the action.
    /// @return The string which describe the current action.
    virtual std::string getDescription() const;

    /// @brief Stops the current action and returns a string which describe the intterruption.
    /// @return The stopping description.
    virtual std::string stop();

    /// @brief Performs the current action.
    /// @return the status after performing the action.
    virtual ActionStatus perform();

    /// @brief Allows to set the remaining time before the action can be triggered.
    /// @param _actionCooldown The cooldown that has to be set.
    void resetCooldown(const unsigned int & _actionCooldown);

    /// @brief Provides the remaining time before the action can be triggered.
    unsigned int getCooldown();

    /// @brief Returns the action <b>statically</b> casted to CombatAction.
    std::shared_ptr<CombatAction> toCombatAction();
};

/// @addtogroup EnumToString
/// @{

/// Return the string describing the type of Action.
std::string GetActionTypeName(ActionType type);

/// @}
