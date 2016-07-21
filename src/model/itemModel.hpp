/// @file   itemModel.hpp
/// @brief  Define item model variables and methods.
/// @author Enrico Fraccaroli
/// @date   Mar 31 2015
/// @copyright
/// Copyright (c) 2015, 2016 Enrico Fraccaroli <enrico.fraccaroli@gmail.com>
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
#include <vector>
#include <set>
#include <map>

#include "../defines.hpp"
#include "../lua/lua_script.hpp"

class Item;
class Player;
class Material;

class ArmorModel;
class BookModel;
class ContainerModel;
class CorpseModel;
class CurrencyModel;
class FoodModel;
class FurnitureModel;
class KeyModel;
class LightModel;
class LiquidContainerModel;
class MechanismModel;
class NodeModel;
class ProjectileModel;
class ResourceModel;
class RopeModel;
class SeedModel;
class ShieldModel;
class ToolModel;
class VehicleModel;
class WeaponModel;

/// @brief Holds details about a model of item.
class ItemModel
{
    public:
        /// Unique vnum.
        int vnum;
        /// The model name.
        std::string name;
        /// The model article.
        std::string article;
        /// The model short description.
        std::string shortdesc;
        /// The model keys.
        std::vector<std::string> keys;
        /// The model description.
        std::string description;
        /// The model type.
        ModelType modelType;
        /// Store here the position where the model can be equipped.
        EquipmentSlot slot;
        /// The model flags.
        unsigned int modelFlags;
        /// The model weight.
        unsigned int weight;
        /// The model price.
        int price;
        /// The model maximum condition.
        int condition;
        /// The model maximum condition.
        int decay;
        /// The model's material.
        MaterialType material;
        /// TileSet of the icon.
        int tileSet;
        /// TileId of the icon.
        int tileId;

        /// @brief Constructor.
        ItemModel();

        /// @brief Disable Copy Construct.
        ItemModel(ItemModel const &) = delete;

        /// @brief Disable Move construct.
        ItemModel(ItemModel &&) = delete;

        /// @brief Disable Copy assign.
        ItemModel & operator=(ItemModel const &) = delete;

        /// @brief Disable Move assign.
        ItemModel & operator=(ItemModel &&) = delete;

        /// @brief Destructor.
        virtual ~ItemModel();

        /// @return Provides the type of model.
        virtual ModelType getType() const;

        /// @return Provides a string representing the type of model.
        virtual std::string getTypeName() const = 0;

        /// @return Extracts from the source string the information
        ///          about the model and set them.
        /// @param source The source string.
        /// @return <b>True</b> if the information are correct,<br>
        ///         <b>False</b> otherwise.
        virtual bool setModel(const std::string & source) = 0;

        /// @brief Check the correctness of the model.
        /// @return <b>True</b> if the model has correct values,<br>
        ///         <b>False</b> otherwise.
        bool check();

        /// @brief Given the argumtens the function replace the symbols inside the source string
        ///         using the provided arguments.
        /// @param source       The source string.
        /// @param itemMaterial The material of which the model is made.
        /// @param itemQuality  The quality of the model.
        /// @return <b>True</b> if something changed inside the string,<br>
        ///         <b>False</b> otherwise.
        bool replaceSymbols(
            std::string & source,
            Material * itemMaterial = nullptr,
            ItemQuality itemQuality = ItemQuality::Normal);

        /// @brief Returns the name of the model depending on the passed arguments.
        /// @param itemMaterial The material of which the model is made.
        /// @param itemQuality  The quality of the model.
        /// @return The specific name of the model.
        std::string getName(Material * itemMaterial = nullptr, ItemQuality itemQuality =
            ItemQuality::Normal);

        /// @brief Returns the description of the model depending on the passed arguments.
        /// @param itemMaterial The material of which the model is made.
        /// @param itemQuality  The quality of the model.
        /// @return The specific description of the model.
        std::string getDescription(Material * itemMaterial = nullptr, ItemQuality itemQuality =
            ItemQuality::Normal);

        /// @brief Create a new item starting from this model.
        /// @param maker       The player that create the item.
        /// @param composition The composition of the item.
        /// @param itemQuality The quality of the item.
        /// @return The newly created item.
        Item * createItem(std::string maker, Material * composition, ItemQuality itemQuality);

        /// @brief Check if the item must be wielded.
        /// @return <b>True</b> if the item must be wielded,<br><b>False</b> Otherwise.
        bool mustBeWielded();

        /// @brief Function used to register inside the lua environment the class.
        /// @param L The lua environment.
        static void luaRegister(lua_State * L);

        /// @brief Returns the tile of the model.
        /// @return The string which contains the code of the tile.
        std::string getTile(int offset = 0);

    public:
        /// @brief Returns the model <b>statically</b> casted to Armor.
        ArmorModel * toArmor();
        /// @brief Returns the model <b>statically</b> casted to Book.
        BookModel * toBook();
        /// @brief Returns the model <b>statically</b> casted to Container.
        ContainerModel * toContainer();
        /// @brief Returns the model <b>statically</b> casted to Corpse.
        CorpseModel * toCorpse();
        /// @brief Returns the model <b>statically</b> casted to Currency.
        CurrencyModel * toCurrency();
        /// @brief Returns the model <b>statically</b> casted to Food.
        FoodModel * toFood();
        /// @brief Returns the model <b>statically</b> casted to Furniture.
        FurnitureModel * toFurniture();
        /// @brief Returns the model <b>statically</b> casted to Key.
        KeyModel * toKey();
        /// @brief Returns the model <b>statically</b> casted to Light.
        LightModel * toLight();
        /// @brief Returns the model <b>statically</b> casted to Liquid Container.
        LiquidContainerModel * toLiquidContainer();
        /// @brief Returns the model <b>statically</b> casted to Mechanism.
        MechanismModel * toMechanism();
        /// @brief Returns the model <b>statically</b> casted to Node.
        NodeModel * toNode();
        /// @brief Returns the model <b>statically</b> casted to Projectile.
        ProjectileModel * toProjectile();
        /// @brief Returns the model <b>statically</b> casted to Resource.
        ResourceModel * toResource();
        /// @brief Returns the model <b>statically</b> casted to Rope.
        RopeModel * toRope();
        /// @brief Returns the model <b>statically</b> casted to Seed.
        SeedModel * toSeed();
        /// @brief Returns the model <b>statically</b> casted to Shield.
        ShieldModel * toShield();
        /// @brief Returns the model <b>statically</b> casted to Tool.
        ToolModel * toTool();
        /// @brief Returns the model <b>statically</b> casted to Vehicle.
        VehicleModel * toVehicle();
        /// @brief Returns the model <b>statically</b> casted to Weapon.
        WeaponModel * toWeapon();
};

/// ItemModel vector handler.
typedef std::vector<ItemModel *> ItemModelList;

/// ItemModel map handler.
typedef std::map<int, ItemModel *> ItemModelMap;

/// @defgroup FlagsToList Flags to List of Strings.
/// @brief All the functions necessary to transform into a list of string a pool of flags.
/// @{

/// Return a list of string containg the Model flags contained inside the value.
std::string GetModelFlagString(unsigned int flags);

/// @}