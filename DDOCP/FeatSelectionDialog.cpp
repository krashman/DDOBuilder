// FeatSelectionDialog.cpp
//

#include "stdafx.h"
#include "FeatSelectionDialog.h"
#include "DDOCPDoc.h"
#include "GlobalSupportFunctions.h"
#include "MainFrm.h"

const UINT UWM_NEW_DOCUMENT = ::RegisterWindowMessage(_T("NewActiveDocument"));

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CFeatSelectionDialog, CDialog)
    //{{AFX_MSG_MAP(CFeatSelectionDialog)
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_BN_CLICKED(IDC_FEAT_BUTTON, OnFeatButtonLeftClick)
    ON_NOTIFY(NM_RCLICK, IDC_FEAT_BUTTON, OnFeatButtonRightClick)
    ON_WM_ERASEBKGND()
    ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
#pragma warning(pop)

CFeatSelectionDialog::CFeatSelectionDialog(CWnd* pParent, const Feat & feat) :
    CDialog(CFeatSelectionDialog::IDD, pParent),
    m_feat(feat),
    m_pCharacter(NULL),
    m_showingTip(false),
    m_tipCreated(false)
{
    //{{AFX_DATA_INIT(CFeatSelectionDialog)
    //}}AFX_DATA_INIT
}

void CFeatSelectionDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CFeatSelectionDialog)
    DDX_Control(pDX, IDC_FEAT_BUTTON, m_featButton);
    //}}AFX_DATA_MAP
}

BOOL CFeatSelectionDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_tooltip.Create(this);
    m_tipCreated = true;
    LoadFeatBitmap();
    SetupControls();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CFeatSelectionDialog::OnNewDocument(WPARAM wParam, LPARAM lParam)
{
    // wParam is the document pointer
    CDocument * pDoc = (CDocument*)(wParam);
    //m_pDocument = pDoc;
    // lParam is the character pointer
    Character * pCharacter = (Character *)(lParam);
    if (pCharacter != m_pCharacter)
    {
        m_pCharacter = pCharacter;
        SetupControls();
    }
    return 0L;
}

void CFeatSelectionDialog::SetupControls()
{
    if (m_pCharacter != NULL)
    {
        size_t trainedCount = m_pCharacter->GetSpecialFeatTrainedCount(m_feat.Name());
        bool canIncrease = (trainedCount < m_feat.MaxTimesAcquire());
        bool canDecrease = (trainedCount > 0);
        CString text;
        text.Format("%d", trainedCount);
        m_featButton.m_bTopImage = true;
        m_featButton.SetTransparent();
        m_featButton.SetWindowText(text);
        m_featButton.EnableWindow(true);
    }
    else
    {
        // no document selected
        m_featButton.EnableWindow(false);
        m_featButton.m_bTopImage = true;
        m_featButton.SetTransparent();
        m_featButton.SetWindowText("-");
    }
}

void CFeatSelectionDialog::LoadFeatBitmap()
{
    CImage image;
    HRESULT result = LoadImageFile(IT_feat, m_feat.Icon(), &image);

    if (result == S_OK)
    {
        m_featButton.SetBitmap(image.Detach());
    }
}

void CFeatSelectionDialog::OnFeatButtonLeftClick()
{
    if (m_pCharacter != NULL)
    {
        size_t trainedCount = m_pCharacter->GetSpecialFeatTrainedCount(m_feat.Name());
        bool canIncrease = (trainedCount < m_feat.MaxTimesAcquire());
        if (canIncrease)
        {
            m_pCharacter->TrainSpecialFeat(m_feat.Name());
            SetupControls();
        }
    }
    // always hide the tip on a button click as interferes with mouse leave events
    if (m_showingTip)
    {
        HideTip();
    }
}

void CFeatSelectionDialog::OnFeatButtonRightClick(
        NMHDR * pNotifyStruct,
        LRESULT * result)
{
    if (m_pCharacter != NULL)
    {
        size_t trainedCount = m_pCharacter->GetSpecialFeatTrainedCount(m_feat.Name());
        bool canDecrease = (trainedCount > 0);
        if (canDecrease)
        {
            m_pCharacter->RevokeSpecialFeat(m_feat.Name());
            SetupControls();
        }
    }
    // always hide the tip on a button click as interferes with mouse leave events
    if (m_showingTip)
    {
        HideTip();
    }
}

BOOL CFeatSelectionDialog::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        IDC_FEAT_BUTTON,
        0 // end marker
    };

    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}

void CFeatSelectionDialog::OnMouseMove(UINT nFlags, CPoint point)
{
    // determine whether the mouse is over the feat icon
    CWnd * pWnd = ChildWindowFromPoint(point);
    if (pWnd == &m_featButton)
    {
        if (!m_showingTip)
        {
            CRect itemRect;
            m_featButton.GetWindowRect(&itemRect);
            ScreenToClient(&itemRect);
            ShowTip(itemRect);
        }
    }
    else
    {
        if (m_showingTip)
        {
            HideTip();
        }
    }
    GetMainFrame()->SetStatusBarPromptText("Left click to train this feat, right click to revoke.");
}

LRESULT CFeatSelectionDialog::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
    // hide any tooltip when the mouse leave the area its being shown for
    HideTip();
    GetMainFrame()->SetStatusBarPromptText("Ready.");
    return 0;
}

void CFeatSelectionDialog::ShowTip(CRect itemRect)
{
    if (m_showingTip)
    {
        m_tooltip.Hide();
    }
    ClientToScreen(&itemRect);
    CPoint tipTopLeft(itemRect.left, itemRect.bottom + 2);
    SetTooltipText(tipTopLeft);
    m_showingTip = true;
    // track the mouse so we know when it leaves our window
    // (us when disabled, our button when enabled)
    TRACKMOUSEEVENT tme;
    tme.cbSize = sizeof(tme);
    tme.hwndTrack = (m_pCharacter != NULL) ? m_featButton.GetSafeHwnd() : m_hWnd;
    tme.dwFlags = TME_LEAVE;
    tme.dwHoverTime = 1;
    _TrackMouseEvent(&tme);
}

void CFeatSelectionDialog::HideTip()
{
    // tip not shown if not over an assay
    if (m_tipCreated && m_showingTip)
    {
        m_tooltip.Hide();
        m_showingTip = false;
    }
}

void CFeatSelectionDialog::SetTooltipText(
        CPoint tipTopLeft)
{
    m_tooltip.SetOrigin(tipTopLeft);
    m_tooltip.SetFeatItem(*m_pCharacter, &m_feat);
    m_tooltip.Show();
}
