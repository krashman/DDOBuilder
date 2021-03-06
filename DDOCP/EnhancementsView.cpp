// EnhancementsView.cpp
//
#include "stdafx.h"
#include "EnhancementsView.h"

#include "DDOCP.h"
#include "EnhancementTreeDialog.h"
#include "GlobalSupportFunctions.h"

namespace
{
    const int c_controlSpacing = 3;
    const UINT UWM_NEW_DOCUMENT = ::RegisterWindowMessage(_T("NewActiveDocument"));
    // enhancement window size
    const size_t c_sizeX = 300;
    const size_t c_sizeY = 466;
    const std::string c_noSelection = "No selection";
}

IMPLEMENT_DYNCREATE(CEnhancementsView, CFormView)

CEnhancementsView::CEnhancementsView() :
    CFormView(CEnhancementsView::IDD),
    m_pDocument(NULL),
    m_pCharacter(NULL),
    m_scrollOffset(0)
{
}

CEnhancementsView::~CEnhancementsView()
{
    for (size_t vi = 0; vi < m_treeViews.size(); ++vi)
    {
        delete m_treeViews[vi];
    }
}

void CEnhancementsView::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BUTTON_LEFT, m_buttonLeft);
    DDX_Control(pDX, IDC_BUTTON_RIGHT, m_buttonRight);
    // no IDC_TREE_SELECT1 as that is the racial tree which cannot be swapped out
    DDX_Control(pDX, IDC_TREE_SELECT2, m_comboTreeSelect[0]);
    DDX_Control(pDX, IDC_TREE_SELECT3, m_comboTreeSelect[1]);
    DDX_Control(pDX, IDC_TREE_SELECT4, m_comboTreeSelect[2]);
    DDX_Control(pDX, IDC_TREE_SELECT5, m_comboTreeSelect[3]);
    DDX_Control(pDX, IDC_TREE_SELECT6, m_comboTreeSelect[4]);
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CEnhancementsView, CFormView)
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_BN_CLICKED(IDC_BUTTON_LEFT, OnButtonLeft)
    ON_BN_CLICKED(IDC_BUTTON_RIGHT, OnButtonRight)
    ON_CONTROL_RANGE(CBN_SELENDOK, IDC_TREE_SELECT2, IDC_TREE_SELECT6, OnTreeSelect)
END_MESSAGE_MAP()
#pragma warning(pop)

#ifdef _DEBUG
void CEnhancementsView::AssertValid() const
{
    CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CEnhancementsView::Dump(CDumpContext& dc) const
{
    CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

void CEnhancementsView::OnInitialUpdate()
{
    CFormView::OnInitialUpdate();
    UpdateWindowTitle();
}

void CEnhancementsView::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    if (m_treeViews.size() > 0
            && IsWindow(m_treeViews[0]->GetSafeHwnd()))
    {
        // we can position and show all the visible enhancement windows
        // note that as these windows are big we may have to show buttons to allow
        // the visible windows to be scrolled left/right
        // calculate whether the scroll buttons are needed
        int requiredWidth = (MST_Number * c_sizeX)      // windows
                + (c_controlSpacing * (MST_Number + 1));   // spacers
        bool showScrollButtons = (requiredWidth > cx);      // true if buttons shown
        if (!showScrollButtons)
        {
            m_scrollOffset = 0;     // all can be fitted in, remove scroll if present
        }

        // default location of first enhancement tree if no scroll buttons present
        CRect itemRect(
                c_controlSpacing,
                c_controlSpacing,
                c_sizeX + c_controlSpacing,
                c_sizeY + c_controlSpacing);

        // handle the scroll left button. Only shown if the window is currently scrolled
        CRect rctButton;
        m_buttonLeft.GetWindowRect(&rctButton);
        rctButton -= rctButton.TopLeft();
        rctButton += CPoint(c_controlSpacing, (cy - rctButton.Height()) / 2);
        m_buttonLeft.MoveWindow(rctButton, FALSE);
        if (m_scrollOffset > 0)
        {
            // button needs to be seen
            m_buttonLeft.ShowWindow(SW_SHOW);
            m_buttonLeft.EnableWindow(TRUE);
            // also bump the first enhancement window right by width of the button
            itemRect += CPoint(rctButton.Width() + c_controlSpacing * 2, 0);
        }
        else
        {
            m_buttonLeft.ShowWindow(SW_HIDE);
            m_buttonLeft.EnableWindow(FALSE);
        }
        m_buttonRight.GetWindowRect(&rctButton);
        rctButton -= rctButton.TopLeft();
        rctButton += CPoint(
                    cx - c_controlSpacing - rctButton.Width(),
                    (cy - rctButton.Height()) / 2);
        m_buttonRight.MoveWindow(rctButton, FALSE);

        ASSERT(m_treeViews.size() == MST_Number);
        std::vector<bool> isShown(MST_Number, false);  // gets set to true when displayed
        for (size_t ti = 0; ti < m_visibleTrees.size(); ++ti)
        {
            if (showScrollButtons)
            {
                // determine whether we can fit any more tree windows
                if (itemRect.right >= rctButton.left - c_controlSpacing)
                {
                    // no more trees fit, break out and the remaining windows get hidden
                    break;
                }
            }
            if (ti >= m_scrollOffset)
            {
                size_t index = m_visibleTrees[ti];
                ASSERT(index >= 0 && index < MST_Number);
                // move the window to the correct location
                m_treeViews[index]->MoveWindow(itemRect, false);
                m_treeViews[index]->ShowWindow(SW_SHOW);        // ensure visible
                if (ti > 0 && ti < MST_Number)
                {
                    CRect rctCombo(itemRect.left, itemRect.bottom, itemRect.right, itemRect.bottom + 300);
                    m_comboTreeSelect[ti-1].MoveWindow(rctCombo);
                    m_comboTreeSelect[ti-1].ShowWindow(SW_SHOW);
                }
                isShown[index] = true;
                // now move the rectangle to the next tree location
                itemRect += CPoint(itemRect.Width() + c_controlSpacing, 0);
                if (ti == m_visibleTrees.size() - 1)
                {
                    // right scroll button no need to be visible
                    showScrollButtons = false;
                }
            }
            else
            {
                // although this item is visible, its scrolled off the left hand side
                // its isShown[] will remain false and window is auto hidden later
            }
        }
        // ensure all the hidden enhancement trees are not visible
        for (size_t ti = 0; ti < MST_Number; ++ti)
        {
            if (!isShown[ti])
            {
                // this tree is not visible
                m_treeViews[ti]->ShowWindow(SW_HIDE);
                if ((ti - 1) < MST_Number)
                {
                    m_comboTreeSelect[ti-1].ShowWindow(SW_HIDE);
                }
            }
        }
        // right scroll button may need to be visible
        if (showScrollButtons)
        {
            // position and show the scroll right button
            m_buttonRight.ShowWindow(SW_SHOW);
            m_buttonRight.EnableWindow(TRUE);
        }
        else
        {
            // no need to show the scroll right button
            m_buttonRight.ShowWindow(SW_HIDE);
            m_buttonRight.EnableWindow(FALSE);
        }
    }
}

LRESULT CEnhancementsView::OnNewDocument(WPARAM wParam, LPARAM lParam)
{
    if (m_pCharacter != NULL)
    {
        m_pCharacter->DetachObserver(this);
    }

    // wParam is the document pointer
    CDocument * pDoc = (CDocument*)(wParam);
    m_pDocument = pDoc;
    // lParam is the character pointer
    Character * pCharacter = (Character *)(lParam);
    m_pCharacter = pCharacter;
    if (m_pCharacter != NULL)
    {
        m_pCharacter->AttachObserver(this);
        // trees definitely change if the character has changed
        m_availableTrees = DetermineTrees();
        DestroyEnhancementWindows();
        CreateEnhancementWindows();
    }
    else
    {
        DestroyEnhancementWindows();
        EnableDisableComboboxes();
    }
    UpdateWindowTitle();
    return 0L;
}

BOOL CEnhancementsView::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        0 // end marker
    };

    // exclude any visible enhancement view windows
    for (size_t i = 0; i < m_treeViews.size(); ++i)
    {
        if (::IsWindow(m_treeViews[i]->GetSafeHwnd())
                && ::IsWindowVisible(m_treeViews[i]->GetSafeHwnd()))
        {
            CRect rctWnd;
            m_treeViews[i]->GetWindowRect(&rctWnd);
            ScreenToClient(&rctWnd);
            pDC->ExcludeClipRect(&rctWnd);
        }
    }

    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}

std::list<EnhancementTree> CEnhancementsView::DetermineTrees()
{
    std::list<EnhancementTree> trees;
    // see which classes we have and then make our race and class trees available
    if (m_pCharacter != NULL)
    {
        // to determine which tree's we need to know how many class levels we have in each class
        std::vector<size_t> classLevels = m_pCharacter->ClassLevels(MAX_LEVEL);
        const std::list<EnhancementTree> & allTrees = EnhancementTrees();
        std::list<EnhancementTree>::const_iterator it = allTrees.begin();
        while (it != allTrees.end())
        {
            // get all the trees that are compatible with the race/class setup
            if ((*it).MeetRequirements(*m_pCharacter)
                    && !(*it).HasIsReaperTree()         // no reaper trees in enhancements please!
                    && !(*it).HasIsEpicDestiny())       // no epic destiny trees in enhancements please!
            {
                // yes this is one of our tree's add it
                trees.push_back((*it));
            }
            ++it;
        }
        // first remove any trees present which are no longer compatible
        // we also revoke those trees AP spends if required
        SelectedEnhancementTrees selTrees = m_pCharacter->SelectedTrees(); // take a copy
        for (size_t ti = 0; ti < MST_Number; ++ti)
        {
            std::string treeName = selTrees.Tree(ti);
            if (!SelectedEnhancementTrees::IsNotSelected(treeName))
            {
                // we have a tree selected here, is it in the new list of trees available?
                bool found = false;
                std::list<EnhancementTree>::iterator tit = trees.begin();
                while (!found && tit != trees.end())
                {
                    if ((*tit).Name() == treeName)
                    {
                        // ok, it's still in the list
                        found = true;
                    }
                    ++tit;
                }
                if (!found)
                {
                    // the tree is no longer valid for this race/class setup
                    // revoke any points spent in it
                    if (m_pCharacter->APSpentInTree(treeName) > 0)
                    {
                        // no user confirmation for this as they have already changed
                        // the base requirement that included the tree. All
                        // APs spent in this tree have to be returned to the pool of
                        // those available.
                        m_pCharacter->Enhancement_ResetEnhancementTree(treeName);   // revokes any trained enhancements also
                    }
                    // now remove it from the selected list
                    selTrees.SetNotSelected(ti);
                }
            }
        }

        // now that we have the tree list, assign them to unused tree selections
        // if there are any left
        std::list<EnhancementTree>::iterator tit = trees.begin();
        while (tit != trees.end())
        {
            if (!selTrees.IsTreePresent((*tit).Name()))
            {
                // tree not present, see if it can be assigned
                for (size_t ti = 0; ti < MST_Number; ++ti)
                {
                    std::string treeName = selTrees.Tree(ti);
                    if (SelectedEnhancementTrees::IsNotSelected(treeName)
                            || ((ti == 0) && treeName == c_noSelection))    // special case for racial tree
                    {
                        // no assignment yet for this tree, assign this tree to it
                        selTrees.SetTree(ti, (*tit).Name());
                        break;  // done
                    }
                }
            }
            // try the next tree
            ++tit;
        }
        m_pCharacter->Enhancement_SetSelectedTrees(selTrees);
    }
    return trees;
}

void CEnhancementsView::CreateEnhancementWindows()
{
    LockWindowUpdate();
    // all enhancement windows are a set size and max of MST_Number available to the user
    CRect itemRect(
            c_controlSpacing,
            c_controlSpacing,
            c_sizeX + c_controlSpacing,
            c_sizeY + c_controlSpacing);
    // show the trees selected by the user
    const SelectedEnhancementTrees & selTrees = m_pCharacter->SelectedTrees(); // take a copy
    for (size_t i = 0; i < MST_Number; ++i)
    {
        std::string treeName = selTrees.Tree(i);
        if (!SelectedEnhancementTrees::IsNotSelected(treeName))
        {
            // we have a selected tree here
            if (i > 0)
            {
                // no combo box selector for racial tree
                PopulateTreeCombo(&m_comboTreeSelect[i-1], treeName);
            }
            // create the tree dialog
            // show an enhancement dialog
            CEnhancementTreeDialog * dlg = new CEnhancementTreeDialog(
                    this,
                    m_pCharacter,
                    GetEnhancementTree(treeName),
                    (i == 0) ? TT_racial : TT_enhancement); // first tree is always racial
            dlg->Create(CEnhancementTreeDialog::IDD, this);
            dlg->MoveWindow(&itemRect);
            dlg->ShowWindow(SW_SHOW);
            m_treeViews.push_back(dlg);
        }
        else
        {
            // show a blank tree and populate the selection combo
            PopulateTreeCombo(&m_comboTreeSelect[i-1], "");
            // create the blank tree dialog
            CEnhancementTreeDialog * dlg = new CEnhancementTreeDialog(
                    this,
                    m_pCharacter,
                    GetEnhancementTree(c_noSelection),
                    TT_enhancement);
            dlg->Create(CEnhancementTreeDialog::IDD, this);
            dlg->MoveWindow(&itemRect);
            dlg->ShowWindow(SW_SHOW);
            m_treeViews.push_back(dlg);
        }
        // TBD this is just so they all get shown for now
        m_visibleTrees.push_back(m_visibleTrees.size());
    }
    UnlockWindowUpdate();
    // reposition and show the windows (handled in OnSize)
    CRect rctWnd;
    GetClientRect(&rctWnd);
    OnSize(SIZE_RESTORED, rctWnd.Width(), rctWnd.Height());
}

void CEnhancementsView::DestroyEnhancementWindows()
{
    LockWindowUpdate();
    // the user has changed either a race or class and the available enhancement trees
    // has changed
    for (size_t i = 0; i < m_treeViews.size(); ++i)
    {
        m_treeViews[i]->DestroyWindow();
        delete m_treeViews[i];
        m_treeViews[i] = NULL;
    }
    m_treeViews.clear();
    m_visibleTrees.clear();
    // left and right scrolls buttons no longer needed, hide them
    m_buttonLeft.ShowWindow(SW_HIDE);
    m_buttonLeft.EnableWindow(FALSE);
    m_buttonRight.ShowWindow(SW_HIDE);
    m_buttonRight.EnableWindow(FALSE);
    UnlockWindowUpdate();
}

void CEnhancementsView::PopulateTreeCombo(
        CComboBox * combo,
        const std::string & selectedTree)
{
    combo->LockWindowUpdate();
    combo->ResetContent();
    // always add a "No selection" item
    combo->AddString(c_noSelection.c_str());
    // now add any trees which are not already selected
    int sel = 0;        // assume "No selection"
    const SelectedEnhancementTrees & selTrees = m_pCharacter->SelectedTrees();
    std::list<EnhancementTree>::iterator tit = m_availableTrees.begin();
    while (tit != m_availableTrees.end())
    {
        if (!selTrees.IsTreePresent((*tit).Name())
                || selectedTree == (*tit).Name())       // include the name of the selected tree in the combo
        {
            // tree not present, list it in the control
            int index = combo->AddString((*tit).Name().c_str());
            if ((*tit).Name() == selectedTree)
            {
                sel = index;        // this is the item that should be selected
            }
        }
        // Move to the next tree
        ++tit;
    }
    // select the active tree
    combo->SetCurSel(sel);
    // disable the control is any points are spent in this tree
    if (m_pCharacter->APSpentInTree(selectedTree) > 0)
    {
        // can't change trees if any points spent in the tree
        combo->EnableWindow(FALSE);
    }
    else
    {
        // if no points spent or "No selection" then enable the control
        combo->EnableWindow(TRUE);
    }
    // done
    combo->UnlockWindowUpdate();
}

// CharacterObserver overrides
void CEnhancementsView::UpdateAlignmentChanged(Character * charData, AlignmentType alignmen)
{
    // if alignment has changed, then classes may have changed also
    // we need to update our windows
    // (Same trees may still be available)
    std::list<EnhancementTree> trees = DetermineTrees();
    if (trees != m_availableTrees)
    {
        // yup, they have changed
        m_availableTrees = trees;
        DestroyEnhancementWindows();
        CreateEnhancementWindows();
    }
}

void CEnhancementsView::UpdateClassChanged(
        Character * charData,
        ClassType type,
        size_t level)
{
    // if a class has changed so whether we need to update our windows
    // (Same trees may still be available)
    std::list<EnhancementTree> trees = DetermineTrees();
    if (trees != m_availableTrees)
    {
        // yup, they have changed
        m_availableTrees = trees;
        DestroyEnhancementWindows();
        CreateEnhancementWindows();
    }
}

void CEnhancementsView::UpdateRaceChanged(
        Character * charData,
        RaceType race)
{
    // if the race has changed, we definitely need to update the available
    // enhancement trees
    m_availableTrees = DetermineTrees();
    DestroyEnhancementWindows();
    CreateEnhancementWindows();
}

void CEnhancementsView::OnButtonLeft()
{
    if (m_scrollOffset > 0)
    {
        --m_scrollOffset;
        // reposition and show the windows (handled in OnSize)
        CRect rctWnd;
        GetClientRect(&rctWnd);
        OnSize(SIZE_RESTORED, rctWnd.Width(), rctWnd.Height());
        Invalidate();
    }
}

void CEnhancementsView::OnButtonRight()
{
    ++m_scrollOffset;
    // reposition and show the windows (handled in OnSize)
    CRect rctWnd;
    GetClientRect(&rctWnd);
    OnSize(SIZE_RESTORED, rctWnd.Width(), rctWnd.Height());
    Invalidate();
}

void CEnhancementsView::UpdateEnhancementTrained(
        Character * charData,
        const std::string & enhancementName, bool isTier5)
{
    UpdateTrees(enhancementName);

    UpdateWindowTitle();
    EnableDisableComboboxes();
}

void CEnhancementsView::UpdateTrees(const std::string & enhancementName)
{
    // some enhancements cause tree selection updates (arcane archer)
    const EnhancementTreeItem * pTreeItem = FindEnhancement(enhancementName);
    // see if it has an effect that causes a tree update
    if (pTreeItem != NULL)
    {
        const std::list<Effect> & effects = pTreeItem->Effects();
        bool updateTrees = false;
        std::list<Effect>::const_iterator eit = effects.begin();
        while (eit != effects.end() && !updateTrees)
        {
            if ((*eit).Type() == Effect_EnhancementTree)
            {
                updateTrees = true;
            }
            ++eit;
        }
        if (updateTrees)
        {
            // only update if there is a change in actual trees
            std::list<EnhancementTree> trees = DetermineTrees();
            if (trees != m_availableTrees)
            {
                // yup, they have changed
                m_availableTrees = trees;
                DestroyEnhancementWindows();
                CreateEnhancementWindows();
            }
        }
    }
}

void CEnhancementsView::UpdateEnhancementRevoked(
        Character * charData,
        const std::string & enhancementName,
        bool isTier5)
{
    UpdateTrees(enhancementName);

    UpdateWindowTitle();
    EnableDisableComboboxes();
}

void CEnhancementsView::UpdateEnhancementTreeReset(Character * charData)
{
    UpdateWindowTitle();
    EnableDisableComboboxes();
}

void CEnhancementsView::UpdateActionPointsChanged(Character * charData)
{
    UpdateWindowTitle();
    EnableDisableComboboxes();
}

void CEnhancementsView::UpdateWindowTitle()
{
    if (m_pCharacter != NULL)
    {
        CString text;
        text.Format("Enhancements - %d points available to spend, Racial APs %d",
                m_pCharacter->AvailableActionPoints(),
                m_pCharacter->BonusActionPoints());
        GetParent()->SetWindowText(text);
    }
    else
    {
        GetParent()->SetWindowText("Enhancements");
    }
}

void CEnhancementsView::OnTreeSelect(UINT nID)
{
    // the user has selected a new enhancement tree from a drop list under one of
    // the enhancement view windows. Switch the selected tree to the one wanted
    size_t treeIndex = (nID - IDC_TREE_SELECT2) + 1;    // tree 0 is the racial tree and cant be changed (no control for it)
    ASSERT(treeIndex > 0 && treeIndex < MST_Number);
    int sel = m_comboTreeSelect[treeIndex-1].GetCurSel();
    if (sel != CB_ERR)
    {
        CString entry;
        m_comboTreeSelect[treeIndex-1].GetLBText(sel, entry);
        std::string treeName = (LPCSTR)entry;
        // select the tree wanted!
        SelectedEnhancementTrees selTrees = m_pCharacter->SelectedTrees(); // take a copy
        selTrees.SetTree(treeIndex, treeName);      // modify
        m_pCharacter->Enhancement_SetSelectedTrees(selTrees);   // update
        // update our state
        DestroyEnhancementWindows();
        CreateEnhancementWindows();
    }
}

void CEnhancementsView::EnableDisableComboboxes()
{
    if (m_pCharacter != NULL)
    {
        const SelectedEnhancementTrees & selTrees = m_pCharacter->SelectedTrees();
        for (size_t i = 1; i < MST_Number; ++i)
        {
            std::string treeName = selTrees.Tree(i);
            // can only select a different tree if no points spent in this one
            bool enable = (m_pCharacter->APSpentInTree(treeName) == 0);
            m_comboTreeSelect[i-1].EnableWindow(enable);
        }
    }
    else
    {
        // all combo boxes should be hidden and disabled
        for (size_t i = 1; i < MST_Number; ++i)
        {
            m_comboTreeSelect[i-1].EnableWindow(false);
            m_comboTreeSelect[i-1].ShowWindow(SW_HIDE);
        }
    }
}
