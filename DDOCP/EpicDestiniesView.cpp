// EpicDestiniesView.cpp
//
#include "stdafx.h"
#include "EpicDestiniesView.h"

#include "DDOCP.h"
#include "DestinyTreeDialog.h"
#include "TwistOfFateDialog.h"
#include "GlobalSupportFunctions.h"

namespace
{
    const int c_controlSpacing = 3;
    const UINT UWM_NEW_DOCUMENT = ::RegisterWindowMessage(_T("NewActiveDocument"));
    // enhancement window size
    const size_t c_sizeX = 300;
    const size_t c_sizeY = 526;
    COLORREF f_fatePointsOverspendColour = RGB(0xE9, 0x96, 0x7A); // dark salmon
}

IMPLEMENT_DYNCREATE(CEpicDestiniesView, CFormView)

CEpicDestiniesView::CEpicDestiniesView() :
    CFormView(CEpicDestiniesView::IDD),
    m_pCharacter(NULL),
    m_pDocument(NULL)
{

}

CEpicDestiniesView::~CEpicDestiniesView()
{
    for (size_t vi = 0; vi < m_treeViews.size(); ++vi)
    {
        delete m_treeViews[vi];
    }
    for (size_t vi = 0; vi < m_twistsOfFate.size(); ++vi)
    {
        delete m_twistsOfFate[vi];
    }
}

void CEpicDestiniesView::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_DESTINY_SELECT, m_comboEpicDestinySelect);
    DDX_Control(pDX, IDC_CHECK_MAKE_ACTIVE_DESTINY, m_buttonMakeActive);
    DDX_Control(pDX, IDC_TWISTS_OF_FATE_LABEL, m_labelTwistsOfFate);
    DDX_Control(pDX, IDC_FATE_POINTS, m_fatePointsSpent);
    DDX_Control(pDX, IDC_STATIC_TOME_OF_FATE, m_labelTomeOfFate);
    DDX_Control(pDX, IDC_COMBO_TOME_OF_FATE, m_comboTomeOfFate);
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CEpicDestiniesView, CFormView)
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_CBN_SELENDOK(IDC_COMBO_DESTINY_SELECT, OnDestinySelect)
    ON_BN_CLICKED(IDC_CHECK_MAKE_ACTIVE_DESTINY, OnButtonMakeActiveDestiny)
    ON_CBN_SELENDOK(IDC_COMBO_TOME_OF_FATE, OnTomeOfFateSelect)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()
#pragma warning(pop)

#ifdef _DEBUG
void CEpicDestiniesView::AssertValid() const
{
    CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CEpicDestiniesView::Dump(CDumpContext& dc) const
{
    CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

void CEpicDestiniesView::OnInitialUpdate()
{
    CFormView::OnInitialUpdate();
    // create the twists of fate sub views
    CRect rctTwistsLabel;
    m_labelTwistsOfFate.GetWindowRect(&rctTwistsLabel);
    ScreenToClient(&rctTwistsLabel);
    CRect itemRect(
            rctTwistsLabel.left,
            rctTwistsLabel.bottom + c_controlSpacing,
            rctTwistsLabel.left + 120,
            rctTwistsLabel.bottom + 70);
    for (size_t twist = 0; twist < MAX_TWISTS; ++twist)
    {
        // show a feat selection dialog
        CTwistOfFateDialog * dlg = new CTwistOfFateDialog(this, twist);
        dlg->Create(CTwistOfFateDialog::IDD, this);
        dlg->MoveWindow(&itemRect);
        dlg->ShowWindow(SW_HIDE);   // start hidden
        m_twistsOfFate.push_back(dlg);
        // move rectangle down for next twist of fate
        itemRect += CPoint(0, itemRect.Height() + c_controlSpacing);
    }
    MoveFatePointControls();
    PopulateCombobox();
}

void CEpicDestiniesView::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    if (m_treeViews.size() > 0
            && IsWindow(m_treeViews[0]->GetSafeHwnd()))
    {
        // we only show the selected destiny tree
        int requiredWidth = c_sizeX         // window
                + (c_controlSpacing * 2);   // spacers

        // location of destiny tree just below the check box
        CRect rctButton;
        m_buttonMakeActive.GetWindowRect(rctButton);
        ScreenToClient(rctButton);
        CRect itemRect(
                c_controlSpacing,
                rctButton.bottom + c_controlSpacing,
                c_sizeX + c_controlSpacing,
                rctButton.bottom + c_sizeY + c_controlSpacing);

        for (size_t ti = 0; ti < m_treeViews.size(); ++ti)
        {
            // move all the destiny windows to the same location
            // only the selected tree is visible
            m_treeViews[ti]->MoveWindow(itemRect, false);
        }
        // fate point controls move depending on number of available twists
        MoveFatePointControls();
    }
}

void CEpicDestiniesView::MoveFatePointControls()
{
    // by default there are 4 twists of fate, unless the character has
    // the epic completionist. Then they have 5
    size_t twistIndex = 3;      // 0 based
    if (m_pCharacter != NULL)
    {
        if (m_pCharacter->HasEpicCompletionist())
        {
            twistIndex = 4;
        }
    }
    // position the other controls under the correct twist of fate
    CRect rctTwist;
    m_twistsOfFate[twistIndex]->GetWindowRect(rctTwist);
    ScreenToClient(rctTwist);
    // controls to be moved are: (all maintain current dimensions
    // [CStatic m_fatePointsSpent           ]
    // [CStatic label tome] [tome drop combo]
    CRect rctFatePointsSpent;
    CRect rctTomeLabel;
    CRect rctTomeCombo;
    m_fatePointsSpent.GetWindowRect(rctFatePointsSpent);
    m_labelTomeOfFate.GetWindowRect(rctTomeLabel);
    m_comboTomeOfFate.GetWindowRect(rctTomeCombo);
    rctFatePointsSpent -= rctFatePointsSpent.TopLeft();
    rctFatePointsSpent += CPoint(rctTwist.left, rctTwist.bottom + c_controlSpacing);
    rctTomeLabel -= rctTomeLabel.TopLeft();
    rctTomeLabel += CPoint(rctTwist.left, rctFatePointsSpent.bottom + c_controlSpacing);
    rctTomeCombo -= rctTomeCombo.TopLeft();
    rctTomeCombo += CPoint(rctTomeLabel.right + c_controlSpacing, rctFatePointsSpent.bottom + c_controlSpacing);
    // move move all the windows
    m_fatePointsSpent.MoveWindow(rctFatePointsSpent);
    m_labelTomeOfFate.MoveWindow(rctTomeLabel);
    m_comboTomeOfFate.MoveWindow(rctTomeCombo);
}

LRESULT CEpicDestiniesView::OnNewDocument(WPARAM wParam, LPARAM lParam)
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
        m_availableEpicDestinies = DetermineTrees();
        DestroyEnhancementWindows();
        CreateEnhancementWindows();
        PopulateCombobox();
    }
    else
    {
        DestroyEnhancementWindows();
    }
    // keep sub windows up to date
    for (size_t vi = 0; vi < m_twistsOfFate.size(); ++vi)
    {
        m_twistsOfFate[vi]->SendMessage(UWM_NEW_DOCUMENT, (WPARAM)m_pDocument, (LPARAM)pCharacter);
    }
    EnableControls();
    return 0L;
}

BOOL CEpicDestiniesView::OnEraseBkgnd(CDC* pDC)
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

std::vector<EnhancementTree> CEpicDestiniesView::DetermineTrees()
{
    std::vector<EnhancementTree> trees;
    // just find all the available epic destiny trees
    if (m_pCharacter != NULL)
    {
        const std::list<EnhancementTree> & allTrees = EnhancementTrees();
        std::list<EnhancementTree>::const_iterator it = allTrees.begin();
        while (it != allTrees.end())
        {
            if ((*it).HasIsEpicDestiny())
            {
                // yes this is one of our tree's add it
                trees.push_back((*it));
            }
            ++it;
        }
    }
    return trees;
}

void CEpicDestiniesView::CreateEnhancementWindows()
{
    LockWindowUpdate();
    // all epic destiny windows are a set size
    // and are at the same location (only selected visible)
    CRect itemRect(
            c_controlSpacing,
            c_controlSpacing,
            c_sizeX + c_controlSpacing,
            c_sizeY + c_controlSpacing);
    // show the trees selected by the user
    for (size_t i = 0; i < m_availableEpicDestinies.size(); ++i)
    {
        std::string treeName = m_availableEpicDestinies[i].Name();
        // create the tree dialog
        // show an enhancement dialog
        CDestinyTreeDialog * dlg = new CDestinyTreeDialog(
                this,
                m_pCharacter,
                GetEnhancementTree(treeName),
                TT_epicDestiny);
        dlg->Create(CDestinyTreeDialog::IDD, this);
        dlg->MoveWindow(&itemRect);
        dlg->ShowWindow(SW_HIDE);       // created hidden, only one visible at a time
        m_treeViews.push_back(dlg);
    }
    UnlockWindowUpdate();
    // reposition and show the windows (handled in OnSize)
    CRect rctWnd;
    GetClientRect(&rctWnd);
    OnSize(SIZE_RESTORED, rctWnd.Width(), rctWnd.Height());
}

void CEpicDestiniesView::DestroyEnhancementWindows()
{
    LockWindowUpdate();
    for (size_t i = 0; i < m_treeViews.size(); ++i)
    {
        m_treeViews[i]->DestroyWindow();
        delete m_treeViews[i];
        m_treeViews[i] = NULL;
    }
    m_treeViews.clear();
    UnlockWindowUpdate();
}

void CEpicDestiniesView::PopulateCombobox()
{
    m_comboEpicDestinySelect.LockWindowUpdate();
    m_comboEpicDestinySelect.ResetContent();        // ensure empty
    // add each of the available trees in order, item data is the index
    // into the destiny vector
    for (size_t i = 0; i < m_availableEpicDestinies.size(); ++i)
    {
        CString treeName = m_availableEpicDestinies[i].Name().c_str();
        int index = m_comboEpicDestinySelect.AddString(treeName);
        m_comboEpicDestinySelect.SetItemData(index, i);
    }
    // default to the active destiny if there is one
    std::string activeDestiny;
    if (m_pCharacter != NULL)
    {
        activeDestiny = m_pCharacter->ActiveEpicDestiny();
    }
    size_t activeIndex = 0;
    if (activeDestiny != "")
    {
        // there is a selection, find its index
        for (size_t i = 0; i < m_availableEpicDestinies.size(); i++)
        {
            int index = m_comboEpicDestinySelect.GetItemData(i);
            ASSERT(index >= 0 && index < (int)m_availableEpicDestinies.size());
            if (m_availableEpicDestinies[index].Name() == activeDestiny)
            {
                // this is the one we want to select
                activeIndex = i;
                break;
            }
        }
    }
    m_comboEpicDestinySelect.SetCurSel(activeIndex);  // select the first tree by default
    for (size_t i = 0; i < m_treeViews.size(); ++i)
    {
        // only show the selected tree
        m_treeViews[i]->ShowWindow(i == activeIndex ? SW_SHOW : SW_HIDE);
    }
    m_comboEpicDestinySelect.UnlockWindowUpdate();

    // tome of fate combo box
    m_comboTomeOfFate.LockWindowUpdate();
    // currently max tome of fate is +3
    m_comboTomeOfFate.ResetContent();
    m_comboTomeOfFate.AddString("None");
    m_comboTomeOfFate.AddString("+1");
    m_comboTomeOfFate.AddString("+2");
    m_comboTomeOfFate.AddString("+3");
    // now select the correct entry for current character
    size_t index = 0;
    if (m_pCharacter != NULL)
    {
        index = m_pCharacter->TomeOfFate();
    }
    m_comboTomeOfFate.SetCurSel(index);
    m_comboTomeOfFate.UnlockWindowUpdate();
}

void CEpicDestiniesView::EnableControls()
{
    if (m_pCharacter != NULL)
    {
        m_comboEpicDestinySelect.EnableWindow(TRUE);
        m_buttonMakeActive.EnableWindow(TRUE);
        m_comboTomeOfFate.EnableWindow(TRUE);
        std::string destinyName;
        int sel = m_comboEpicDestinySelect.GetCurSel();
        if (sel != CB_ERR)
        {
            sel = m_comboEpicDestinySelect.GetItemData(sel);
            destinyName = m_availableEpicDestinies[sel].Name();
        }
        m_buttonMakeActive.SetCheck((m_pCharacter->ActiveEpicDestiny() == destinyName)
                ? BST_CHECKED
                : BST_UNCHECKED);
        // show available twists of fate
        for (size_t vi = 0; vi < m_twistsOfFate.size(); ++vi)
        {
            if (vi == MAX_TWISTS - 1)
            {
                // must have epic completionist to see this twist

                m_twistsOfFate[vi]->ShowWindow(
                        m_pCharacter->HasEpicCompletionist()
                                ? SW_SHOW
                                : SW_HIDE);
            }
            else
            {
                m_twistsOfFate[vi]->ShowWindow(SW_SHOW);
            }
            m_twistsOfFate[vi]->EnableWindow(m_pCharacter->IsTwistActive(vi));
        }
        // show how many fate points are available
        int totalFatePoints = m_pCharacter->FatePoints();
        size_t spentFatePoints = m_pCharacter->SpentFatePoints();
        CString text;
        text.Format("Fate Points: %d of %d",
                (totalFatePoints - (int)spentFatePoints),
                totalFatePoints);
        m_fatePointsSpent.SetWindowText(text);
    }
    else
    {
        // UI controls disabled if no document
        m_comboEpicDestinySelect.EnableWindow(FALSE);
        m_buttonMakeActive.EnableWindow(FALSE);
        for (size_t vi = 0; vi < m_twistsOfFate.size(); ++vi)
        {
            m_twistsOfFate[vi]->ShowWindow(SW_HIDE);
        }
        m_comboTomeOfFate.EnableWindow(FALSE);
    }
}

void CEpicDestiniesView::OnDestinySelect()
{
    int sel = m_comboEpicDestinySelect.GetCurSel();
    if (sel != CB_ERR)
    {
        sel = m_comboEpicDestinySelect.GetItemData(sel);
        ASSERT(sel < (int)m_availableEpicDestinies.size());
        for (size_t i = 0; i < m_treeViews.size(); ++i)
        {
            // only show the selected tree
            m_treeViews[i]->ShowWindow(i == sel ? SW_SHOW : SW_HIDE);
        }
        EnableControls();
    }
}

void CEpicDestiniesView::OnButtonMakeActiveDestiny()
{
    bool checked = (m_buttonMakeActive.GetCheck() == BST_CHECKED);
    if (checked)
    {
        // make the displayed destiny the active one
        int sel = m_comboEpicDestinySelect.GetCurSel();
        if (sel != CB_ERR)
        {
            sel = m_comboEpicDestinySelect.GetItemData(sel);
            ASSERT(sel < (int)m_availableEpicDestinies.size());
            m_pCharacter->EpicDestiny_SetActiveDestiny(m_availableEpicDestinies[sel].Name());
        }
    }
    else
    {
        // clear the active destiny
        m_pCharacter->EpicDestiny_SetActiveDestiny("");
    }
    EnableControls();
}

// CharacterObserver overrides
void CEpicDestiniesView::UpdateEnhancementTrained(
        Character * charData,
        const std::string & enhancementName,
        bool isTier5)
{
        EnableControls();
}

void CEpicDestiniesView::UpdateEnhancementRevoked(
        Character * charData,
        const std::string & enhancementName,
        bool isTier5)
{
        EnableControls();
}

void CEpicDestiniesView::UpdateEnhancementTreeReset(
        Character * charData)
{
    EnableControls();
}

void CEpicDestiniesView::UpdateFatePointsChanged(Character * charData)
{
    EnableControls();
}

void CEpicDestiniesView::UpdateEpicCompletionistChanged(Character * charData)
{
    EnableControls();
    MoveFatePointControls();
}

void CEpicDestiniesView::OnTomeOfFateSelect()
{
    // set the new tome of fate value on the current character
    if (m_pCharacter != NULL)
    {
        int sel = m_comboTomeOfFate.GetCurSel();
        m_pCharacter->SetTomeOfFate((size_t)sel);
    }
}

HBRUSH CEpicDestiniesView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
    HBRUSH hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);
    // colour the control based on whether the user has over spent
    // the number of fate points available. This can happen
    // if the number of available fate points changes.
    bool setWarning = false;
    if (m_pCharacter != NULL)
    {
        if (pWnd == &m_fatePointsSpent)
        {
            int totalFatePoints = m_pCharacter->FatePoints();
            size_t spentFatePoints = m_pCharacter->SpentFatePoints();
            setWarning = ((int)spentFatePoints > totalFatePoints);
        }
    }
    if (setWarning)
    {
        pDC->SetTextColor(f_fatePointsOverspendColour);
    }

    return hbr;
}
