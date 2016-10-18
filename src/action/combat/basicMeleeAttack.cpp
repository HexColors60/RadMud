/// @file   basicMeleeAttack.cpp
/// @brief  Contais the implementation of the class for the basic melee attacks.
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

#include "basicMeleeAttack.hpp"
#include "../../structure/room.hpp"
#include "../../item/meleeWeaponItem.hpp"
#include "../../model/meleeWeaponModel.hpp"

BasicMeleeAttack::BasicMeleeAttack(Character * _actor) :
    CombatAction(_actor)
{
    Logger::log(LogLevel::Debug, "Created BasicMeleeAttack.");
}

BasicMeleeAttack::~BasicMeleeAttack()
{
    Logger::log(LogLevel::Debug, "Deleted BasicMeleeAttack.");
}

bool BasicMeleeAttack::check() const
{
    bool correct = CombatAction::check();
    return correct;
}

ActionType BasicMeleeAttack::getType() const
{
    return ActionType::Combat;
}

std::string BasicMeleeAttack::getDescription() const
{
    return "fighting";
}

std::string BasicMeleeAttack::stop()
{
    return "You stop fighting.";
}

ActionStatus BasicMeleeAttack::perform()
{
    // Check if the cooldown is ended.
    if (!this->checkElapsed())
    {
        return ActionStatus::Running;
    }
    Logger::log(LogLevel::Debug, "[%s] Perform a BasicMeleeAttack.", actor->getName());
    auto activeWeapons = actor->getActiveMeleeWeapons();
    if (activeWeapons.empty())
    {
        actor->sendMsg("You do not have a valid weapon equipped.\n");
    }
    else
    {
        for (auto iterator : activeWeapons)
        {
            // Get the top aggro enemy at range.
            Character * enemy = actor->getNextOpponentAtRange(1);
            if (enemy == nullptr)
            {
                actor->sendMsg(
                    "You do not have opponents at reange for %s.\n",
                    iterator->getName(true));
                continue;
            }
            // Get the required stamina.
            unsigned int consumedStamina = this->getConsumedStamina(actor, iterator);
            // Check if the actor has enough stamina to execute the action.
            if (consumedStamina > actor->getStamina())
            {
                actor->sendMsg("You are too tired to attack with %s.\n", iterator->getName(true));
                Logger::log(
                    LogLevel::Debug,
                    "[%s] Has %s stamina and needs %s.",
                    actor->getName(),
                    ToString(actor->getStamina()),
                    ToString(consumedStamina));
                continue;
            }
            // Natural roll for the attack.
            unsigned int ATK = TRandInteger<unsigned int>(1, 20);
            // Log the rolled value.
            Logger::log(
                LogLevel::Debug,
                "[%s] Natural roll of %s.",
                actor->getName(),
                ToString(ATK));
            // Check if:
            //  1. The number of active weapons is more than 1, then we have to apply
            //      a penality to the attack roll.
            //  2. The value is NOT a natural 20 (hit).
            if ((activeWeapons.size() > 1) && (ATK != 20))
            {
                // Evaluate the penality to the attack roll.
                unsigned int penality = 0;
                // On the main hand the penality is 6.
                if (iterator->currentSlot == EquipmentSlot::RightHand)
                {
                    penality = 6;
                }
                    // On the off hand the penality is 10.
                else if (iterator->currentSlot == EquipmentSlot::LeftHand)
                {
                    penality = 10;
                }
                // Log that we have applied a penality.
                Logger::log(
                    LogLevel::Debug,
                    "[%s] Suffer a %s penalty with its %s.",
                    actor->getName(),
                    ToString(penality),
                    GetEquipmentSlotName(iterator->currentSlot));
                // Safely apply the penality.
                if (ATK < penality)
                {
                    ATK = 0;
                }
                else
                {
                    ATK -= penality;
                }
            }
            // Evaluate the armor class of the enemy.
            unsigned int AC = enemy->getArmorClass();
            // Log the overall attack roll.
            Logger::log(
                LogLevel::Debug,
                "[%s] Attack roll %s vs %s.",
                actor->getName(),
                ToString(ATK),
                ToString(AC));
            // Check if:
            //  1. The attack roll is lesser than the armor class of the opponent.
            //  2. The attack roll is not a natural 20.
            if ((ATK < AC) && (ATK != 20))
            {
                // Notify the actor.
                actor->sendMsg("You miss %s with %s.\n\n", enemy->getName(), iterator->getName(true));
                // Notify the enemy.
                enemy->sendMsg("%s misses you with %s.\n\n", actor->getName(), iterator->getName(true));
                // Notify the others.
                enemy->room->sendToAll(
                    "%s miss %s with %s.\n",
                    {actor, enemy},
                    actor->getName(),
                    enemy->getName(),
                    iterator->getName(true));
                // Consume half the stamina.
                actor->remStamina(consumedStamina / 2, true);
            }
            else
            {
                // Consume the stamina.
                actor->remStamina(consumedStamina, true);
                // Store the type of attack.
                bool isCritical = false;
                // Natural roll for the damage.
                unsigned int DMG = iterator->rollDamage();
                // Log the damage roll.
                Logger::log(
                    LogLevel::Debug,
                    "[%s] Rolls a damage of %s.",
                    actor->getName(),
                    ToString(DMG));
                // Check if the character is wielding a two-handed weapon.
                if (activeWeapons.size() == 1)
                {
                    if (HasFlag(iterator->model->modelFlags, ModelFlag::TwoHand))
                    {
                        // Get the strenth modifier.
                        unsigned int STR = actor->getAbilityModifier(Ability::Strength);
                        // Add to the damage rool one and half the strenth value.
                        DMG += STR + (STR / 2);
                        // Log the additional damage.
                        Logger::log(
                            LogLevel::Debug,
                            "[%s] Add 1-1/2 times its Strength.",
                            actor->getName());
                    }
                }
                // If the character has rolled a natural 20, then multiply the damage by two.
                if (ATK == 20)
                {
                    DMG *= 2;
                    isCritical = true;
                }
                // Log the damage.
                Logger::log(
                    LogLevel::Debug,
                    "[%s] Hits %s for with %s.",
                    actor->getName(),
                    enemy->getName(),
                    ToString(DMG),
                    iterator->getName(true));
                // Procede and remove the damage from the health of the target.
                if (!enemy->remHealth(DMG))
                {
                    actor->sendMsg(
                        "You %shit %s with %s and kill %s.\n\n",
                        (isCritical ? "critically " : ""),
                        enemy->getName(),
                        iterator->getName(true),
                        enemy->getObjectPronoun());
                    // Notify the enemy.
                    enemy->sendMsg(
                        "%s %shits you with %s and kill you.\n\n",
                        actor->getName(),
                        (isCritical ? "critically " : ""),
                        iterator->getName(true));
                    // Notify the others.
                    enemy->room->sendToAll(
                        "%s %shits %s with %s and kill %s.\n",
                        {actor, enemy},
                        actor->getName(),
                        (isCritical ? "critically " : ""),
                        enemy->getName(),
                        iterator->getName(true),
                        enemy->getObjectPronoun());
                    // The enemy has received the damage and now it is dead.
                    enemy->kill();
                    continue;
                }
                else
                {
                    // The enemy has received the damage but it is still alive.
                    actor->sendMsg(
                        "You %shit %s with %s for %s.\n\n",
                        (isCritical ? "critically " : ""),
                        enemy->getName(),
                        iterator->getName(true),
                        ToString(DMG));
                    // Notify the enemy.
                    enemy->sendMsg(
                        "%s %shits you with %s for %s.\n\n",
                        actor->getName(),
                        (isCritical ? "critically " : ""),
                        iterator->getName(true),
                        ToString(DMG));
                    // Notify the others.
                    enemy->room->sendToAll(
                        "%s %shits %s with %s for %s.\n",
                        {actor, enemy},
                        actor->getName(),
                        (isCritical ? "critically " : ""),
                        enemy->getName(),
                        iterator->getName(true),
                        ToString(DMG));
                }
            }
        }
    }
    // By default set the next combat action to basic attack.
    if (!actor->setNextCombatAction(CombatActionType::BasicMeleeAttack))
    {
        actor->sendMsg(this->stop() + "\n\n");
        return ActionStatus::Finished;
    }
    return ActionStatus::Running;
}

CombatActionType BasicMeleeAttack::getCombatActionType() const
{
    return CombatActionType::BasicMeleeAttack;
}

unsigned int BasicMeleeAttack::getConsumedStamina(Character * character, MeleeWeaponItem * weapon)
{
    if (weapon->model->toMeleeWeapon()->meleeWeaponType == MeleeWeaponType::Placed)
    {
        return 0;
    }
    // BASE     [+1.0]
    // STRENGTH [-0.0 to -2.80]
    // WEIGHT   [+1.6 to +2.51]
    // CARRIED  [+0.0 to +2.48]
    // WEAPON   [+0.0 to +1.60]
    return static_cast<unsigned int>(1.0
                                     - character->getAbilityLog(Ability::Strength, 0.0, 1.0)
                                     + SafeLog10(character->weight)
                                     + SafeLog10(character->getCarryingWeight())
                                     + SafeLog10(weapon->getWeight(true)));
}
