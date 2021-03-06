// BreakdownItemSpellPoints.cpp
//
#include "stdafx.h"
#include "BreakdownItemSpellPoints.h"
#include "BreakdownsView.h"
#include "GlobalSupportFunctions.h"

BreakdownItemSpellPoints::BreakdownItemSpellPoints(
        BreakdownType type,
        EffectType effect,
        const CString & title,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(type, treeList, hItem),
    m_title(title),
    m_effect(effect)
{
}

BreakdownItemSpellPoints::~BreakdownItemSpellPoints()
{
}

// required overrides
CString BreakdownItemSpellPoints::Title() const
{
    return m_title;
}

CString BreakdownItemSpellPoints::Value() const
{
    CString value;
    value.Format(
            "%3d",
            (int)Total());
    return value;
}

void BreakdownItemSpellPoints::CreateOtherEffects()
{
    m_otherEffects.clear();
    // create the class specific spell point contributions
    if (m_pCharacter != NULL)
    {
        std::vector<size_t> classLevels = m_pCharacter->ClassLevels(MAX_LEVEL);
        for (size_t ci = 0; ci < classLevels.size(); ++ci)
        {
            // class level spell points
            if (classLevels[ci] > 0) // must have some levels in the class
            {
                //?? should this be based of the breakdown for this class caster level?
                size_t classSpellPoints = ClassSpellPoints((ClassType)ci, classLevels[ci]);
                if (classSpellPoints > 0)
                {
                    CString bonusName;
                    bonusName.Format("%s(%d) Spell points",
                            EnumEntryText((ClassType)ci, classTypeMap),
                            classLevels[ci]);
                    ActiveEffect classSps(
                            ET_class,
                            (LPCSTR)bonusName,
                            1,
                            classSpellPoints,
                            "");        // no tree
                    AddOtherEffect(classSps);
                    // if we have class spell points we must have bonus class ability spell points
                    // from the DDOWiki:
                    // Your casting ability grants you a number of bonus spell points equal
                    // to (caster level + 9) * (casting ability modifier)
                    AbilityType at = ClassCastingStat((ClassType)ci);
                    BreakdownItem * pBI = m_pBreakdownView->FindBreakdown(StatToBreakdown(at));
                    ASSERT(pBI != NULL);
                    pBI->AttachObserver(this);  // need to know about changes to this stat
                    int abilityBonus = BaseStatToBonus(pBI->Total());
                    int abilitySps = (classLevels[ci] + 9) * abilityBonus;
                    if (abilitySps != 0)
                    {
                        bonusName.Format("%s(%s) ability Spell points",
                                EnumEntryText((ClassType)ci, classTypeMap),
                                EnumEntryText((AbilityType)at, abilityTypeMap));
                        ActiveEffect abilitySpBonus(
                                ET_ability,
                                (LPCSTR)bonusName,
                                1,
                                abilitySps,
                                "");        // no tree
                        AddOtherEffect(abilitySpBonus);
                    }
                }
            }
        }
   }
}

bool BreakdownItemSpellPoints::AffectsUs(const Effect & effect) const
{
    bool isUs = false;
    if (effect.Type() == m_effect)
    {
        isUs = true;
    }
    return isUs;
}

// CharacterObserver overrides
void BreakdownItemSpellPoints::UpdateClassChanged(
        Character * charData,
        ClassType type,
        size_t level)
{
    BreakdownItem::UpdateClassChanged(charData, type, level);
    // if a class has changed, just recreate our other effects
    // this will update spell point totals
    CreateOtherEffects();
}

void BreakdownItemSpellPoints::UpdateTotalChanged(
        BreakdownItem * item,
        BreakdownType type)
{
    // just re-create our other effects
    CreateOtherEffects();
}

//?? Note that favored souls get up to double spell points from item effects
//?? (#favoredSoulLevels/20 + 1) * itemSp