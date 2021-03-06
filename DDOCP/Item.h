// Item.h
//
// An XML object wrapper that holds information on an item that can be equipped
#pragma once
#include "XmlLib\DLMacros.h"
#include "Augment.h"
#include "Effect.h"
#include "EquipmentSlot.h"

class Item :
    public XmlLib::SaxContentElement
{
    public:
        Item(void);
        void Write(XmlLib::SaxWriter * writer) const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Item_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_STRING(_, Icon) \
                DL_STRING(_, Description) \
                DL_OPTIONAL_SIMPLE(_, size_t, Enchantment, 0) \
                DL_OPTIONAL_SIMPLE(_, size_t, AC, 0) \
                DL_OPTIONAL_SIMPLE(_, size_t, AC_CompositePlating, 0) \
                DL_OPTIONAL_SIMPLE(_, size_t, AC_MithralBody, 0) \
                DL_OPTIONAL_SIMPLE(_, size_t, AC_AdamantineBody, 0) \
                DL_OPTIONAL_SIMPLE(_, size_t, ShieldBonus, 0) \
                DL_OPTIONAL_SIMPLE(_, size_t, MaxDexBonus, 0) \
                DL_OPTIONAL_SIMPLE(_, size_t, BlockingDR, 0) \
                DL_OPTIONAL_SIMPLE(_, size_t, ArmorCheckPenalty, 0) \
                DL_OPTIONAL_SIMPLE(_, int, ArcaneSpellFailure, 0) \
                DL_OPTIONAL_OBJECT(_, Dice, Damage) \
                DL_STRING_LIST(_, DRBypass) \
                DL_OPTIONAL_SIMPLE(_, size_t, CriticalRoll, 20) \
                DL_OPTIONAL_SIMPLE(_, size_t, CriticalMultiplier, 2) \
                DL_OPTIONAL_ENUM(_, WeaponType, Weapon, Weapon_Unknown, weaponTypeMap) \
                DL_OPTIONAL_ENUM(_, WeaponDamageType, DamageType, WeaponDamage_Unknown, weaponDamageTypeMap) \
                DL_OPTIONAL_ENUM(_, AbilityType, AttackModifier, Ability_Unknown, abilityTypeMap) \
                DL_OPTIONAL_ENUM(_, AbilityType, DamageModifier, Ability_Unknown, abilityTypeMap) \
                DL_OBJECT(_, EquipmentSlot, Slots) \
                DL_OPTIONAL_SIMPLE(_, size_t, MinLevel, 0) \
                DL_OBJECT_VECTOR(_, Effect, Effects) \
                DL_OBJECT_VECTOR(_, Augment, Augments)

        DL_DECLARE_ACCESS(Item_PROPERTIES)
        DL_DECLARE_VARIABLES(Item_PROPERTIES)
};
