// Effect.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "Dice.h"

#include "AbilityTypes.h"
#include "ClassTypes.h"
#include "DamageReductionTypes.h"
#include "EnergyTypes.h"
#include "FavoredEnemyTypes.h"
#include "ImmunityTypes.h"
#include "SaveTypes.h"
#include "SkillTypes.h"
#include "SpellPowerTypes.h"
#include "SpellSchoolTypes.h"
#include "TacticalTypes.h"
#include "WeaponClassTypes.h"
#include "WeaponTypes.h"
#include "MeleeStyleTypes.h"
#include "WeaponDamageTypes.h"
#include "ImmunityTypes.h"

// add new feat effects here and update the map for the text saved for that
// effect type so it will be loaded/saved correctly.
enum EffectType
{
    Effect_Unknown = 0,
    Effect_AbilityBonus,
    Effect_ACBonus,
    Effect_AdditionalSpellPointCost,
    Effect_ArcaneSpellFailure,
    Effect_ArcaneSpellFailureShields,
    Effect_ArmorCheckPenalty,
    Effect_APBonus,
    Effect_Alacrity,
    Effect_AssassinateDC,
    Effect_AttackBonus,
    Effect_AttackSpeed,
    Effect_CasterLevel,
    Effect_CriticalAttackBonus,
    Effect_CriticalDamageBonus,
    Effect_CriticalMultiplier,
    Effect_CriticalRange,
    Effect_DamageBonus,
    Effect_DamageReduction,
    Effect_DragonmarkUse,
    Effect_DodgeBonus,
    Effect_DodgeBypass,
    Effect_DodgeCapBonus,
    Effect_DoubleShot,
    Effect_DoubleStrike,
    Effect_DR,
    Effect_DRBypass,
    Effect_EnhancementTree,
    Effect_EnchantArmor,
    Effect_EnchantWeapon,
    Effect_EnergyAbsorbance,
    Effect_EnergyResistance,
    Effect_ExcludeFeatSelection,
    Effect_ExtraActionBoost,
    Effect_ExtraLayOnHands,
    Effect_ExtraRage,
    Effect_ExtraRemoveDisease,
    Effect_ExtraSmite,
    Effect_ExtraTurns,
    Effect_ExtraWildEmpathy,
    Effect_FatePoint,
    Effect_Fortification,
    Effect_FortificationBypass,
    Effect_GlancingBlows,
    Effect_GlancingBlowsDamage,
    Effect_GlancingBlowEffects,
    Effect_GrantFeat,
    Effect_GrantSpell,
    Effect_HealingAmplification,
    Effect_HirelingAbilityBonus,
    Effect_HirelingHitpoints,
    Effect_HirelingFortification,
    Effect_HirelingPRR,
    Effect_HirelingMRR,
    Effect_HirelingDodge,
    Effect_HirelingMeleePower,
    Effect_HirelingRangedPower,
    Effect_HirelingSpellPower,
    Effect_Hitpoints,
    Effect_Immunity,
    Effect_MaxDexBonus,
    Effect_MaxDexBonusTowerShield,
    Effect_MaxDodgeBonus,
    Effect_MeleePower,
    Effect_MinionBonus,
    Effect_MRR,
    Effect_MonkWeaponDie,
    Effect_MovementSpeed,
    Effect_NegativeHealingAmplification,
    Effect_OffHandAttackBonus,
    Effect_OffHandDoublestrike,
    Effect_PointBlankShotRange,
    Effect_PRR,
    Effect_RangedPower,
    Effect_Regeneration,
    Effect_RepairAmplification,
    Effect_RustSusceptability,
    Effect_SaveBonus,
    Effect_SecondaryShieldBash,
    Effect_SkillBonus,
    Effect_SneakAttackDamage,
    Effect_SneakAttackRange,
    Effect_SongBonus,
    Effect_SongDuration,
    Effect_SpellCriticalChance,
    Effect_SpellCriticalDamage,
    Effect_SpellDC,
    Effect_SpellLikeAbility,
    Effect_SpellPenetrationBonus,
    Effect_SpellPoints,
    Effect_SpellPower,
    Effect_SpellResistance,
    Effect_TacticalDC,
    Effect_ThreatBonus,
    Effect_TurnBonus,
    Effect_TurnLevelBonus,
    Effect_TwistOfFate,
    Effect_UnconsciousRange,
    Effect_VorpalRange,
    Effect_WeaponBaseDamageBonus,
    Effect_WeaponBonus,
    Effect_WeaponDamageBonus,
    Effect_WeaponEnchantment,
    Effect_WeaponProficiency,
};
const XmlLib::enumMapEntry<EffectType> effectTypeMap[] =
{
    {Effect_Unknown, L"Unknown"},
    {Effect_AbilityBonus, L"AbilityBonus"},
    {Effect_ACBonus, L"ACBonus"},
    {Effect_AdditionalSpellPointCost, L"AdditionalSpellPointCost"},
    {Effect_ArcaneSpellFailure, L"ArcaneSpellFailure"},
    {Effect_ArcaneSpellFailureShields, L"ArcaneSpellFailureShields"},
    {Effect_ArmorCheckPenalty, L"ArmorCheckPenalty"},
    {Effect_APBonus, L"APBonus"},
    {Effect_Alacrity, L"Alacrity"},
    {Effect_AssassinateDC, L"AssassinateDC"},
    {Effect_AttackBonus, L"AttackBonus"},
    {Effect_AttackSpeed, L"AttackSpeed"},
    {Effect_CasterLevel, L"CasterLevel"},
    {Effect_CriticalAttackBonus, L"CriticalAttackBonus"},
    {Effect_CriticalDamageBonus, L"CriticalDamageBonus"},
    {Effect_CriticalMultiplier, L"CriticalMultiplier"},
    {Effect_CriticalRange, L"CriticalRange"},
    {Effect_DamageBonus, L"DamageBonus"},
    {Effect_DamageReduction, L"DamageReduction"},
    {Effect_DragonmarkUse, L"DragonmarkUse"},
    {Effect_DodgeBonus, L"DodgeBonus"},
    {Effect_DodgeBypass, L"DodgeBypass"},
    {Effect_DodgeCapBonus, L"DodgeCapBonus"},
    {Effect_DoubleShot, L"Doubleshot"},
    {Effect_DoubleStrike, L"Doublestrike"},
    {Effect_DR, L"DR"},
    {Effect_DRBypass, L"DRBypass"},
    {Effect_EnhancementTree, L"EnhancementTree"},
    {Effect_EnchantArmor, L"EnchantArmor"},
    {Effect_EnchantWeapon, L"EnchantWeapon"},
    {Effect_EnergyAbsorbance, L"EnergyAbsorbance"},
    {Effect_EnergyResistance, L"EnergyResistance"},
    {Effect_ExcludeFeatSelection, L"ExcludeFeatSelection"},
    {Effect_ExtraActionBoost, L"ExtraActionBoost"},
    {Effect_ExtraLayOnHands, L"ExtraLayOnHands"},
    {Effect_ExtraRage, L"ExtraRage"},
    {Effect_ExtraRemoveDisease, L"ExtraRemoveDisease"},
    {Effect_ExtraSmite, L"ExtraSmite"},
    {Effect_ExtraTurns, L"ExtraTurns"},
    {Effect_ExtraWildEmpathy, L"ExtraWildEmpathy"},
    {Effect_FatePoint, L"FatePoint"},
    {Effect_Fortification, L"Fortification"},
    {Effect_FortificationBypass, L"FortificationBypass"},
    {Effect_GlancingBlows, L"GlancingBlows"},
    {Effect_GlancingBlowsDamage, L"GlancingBlowsDamage"},
    {Effect_GlancingBlowEffects, L"GlancingBlowEffects"},
    {Effect_GrantFeat, L"GrantFeat"},
    {Effect_GrantSpell, L"GrantSpell"},
    {Effect_HealingAmplification, L"HealingAmplification"},
    {Effect_HirelingAbilityBonus, L"HirelingAbilityBonus"},
    {Effect_HirelingHitpoints, L"HirelingHitpoints"},
    {Effect_HirelingFortification, L"HirelingFortification"},
    {Effect_HirelingPRR, L"HirelingPRR"},
    {Effect_HirelingMRR, L"HirelingMRR"},
    {Effect_HirelingDodge, L"HirelingDodge"},
    {Effect_HirelingMeleePower, L"HirelingMeleePower"},
    {Effect_HirelingRangedPower, L"HirelingRangedPower"},
    {Effect_HirelingSpellPower, L"HirelingSpellPower"},
    {Effect_Hitpoints, L"Hitpoints"},
    {Effect_Immunity, L"Immunity"},
    {Effect_MaxDexBonus, L"MaxDexBonus"},
    {Effect_MaxDexBonusTowerShield, L"MaxDexBonusTowerShield"},
    {Effect_MaxDodgeBonus, L"MaxDodgeBonus"},
    {Effect_MeleePower, L"MeleePower"},
    {Effect_MinionBonus, L"MinionBonus"},
    {Effect_MRR, L"MRR"},
    {Effect_MonkWeaponDie, L"MonkWeaponDie"},
    {Effect_MovementSpeed, L"MovementSpeed"},
    {Effect_NegativeHealingAmplification, L"NegativeHealingAmplification"},
    {Effect_OffHandAttackBonus, L"OffHandAttackBonus"},
    {Effect_OffHandDoublestrike, L"OffHandDoublestrike"},
    {Effect_PointBlankShotRange, L"PointBlankShotRange"},
    {Effect_PRR, L"PRR"},
    {Effect_RangedPower, L"RangedPower"},
    {Effect_Regeneration, L"Regeneration"},
    {Effect_RepairAmplification, L"RepairAmplification"},
    {Effect_RustSusceptability, L"RustSusceptability"},
    {Effect_SaveBonus, L"SaveBonus"},
    {Effect_SecondaryShieldBash, L"SecondaryShieldBash"},
    {Effect_SkillBonus, L"SkillBonus"},
    {Effect_SneakAttackDamage, L"SneakAttackDamage"},
    {Effect_SneakAttackRange, L"SneakAttackRange"},
    {Effect_SongBonus, L"SongBonus"},
    {Effect_SongDuration, L"SongDuration"},
    {Effect_SpellCriticalChance, L"SpellCriticalChance"},
    {Effect_SpellCriticalDamage, L"SpellCriticalDamage"},
    {Effect_SpellDC, L"SpellDC"},
    {Effect_SpellLikeAbility, L"SpellLikeAbility"},
    {Effect_SpellPenetrationBonus, L"SpellPenetrationBonus"},
    {Effect_SpellPoints, L"SpellPoints"},
    {Effect_SpellPower, L"SpellPower"},
    {Effect_SpellResistance, L"SpellResistance"},
    {Effect_TacticalDC, L"TacticalDC"},
    {Effect_ThreatBonus, L"ThreatBonus"},
    {Effect_TurnBonus, L"TurnBonus"},
    {Effect_TurnLevelBonus, L"TurnLevelBonus"},
    {Effect_TwistOfFate, L"TwistOfFate"},
    {Effect_UnconsciousRange, L"UnconsciousRange"},
    {Effect_VorpalRange, L"VorpalRange"},
    {Effect_WeaponBaseDamageBonus,L"WeaponBaseDamageBonus"},
    {Effect_WeaponBonus, L"WeaponBonus"},
    {Effect_WeaponDamageBonus, L"WeaponDamageBonus"},
    {Effect_WeaponEnchantment, L"WeaponEnchantment"},
    {Effect_WeaponProficiency, L"WeaponProficiency"},
    {EffectType(0), NULL}
};

class Effect :
    public XmlLib::SaxContentElement
{
    public:
        Effect(void);
        void Write(XmlLib::SaxWriter * writer) const;

        double Amount(size_t tier) const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        // note that all the optional XML elements can be
        // used by a variety of feat effects as different interpretations
        #define Effect_PROPERTIES(_) \
                DL_OPTIONAL_STRING(_, DisplayName) \
                DL_ENUM(_, EffectType, Type, Effect_Unknown, effectTypeMap) \
                DL_OPTIONAL_SIMPLE(_, double, Amount, 0) \
                DL_OPTIONAL_SIMPLE(_, int, Percentage, 0) \
                DL_OPTIONAL_SIMPLE(_, int, AmountPerLevel, 0) \
                DL_OPTIONAL_OBJECT(_, Dice, DiceRoll) \
                DL_OPTIONAL_SIMPLE(_, size_t, DieRoll, 20) \
                DL_OPTIONAL_SIMPLE(_, size_t, AppliesFrom, 0) \
                DL_OPTIONAL_SIMPLE(_, size_t, Duration, 0) \
                DL_OPTIONAL_SIMPLE(_, size_t, Cooldown, 0) \
                DL_OPTIONAL_VECTOR(_, double, AmountVector) \
                DL_OPTIONAL_SIMPLE(_, int, Divider, 0) \
                DL_OPTIONAL_STRING(_, Base) \
                DL_OPTIONAL_STRING(_, Feat) \
                DL_FLAG(_, MustBeCentred) \
                DL_OPTIONAL_VECTOR(_, size_t, NoFailOn1) \
                DL_FLAG(_, AmountPerActionPoint) \
                DL_OPTIONAL_STRING(_, Spell) \
                DL_OPTIONAL_SIMPLE(_, int, SpellLevel, 0) \
                DL_STRING_VECTOR(_, Stance) \
                DL_OPTIONAL_STRING(_, EnhancementTree) \
                DL_OPTIONAL_SIMPLE(_, double, AmountPerAP, 0.0) \
                DL_OPTIONAL_STRING(_, SpellLikeAbility) \
                DL_OPTIONAL_ENUM(_, AbilityType, Ability, Ability_Unknown, abilityTypeMap) \
                DL_OPTIONAL_ENUM(_, ClassType, Class, Class_Unknown, classTypeMap) \
                DL_OPTIONAL_ENUM(_, DamageReductionType, DR, DR_Unknown, drTypeMap) \
                DL_OPTIONAL_ENUM(_, EnergyType, Energy, Energy_Unknown, energyTypeMap) \
                DL_OPTIONAL_ENUM(_, FavoredEnemyType, FavoredEnemy, FavoredEnemy_Unknown, favoredEnemyTypeMap) \
                DL_OPTIONAL_ENUM(_, ImmunityType, Immunity, Immunity_Unknown, immunityTypeMap) \
                DL_OPTIONAL_ENUM(_, MeleeStyle, Style, MeleeStyle_Unknown, meleeStyleMap) \
                DL_OPTIONAL_ENUM(_, SaveType, Save, Save_Unknown, saveTypeMap) \
                DL_OPTIONAL_ENUM(_, SkillType, Skill, Skill_Unknown, skillTypeMap) \
                DL_OPTIONAL_ENUM(_, SpellPowerType, SpellPower, SpellPower_Unknown, spellPowerTypeMap) \
                DL_OPTIONAL_ENUM(_, SpellSchoolType, School, SpellSchool_Unknown, spellSchoolTypeMap) \
                DL_OPTIONAL_ENUM(_, TacticalType, Tactical, Tactical_Unknown, tacticalTypeMap) \
                DL_OPTIONAL_ENUM(_, WeaponClassType, WeaponClass, WeaponClass_Unknown, weaponClassTypeMap) \
                DL_OPTIONAL_ENUM(_, WeaponDamageType, DamageType, WeaponDamage_Unknown, weaponDamageTypeMap) \
                DL_OPTIONAL_ENUM(_, WeaponType, Weapon, Weapon_Unknown, weaponTypeMap)

        DL_DECLARE_ACCESS(Effect_PROPERTIES)
        DL_DECLARE_VARIABLES(Effect_PROPERTIES)
};
