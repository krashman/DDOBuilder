// GlobalSupportFunctions.h
//
#include "stdafx.h"
#include "GlobalSupportFunctions.h"

#include "Character.h"
#include "DDOCP.h"
#include "MainFrm.h"
#include <boost/assign/std/vector.hpp>
#include <algorithm>

namespace
{
    const int f_noWidthSetup = -1;
    const char f_registrySection[] = "ColumnWidths";
    const int c_textFieldSize = 256;

    void f_RemoveInvalidKeyCharacters(CString * key)
    {
        // backslash is not allowed in registry key names
        key->Remove('\\');
        // line breaks now allowed either
        key->Remove('\r');
        key->Remove('\n');
    }
}

void LoadColumnWidthsByName(
        CListCtrl * control, 
        const std::string & fmtString)
{
    ASSERT(control);
    // make sure columns have correct width
    CHeaderCtrl* pHeaderCtrl = control->GetHeaderCtrl();
    if (pHeaderCtrl != NULL)
    {
        // some controls set the control Id to a specific value, only set to
        // control ID if its not already set to something other than 0, else we
        // could break some ON_HDN_NOTIFICATION messages
        if (pHeaderCtrl->GetDlgCtrlID() == 0)
        {
            pHeaderCtrl->SetDlgCtrlID(control->GetDlgCtrlID());
        }
        int nColumnCount = pHeaderCtrl->GetItemCount();
        for (int column = 0; column < nColumnCount; column++)
        {
            CString valueKeyName;

            HDITEM item;
            char itemText[c_textFieldSize];
            item.mask = HDI_TEXT;
            item.pszText = itemText;
            item.cchTextMax = c_textFieldSize;
            pHeaderCtrl->GetItem(column, &item);

            if (strlen(itemText) == 0)
            {
                // its an empty column header, use the column index instead
                sprintf_s(
                        itemText,
                        "%d",
                        column);
            }

            valueKeyName.Format(fmtString.c_str(), itemText);
            f_RemoveInvalidKeyCharacters(&valueKeyName);
            int width = AfxGetApp()->GetProfileInt(
                    f_registrySection, 
                    valueKeyName, 
                    f_noWidthSetup);
            if (width != f_noWidthSetup)
            {
                control->SetColumnWidth(column, width);
            }
        }
    }
}

void SaveColumnWidthsByName(
        CListCtrl * control, 
        const std::string & fmtString)
{
    ASSERT(control);
    // save the new column widths to the registry
    CHeaderCtrl* pHeaderCtrl = control->GetHeaderCtrl();
    if (pHeaderCtrl != NULL)
    {
        int nColumnCount = pHeaderCtrl->GetItemCount();
        for (int column = 0; column < nColumnCount; column++)
        {
            CString valueKeyName;

            HDITEM item;
            char itemText[c_textFieldSize];
            item.mask = HDI_TEXT;
            item.pszText = itemText;
            item.cchTextMax = c_textFieldSize;
            pHeaderCtrl->GetItem(column, &item);

            if (strlen(itemText) == 0)
            {
                // its an empty column header, use the column index instead
                sprintf_s(
                        itemText,
                        "%d",
                        column);
            }

            valueKeyName.Format(fmtString.c_str(), itemText);
            f_RemoveInvalidKeyCharacters(&valueKeyName);
            int width = control->GetColumnWidth(column);
            AfxGetApp()->WriteProfileInt(
                    f_registrySection, 
                    valueKeyName, 
                    width);
        }
    }
}

CSize RequiredControlSizeForContent(
        CListCtrl * control, 
        int maxItemsToShow,
        int requiredItems)
{
    // add up the widths of all the columns
    size_t columnCount = control->GetHeaderCtrl()->GetItemCount();
    int width = 0;
    int height = 0;

    for (size_t column = 0 ; column < columnCount ; ++column)
    {
        int columnWidth = control->GetColumnWidth(column);

        width += columnWidth;
    }
    // take control border into account
    width += (GetSystemMetrics(SM_CXBORDER) * 4);
    // allow space for a vertical scrollbar
    width += GetSystemMetrics(SM_CXHSCROLL);

    // now work out the required height of the control
    if (control->GetHeaderCtrl() != NULL
            && control->GetHeaderCtrl()->IsWindowVisible())
    {
        CRect headerRect;
        control->GetHeaderCtrl()->GetWindowRect(&headerRect);
        height += headerRect.Height();
    }
    if (requiredItems > 0)
    {
        CRect itemRect;
        control->GetItemRect(0, &itemRect, LVIR_BOUNDS);
        height += itemRect.Height() * requiredItems;
    }
    else
    {
        int numItems = control->GetItemCount();
        if (numItems > 0)
        {
            CRect itemRect;
            control->GetItemRect(0, &itemRect, LVIR_BOUNDS);
            height += itemRect.Height() * min(numItems, maxItemsToShow);
        }
        else
        {
            // assume default item height of icon only
            height += GetSystemMetrics(SM_CYICON);
        }
    }
    // take control border into account
    height += (GetSystemMetrics(SM_CYBORDER) * 4);
    // allow space for a horizontal scrollbar
    height += GetSystemMetrics(SM_CYHSCROLL);
    return CSize(width, height);
}

BOOL OnEraseBackground(
        CWnd * pWnd,
        CDC * pDC,
        const int controlsNotToBeErased[])
{
    pDC->SaveDC();

    const int * pId = controlsNotToBeErased;
    while (*pId != 0)
    {
        // Get rectangle of the control.
        CWnd * pControl = pWnd->GetDlgItem(*pId);
        if (pControl && pControl->IsWindowVisible())
        {
            CRect controlClip;
            pControl->GetWindowRect(&controlClip);
            pWnd->ScreenToClient(&controlClip);
            if (pControl->IsKindOf(RUNTIME_CLASS(CComboBox)))
            {
                // combo boxes return the height of the whole control, including the drop rectangle
                // reduce the height to the control size without it shown
                controlClip.bottom = controlClip.top
                        + GetSystemMetrics(SM_CYHSCROLL)
                        + GetSystemMetrics(SM_CYEDGE) * 2;
                // special case for combo boxes with image lists
                CComboBoxEx * pCombo = dynamic_cast<CComboBoxEx*>(pControl);
                if (pCombo != NULL)
                {
                    CImageList * pImage = pCombo->GetImageList();
                    if (pImage != NULL)
                    {
                        IMAGEINFO info;
                        pImage->GetImageInfo(0, &info);
                        // limit to the the height of the selection combo
                        controlClip.bottom = controlClip.top
                                + info.rcImage.bottom
                                - info.rcImage.top
                                + GetSystemMetrics(SM_CYEDGE) * 2;
                    }
                }
            }
            pDC->ExcludeClipRect(&controlClip);
        }

        // next in list
        ++pId;
    }
    CRect rctClient;
    pWnd->GetWindowRect(&rctClient);
    pWnd->ScreenToClient(&rctClient);

    // use the selected theme in use for the background colour selection
    CMFCVisualManager * manager = CMFCVisualManager::GetInstance();
    if (manager != NULL)
    {
        manager->OnFillPopupWindowBackground(pDC, rctClient);
    }
    else
    {
        pDC->FillSolidRect(rctClient, GetSysColor(COLOR_BTNFACE));
    }
    pDC->RestoreDC(-1);

    return TRUE;
}

const std::list<Feat> & AllFeats()
{
    CDDOCPApp * pApp = dynamic_cast<CDDOCPApp*>(AfxGetApp());
    return pApp->AllFeats();
}

const std::list<Feat> & StandardFeats()
{
    CDDOCPApp * pApp = dynamic_cast<CDDOCPApp*>(AfxGetApp());
    return pApp->StandardFeats();
}

const std::list<Feat> & HeroicPastLifeFeats()
{
    CDDOCPApp * pApp = dynamic_cast<CDDOCPApp*>(AfxGetApp());
    return pApp->HeroicPastLifeFeats();
}

const std::list<Feat> & RacialPastLifeFeats()
{
    CDDOCPApp * pApp = dynamic_cast<CDDOCPApp*>(AfxGetApp());
    return pApp->RacialPastLifeFeats();
}

const std::list<Feat> & IconicPastLifeFeats()
{
    CDDOCPApp * pApp = dynamic_cast<CDDOCPApp*>(AfxGetApp());
    return pApp->IconicPastLifeFeats();
}

const std::list<Feat> & EpicPastLifeFeats()
{
    CDDOCPApp * pApp = dynamic_cast<CDDOCPApp*>(AfxGetApp());
    return pApp->EpicPastLifeFeats();
}

const std::list<Feat> & SpecialFeats()
{
    CDDOCPApp * pApp = dynamic_cast<CDDOCPApp*>(AfxGetApp());
    return pApp->SpecialFeats();
}

const std::list<Feat> & StanceFeats()
{
    CDDOCPApp * pApp = dynamic_cast<CDDOCPApp*>(AfxGetApp());
    return pApp->StanceFeats();
}

const std::list<EnhancementTree> & EnhancementTrees()
{
    CDDOCPApp * pApp = dynamic_cast<CDDOCPApp*>(AfxGetApp());
    return pApp->EnhancementTrees();
}

const std::list<Spell> & Spells()
{
    CDDOCPApp * pApp = dynamic_cast<CDDOCPApp*>(AfxGetApp());
    return pApp->Spells();
}

const std::list<Item> & Items()
{
    CDDOCPApp * pApp = dynamic_cast<CDDOCPApp*>(AfxGetApp());
    return pApp->Items();
}

std::vector<Spell> FilterSpells(ClassType ct, int level)
{
    // return the list of spells for this class at this level
    // note that negative spell levels can be given to get the list of
    // automatically assigned spells at that level (e.g. -1 = all 1st level
    // auto assigned spells)
    const std::list<Spell> & spells = Spells();
    std::vector<Spell> availableSpells;
    std::list<Spell>::const_iterator si = spells.begin();
    while (si != spells.end())
    {
        bool isClassLevelSpell = false;
        switch (ct)
        {
        case Class_Artificer:
            if ((*si).HasArtificer()
                    && (*si).Artificer() == level)
            {
                isClassLevelSpell = true;
            }
            break;
        case Class_Bard:
            if ((*si).HasBard()
                    && (*si).Bard() == level)
            {
                isClassLevelSpell = true;
            }
            break;
        case Class_Cleric:
            if ((*si).HasCleric()
                    && (*si).Cleric() == level)
            {
                isClassLevelSpell = true;
            }
            break;
        case Class_Druid:
            if ((*si).HasDruid()
                    && (*si).Druid() == level)
            {
                isClassLevelSpell = true;
            }
            break;
        case Class_FavoredSoul:
            if ((*si).HasFavoredSoul()
                    && (*si).FavoredSoul() == level)
            {
                isClassLevelSpell = true;
            }
            break;
        case Class_Paladin:
            if ((*si).HasPaladin()
                    && (*si).Paladin() == level)
            {
                isClassLevelSpell = true;
            }
            break;
        case Class_Ranger:
            if ((*si).HasRanger()
                    && (*si).Ranger() == level)
            {
                isClassLevelSpell = true;
            }
            break;
        case Class_Sorcerer:
            if ((*si).HasSorcerer()
                    && (*si).Sorcerer() == level)
            {
                isClassLevelSpell = true;
            }
            break;
        case Class_Warlock:
            if ((*si).HasWarlock()
                    && (*si).Warlock() == level)
            {
                isClassLevelSpell = true;
            }
            break;
        case Class_Wizard:
            if ((*si).HasWizard()
                    && (*si).Wizard() == level)
            {
                isClassLevelSpell = true;
            }
            break;
        }
        if (isClassLevelSpell)
        {
            // add this one, its needed
            availableSpells.push_back(*si);
        }
        ++si;
    }
    // finally sort the spells into alphanumeric order
    std::sort(availableSpells.begin(), availableSpells.end());
    return availableSpells;
}

Spell FindSpellByName(const std::string & name)
{
    Spell spell;
    const std::list<Spell> & spells = Spells();
    std::list<Spell>::const_iterator si = spells.begin();
    while (si != spells.end())
    {
        if ((*si).Name() == name)
        {
            spell = (*si);
            break;  // found, no need to check the rest
        }
        ++si;
    }
    //if (spell.Name() == "")
    //{
    //    CString errMsg;
    //    errMsg.Format("Spell %s not found in Spells.xml file", name.c_str());
    //    AfxMessageBox(errMsg, MB_ICONERROR | MB_OK);
    //}
    return spell;
}

const EnhancementTree & GetEnhancementTree(const std::string & treeName)
{
    const std::list<EnhancementTree> & allTrees = EnhancementTrees();
    std::list<EnhancementTree>::const_iterator it = allTrees.begin();
    while (it != allTrees.end())
    {
        // find the requested tree
        if ((*it).Name() == treeName)
        {
            // this is the one we want
            return (*it);
        }
        ++it;
    }
    throw "Failed to find required tree";
}

const EnhancementTreeItem * FindEnhancement(
        const std::string & internalName,
        std::string * treeName) // can be NULL
{
    const EnhancementTreeItem * item = NULL;
    const std::list<EnhancementTree> & trees = EnhancementTrees();
    bool found = false;
    std::list<EnhancementTree>::const_iterator tit = trees.begin();
    while (!found && tit != trees.end())
    {
        item = (*tit).FindEnhancementItem(internalName);
        found = (item != NULL);
        if (found && treeName != NULL)
        {
            *treeName = (*tit).Name();
        }
        ++tit;
    }
    return item;
}

std::vector<TrainableFeatTypes> ClassSpecificFeatTypes(ClassType type)
{
    // some classes have specific trainable feat types associated with them
    std::vector<TrainableFeatTypes> types;
    switch (type)
    {
        case Class_Artificer:
            types.push_back(TFT_ArtificerBonus);
            break;
        case Class_Cleric:
            types.push_back(TFT_FollowerOf);    // level 1
            types.push_back(TFT_Deity);         // level 6
            break;
        case Class_Druid:
            types.push_back(TFT_DruidWildShape);
            break;
        case Class_FavoredSoul:
            types.push_back(TFT_FollowerOf);    // level 1
            types.push_back(TFT_ChildOf);       // level 3
            types.push_back(TFT_EnergyResistance); // level 5/10/15
            types.push_back(TFT_Deity);         // level 6
            types.push_back(TFT_BelovedOf);     // level 12
            types.push_back(TFT_DamageReduction); // level 20
            break;
        case Class_Fighter:
            types.push_back(TFT_FighterBonus);
            break;
        case Class_Paladin:
            types.push_back(TFT_FollowerOf);    // level 1
            types.push_back(TFT_Deity);         // level 6
            break;
        case Class_Ranger:
            types.push_back(TFT_RangerFavoredEnemy);
            break;
        case Class_Rogue:
            types.push_back(TFT_RogueSpecialAbility);
            break;
        case Class_Monk:
            types.push_back(TFT_MonkBonus);
            types.push_back(TFT_MonkBonus6);
            types.push_back(TFT_MonkPhilosphy);
            break;
        case Class_Warlock:
            types.push_back(TFT_WarlockPact);
            types.push_back(TFT_WarlockPactAbility);
            types.push_back(TFT_WarlockPactSaveBonus);
            types.push_back(TFT_WarlockPactSpell);
            break;
        case Class_Wizard:
            types.push_back(TFT_WizardMetamagic);
            break;
        case Class_Sorcerer:
        case Class_Barbarian:
        case Class_Bard:
            // these classes have no specific feat types associated with them
            break;
    }
    return types;
}

std::vector<TrainableFeatTypes> RaceSpecificFeatTypes(RaceType type)
{
    // some races have specific trainable feat types associated with them
    std::vector<TrainableFeatTypes> types;
    switch (type)
    {
    case Race_HalfElf:
        types.push_back(TFT_HalfElfDilettanteBonus);
        break;
    case Race_Human:
        types.push_back(TFT_HumanBonus);
        break;
    case Race_PurpleDragonKnight:
        types.push_back(TFT_PDKBonus);
        break;
    case Race_Dragonborn:
        types.push_back(TFT_DragonbornRacial);
        break;
        // all other races do not have special feat types
    }
    return types;
}

AbilityType StatFromSkill(SkillType skill)
{
    // return which ability provides the bonus to the skill type
    AbilityType at = Ability_Unknown;
    switch (skill)
    {
    case Skill_Bluff:
    case Skill_Diplomacy:
    case Skill_Haggle:
    case Skill_Intimidate:
    case Skill_Perform:
    case Skill_UMD:
        at = Ability_Charisma;
        break;

    case Skill_Concentration:
        at = Ability_Constitution;
        break;

    case Skill_Balance:
    case Skill_Hide:
    case Skill_MoveSilently:
    case Skill_OpenLock:
    case Skill_Tumble:
        at = Ability_Dexterity;
        break;

    case Skill_DisableDevice:
    case Skill_Repair:
    case Skill_Search:
    case Skill_SpellCraft:
        at = Ability_Intelligence;
        break;

    case Skill_Heal:
    case Skill_Listen:
    case Skill_Spot:
        at = Ability_Wisdom;
        break;

    case Skill_Jump:
    case Skill_Swim:
        at = Ability_Strength;
        break;

    default:
        break;
    }
    ASSERT(at != Ability_Unknown);  // should have been found
    return at;
}

int ArmorCheckPenalty_Multiplier(SkillType skill)
{
    int multiplier = 0; // default
    switch (skill)
    {
    case Skill_Balance:
    case Skill_Hide:
    case Skill_Jump:
    case Skill_MoveSilently:
    case Skill_Tumble:
        multiplier = 1;
        break;
    case Skill_Swim:
        // is subject to double the standard Armor check penalty 
        multiplier = 2;
        break;
    }
    return multiplier;
}

int FindItemIndexByItemData(
        CListCtrl * pControl,
        DWORD itemData)
{
    // look through each items item data in the control
    // and return the index of the item that matches what we are looking for
    int index = -1;     // assume fail
    size_t count = pControl->GetItemCount();
    for (size_t ii = 0 ; ii < count; ++ii)
    {
        DWORD item = pControl->GetItemData(ii);
        if (item == itemData)
        {
            // found it
            index = ii;
            break;      // no need to keep looking
        }
    }
    return index;
}

bool IsClassSkill(ClassType type, SkillType skill)
{
    bool isClassSkill = false;
    switch (skill)
    {
    case Skill_Balance:
        if (type == Class_Bard
                || type == Class_Monk
                || type == Class_Rogue)
        {
            isClassSkill = true;
        }
        break;
    case Skill_Bluff:
        if (type == Class_Bard
                || type == Class_Rogue
                || type == Class_Sorcerer
                || type == Class_Warlock)
        {
            isClassSkill = true;
        }
        break;
    case Skill_Concentration:
        if (type == Class_Artificer
                || type == Class_Bard
                || type == Class_Cleric
                || type == Class_Druid
                || type == Class_FavoredSoul
                || type == Class_Monk
                || type == Class_Paladin
                || type == Class_Ranger
                || type == Class_Sorcerer
                || type == Class_Warlock
                || type == Class_Wizard)
        {
            isClassSkill = true;
        }
        break;
    case Skill_Diplomacy:
        if (type == Class_Bard
                || type == Class_Cleric
                || type == Class_Druid
                || type == Class_FavoredSoul
                || type == Class_Monk
                || type == Class_Paladin
                || type == Class_Rogue)
        {
            isClassSkill = true;
        }
        break;
    case Skill_DisableDevice:
        if (type == Class_Artificer
                || type == Class_Rogue)
        {
            isClassSkill = true;
        }
        break;
    case Skill_Haggle:
        if (type == Class_Artificer
                || type == Class_Bard
                || type == Class_Rogue)
        {
            isClassSkill = true;
        }
        break;
    case Skill_Heal:
        if (type == Class_Cleric
                || type == Class_Druid
                || type == Class_FavoredSoul
                || type == Class_Paladin
                || type == Class_Ranger)
        {
            isClassSkill = true;
        }
        break;
    case Skill_Hide:
        if (type == Class_Bard
                || type == Class_Monk
                || type == Class_Ranger
                || type == Class_Rogue)
        {
            isClassSkill = true;
        }
        break;
    case Skill_Intimidate:
        if (type == Class_Barbarian
                || type == Class_Druid
                || type == Class_Fighter
                || type == Class_Paladin
                || type == Class_Rogue
                || type == Class_Warlock)
        {
            isClassSkill = true;
        }
        break;
    case Skill_Jump:
        if (type == Class_Barbarian
                || type == Class_Bard
                || type == Class_FavoredSoul
                || type == Class_Fighter
                || type == Class_Monk
                || type == Class_Ranger
                || type == Class_Rogue
                || type == Class_Warlock)
        {
            isClassSkill = true;
        }
        break;
    case Skill_Listen:
        if (type == Class_Barbarian
                || type == Class_Bard
                || type == Class_Druid
                || type == Class_Monk
                || type == Class_Ranger
                || type == Class_Rogue)
        {
            isClassSkill = true;
        }
        break;
    case Skill_MoveSilently:
        if (type == Class_Bard
                || type == Class_Monk
                || type == Class_Ranger
                || type == Class_Rogue)
        {
            isClassSkill = true;
        }
        break;
    case Skill_OpenLock:
        if (type == Class_Artificer
                || type == Class_Rogue)
        {
            isClassSkill = true;
        }
        break;
    case Skill_Perform:
        if (type == Class_Bard)
        {
            isClassSkill = true;
        }
        break;
    case Skill_Repair:
        if (type == Class_Artificer
                || type == Class_Fighter
                || type == Class_Rogue
                || type == Class_Wizard)
        {
            isClassSkill = true;
        }
        break;
    case Skill_Search:
        if (type == Class_Artificer
                || type == Class_Ranger
                || type == Class_Rogue)
        {
            isClassSkill = true;
        }
        break;
    case Skill_SpellCraft:
        if (type == Class_Artificer
                || type == Class_Bard
                || type == Class_Cleric
                || type == Class_Druid
                || type == Class_FavoredSoul
                || type == Class_Sorcerer
                || type == Class_Warlock
                || type == Class_Wizard)
        {
            isClassSkill = true;
        }
        break;
    case Skill_Spot:
        if (type == Class_Artificer
                || type == Class_Druid
                || type == Class_Monk
                || type == Class_Ranger
                || type == Class_Rogue)
        {
            isClassSkill = true;
        }
        break;
    case Skill_Swim:
        if (type == Class_Barbarian
                || type == Class_Bard
                || type == Class_Druid
                || type == Class_Fighter
                || type == Class_Monk
                || type == Class_Ranger
                || type == Class_Rogue)
        {
            isClassSkill = true;
        }
        break;
    case Skill_Tumble:
        if (type == Class_Bard
                || type == Class_Monk
                || type == Class_Rogue)
        {
            isClassSkill = true;
        }
        break;
    case Skill_UMD:
        if (type == Class_Artificer
                || type == Class_Bard
                || type == Class_Rogue
                || type == Class_Warlock)
        {
            isClassSkill = true;
        }
        break;

    default:
        break;
    }
    return isClassSkill;
}

size_t SkillPoints(
        ClassType type,
        RaceType race,
        size_t intelligence,
        size_t level)
{
    size_t skillPoints = 0;
    switch (type)
    {
    case Class_Cleric:
    case Class_Fighter:
    case Class_FavoredSoul:
    case Class_Paladin:
    case Class_Sorcerer:
    case Class_Warlock:
    case Class_Wizard:
        skillPoints = 2;
        break;

    case Class_Artificer:
    case Class_Barbarian:
    case Class_Druid:
    case Class_Monk:
        skillPoints = 4;
        break;

    case Class_Bard:
    case Class_Ranger:
        skillPoints = 6;
        break;

    case Class_Rogue:
        skillPoints = 8;
        break;

    default:
        break;
    }
    if (race == Race_Human)
    {
        // humans gain 1 extra skill point per level
        ++skillPoints;
    }
    // int modifier
    int mod = (int)(intelligence / 2) - 5;
    skillPoints += mod;
    if (skillPoints < 1)
    {
        // minimum of 1 skill point always
        skillPoints = 1;
    }
    if (level == 0) // level is 0 based
    {
        // 4 times the skill points at 1st level
        skillPoints *= 4;
    }
    return skillPoints;
}

void SelectComboboxEntry(
        size_t itemData,
        CComboBox * pCombo)
{
    // select a combo box entry via its item data
    int sel = CB_ERR;           // assume select no item
    size_t numItems = pCombo->GetCount();
    for (size_t i = 0; i < numItems; ++i)
    {
        DWORD comboItemData = pCombo->GetItemData(i);
        if (itemData == comboItemData)
        {
            // this is the one we need to select
            sel = i;
            break;      // were done
        }
    }
    pCombo->SetCurSel(sel);
}

size_t GetComboboxSelection(CComboBox * pCombo)
{
    size_t selectedItem = 0;        // translates to enum Type_Unknown
    int sel = pCombo->GetCurSel();
    ASSERT(sel != CB_ERR);  // we should have a valid selection
    if (sel != CB_ERR)
    {
        selectedItem = (size_t)pCombo->GetItemData(sel);
    }
    return selectedItem;
}

int RacialModifier(
        RaceType race,
        AbilityType ability)
{
    // return the racial modifier to the given ability type
    int mod = 0;
    switch (ability)
    {
    case Ability_Strength:
        if (race == Race_Halfling
                || race == Race_Gnome
                || race == Race_DeepGnome)
        {
            mod = -2;
        }
        if (race == Race_HalfOrc
                || race == Race_Dragonborn)
        {
            mod = +2;
        }
        break;
    case Ability_Dexterity:
        if (race == Race_BladeForged
                || race == Race_Dragonborn)
        {
            mod = -2;
        }
        if (race == Race_Drow
                || race == Race_Halfling
                || race == Race_Elf
                || race == Race_ShadarKai)
        {
            mod = +2;
        }
        break;
    case Ability_Constitution:
        if (race == Race_Drow
                || race == Race_Elf
                || race == Race_Morninglord)
        {
            mod = -2;
        }
        if (race == Race_Dwarf
                || race == Race_Warforged
                || race == Race_BladeForged)
        {
            mod = +2;
        }
        break;
    case Ability_Intelligence:
        if (race == Race_HalfOrc)
        {
            mod = -2;
        }
        if (race == Race_Drow
                || race == Race_Gnome
                || race == Race_DeepGnome
                || race == Race_Morninglord)
        {
            mod = +2;
        }
        break;
    case Ability_Wisdom:
        if (race == Race_Warforged
                || race == Race_BladeForged)
        {
            mod = -2;
        }
        if (race == Race_DeepGnome)
        {
            mod = +2;
        }
        break;
    case Ability_Charisma:
        if (race == Race_Dwarf
                || race == Race_HalfOrc
                || race == Race_Warforged
                || race == Race_DeepGnome
                || race == Race_ShadarKai)
        {
            mod = -2;
        }
        if (race == Race_Drow
                || race == Race_Dragonborn)
        {
            mod = +2;
        }
        break;
    }
    return mod;
}

bool CanTrainClass(
        ClassType type,
        AlignmentType alignment)
{
    bool canTrain = false;
    switch (type)
    {
    case Class_Paladin:
        canTrain = (alignment == Alignment_LawfulGood);
        break;

    case Class_Barbarian:
    case Class_Bard:
        // any non-lawful
        canTrain = (alignment == Alignment_TrueNeutral)
                || (alignment == Alignment_ChaoticNeutral)
                || (alignment == Alignment_ChaoticGood);
        break;

    case Class_Druid:
        // any neutral variant
        canTrain = (alignment == Alignment_TrueNeutral)
                || (alignment == Alignment_ChaoticNeutral)
                || (alignment == Alignment_LawfulNeutral);
        break;

    case Class_Monk:
        // any lawful variant
        canTrain = (alignment == Alignment_LawfulGood)
                || (alignment == Alignment_LawfulNeutral);
        break;

    case Class_Artificer:
    case Class_Cleric:
    case Class_FavoredSoul:
    case Class_Fighter:
    case Class_Ranger:
    case Class_Rogue:
    case Class_Sorcerer:
    case Class_Warlock:
    case Class_Wizard:
        // these classes can be any alignment
        canTrain = true;
        break;

    case Class_Epic:
        // can never train epic at heroic levels
        break;
    }
    return canTrain;
}

bool IsInGroup(TrainableFeatTypes type, const FeatGroup & group)
{
    bool inGroup = false;
    switch (type)
    {
    case TFT_Standard:
    case TFT_HumanBonus:
    case TFT_PDKBonus:
        inGroup = group.HasIsStandardFeat();
        break;

    case TFT_ArtificerBonus:
        inGroup = group.HasIsArtificerBonus();
        break;

    case TFT_BelovedOf:
        inGroup = group.HasIsBelovedOf();
        break;

    case TFT_ChildOf:
        inGroup = group.HasIsChildOf();
        break;

    case TFT_DamageReduction:
        inGroup = group.HasIsDamageReduction();
        break;

    case TFT_Deity:
        inGroup = group.HasIsDeity();
        break;

    case TFT_DragonbornRacial:
        inGroup = group.HasIsDragonbornRacial();
        break;

    case TFT_DruidWildShape:
        inGroup = group.HasIsDruidWildShape();
        break;

    case TFT_EnergyResistance:
        inGroup = group.HasIsEnergyResistance();
        break;

    case TFT_EpicDestinyFeat:
        inGroup = group.HasIsEpicDestinyFeat();
        break;

    case TFT_EpicFeat:
        inGroup = group.HasIsStandardFeat() // regular feats can be trained in epic
                || group.HasIsEpicFeat();
        break;

    case TFT_FollowerOf:
        inGroup = group.HasIsFollowerOf();
        break;

    case TFT_FighterBonus:
        inGroup = group.HasIsFighterBonus();
        break;

    case TFT_HalfElfDilettanteBonus:
        inGroup = group.HasIsDilettante();
        break;

    case TFT_LegendaryFeat:
        inGroup = group.HasIsLegendaryFeat();
        break;

    case TFT_MonkBonus:
        inGroup = group.HasIsMonkBonus();
        break;

    case TFT_MonkBonus6:
        inGroup = group.HasIsMonkBonus()
                || group.HasIsMonkBonus6(); // spring attack is a special case
        break;

    case TFT_MonkPhilosphy:
        inGroup = group.HasIsMonkPhilosphy();
        break;

    case TFT_RangerFavoredEnemy:
        inGroup = group.HasIsFavoredEnemy();
        break;

    case TFT_RogueSpecialAbility:
        inGroup = group.HasIsRogueSpecialAbility();
        break;

    case TFT_WarlockPact:
        inGroup = group.HasIsWarlockPact();
        break;

    case TFT_WarlockPactAbility:
        inGroup = group.HasIsWarlockPactAbility();
        break;

    case TFT_WarlockPactSaveBonus:
        inGroup = group.HasIsWarlockSaveBonus();
        break;

    case TFT_WarlockPactSpell:
        inGroup = group.HasIsWarlockPactSpell();
        break;

    case TFT_WizardMetamagic:
        inGroup = group.HasIsMetamagic();
        break;
    }
    return inGroup;
}

size_t TrainedCount(
        const std::list<TrainedFeat> & currentFeats,
        const std::string & featName)
{
    // look through the list of all feats trained and count how many times the
    // given entry appears in the list
    size_t count = 0;
    std::list<TrainedFeat>::const_iterator it = currentFeats.begin();
    while (it != currentFeats.end())
    {
        if ((*it).FeatName() == featName)
        {
            ++count;
        }
        ++it;
    }
    return count;
}

const Feat & FindFeat(const std::string & featName)
{
    // find the information about a specific feat from the feat list
    static Feat featNotFound;
    CDDOCPApp * pApp = dynamic_cast<CDDOCPApp*>(AfxGetApp());
    if (pApp != NULL)
    {
        const std::list<Feat> & allFeats = pApp->AllFeats();
        std::list<Feat>::const_iterator it = allFeats.begin();
        while (it != allFeats.end())
        {
            if ((*it).Name() == featName)
            {
                // this is the feat were looking for
                return (*it);
            }
            ++it;
        }
    }
    return featNotFound;
}

const TrainedFeat * FindFeatInList(
        const std::list<TrainedFeat> & featList,
        const std::string & featName)
{
    // looks through a list of feats and returns a pointer to the required feat
    // if its found, NULL otherwise
    const TrainedFeat * feat = NULL;
    std::list<TrainedFeat>::const_iterator it = featList.begin();
    while (feat == NULL && it != featList.end())
    {
        if ((*it).FeatName() == featName)
        {
            feat = &(*it);
        }
        ++it;
    }
    return feat;
}

size_t ClassHitpoints(ClassType type)
{
    int hp = 0;
    switch (type)
    {
        // d4 classes
        case Class_Sorcerer:
        case Class_Wizard:
            hp = 4;
            break;

        // d6 classes
        case Class_Artificer:
        case Class_Bard:
        case Class_Rogue:
        case Class_Warlock:
            hp = 6;
            break;

        // d8 classes
        case Class_Cleric:
        case Class_Druid:
        case Class_FavoredSoul:
        case Class_Monk:
        case Class_Ranger:
            hp = 8;
            break;

        // d10 classes
        case Class_Epic:
        case Class_Fighter:
        case Class_Paladin:
            hp = 10;
            break;

        // d12 classes
        case Class_Barbarian:
            hp = 12;
            break;
    }
    return hp;
}

size_t ClassSave(SaveType st, ClassType ct, size_t level)
{
    size_t save = 0;
    if (ct != Class_Epic)
    {
        ASSERT(level <= MAX_CLASS_LEVEL);
        // saves due to class levels come in two types:
        // Level    1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20
        // -------------------------------------------------------------------
        // Type1    0  0  1  1  1  2  2  2  3  3  3  4  4  4  5  5  5  6  6  6
        // Type2    2  3  3  4  4  5  5  6  6  7  7  8  8  9  9 10 10 11 11 12
        bool type1 = false;
        bool type2 = false;
        switch (st)
        {
        case Save_Fortitude:
            switch (ct)
            {
            case Class_Artificer:
            case Class_Bard:
            case Class_Rogue:
            case Class_Sorcerer:
            case Class_Warlock:
            case Class_Wizard:
                type1 = true;
                break;
            case Class_Barbarian:
            case Class_Cleric:
            case Class_Druid:
            case Class_FavoredSoul:
            case Class_Fighter:
            case Class_Monk:
            case Class_Paladin:
            case Class_Ranger:
                type2 = true;
                break;
            }
            break;
        case Save_Reflex:
            switch (ct)
            {
            case Class_Artificer:
            case Class_Barbarian:
            case Class_Cleric:
            case Class_Druid:
            case Class_Fighter:
            case Class_Paladin:
            case Class_Sorcerer:
            case Class_Warlock:
            case Class_Wizard:
                type1 = true;
                break;
            case Class_Bard:
            case Class_FavoredSoul:
            case Class_Monk:
            case Class_Ranger:
            case Class_Rogue:
                type2 = true;
                break;
            }
            break;
        case Save_Will:
            switch (ct)
            {
            case Class_Barbarian:
            case Class_Fighter:
            case Class_Paladin:
            case Class_Ranger:
            case Class_Rogue:
                type1 = true;
                break;
            case Class_Artificer:
            case Class_Bard:
            case Class_Cleric:
            case Class_Druid:
            case Class_FavoredSoul:
            case Class_Monk:
            case Class_Sorcerer:
            case Class_Warlock:
            case Class_Wizard:
                type2 = true;
                break;
            }
            break;
        default:
            ASSERT(FALSE);      // should not be called with any other save type which are sub
            // types of the other saves
            break;
        }
        if (type1)
        {
            save = (size_t)floor(level / 3.0);
        }
        else if (type2)
        {
            save = 2 + (size_t)floor(level / 2.0);
        }
        else
        {
            if (ct != Class_Unknown)
            {
                ASSERT(FALSE);  // save should be one of the two types, check the combination passed in
            }
        }
    }
    else
    {
        // epic saves handled by feats
    }
    return save;
}

HRESULT LoadImageFile(
        ImageType type,
        const std::string & name,
        CImage * pImage,
        bool loadDefaultOnFail)
{
    // load all the images for the skills and add to the image list
    char fullPath[MAX_PATH];
    ::GetModuleFileName(
            NULL,
            fullPath,
            MAX_PATH);

    char drive[_MAX_DRIVE];
    char folder[_MAX_PATH];
    _splitpath_s(fullPath,
            drive, _MAX_DRIVE,
            folder, _MAX_PATH,
            NULL, 0,        // filename
            NULL, 0);       // extension

    char path[_MAX_PATH];
    _makepath_s(path, _MAX_PATH, drive, folder, NULL, NULL);

    std::string location = path;
    switch (type)
    {
    case IT_enhancement:
        location += "EnhancementImages\\";
        break;
    case IT_feat:
        location += "FeatImages\\";
        break;
    case IT_spell:
        location += "SpellImages\\";
        break;
    case IT_ui:
        location += "UIImages\\";
        break;
    }
    std::string filename = location;
    filename += name;
    filename += ".png";
    HRESULT result = pImage->Load(filename.c_str());
    if (result != S_OK)
    {
        if (loadDefaultOnFail)
        {
            std::stringstream ss;
            ss << "Failed to find image file \"" << filename.c_str() << "\"\n";
            ::OutputDebugString(ss.str().c_str());
            std::string alternate = path;
            alternate += "UIImages\\NoImage.png";
            CImage image;
            result = pImage->Load(alternate.c_str());
        }
    }
    if (result == S_OK)
    {
        pImage->SetTransparentColor(RGB(255, 128, 255));
        if (type == IT_enhancement
                || type == IT_feat
                || type == IT_spell)
        {
            // check the image is the correct size
            if (pImage->GetHeight() != 32
                    || pImage->GetWidth() != 32)
            {
                std::stringstream ss;
                ss << "Image in file \"" << filename.c_str() << "\" wrong size\n";
                ::OutputDebugString(ss.str().c_str());
            }
        }
    }
    return result;
}

bool ImageFileExists(ImageType type, const std::string & name)
{
    // load all the images for the skills and add to the image list
    char fullPath[MAX_PATH];
    ::GetModuleFileName(
            NULL,
            fullPath,
            MAX_PATH);

    char drive[_MAX_DRIVE];
    char folder[_MAX_PATH];
    _splitpath_s(fullPath,
            drive, _MAX_DRIVE,
            folder, _MAX_PATH,
            NULL, 0,        // filename
            NULL, 0);       // extension

    char path[_MAX_PATH];
    _makepath_s(path, _MAX_PATH, drive, folder, NULL, NULL);

    std::string location = path;
    switch (type)
    {
    case IT_enhancement:
        location += "EnhancementImages\\";
        break;
    case IT_feat:
        location += "FeatImages\\";
        break;
    case IT_spell:
        location += "SpellImages\\";
        break;
    case IT_ui:
        location += "UIImages\\";
        break;
    }
    std::string filename = location;
    filename += name;
    filename += ".png";

    bool exists = false;
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(filename.c_str(), &findFileData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
        exists = true;
    }
    return exists;
}

CMainFrame * GetMainFrame()
{
    CMainFrame * pMainFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
    ASSERT(pMainFrame != NULL);
    return pMainFrame;
}

std::vector<size_t> SpellSlotsForClass(ClassType ct, size_t level)
{
    using namespace boost::assign;
    std::vector<size_t> spellsSlotsPerSpellLevel;

    ASSERT(level <= MAX_CLASS_LEVEL);
    switch (ct)
    {
    case Class_Artificer:
        switch (level)
        {
            case  0: break; // no spells, as no class levels in artificer
            case  1: spellsSlotsPerSpellLevel += 2, 0, 0, 0, 0, 0; break;
            case  2: spellsSlotsPerSpellLevel += 3, 0, 0, 0, 0, 0; break;
            case  3: spellsSlotsPerSpellLevel += 3, 1, 0, 0, 0, 0; break;
            case  4: spellsSlotsPerSpellLevel += 3, 2, 0, 0, 0, 0; break;
            case  5: spellsSlotsPerSpellLevel += 3, 2, 0, 0, 0, 0; break;
            case  6: spellsSlotsPerSpellLevel += 3, 2, 1, 0, 0, 0; break;
            case  7: spellsSlotsPerSpellLevel += 3, 2, 2, 0, 0, 0; break;
            case  8: spellsSlotsPerSpellLevel += 3, 3, 2, 0, 0, 0; break;
            case  9: spellsSlotsPerSpellLevel += 3, 3, 2, 1, 0, 0; break;
            case 10: spellsSlotsPerSpellLevel += 3, 3, 2, 2, 0, 0; break;
            case 11: spellsSlotsPerSpellLevel += 4, 3, 3, 2, 0, 0; break;
            case 12: spellsSlotsPerSpellLevel += 4, 3, 3, 2, 1, 0; break;
            case 13: spellsSlotsPerSpellLevel += 4, 4, 3, 2, 2, 0; break;
            case 14: spellsSlotsPerSpellLevel += 4, 4, 3, 3, 2, 0; break;
            case 15: spellsSlotsPerSpellLevel += 4, 4, 4, 3, 2, 1; break;
            case 16: spellsSlotsPerSpellLevel += 4, 4, 4, 3, 2, 2; break;
            case 17: spellsSlotsPerSpellLevel += 5, 4, 4, 4, 3, 2; break;
            case 18: spellsSlotsPerSpellLevel += 5, 5, 4, 4, 3, 2; break;
            case 19: spellsSlotsPerSpellLevel += 5, 5, 5, 4, 4, 3; break;
            case 20: spellsSlotsPerSpellLevel += 5, 5, 5, 5, 4, 4; break;
            default: ASSERT(FALSE); break;
        }
        break;
    case Class_Barbarian:
        // no spells of any level
        break;
    case Class_Bard:
        switch (level)
        {
            case  0: break; // no spells, as no class levels in bard
            case  1: spellsSlotsPerSpellLevel += 1, 0, 0, 0, 0, 0; break;
            case  2: spellsSlotsPerSpellLevel += 2, 0, 0, 0, 0, 0; break;
            case  3: spellsSlotsPerSpellLevel += 3, 0, 0, 0, 0, 0; break;
            case  4: spellsSlotsPerSpellLevel += 3, 2, 0, 0, 0, 0; break;
            case  5: spellsSlotsPerSpellLevel += 4, 3, 0, 0, 0, 0; break;
            case  6: spellsSlotsPerSpellLevel += 4, 3, 0, 0, 0, 0; break;
            case  7: spellsSlotsPerSpellLevel += 4, 4, 2, 0, 0, 0; break;
            case  8: spellsSlotsPerSpellLevel += 4, 4, 3, 0, 0, 0; break;
            case  9: spellsSlotsPerSpellLevel += 4, 4, 3, 0, 0, 0; break;
            case 10: spellsSlotsPerSpellLevel += 4, 4, 4, 2, 0, 0; break;
            case 11: spellsSlotsPerSpellLevel += 4, 4, 4, 3, 0, 0; break;
            case 12: spellsSlotsPerSpellLevel += 4, 4, 4, 3, 0, 0; break;
            case 13: spellsSlotsPerSpellLevel += 4, 4, 4, 4, 2, 0; break;
            case 14: spellsSlotsPerSpellLevel += 4, 4, 4, 4, 3, 0; break;
            case 15: spellsSlotsPerSpellLevel += 4, 4, 4, 4, 3, 0; break;
            case 16: spellsSlotsPerSpellLevel += 5, 4, 4, 4, 4, 2; break;
            case 17: spellsSlotsPerSpellLevel += 5, 5, 4, 4, 4, 3; break;
            case 18: spellsSlotsPerSpellLevel += 5, 5, 5, 4, 4, 3; break;
            case 19: spellsSlotsPerSpellLevel += 5, 5, 5, 5, 4, 4; break;
            case 20: spellsSlotsPerSpellLevel += 5, 5, 5, 5, 5, 4; break;
            default: ASSERT(FALSE); break;
        }
        break;
    case Class_Cleric:
        switch (level)
        {
            case  0: break; // no spells, as no class levels in cleric
            case  1: spellsSlotsPerSpellLevel += 3, 0, 0, 0, 0, 0, 0, 0, 0; break;
            case  2: spellsSlotsPerSpellLevel += 4, 0, 0, 0, 0, 0, 0, 0, 0; break;
            case  3: spellsSlotsPerSpellLevel += 4, 3, 0, 0, 0, 0, 0, 0, 0; break;
            case  4: spellsSlotsPerSpellLevel += 5, 4, 0, 0, 0, 0, 0, 0, 0; break;
            case  5: spellsSlotsPerSpellLevel += 5, 4, 3, 0, 0, 0, 0, 0, 0; break;
            case  6: spellsSlotsPerSpellLevel += 5, 5, 4, 0, 0, 0, 0, 0, 0; break;
            case  7: spellsSlotsPerSpellLevel += 6, 5, 4, 3, 0, 0, 0, 0, 0; break;
            case  8: spellsSlotsPerSpellLevel += 6, 5, 5, 4, 0, 0, 0, 0, 0; break;
            case  9: spellsSlotsPerSpellLevel += 6, 6, 5, 4, 3, 0, 0, 0, 0; break;
            case 10: spellsSlotsPerSpellLevel += 6, 6, 5, 5, 4, 0, 0, 0, 0; break;
            case 11: spellsSlotsPerSpellLevel += 6, 6, 6, 5, 4, 3, 0, 0, 0; break;
            case 12: spellsSlotsPerSpellLevel += 6, 6, 6, 5, 5, 4, 0, 0, 0; break;
            case 13: spellsSlotsPerSpellLevel += 6, 6, 6, 6, 5, 4, 3, 0, 0; break;
            case 14: spellsSlotsPerSpellLevel += 6, 6, 6, 6, 5, 5, 4, 0, 0; break;
            case 15: spellsSlotsPerSpellLevel += 6, 6, 6, 6, 6, 5, 4, 3, 0; break;
            case 16: spellsSlotsPerSpellLevel += 6, 6, 6, 6, 6, 5, 5, 4, 0; break;
            case 17: spellsSlotsPerSpellLevel += 6, 6, 6, 6, 6, 6, 5, 4, 2; break;
            case 18: spellsSlotsPerSpellLevel += 6, 6, 6, 6, 6, 6, 5, 5, 3; break;
            case 19: spellsSlotsPerSpellLevel += 6, 6, 6, 6, 6, 6, 6, 5, 4; break;
            case 20: spellsSlotsPerSpellLevel += 7, 6, 6, 6, 6, 6, 6, 6, 5; break;
            default: ASSERT(FALSE); break;
        }
        break;
    case Class_Druid:
        switch (level)
        {
            case  0: break; // no spells, as no class levels in druid
            case  1: spellsSlotsPerSpellLevel += 2, 0, 0, 0, 0, 0, 0, 0, 0; break;
            case  2: spellsSlotsPerSpellLevel += 3, 0, 0, 0, 0, 0, 0, 0, 0; break;
            case  3: spellsSlotsPerSpellLevel += 3, 2, 0, 0, 0, 0, 0, 0, 0; break;
            case  4: spellsSlotsPerSpellLevel += 4, 3, 0, 0, 0, 0, 0, 0, 0; break;
            case  5: spellsSlotsPerSpellLevel += 4, 3, 2, 0, 0, 0, 0, 0, 0; break;
            case  6: spellsSlotsPerSpellLevel += 4, 4, 3, 0, 0, 0, 0, 0, 0; break;
            case  7: spellsSlotsPerSpellLevel += 5, 4, 3, 2, 0, 0, 0, 0, 0; break;
            case  8: spellsSlotsPerSpellLevel += 5, 4, 4, 3, 0, 0, 0, 0, 0; break;
            case  9: spellsSlotsPerSpellLevel += 5, 5, 4, 3, 2, 0, 0, 0, 0; break;
            case 10: spellsSlotsPerSpellLevel += 5, 5, 4, 4, 3, 0, 0, 0, 0; break;
            case 11: spellsSlotsPerSpellLevel += 5, 5, 5, 4, 3, 2, 0, 0, 0; break;
            case 12: spellsSlotsPerSpellLevel += 5, 5, 5, 4, 4, 3, 0, 0, 0; break;
            case 13: spellsSlotsPerSpellLevel += 5, 5, 5, 5, 4, 3, 2, 0, 0; break;
            case 14: spellsSlotsPerSpellLevel += 5, 5, 5, 5, 4, 4, 3, 0, 0; break;
            case 15: spellsSlotsPerSpellLevel += 5, 5, 5, 5, 5, 4, 3, 2, 0; break;
            case 16: spellsSlotsPerSpellLevel += 5, 5, 5, 5, 5, 4, 4, 3, 0; break;
            case 17: spellsSlotsPerSpellLevel += 5, 5, 5, 5, 5, 5, 4, 3, 2; break;
            case 18: spellsSlotsPerSpellLevel += 5, 5, 5, 5, 5, 5, 4, 4, 3; break;
            case 19: spellsSlotsPerSpellLevel += 5, 5, 5, 5, 5, 5, 5, 4, 4; break;
            case 20: spellsSlotsPerSpellLevel += 5, 5, 5, 5, 5, 5, 5, 5, 5; break;
            default: ASSERT(FALSE); break;
        }
        break;
    case Class_FavoredSoul:
        switch (level)
        {
            case  0: break; // no spells, as no class levels in favored soul
            case  1: spellsSlotsPerSpellLevel += 2, 0, 0, 0, 0, 0, 0, 0, 0; break;
            case  2: spellsSlotsPerSpellLevel += 3, 0, 0, 0, 0, 0, 0, 0, 0; break;
            case  3: spellsSlotsPerSpellLevel += 4, 0, 0, 0, 0, 0, 0, 0, 0; break;
            case  4: spellsSlotsPerSpellLevel += 4, 1, 0, 0, 0, 0, 0, 0, 0; break;
            case  5: spellsSlotsPerSpellLevel += 4, 2, 0, 0, 0, 0, 0, 0, 0; break;
            case  6: spellsSlotsPerSpellLevel += 4, 2, 1, 0, 0, 0, 0, 0, 0; break;
            case  7: spellsSlotsPerSpellLevel += 4, 3, 2, 0, 0, 0, 0, 0, 0; break;
            case  8: spellsSlotsPerSpellLevel += 4, 3, 2, 1, 0, 0, 0, 0, 0; break;
            case  9: spellsSlotsPerSpellLevel += 4, 4, 3, 2, 0, 0, 0, 0, 0; break;
            case 10: spellsSlotsPerSpellLevel += 4, 4, 3, 2, 1, 0, 0, 0, 0; break;
            case 11: spellsSlotsPerSpellLevel += 4, 4, 4, 3, 2, 0, 0, 0, 0; break;
            case 12: spellsSlotsPerSpellLevel += 4, 4, 4, 3, 2, 1, 0, 0, 0; break;
            case 13: spellsSlotsPerSpellLevel += 4, 4, 4, 4, 3, 2, 0, 0, 0; break;
            case 14: spellsSlotsPerSpellLevel += 4, 4, 4, 4, 3, 2, 1, 0, 0; break;
            case 15: spellsSlotsPerSpellLevel += 4, 4, 4, 4, 4, 3, 2, 0, 0; break;
            case 16: spellsSlotsPerSpellLevel += 4, 4, 4, 4, 4, 3, 2, 1, 0; break;
            case 17: spellsSlotsPerSpellLevel += 4, 4, 4, 4, 4, 3, 3, 2, 0; break;
            case 18: spellsSlotsPerSpellLevel += 4, 4, 4, 4, 4, 3, 3, 2, 1; break;
            case 19: spellsSlotsPerSpellLevel += 4, 4, 4, 4, 4, 3, 3, 3, 2; break;
            case 20: spellsSlotsPerSpellLevel += 4, 4, 4, 4, 4, 3, 3, 3, 3; break;
            default: ASSERT(FALSE); break;
        }
        break;
    case Class_Fighter:
        // no spells of any level
        break;
    case Class_Monk:
        // no spells of any level
        break;
    case Class_Paladin:
        switch (level)
        {
            case  0: break; // no spells, as no class levels in paladin
            case  1: spellsSlotsPerSpellLevel += 0, 0, 0, 0; break;
            case  2: spellsSlotsPerSpellLevel += 0, 0, 0, 0; break;
            case  3: spellsSlotsPerSpellLevel += 0, 0, 0, 0; break;
            case  4: spellsSlotsPerSpellLevel += 0, 0, 0, 0; break;
            case  5: spellsSlotsPerSpellLevel += 1, 0, 0, 0; break;
            case  6: spellsSlotsPerSpellLevel += 2, 0, 0, 0; break;
            case  7: spellsSlotsPerSpellLevel += 2, 0, 0, 0; break;
            case  8: spellsSlotsPerSpellLevel += 2, 1, 0, 0; break;
            case  9: spellsSlotsPerSpellLevel += 2, 1, 0, 0; break;
            case 10: spellsSlotsPerSpellLevel += 2, 2, 0, 0; break;
            case 11: spellsSlotsPerSpellLevel += 2, 2, 1, 0; break;
            case 12: spellsSlotsPerSpellLevel += 2, 2, 2, 0; break;
            case 13: spellsSlotsPerSpellLevel += 2, 2, 2, 0; break;
            case 14: spellsSlotsPerSpellLevel += 3, 2, 2, 1; break;
            case 15: spellsSlotsPerSpellLevel += 3, 2, 2, 2; break;
            case 16: spellsSlotsPerSpellLevel += 3, 3, 2, 2; break;
            case 17: spellsSlotsPerSpellLevel += 3, 3, 2, 2; break;
            case 18: spellsSlotsPerSpellLevel += 4, 3, 2, 2; break;
            case 19: spellsSlotsPerSpellLevel += 4, 4, 4, 3; break;
            case 20: spellsSlotsPerSpellLevel += 4, 4, 4, 4; break;
            default: ASSERT(FALSE); break;
        }
        break;
    case Class_Ranger:
        switch (level)
        {
            case  0: break; // no spells, as no class levels in Ranger
            case  1: spellsSlotsPerSpellLevel += 0, 0, 0, 0; break;
            case  2: spellsSlotsPerSpellLevel += 0, 0, 0, 0; break;
            case  3: spellsSlotsPerSpellLevel += 0, 0, 0, 0; break;
            case  4: spellsSlotsPerSpellLevel += 1, 0, 0, 0; break;
            case  5: spellsSlotsPerSpellLevel += 1, 0, 0, 0; break;
            case  6: spellsSlotsPerSpellLevel += 2, 0, 0, 0; break;
            case  7: spellsSlotsPerSpellLevel += 2, 0, 0, 0; break;
            case  8: spellsSlotsPerSpellLevel += 2, 1, 0, 0; break;
            case  9: spellsSlotsPerSpellLevel += 2, 1, 0, 0; break;
            case 10: spellsSlotsPerSpellLevel += 2, 2, 0, 0; break;
            case 11: spellsSlotsPerSpellLevel += 2, 2, 1, 0; break;
            case 12: spellsSlotsPerSpellLevel += 2, 2, 2, 0; break;
            case 13: spellsSlotsPerSpellLevel += 2, 2, 2, 0; break;
            case 14: spellsSlotsPerSpellLevel += 3, 2, 2, 1; break;
            case 15: spellsSlotsPerSpellLevel += 3, 2, 2, 2; break;
            case 16: spellsSlotsPerSpellLevel += 3, 3, 2, 2; break;
            case 17: spellsSlotsPerSpellLevel += 3, 3, 2, 2; break;
            case 18: spellsSlotsPerSpellLevel += 4, 3, 2, 2; break;
            case 19: spellsSlotsPerSpellLevel += 4, 4, 4, 3; break;
            case 20: spellsSlotsPerSpellLevel += 4, 4, 4, 4; break;
            default: ASSERT(FALSE); break;
        }
        break;
    case Class_Rogue:
        // no spells of any level
        break;
    case Class_Sorcerer:
        switch (level)
        {
            case  0: break; // no spells, as no class levels in sorcerer
            case  1: spellsSlotsPerSpellLevel += 2, 0, 0, 0, 0, 0, 0, 0, 0; break;
            case  2: spellsSlotsPerSpellLevel += 3, 0, 0, 0, 0, 0, 0, 0, 0; break;
            case  3: spellsSlotsPerSpellLevel += 4, 0, 0, 0, 0, 0, 0, 0, 0; break;
            case  4: spellsSlotsPerSpellLevel += 4, 1, 0, 0, 0, 0, 0, 0, 0; break;
            case  5: spellsSlotsPerSpellLevel += 4, 2, 0, 0, 0, 0, 0, 0, 0; break;
            case  6: spellsSlotsPerSpellLevel += 4, 2, 1, 0, 0, 0, 0, 0, 0; break;
            case  7: spellsSlotsPerSpellLevel += 4, 3, 2, 0, 0, 0, 0, 0, 0; break;
            case  8: spellsSlotsPerSpellLevel += 4, 3, 2, 1, 0, 0, 0, 0, 0; break;
            case  9: spellsSlotsPerSpellLevel += 4, 4, 3, 2, 0, 0, 0, 0, 0; break;
            case 10: spellsSlotsPerSpellLevel += 4, 4, 3, 2, 1, 0, 0, 0, 0; break;
            case 11: spellsSlotsPerSpellLevel += 4, 4, 4, 3, 2, 0, 0, 0, 0; break;
            case 12: spellsSlotsPerSpellLevel += 4, 4, 4, 3, 2, 1, 0, 0, 0; break;
            case 13: spellsSlotsPerSpellLevel += 4, 4, 4, 4, 3, 2, 0, 0, 0; break;
            case 14: spellsSlotsPerSpellLevel += 4, 4, 4, 4, 3, 2, 1, 0, 0; break;
            case 15: spellsSlotsPerSpellLevel += 4, 4, 4, 4, 4, 3, 2, 0, 0; break;
            case 16: spellsSlotsPerSpellLevel += 4, 4, 4, 4, 4, 3, 2, 1, 0; break;
            case 17: spellsSlotsPerSpellLevel += 4, 4, 4, 4, 4, 3, 3, 2, 0; break;
            case 18: spellsSlotsPerSpellLevel += 4, 4, 4, 4, 4, 3, 3, 2, 1; break;
            case 19: spellsSlotsPerSpellLevel += 4, 4, 4, 4, 4, 3, 3, 3, 2; break;
            case 20: spellsSlotsPerSpellLevel += 4, 4, 4, 4, 4, 3, 3, 3, 3; break;
            default: ASSERT(FALSE); break;
        }
        break;
    case Class_Warlock:
        switch (level)
        {
            case  0: break; // no spells, as no class levels in warlock
            case  1: spellsSlotsPerSpellLevel += 1, 0, 0, 0, 0, 0; break;
            case  2: spellsSlotsPerSpellLevel += 2, 0, 0, 0, 0, 0; break;
            case  3: spellsSlotsPerSpellLevel += 2, 0, 0, 0, 0, 0; break;
            case  4: spellsSlotsPerSpellLevel += 2, 1, 0, 0, 0, 0; break;
            case  5: spellsSlotsPerSpellLevel += 2, 1, 0, 0, 0, 0; break;
            case  6: spellsSlotsPerSpellLevel += 2, 1, 0, 0, 0, 0; break;
            case  7: spellsSlotsPerSpellLevel += 2, 1, 1, 0, 0, 0; break;
            case  8: spellsSlotsPerSpellLevel += 2, 2, 1, 0, 0, 0; break;
            case  9: spellsSlotsPerSpellLevel += 2, 2, 1, 0, 0, 0; break;
            case 10: spellsSlotsPerSpellLevel += 2, 2, 1, 1, 0, 0; break;
            case 11: spellsSlotsPerSpellLevel += 2, 2, 2, 1, 0, 0; break;
            case 12: spellsSlotsPerSpellLevel += 2, 2, 2, 1, 0, 0; break;
            case 13: spellsSlotsPerSpellLevel += 2, 2, 2, 1, 1, 0; break;
            case 14: spellsSlotsPerSpellLevel += 2, 2, 2, 2, 1, 0; break;
            case 15: spellsSlotsPerSpellLevel += 2, 2, 2, 2, 1, 0; break;
            case 16: spellsSlotsPerSpellLevel += 2, 2, 2, 2, 1, 1; break;
            case 17: spellsSlotsPerSpellLevel += 2, 2, 2, 2, 2, 1; break;
            case 18: spellsSlotsPerSpellLevel += 2, 2, 2, 2, 2, 1; break;
            case 19: spellsSlotsPerSpellLevel += 2, 2, 2, 2, 2, 2; break;
            case 20: spellsSlotsPerSpellLevel += 2, 2, 2, 2, 2, 2; break;
            default: ASSERT(FALSE); break;
        }
        break;
    case Class_Wizard:
        switch (level)
        {
            case  0: break; // no spells, as no class levels in wizard
            case  1: spellsSlotsPerSpellLevel += 3, 0, 0, 0, 0, 0, 0, 0, 0; break;
            case  2: spellsSlotsPerSpellLevel += 4, 0, 0, 0, 0, 0, 0, 0, 0; break;
            case  3: spellsSlotsPerSpellLevel += 4, 2, 0, 0, 0, 0, 0, 0, 0; break;
            case  4: spellsSlotsPerSpellLevel += 4, 3, 0, 0, 0, 0, 0, 0, 0; break;
            case  5: spellsSlotsPerSpellLevel += 4, 3, 2, 0, 0, 0, 0, 0, 0; break;
            case  6: spellsSlotsPerSpellLevel += 4, 4, 3, 0, 0, 0, 0, 0, 0; break;
            case  7: spellsSlotsPerSpellLevel += 5, 4, 3, 2, 0, 0, 0, 0, 0; break;
            case  8: spellsSlotsPerSpellLevel += 5, 4, 4, 3, 0, 0, 0, 0, 0; break;
            case  9: spellsSlotsPerSpellLevel += 5, 5, 4, 3, 2, 0, 0, 0, 0; break;
            case 10: spellsSlotsPerSpellLevel += 5, 5, 4, 4, 3, 0, 0, 0, 0; break;
            case 11: spellsSlotsPerSpellLevel += 5, 5, 5, 4, 3, 2, 0, 0, 0; break;
            case 12: spellsSlotsPerSpellLevel += 5, 5, 5, 4, 4, 3, 0, 0, 0; break;
            case 13: spellsSlotsPerSpellLevel += 5, 5, 5, 5, 4, 3, 2, 0, 0; break;
            case 14: spellsSlotsPerSpellLevel += 5, 5, 5, 5, 4, 4, 3, 0, 0; break;
            case 15: spellsSlotsPerSpellLevel += 5, 5, 5, 5, 5, 4, 3, 2, 0; break;
            case 16: spellsSlotsPerSpellLevel += 5, 5, 5, 5, 5, 4, 4, 3, 0; break;
            case 17: spellsSlotsPerSpellLevel += 5, 5, 5, 5, 5, 5, 4, 3, 2; break;
            case 18: spellsSlotsPerSpellLevel += 5, 5, 5, 5, 5, 5, 4, 4, 3; break;
            case 19: spellsSlotsPerSpellLevel += 5, 5, 5, 5, 5, 5, 5, 4, 4; break;
            case 20: spellsSlotsPerSpellLevel += 5, 5, 5, 5, 5, 5, 5, 5, 5; break;
            default: ASSERT(FALSE); break;
        }
        break;
    }
    return spellsSlotsPerSpellLevel;
}

size_t ClassSpellPoints(ClassType ct, size_t level)
{
    using namespace boost::assign;
    std::vector<size_t> spPerLevel;
    ASSERT(level <= MAX_CLASS_LEVEL);
    switch (ct)
    {
    case Class_Artificer:
        spPerLevel += 50, 75, 100, 125, 150, 180, 210, 245, 280, 320, 360 ,405 ,450 ,500, 550, 600, 660, 720, 780, 845;
        break;
    case Class_Bard:
    case Class_Warlock:
        spPerLevel += 50, 75, 100, 125, 150, 175, 200, 225, 250, 275, 300, 325, 350, 375, 400, 425, 450, 475, 500, 525;
        break;
    case Class_Cleric:
    case Class_Druid:
    case Class_Wizard:
        spPerLevel += 50, 75, 100, 125, 150, 180, 215, 255, 300, 350, 405, 465, 530, 600, 675, 755, 840, 930, 1025, 1125;
        break;
    case Class_FavoredSoul:
    case Class_Sorcerer:
        spPerLevel += 100, 150, 200, 250, 300, 355, 415, 480, 550, 625, 705, 790, 880, 975, 1075, 1180, 1290, 1405, 1525, 1650;
        break;
    case Class_Paladin:
    case Class_Ranger:
        spPerLevel += 0, 0, 0, 20, 35, 50, 65, 80, 95, 110, 125, 140, 155, 170, 185, 200, 215, 230, 245, 260;
        break;
    default:
        // unknown class's have 0 sp
        spPerLevel.resize(MAX_CLASS_LEVEL, 0);
        break;
    }
    ASSERT(spPerLevel.size() >= level);
    ASSERT(level > 0);
    size_t spellPoints = spPerLevel[level-1];    // 1 based
    return spellPoints;
}

AbilityType ClassCastingStat(ClassType ct)
{
    AbilityType at = Ability_Unknown;
    switch (ct)
    {
    case Class_Artificer:
    case Class_Wizard:
        at = Ability_Intelligence;
        break;
    case Class_Bard:
    case Class_Sorcerer:
    case Class_Warlock:
        at = Ability_Charisma;
        break;
    case Class_Cleric:
    case Class_Druid:
    case Class_FavoredSoul:
    case Class_Paladin:
    case Class_Ranger:
        at = Ability_Wisdom;
        break;
    }
    ASSERT(at != Ability_Unknown);
    return at;
}

// CImage convert to GrayScale by changing the CImage color table
void MakeGrayScale(CImage * pImage, COLORREF transparent)
{
    if (pImage->IsIndexed())
    {
        // if the image is indexed, we can change all the colors by manipulating the
        // color table in use by the bitmap
        size_t numColors = pImage->GetMaxColorTableEntries();
        RGBQUAD * pColors = new RGBQUAD[numColors];
        pImage->GetColorTable(0, numColors, pColors);
        // now change each entry to it's gray scale equivalent unless
        // its the background transparent color
        for (size_t ci = 0; ci < numColors; ++ci)
        {
            COLORREF color = RGB(pColors[ci].rgbRed, pColors[ci].rgbGreen, pColors[ci].rgbBlue);
            if (color != transparent)
            {
                // this is not the transparent background color
                // change it to gray scale using the luminosity method
                // luminosity = 0.21 R  + 0.72 G + 0.07 B.
                BYTE luminosity =
                        (BYTE)(0.21 * pColors[ci].rgbRed
                        + 0.72 * pColors[ci].rgbGreen
                        + 0.07 * pColors[ci].rgbBlue);
                pColors[ci].rgbRed = luminosity;
                pColors[ci].rgbGreen = luminosity;
                pColors[ci].rgbBlue = luminosity;
            }
        }
        // now all colors have been made gray scale set them back on the image
        pImage->SetColorTable(0, numColors, pColors);
        delete []pColors;
    }
    else
    {
        // if its not indexed, we have to manually get and set all bitmap pixels
        size_t xSize = pImage->GetWidth();
        size_t ySize = pImage->GetHeight();
        for (size_t x = 0; x < xSize; ++x)
        {
            for (size_t y = 0; y < ySize; ++y)
            {
                COLORREF color = pImage->GetPixel(x, y);
                if (color != transparent)
                {
                    // HACK, binary representations must match
                    RGBQUAD pixel;
                    memcpy(&pixel, &color, sizeof(COLORREF));
                    BYTE luminosity =
                            (BYTE)(0.21 * pixel.rgbRed
                            + 0.72 * pixel.rgbGreen
                            + 0.07 * pixel.rgbBlue);
                    COLORREF grey = RGB(luminosity, luminosity, luminosity);
                    pImage->SetPixel(x, y, grey);
                }
            }
        }
    }
}

