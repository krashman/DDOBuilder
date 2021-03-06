// EpicDestiniesView.h
//
#pragma once
#include "Resource.h"
#include <vector>
#include "Character.h"

class EnhancementTree;

class CEpicDestiniesView :
    public CFormView,
    public CharacterObserver
{
    public:
        enum { IDD = IDD_EPIC_DESTINIES_VIEW };
        DECLARE_DYNCREATE(CEpicDestiniesView)

#ifdef _DEBUG
        virtual void AssertValid() const;
    #ifndef _WIN32_WCE
    virtual void Dump(CDumpContext& dc) const;
#endif
#endif
        virtual void OnInitialUpdate();
    protected:
        CEpicDestiniesView();           // protected constructor used by dynamic creation
        virtual ~CEpicDestiniesView();

        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
        afx_msg void OnDestinySelect();
        afx_msg void OnButtonMakeActiveDestiny();
        afx_msg void OnTomeOfFateSelect();
        afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
        DECLARE_MESSAGE_MAP()

        // CharacterObserver overrides
        void UpdateEnhancementTrained(Character * charData, const std::string & enhancementName, bool isTier5) override;
        void UpdateEnhancementRevoked(Character * charData, const std::string & enhancementName, bool isTier5) override;
        void UpdateEnhancementTreeReset(Character * charData) override;
        void UpdateFatePointsChanged(Character * charData) override;
        void UpdateEpicCompletionistChanged(Character * charData) override;
    private:
        std::vector<EnhancementTree> DetermineTrees();
        void CreateEnhancementWindows();
        void DestroyEnhancementWindows();
        void PopulateCombobox();
        void EnableControls();
        void MoveFatePointControls();

        CDocument * m_pDocument;
        Character * m_pCharacter;
        std::vector<EnhancementTree> m_availableEpicDestinies;
        std::vector<CDialog *> m_treeViews;
        CComboBox m_comboEpicDestinySelect;
        CButton m_buttonMakeActive;
        CStatic m_labelTwistsOfFate;
        CStatic m_fatePointsSpent;
        CStatic m_labelTomeOfFate;
        CComboBox m_comboTomeOfFate;
        std::vector<CDialog *> m_twistsOfFate;
};
