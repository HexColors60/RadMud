/// @file   loadAction.hpp
/// @brief  Class which manage loading activities.
/// @author Enrico Fraccaroli
/// @date   Oct 10 2016
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

#include "generalAction.hpp"

class MagazineItem;
class Item;

/// @brief Allows to load something.
class LoadAction : public GeneralAction {
private:
	/// The magazine which has to be loaded.
	MagazineItem *magazine;
	/// The projectile used to load the item.
	Item *projectile;
	/// The amount that has to be loaded.
	unsigned int amount;

public:
	/// @brief Constructor.
	LoadAction(Character *_actor, MagazineItem *_magazine, Item *_projectile,
			   const unsigned int &_amount);

	/// @brief Destructor.
	virtual ~LoadAction();

	bool check(std::string &error) const override;

	ActionType getType() const override;

	std::string getDescription() const override;

	bool start() override;

	std::string stop() override;

	ActionStatus perform() override;

	unsigned int getCooldown() override;
};