// InventoryDialog.cpp
//

#include "stdafx.h"
#include "InventoryDialog.h"
#include "DDOCPDoc.h"
#include "GlobalSupportFunctions.h"
#include "SelectionSelectDialog.h"
#include "MainFrm.h"

namespace
{
    const size_t c_controlSpacing = 3;
    // inventory window size
    const size_t c_sizeX = 223;
    const size_t c_sizeY = 290;
    COLORREF c_transparentColour = RGB(255, 128, 255);
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CInventoryDialog, CDialog)
    //{{AFX_MSG_MAP(CInventoryDialog)
    ON_WM_ERASEBKGND()
    ON_WM_PAINT()
    //}}AFX_MSG_MAP
    ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()
#pragma warning(pop)

CInventoryDialog::CInventoryDialog(CWnd* pParent) :
    CDialog(CInventoryDialog::IDD, pParent),
    m_pCharacter(NULL),
    m_bitmapSize(CSize(0, 0)),
    m_tipCreated(false),
    m_selectedItem(Inventory_Unknown),
    m_tooltipItem(Inventory_Unknown)
{
    //{{AFX_DATA_INIT(CInventoryDialog)
    //}}AFX_DATA_INIT
    // there are a fixed list of hit boxes which are hard coded here
    m_hitBoxes.push_back(InventoryHitBox(Inventory_Helmet, CRect(72, 24, 104, 56)));
    m_hitBoxes.push_back(InventoryHitBox(Inventory_Necklace, CRect(117, 24, 149, 56)));
    m_hitBoxes.push_back(InventoryHitBox(Inventory_Trinket, CRect(164, 35, 196, 67)));
    m_hitBoxes.push_back(InventoryHitBox(Inventory_Cloak, CRect(164, 78, 196, 110)));
    m_hitBoxes.push_back(InventoryHitBox(Inventory_Belt, CRect(164, 124, 196, 156)));
    m_hitBoxes.push_back(InventoryHitBox(Inventory_Ring2, CRect(164, 168, 196, 200)));
    m_hitBoxes.push_back(InventoryHitBox(Inventory_Gloves, CRect(117, 180, 149, 212)));
    m_hitBoxes.push_back(InventoryHitBox(Inventory_Boots, CRect(72, 180, 104, 212)));
    m_hitBoxes.push_back(InventoryHitBox(Inventory_Ring1, CRect(30, 169, 62, 200)));
    m_hitBoxes.push_back(InventoryHitBox(Inventory_Bracers, CRect(30, 124, 62, 156)));
    m_hitBoxes.push_back(InventoryHitBox(Inventory_Armor, CRect(30, 78, 62, 110)));
    m_hitBoxes.push_back(InventoryHitBox(Inventory_Goggles, CRect(30, 35, 62, 67)));
    m_hitBoxes.push_back(InventoryHitBox(Inventory_Weapon1, CRect(30, 237, 62, 269)));
    m_hitBoxes.push_back(InventoryHitBox(Inventory_Weapon2, CRect(72, 237, 104, 269)));
    m_hitBoxes.push_back(InventoryHitBox(Inventory_Quiver, CRect(117, 237, 149, 269)));
    m_hitBoxes.push_back(InventoryHitBox(Inventory_Arrows, CRect(164, 237, 196, 269)));

    // load images used
    LoadImageFile(IT_ui, "Inventory", &m_imageBackground);
    LoadImageFile(IT_ui, "Inventory", &m_imageBackgroundDisabled);
    MakeGrayScale(&m_imageBackgroundDisabled, c_transparentColour);
    LoadImageFile(IT_ui, "InventorySelectedSlot", &m_selectedImage);      // 42 * 42
}

void CInventoryDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CInventoryDialog)
    //}}AFX_DATA_MAP
}

BOOL CInventoryDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_tooltip.Create(this);
    m_tipCreated = true;

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CInventoryDialog::OnEraseBkgnd(CDC* pDC)
{
    return 0;
}

void CInventoryDialog::OnPaint()
{
    CPaintDC pdc(this); // validates the client area on destruction
    pdc.SaveDC();
    // ensure we have a background bitmap
    if (m_bitmapSize == CSize(0, 0))
    {
        m_cachedDisplay.DeleteObject(); // ensure
        // create the bitmap we will render to
        m_cachedDisplay.CreateCompatibleBitmap(
                &pdc,
                c_sizeX,
                c_sizeY);
        m_bitmapSize = CSize(c_sizeX, c_sizeY);
    }
    // draw to a compatible device context and then splat to screen
    CDC memoryDc;
    memoryDc.CreateCompatibleDC(&pdc);
    memoryDc.SaveDC();
    memoryDc.SelectObject(&m_cachedDisplay);
    memoryDc.SelectStockObject(DEFAULT_GUI_FONT);

    // first render the inventory background
    if (IsWindowEnabled())
    {
        m_imageBackground.BitBlt(
                memoryDc.GetSafeHdc(),
                CRect(0, 0, c_sizeX, c_sizeY),
                CPoint(0, 0),
                SRCCOPY);
    }
    else
    {
        m_imageBackgroundDisabled.BitBlt(
                memoryDc.GetSafeHdc(),
                CRect(0, 0, c_sizeX, c_sizeY),
                CPoint(0, 0),
                SRCCOPY);
        // no selection if we are disabled
        m_selectedItem = Inventory_Unknown;
    }

    // now iterate the current inventory and draw the item icons

    // now add the highlight to the selected item
    if (m_selectedItem != Inventory_Unknown)
    {
        // show the highlight
        CRect itemRect = GetItemRect(m_selectedItem);
        ASSERT(itemRect.Width() == 32);
        ASSERT(itemRect.Height() == 32);
        // expand the rectangle to the size of the image
        itemRect.InflateRect(8, 8, 8, 8);
        itemRect += CPoint(2, 2);       // adjust relative to view location
        m_selectedImage.TransparentBlt(
                memoryDc.GetSafeHdc(),
                itemRect.left,
                itemRect.top,
                42,
                42);
    }

    // now draw to display
    pdc.BitBlt(
            0,
            0,
            m_bitmapSize.cx,
            m_bitmapSize.cy,
            &memoryDc,
            0,
            0,
            SRCCOPY);
    memoryDc.RestoreDC(-1);
    memoryDc.DeleteDC();
    pdc.RestoreDC(-1);
}

void CInventoryDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
    CDialog::OnLButtonDown(nFlags, point);
    InventorySlotType slotClicked = FindByPoint(NULL);
    if (slotClicked != m_selectedItem)
    {
        m_selectedItem = slotClicked;
        Invalidate();       // re-draw
        if (slotClicked != Inventory_Unknown)
        {
            NotifySlotLeftClicked(slotClicked);
        }
    }
}

void CInventoryDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
    CDialog::OnRButtonDown(nFlags, point);
    InventorySlotType slotClicked = FindByPoint(NULL);
    if (slotClicked != m_selectedItem)
    {
        m_selectedItem = slotClicked;
        Invalidate();       // re-draw
        if (slotClicked != Inventory_Unknown)
        {
            NotifySlotRightClicked(slotClicked);
        }
    }
}

InventorySlotType CInventoryDialog::FindByPoint(CRect * pRect) const
{
    CPoint point;
    GetCursorPos(&point);
    ScreenToClient(&point);
    // see if we need to highlight the item under the cursor
    InventorySlotType slot = Inventory_Unknown;
    std::list<InventoryHitBox>::const_iterator it = m_hitBoxes.begin();
    while (slot == Inventory_Unknown && it != m_hitBoxes.end())
    {
        if ((*it).IsInRect(point))
        {
            // mouse is over this item
            slot = (*it).Slot();
            if (pRect != NULL)
            {
                *pRect = (*it).Rect();
            }
        }
        ++it;
    }
    return slot;
}

void CInventoryDialog::OnMouseMove(UINT nFlags, CPoint point)
{
    // determine which item the mouse may be over
    CRect itemRect;
    InventorySlotType slot = FindByPoint(&itemRect);
    if (slot != Inventory_Unknown
            && slot != m_tooltipItem)
    {
        // over a new item or a different item
        m_tooltipItem = slot;
        //ShowTip(*item, itemRect);
    }
    else
    {
        if (m_showingTip
                && slot != m_tooltipItem)
        {
            // no longer over the same item
            HideTip();
        }
    }
    // as the mouse is over the enhancement tree, ensure the status bar message prompts available actions
    //GetMainFrame()->SetStatusBarPromptText("Left click to train highlighted enhancement, right click to revoke last enhancement trained in this tree");
}

LRESULT CInventoryDialog::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
    // hide any tooltip when the mouse leaves the area its being shown for
    HideTip();
    GetMainFrame()->SetStatusBarPromptText("Ready.");
    return 0;
}

void CInventoryDialog::ShowTip(const EnhancementTreeItem & item, CRect itemRect)
{
//    if (m_showingTip)
//    {
//        m_tooltip.Hide();
//    }
//    ClientToScreen(&itemRect);
//    CPoint tipTopLeft(itemRect.left, itemRect.bottom + 2);
//    SetTooltipText(item, tipTopLeft);
//    m_showingTip = true;
//    // track the mouse so we know when it leaves our window
//    TRACKMOUSEEVENT tme;
//    tme.cbSize = sizeof(tme);
//    tme.hwndTrack = m_hWnd;
//    tme.dwFlags = TME_LEAVE;
//    tme.dwHoverTime = 1;
//    _TrackMouseEvent(&tme);
}

void CInventoryDialog::HideTip()
{
    // tip not shown if not over an assay
    //if (m_tipCreated && m_showingTip)
    //{
    //    m_tooltip.Hide();
    //    m_showingTip = false;
    //    m_tooltipItem = NULL;
    //}
}

void CInventoryDialog::SetTooltipText(
        const EnhancementTreeItem & item,
        CPoint tipTopLeft)
{
    //m_tooltip.SetOrigin(tipTopLeft);
    //m_tooltip.SetEnhancementTreeItem(*m_pCharacter, &item, m_pCharacter->APSpentInTree(m_tree.Name()));
    //m_tooltip.Show();
}

CRect CInventoryDialog::GetItemRect(InventorySlotType slot) const
{
    // iterate the list of hit boxes looking for the item that matches
    CRect itemRect(0, 0, 0, 0);
    bool found = false;
    std::list<InventoryHitBox>::const_iterator it = m_hitBoxes.begin();
    while (!found && it != m_hitBoxes.end())
    {
        if ((*it).Slot() == slot)
        {
            found = true;
            itemRect = (*it).Rect();
            break;
        }
        ++it;
    }
    ASSERT(found);
    return itemRect;
}

void CInventoryDialog::NotifySlotLeftClicked(InventorySlotType slot)
{
    NotifyAll(&InventoryObserver::UpdateSlotLeftClicked, this, slot);
}

void CInventoryDialog::NotifySlotRightClicked(InventorySlotType slot)
{
    NotifyAll(&InventoryObserver::UpdateSlotRightClicked, this, slot);
}

