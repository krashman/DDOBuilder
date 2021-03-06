// MainFrm.cpp
//
#include "stdafx.h"
#include "MainFrm.h"

#include "BreakdownsView.h"
#include "CustomDockablePane.h"
#include "DDOCP.h"
#include "EnhancementsView.h"
#include "EpicDestiniesView.h"
#include "EquipmentView.h"
#include "ItemEditorDialog.h"
#include "LevelUpView.h"
#include "ReaperEnhancementsView.h"
#include "SpecialFeatsView.h"
#include "SpellsView.h"
#include "StancesView.h"
#include "afxdatarecovery.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
    ON_WM_CREATE()
    ON_COMMAND(ID_WINDOW_MANAGER, &CMainFrame::OnWindowManager)
    ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
    ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
    ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook)
    ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnUpdateApplicationLook)
    ON_WM_CLOSE()
    ON_COMMAND(ID_EDIT_ITEMEDITOR, &CMainFrame::OnItemEditor)
END_MESSAGE_MAP()

static UINT indicators[] =
{
    ID_SEPARATOR,           // status line indicator
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction
CMainFrame::CMainFrame() :
    m_pDocument(NULL),
    m_pCharacter(NULL)
{
    // TODO: add member initialization code here
    theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_VS_2008);
}

CMainFrame::~CMainFrame()
{
    for (size_t x = 0; x < m_dockablePanes.size(); x++)
    {
        delete m_dockablePanes[x];
    }
    m_dockablePanes.clear();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    BOOL bNameValid;
    // set the visual manager and style based on persisted value
    OnApplicationLook(theApp.m_nAppLook);

    CMDITabInfo mdiTabParams;
    mdiTabParams.m_style = CMFCTabCtrl::STYLE_3D_ONENOTE; // other styles available...
    mdiTabParams.m_bActiveTabCloseButton = TRUE;      // set to FALSE to place close button at right of tab area
    mdiTabParams.m_bTabIcons = FALSE;    // set to TRUE to enable document icons on MDI tabs
    mdiTabParams.m_bAutoColor = TRUE;    // set to FALSE to disable auto-colouring of MDI tabs
    mdiTabParams.m_bDocumentMenu = TRUE; // enable the document menu at the right edge of the tab area
    EnableMDITabbedGroups(TRUE, mdiTabParams);

    if (!m_wndMenuBar.Create(this))
    {
        return -1;      // fail to create
    }

    m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

    // prevent the menu bar from taking the focus on activation
    CMFCPopupMenu::SetForceMenuFocus(FALSE);

    if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
        !m_wndToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME))
    {
        return -1;      // fail to create
    }

    m_menuToolbar.LoadToolBar(IDR_MENUICONS_TOOLBAR);

    CString strToolBarName;
    bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
    ASSERT(bNameValid);
    m_wndToolBar.SetWindowText(strToolBarName);

    CString strCustomize;
    bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
    ASSERT(bNameValid);
    m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);

    // Allow user-defined toolbars operations:
    InitUserToolbars(NULL, uiFirstUserToolBarId, uiLastUserToolBarId);

    if (!m_wndStatusBar.Create(this))
    {
        return -1;      // fail to create
    }
    m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

    // TODO: Delete these five lines if you don't want the toolbar and menu bar to be dockable
    m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
    m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockPane(&m_wndMenuBar);
    DockPane(&m_wndToolBar);

    // enable Visual Studio 2005 style docking window behaviour
    CDockingManager::SetDockingMode(DT_SMART);

    // enable Visual Studio 2005 style docking window auto-hide behaviour
    EnableAutoHidePanes(CBRS_ALIGN_ANY);

    // Enable enhanced windows management dialog
    EnableWindowsDialog(ID_WINDOW_MANAGER, ID_WINDOW_MANAGER, TRUE);

    // Enable toolbar and docking window menu replacement
    EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);

    // enable quick (Alt+drag) toolbar customization
    CMFCToolBar::EnableQuickCustomization();

    if (CMFCToolBar::GetUserImages() == NULL)
    {
        // load user-defined toolbar images
        if (m_UserImages.Load(_T(".\\UserImages.bmp")))
        {
            CMFCToolBar::SetUserImages(&m_UserImages);
        }
    }

    // enable menu personalization (most-recently used commands)
    // TODO: define your own basic commands, ensuring that each pull down menu has at least one basic command.
    CList<UINT, UINT> lstBasicCommands;

    lstBasicCommands.AddTail(ID_FILE_NEW);
    lstBasicCommands.AddTail(ID_FILE_OPEN);
    lstBasicCommands.AddTail(ID_FILE_SAVE);
    lstBasicCommands.AddTail(ID_FILE_PRINT);
    lstBasicCommands.AddTail(ID_APP_EXIT);
    lstBasicCommands.AddTail(ID_EDIT_CUT);
    lstBasicCommands.AddTail(ID_EDIT_PASTE);
    lstBasicCommands.AddTail(ID_EDIT_UNDO);
    lstBasicCommands.AddTail(ID_APP_ABOUT);
    lstBasicCommands.AddTail(ID_VIEW_STATUS_BAR);
    lstBasicCommands.AddTail(ID_VIEW_TOOLBAR);
    lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2003);
    lstBasicCommands.AddTail(ID_VIEW_APPLOOK_VS_2005);
    lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLUE);
    lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_SILVER);
    lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLACK);
    lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_AQUA);
    lstBasicCommands.AddTail(ID_VIEW_APPLOOK_WINDOWS_7);

    CMFCToolBar::SetBasicCommands(lstBasicCommands);

    // Switch the order of document name and application name on the window title bar. This
    // improves the usability of the task bar because the document name is visible with the thumbnail.
    ModifyStyle(0, FWS_PREFIXTITLE);

    return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if( !CMDIFrameWndEx::PreCreateWindow(cs) )
        return FALSE;
    // TODO: Modify the Window class or styles here by modifying
    //  the CREATESTRUCT cs

    return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers
void CMainFrame::OnWindowManager()
{
    ShowWindowsDialog();
}

void CMainFrame::OnViewCustomize()
{
    CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
    pDlgCust->EnableUserDefinedToolbars();
    pDlgCust->Create();
}

LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
    LRESULT lres = CMDIFrameWndEx::OnToolbarCreateNew(wp,lp);
    if (lres == 0)
    {
        return 0;
    }

    CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
    ASSERT_VALID(pUserToolbar);

    BOOL bNameValid;
    CString strCustomize;
    bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
    ASSERT(bNameValid);

    pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
    return lres;
}

void CMainFrame::OnApplicationLook(UINT id)
{
    CWaitCursor wait;

    theApp.m_nAppLook = id;

    switch (theApp.m_nAppLook)
    {
    case ID_VIEW_APPLOOK_WIN_2000:
        CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
        break;

    case ID_VIEW_APPLOOK_OFF_XP:
        CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
        break;

    case ID_VIEW_APPLOOK_WIN_XP:
        CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
        CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
        break;

    case ID_VIEW_APPLOOK_OFF_2003:
        CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
        CDockingManager::SetDockingMode(DT_SMART);
        break;

    case ID_VIEW_APPLOOK_VS_2005:
        CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
        CDockingManager::SetDockingMode(DT_SMART);
        break;

    case ID_VIEW_APPLOOK_VS_2008:
        CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
        CDockingManager::SetDockingMode(DT_SMART);
        break;

    case ID_VIEW_APPLOOK_WINDOWS_7:
        CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
        CDockingManager::SetDockingMode(DT_SMART);
        break;

    default:
        switch (theApp.m_nAppLook)
        {
        case ID_VIEW_APPLOOK_OFF_2007_BLUE:
            CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
            break;

        case ID_VIEW_APPLOOK_OFF_2007_BLACK:
            CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
            break;

        case ID_VIEW_APPLOOK_OFF_2007_SILVER:
            CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
            break;

        case ID_VIEW_APPLOOK_OFF_2007_AQUA:
            CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
            break;
        }

        CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
        CDockingManager::SetDockingMode(DT_SMART);
    }

    RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

    theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
    pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
    // base class does the real work
    if (!CMDIFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
    {
        return FALSE;
    }

    // enable customization button for all user toolbars
    BOOL bNameValid;
    CString strCustomize;
    bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
    ASSERT(bNameValid);

    for (int i = 0; i < iMaxUserToolbars; i ++)
    {
        CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
        if (pUserToolbar != NULL)
        {
            pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
        }
    }

    return TRUE;
}

CCustomDockablePane* CMainFrame::CreateDockablePane(
        const char* paneTitle, 
        CDocument* doc, 
        CRuntimeClass* runtimeClass,
        UINT viewID)
{
    CCreateContext createContext;
    createContext.m_pCurrentDoc = doc;
    createContext.m_pNewViewClass = runtimeClass;

    CCustomDockablePane* pane = new CCustomDockablePane;

    pane->Create(
            _T(paneTitle),
            this,
            CRect(0,0,400,400),
            TRUE,
            viewID, 
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI, 
            32UL,
            15UL,
            &createContext);
    pane->EnableDocking(CBRS_ALIGN_ANY);
    DockPane(pane);

    // Assorted functionality is applied to all panes
    m_dockablePanes.push_back(pane);

    return pane;
}

BOOL CMainFrame::OnCmdMsg(
        UINT nID,
        int nCode,
        void * pExtra,
        AFX_CMDHANDLERINFO * pHandlerInfo)
{
    BOOL bReturn = FALSE;

    // see if a child pane can handle it
    if (FALSE == bReturn)
    {
        // Offer command to child views by looking in each docking window which
        // holds a view until we get the 1st view that handles the message.
        for (size_t x = 0; bReturn == FALSE && x < m_dockablePanes.size(); x++)
        {
            CView * pView = m_dockablePanes[x]->GetView();
            bReturn = pView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
        }
    }

    if (FALSE == bReturn)
    {
        // also offer the message to any views for the active open document
        CDocument * pDoc = GetActiveDocument();
        if (pDoc != NULL)
        {
            POSITION pos = pDoc->GetFirstViewPosition();
            while (pos != NULL && bReturn == FALSE)
            {
                CView * pView = pDoc->GetNextView(pos);
                bReturn = pView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
            }
        }
    }

    // try to handle the message ourselves
    if (FALSE == bReturn)
    {
        bReturn = CMDIFrameWndEx::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
    }

    return bReturn;
}

void CMainFrame::CreateViews()
{
    int dockingWindowId = 1;
    // create the floating views
    CCustomDockablePane * pBreakdownsPane = CreateDockablePane(
            "Breakdowns",
            GetActiveDocument(),
            RUNTIME_CLASS(CBreakdownsView),
            dockingWindowId++);
    pBreakdownsPane->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    // create the floating views
    CCustomDockablePane * pLevelUpPane = CreateDockablePane(
            "Level Up",
            GetActiveDocument(),
            RUNTIME_CLASS(CLevelUpView),
            dockingWindowId++);
    pLevelUpPane->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    // create the floating views
    CCustomDockablePane * pSpecialFeatsPane = CreateDockablePane(
            "Special Feats",
            GetActiveDocument(),
            RUNTIME_CLASS(CSpecialFeatsView),
            dockingWindowId++);
    pSpecialFeatsPane->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    // create the floating views
    CCustomDockablePane * pEnhancementsPane = CreateDockablePane(
            "Enhancements",
            GetActiveDocument(),
            RUNTIME_CLASS(CEnhancementsView),
            dockingWindowId++);
    pEnhancementsPane->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    // create the floating views
    CCustomDockablePane * pEquipmentPane = CreateDockablePane(
            "Equipment",
            GetActiveDocument(),
            RUNTIME_CLASS(CEquipmentView),
            dockingWindowId++);
    pEquipmentPane->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    // create the floating views
    CCustomDockablePane * pSpellsPane = CreateDockablePane(
            "Spells",
            GetActiveDocument(),
            RUNTIME_CLASS(CSpellsView),
            dockingWindowId++);
    pSpellsPane->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    // create the floating views
    CCustomDockablePane * pReaperPane = CreateDockablePane(
            "Reaper Enhancements",
            GetActiveDocument(),
            RUNTIME_CLASS(CReaperEnhancementsView),
            dockingWindowId++);
    pReaperPane->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    // create the floating views
    CCustomDockablePane * pEpicDestiniesPane = CreateDockablePane(
            "Epic Destinies",
            GetActiveDocument(),
            RUNTIME_CLASS(CEpicDestiniesView),
            dockingWindowId++);
    pEpicDestiniesPane->SetDocumentAndCharacter(GetActiveDocument(), NULL);

    // create the floating views
    CCustomDockablePane * pStancesPane = CreateDockablePane(
            "Stances",
            GetActiveDocument(),
            RUNTIME_CLASS(CStancesView),
            dockingWindowId++);
    pStancesPane->SetDocumentAndCharacter(GetActiveDocument(), NULL);
}

/////////////////////////////////////////////////////////////////////////////
// ProgressBar operations

// Prepare progress bar
void CMainFrame::BeginProgress(CString fixedText, CString variableText)
{
    // find the rectangle of the pane where we want the status bar
    CSize size;
    CString text = fixedText + variableText;

    if (text.GetLength() == 0)
    {
        BeginProgress(0, fixedText, variableText);
    }
    else
    {
        // if we have text, the we need to move the position of the
        // progress control to give room for the text to be displayed
        CDC * pDC = NULL;

        pDC = GetDC();
        ASSERT(pDC) ;
        pDC->SaveDC();
        pDC->SelectObject(m_wndStatusBar.GetFont());
        size = pDC->GetTextExtent(text);
        VERIFY(pDC->RestoreDC(-1));
        VERIFY(ReleaseDC(pDC));

        BeginProgress(size.cx, fixedText, variableText);
    }
}

void CMainFrame::BeginProgress(int width, CString fixedText, CString variableText)
{
    // find the rectangle of the pane where we want the status bar
    CRect rc;
    CString text = fixedText + variableText;

    if (m_wndStatusBar.m_hWnd != NULL)
    {
        m_wndStatusBar.GetItemRect(0, &rc);
        rc.left += width + 5;
        
        m_wndStatusBar.SetPaneText(0, text);
        
        VERIFY(m_ctlProgress.Create(WS_CHILD | WS_VISIBLE | PBS_SMOOTH, rc, &m_wndStatusBar, 1));

        m_ctlProgress.SetRange(0, 100);
        m_ctlProgress.SetPos(0);
    }
}

// Set progress bar
void CMainFrame::SetProgress(int nProg, CString fixedText, CString variableText)
{
    CString text = fixedText + variableText;

    ASSERT(nProg <= 100);

    if (m_ctlProgress.m_hWnd != NULL)
    {
        m_ctlProgress.SetPos(nProg);

        if (text != "")
        {
            // if we have text, the we need to check whether it will fit
            // into the space provided for it - if it doesn't fit, we'll
            // need to cut the string into the right size
            CSize   size;
            CDC     *pDC = NULL;
            CRect   rctPane,
                    rctProgress,
                    rctParent;
            int     textMaxWidth    = 0;

            m_wndStatusBar.GetItemRect(0, &rctPane);
            m_ctlProgress.GetWindowRect(&rctProgress);
            m_ctlProgress.GetParent()->GetWindowRect(&rctParent);
            rctProgress -= CPoint(rctParent.left, rctParent.top);

            textMaxWidth = rctProgress.left - rctPane.left - 5;

            pDC = GetDC();
            ASSERT(pDC) ;
            pDC->SaveDC();
            pDC->SelectObject(m_wndStatusBar.GetFont());

            size = pDC->GetTextExtent(text);

            // is text too big?
            if (size.cx > textMaxWidth)
            {
                // yes, it is - so cut it down from the middle
                CString left    = "",
                        right   = "";
                int     middle  = 0;

                // split string into half
                middle = variableText.GetLength()/2;

                // left half, right half
                left = variableText.Left(middle);
                right = variableText.Right(variableText.GetLength()-middle);

                // still too big (yes, will be the first time round)
                while (size.cx > textMaxWidth)
                {
                    // remove one character from the left half
                    left = left.Left(left.GetLength()-1);
                    text = fixedText + left + "..." + right;

                    // get new size
                    size = pDC->GetTextExtent(text);

                    // does it fit now?
                    if (size.cx <= textMaxWidth)
                    {
                        // yes, it fits, so we can break
                        break;
                    }

                    // remove one character from the right half
                    right = right.Right(right.GetLength()-1);
                    text = fixedText + left + "..." + right;

                    // get new size
                    size = pDC->GetTextExtent(text);

                    // does it fit now?
                    if (size.cx <= textMaxWidth)
                    {
                        // yes, it fits, so we can break
                        break;
                    }
                }
            }
            
            VERIFY(pDC->RestoreDC(-1));
            VERIFY(ReleaseDC(pDC));

            m_wndStatusBar.SetPaneText(0, text);
        }
    }
}

// Clear up progress bar
void CMainFrame::EndProgress()
{
    if (m_ctlProgress.m_hWnd != NULL)
    {
        m_ctlProgress.DestroyWindow();
        m_wndStatusBar.SetPaneText(0, "");
    }
}

void CMainFrame::SetActiveDocumentAndCharacter(CDocument * pDoc, Character * pCharacter)
{
    if (m_pCharacter != pCharacter)
    {
        m_pDocument = pDoc;
        m_pCharacter = pCharacter;
        // update all the floating windows with the new active character
        for (size_t x = 0; x < m_dockablePanes.size(); x++)
        {
            m_dockablePanes[x]->SetDocumentAndCharacter(pDoc, pCharacter);
        }
        if (m_pCharacter != NULL)
        {
            pCharacter->NowActive();
        }
    }
}

// this is the exact same code from CFrameWnd::OnClose() except for a fix to stop
// access to a closed pDocument pointer which has since been deleted.
void CMainFrame::OnClose()
{
    if (m_lpfnCloseProc != NULL)
    {
        // if there is a close proc, then defer to it, and return
        // after calling it so the frame itself does not close.
        (*m_lpfnCloseProc)(this);
        return;
    }

    // Note: only queries the active document
    CDocument* pDocument = GetActiveDocument();
    if (pDocument != NULL && !pDocument->CanCloseFrame(this))
    {
        // document can't close right now -- don't close it
        return;
    }
    CWinApp* pApp = AfxGetApp();
    if (pApp != NULL && pApp->m_pMainWnd == this)
    {
        CDataRecoveryHandler *pHandler = pApp->GetDataRecoveryHandler();
        if ((pHandler != NULL) && (pHandler->GetShutdownByRestartManager()))
        {
            // If the application is being shut down by the Restart Manager, do
            // a final auto save.  This will mark all the documents as not dirty,
            // so the SaveAllModified call below won't prompt for save.
            pHandler->AutosaveAllDocumentInfo();
            pHandler->SaveOpenDocumentList();
        }

        // attempt to save all documents
        if (pDocument == NULL && !pApp->SaveAllModified())
            return;     // don't close it

        if ((pHandler != NULL) && (!pHandler->GetShutdownByRestartManager()))
        {
            // If the application is not being shut down by the Restart Manager,
            // delete any auto saved documents since everything is now fully saved.
            pHandler->DeleteAllAutosavedFiles();
        }

        // hide the application's windows before closing all the documents
        pApp->HideApplication();

        // close all documents first
        pApp->CloseAllDocuments(FALSE);
        pDocument = GetActiveDocument();        // bug fix, pDocument may be NULL now

        // don't exit if there are outstanding component objects
        if (!AfxOleCanExitApp())
        {
            // take user out of control of the app
            AfxOleSetUserCtrl(FALSE);

            // don't destroy the main window and close down just yet
            //  (there are outstanding component (OLE) objects)
            return;
        }

        // there are cases where destroying the documents may destroy the
        //  main window of the application.
        if (!afxContextIsDLL && pApp->m_pMainWnd == NULL)
        {
            AfxPostQuitMessage(0);
            return;
        }
    }

    // detect the case that this is the last frame on the document and
    // shut down with OnCloseDocument instead.
    if (pDocument != NULL && pDocument->m_bAutoDelete)
    {
        BOOL bOtherFrame = FALSE;
        POSITION pos = pDocument->GetFirstViewPosition();
        while (pos != NULL)
        {
            CView* pView = pDocument->GetNextView(pos);
            ENSURE_VALID(pView);
            if (pView->GetParentFrame() != this)
            {
                bOtherFrame = TRUE;
                break;
            }
        }
        if (!bOtherFrame)
        {
            pDocument->OnCloseDocument();
            return;
        }

        // allow the document to cleanup before the window is destroyed
        pDocument->PreCloseFrame(this);
    }
    // let the base class do its thing now
    CMDIFrameWndEx::OnClose();
}

void CMainFrame::SetStatusBarPromptText(const CString & text)
{
    m_wndStatusBar.SetPaneText(
            0, 
            text, 
            TRUE);
}

void CMainFrame::OnItemEditor()
{
    CItemEditorDialog dlg(NULL);
    dlg.DoModal();
}

