// EnhancementTreeDialog.h
//

#pragma once

#include "Resource.h"
#include "EnhancementTree.h"
#include "InfoTip.h"
#include "Character.h"

class EnhancementHitBox
{
    public:
        EnhancementHitBox(const EnhancementTreeItem & item, const CRect & rect) :
                m_item(item), m_rect(rect)
        {
        };
        ~EnhancementHitBox() {};

        bool IsInRect(CPoint point) const
        {
            return (m_rect.PtInRect(point) != 0);
        };
        const EnhancementTreeItem & Item() const
        {
            return m_item;
        };
        CRect Rect() const
        {
            return m_rect;
        };
    private:
        const EnhancementTreeItem & m_item;
        CRect m_rect;
};

// this dialog handles buying and revoking enhancements from enhancement and reaper trees
class CEnhancementTreeDialog :
    public CDialog,
    public CharacterObserver
{
    public:
        CEnhancementTreeDialog(
                CWnd* pParent,
                Character * pCharacter,
                const EnhancementTree & tree,
                TreeType type);

    ///////////////////////////////////////////////////////////////////
    // MFC

    public:
        //{{AFX_DATA(CEnhancementTreeDialog)
        enum { IDD = IDD_ENHANCEMENT_SELECTION };
        //}}AFX_DATA

    protected:
        //{{AFX_VIRTUAL(CEnhancementTreeDialog)
        virtual void DoDataExchange(CDataExchange* pDX);
        virtual BOOL OnInitDialog();
        //}}AFX_VIRTUAL

        //{{AFX_MSG(CEnhancementTreeDialog)
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg void OnPaint();
        afx_msg void OnMouseMove(UINT nFlags, CPoint point);
        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
        afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
        afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()

    private:
        // CharacterObserver overrides
        void UpdateFeatTrained(Character * charData, const std::string & featName);
        void UpdateFeatRevoked(Character * charData, const std::string & featName);
        void UpdateEnhancementTrained(Character * charData, const std::string & enhancementName, bool isTier5) override;
        void UpdateEnhancementRevoked(Character * charData, const std::string & enhancementName, bool isTier5) override;
        void UpdateEnhancementTreeReset(Character * charData) override;
        void UpdateActionPointsChanged(Character * charData) override;

        static void InitialiseStaticImages();
        void RenderTreeItem(const EnhancementTreeItem & item, CDC * pDC);
        void RenderItemCore(const EnhancementTreeItem & item, CDC * pDC);
        void RenderItemClickie(const EnhancementTreeItem & item, CDC * pDC, CRect itemRect);
        void RenderItemPassive(const EnhancementTreeItem & item, CDC * pDC, CRect itemRect);
        void RenderItemState(const EnhancementTreeItem & item, CDC * pDC, CRect itemRect);
        void RenderItemSelection(const EnhancementTreeItem & item, CDC * pDC, const CRect & itemRect);
        const EnhancementTreeItem * FindByPoint(CRect * pRect = NULL) const;
        void ShowTip(const EnhancementTreeItem & item, CRect itemRect);
        void HideTip();
        void SetTooltipText(const EnhancementTreeItem & item, CPoint tipTopLeft);
        EnhancementTree m_tree;
        TreeType m_type;
        Character * m_pCharacter;
        CSize m_bitmapSize;
        CBitmap m_cachedDisplay;
        CImage m_imageBackground;
        std::list<EnhancementHitBox> m_hitBoxes;
        bool m_bCreateHitBoxes;
        CInfoTip m_tooltip;
        bool m_showingTip;
        bool m_tipCreated;
        const EnhancementTreeItem * m_pTooltipItem;
};

//{{AFX_INSERT_LOCATION}}
