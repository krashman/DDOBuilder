// InfoTip.h
//
#pragma once
#include <vector>

class Character;
class EnhancementSelection;
class EnhancementTreeItem;
class Feat;
class Stance;

class CInfoTip :
        public CWnd
{
    public:
        CInfoTip();
        virtual ~CInfoTip();

        BOOL Create(CWnd * parent);

        void SetEnhancementTreeItem(
                const Character & charData,
                const EnhancementTreeItem * pItem,
                size_t spentInTree);
        void SetEnhancementSelectionItem(
                const Character & charData,
                const EnhancementSelection * pItem,
                size_t ranks);
        void SetFeatItem(
                const Character & charData,
                const Feat * pItem);
        void SetStanceItem(
                const Character & charData,
                const Stance * pItem);

        void SetOrigin(CPoint point);
        void Show();
        void Hide();

    protected:
        BOOL GetWindowSize(CDC * pDC, CSize * size);
        //{{AFX_MSG(CInfoTip)
        afx_msg void OnPaint();
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()

    private:
        void GenerateLineBreaks(CString * text);
        CPoint m_origin;
        CFont m_standardFont;
        CFont m_boldFont;
        CImage m_image;
        CString m_title;
        CString m_description;
        std::vector<CString> m_requirements;
        std::vector<bool> m_bRequirementMet;
        CString m_cost;
        CString m_ranks;
};