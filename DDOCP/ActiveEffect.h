// ActiveEffect.h
//
// these are the effects in the list for any given breakdown
#pragma once

#include "BreakdownTypes.h"
#include "ClassTypes.h"
#include "Dice.h"
#include "EnergyTypes.h"

enum ActiveEffectType
{
    ET_unknown,     // default constructor, never used as a real type
    // these effects always stack
    ET_base,
    ET_ability,
    ET_class,
    ET_tome,
    ET_racial,
    ET_levelUp,
    ET_epic,
    ET_feat,
    ET_enhancement,
    ET_enhancementPerAP,
    ET_enhancementPerLevel,

    // only the highest of these effects stack if multiple present
    ET_artifact,
    ET_armor,
    ET_competence,
    ET_deflection,
    ET_enchantment,
    //ET_enhancement,
    ET_equipment,
    ET_exceptional,
    ET_guild,
    ET_insightful,
    ET_implement,
    ET_luck,
    ET_mythic,
    ET_penalty,
    ET_profane,
    ET_quality,
    ET_raging,
    ET_resistance,
    ET_spooky,
    ET_unique,
    ET_vitality,
};

class Character;

class ActiveEffect
{
    public:
        explicit ActiveEffect();
        ActiveEffect(
                ActiveEffectType type,
                const std::string & name,
                size_t stacks,
                double amount,
                const std::string & tree);
        ActiveEffect(
                ActiveEffectType type,
                const std::string & name,
                size_t stacks,
                const Dice & dice,
                const std::string & tree);
        ActiveEffect(
                ActiveEffectType type,
                const std::string & name,
                size_t stacks,
                const std::vector<double> & amounts);
        ActiveEffect(
                ActiveEffectType type,
                const std::string & name,
                double amountPerLevel,
                size_t stacks,
                ClassType classType);
        ActiveEffectType Type() const;
        CString Name() const;
        CString Stacks() const;
        CString AmountAsText() const;
        double TotalAmount() const;
        void AddFeat(const std::string & featName);
        void AddStance(const std::string & stance);
        const std::vector<std::string> & Stances() const;
        const std::string & Tree() const;
        bool IsActive(const Character * pCharacter) const;
        // for comparison and removal of duplicate entries
        bool operator<=(const ActiveEffect & other) const;
        bool operator==(const ActiveEffect & other) const;
        void AddStack();
        bool RevokeStack();
        void SetStacks(size_t count);
        // optional
        void SetAmount(double amount);
        void SetEnergy(EnergyType type);
        void SetBreakdownDependency(BreakdownType bt);
        bool HasBreakdownDependency(BreakdownType bt) const;
        bool HasClass(ClassType type) const;
    private:
        double Amount() const;
        ActiveEffectType m_type;
        std::string m_effectName;
        size_t m_numStacks;
        double m_amount;
        std::vector<double> m_amounts;
        bool m_bHasDice;                    // true if m_dice is valid
        Dice m_dice;
        bool m_bHasEnergy;                  // true if m_energy is valid
        EnergyType m_energy;
        std::vector<std::string> m_stances; // non blank if only active when a specific stance is active
        std::string m_tree;                 // non blank if total depends on number of AP spent in tree
        BreakdownType m_bt;                 // specific breakdown we may be dependent on
        std::vector<std::string> m_feats;   // non blank if only active when a specific feat is trained
        // amount per trained level
        double m_amountPerLevel;
        ClassType m_class;
};
