// DDOCPView.h
//
#pragma once

#include "resource.h"
#include "DDOCPDoc.h"
#include "Character.h"

class CActiveStanceDialog;

class CDDOCPView :
    public CFormView,
    public CharacterObserver
{
    protected: // create from serialization only
        CDDOCPView();
        virtual ~CDDOCPView();
#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif
        DECLARE_DYNCREATE(CDDOCPView)

    public:
        enum{ IDD = IDD_DDOCP_FORM };
        CDDOCPDoc* GetDocument() const;

    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        virtual void OnInitialUpdate(); // called first time after construct
        virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);

        afx_msg void OnFilePrintPreview();
        afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
        afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

        afx_msg void OnButtonStrPlus();
        afx_msg void OnButtonStrMinus();
        afx_msg void OnButtonDexPlus();
        afx_msg void OnButtonDexMinus();
        afx_msg void OnButtonConPlus();
        afx_msg void OnButtonConMinus();
        afx_msg void OnButtonIntPlus();
        afx_msg void OnButtonIntMinus();
        afx_msg void OnButtonWisPlus();
        afx_msg void OnButtonWisMinus();
        afx_msg void OnButtonChaPlus();
        afx_msg void OnButtonChaMinus();
        afx_msg void OnSelendokComboRace();
        afx_msg void OnSelendokComboAlignment();
        afx_msg void OnSelendokComboTomeStr();
        afx_msg void OnSelendokComboTomeDex();
        afx_msg void OnSelendokComboTomeCon();
        afx_msg void OnSelendokComboTomeInt();
        afx_msg void OnSelendokComboTomeWis();
        afx_msg void OnSelendokComboTomeCha();
        afx_msg void OnChangeName();
        afx_msg void OnSelendokComboAbilityLevel4();
        afx_msg void OnSelendokComboAbilityLevel8();
        afx_msg void OnSelendokComboAbilityLevel12();
        afx_msg void OnSelendokComboAbilityLevel16();
        afx_msg void OnSelendokComboAbilityLevel20();
        afx_msg void OnSelendokComboAbilityLevel24();
        afx_msg void OnSelendokComboAbilityLevel28();
        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
        DECLARE_MESSAGE_MAP()

        // Character overrides
        void UpdateAvailableBuildPointsChanged(Character * pCharacter) override;
        void UpdateClassChanged(Character * pCharacter, ClassType type, size_t level) override;

    private:
        void PopulateComboboxes();
        void RestoreControls();
        void EnableButtons();
        void DisplayAbilityValue(AbilityType ability, CEdit * control);
        void DisplaySpendCost(AbilityType ability, CEdit * control);
        void UpdateAvailableSpend();
        void UpdateBuildDescription();

        Character * m_pCharacter;
        CStatic m_staticBuildDescription;
        CEdit m_editName;
        CComboBox m_comboRace;
        CComboBox m_comboAlignment;
        CEdit m_editStr;
        CEdit m_editDex;
        CEdit m_editCon;
        CEdit m_editInt;
        CEdit m_editWis;
        CEdit m_editCha;
        CButton m_buttonStrPlus;
        CButton m_buttonStrMinus;
        CButton m_buttonDexPlus;
        CButton m_buttonDexMinus;
        CButton m_buttonConPlus;
        CButton m_buttonConMinus;
        CButton m_buttonIntPlus;
        CButton m_buttonIntMinus;
        CButton m_buttonWisPlus;
        CButton m_buttonWisMinus;
        CButton m_buttonChaPlus;
        CButton m_buttonChaMinus;
        CEdit m_editCostStr;
        CEdit m_editCostDex;
        CEdit m_editCostCon;
        CEdit m_editCostInt;
        CEdit m_editCostWis;
        CEdit m_editCostCha;
        CComboBox m_comboTomeStr;
        CComboBox m_comboTomeDex;
        CComboBox m_comboTomeCon;
        CComboBox m_comboTomeInt;
        CComboBox m_comboTomeWis;
        CComboBox m_comboTomeCha;
        CStatic m_staticAvailableSpend;
        CComboBox m_comboAILevel4;
        CComboBox m_comboAILevel8;
        CComboBox m_comboAILevel12;
        CComboBox m_comboAILevel16;
        CComboBox m_comboAILevel20;
        CComboBox m_comboAILevel24;
        CComboBox m_comboAILevel28;
};

#ifndef _DEBUG  // debug version in DDOCPView.cpp
inline CDDOCPDoc* CDDOCPView::GetDocument() const
   { return reinterpret_cast<CDDOCPDoc*>(m_pDocument); }
#endif

