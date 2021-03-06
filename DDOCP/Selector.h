// Selector.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "EnhancementSelection.h"
#include "SelectorExclusions.h"

class EnhancementTree;
class Feat;

class Selector :
    public XmlLib::SaxContentElement
{
    public:
        Selector(void);
        void Write(XmlLib::SaxWriter * writer) const;
        std::string SelectedIcon(const std::string & selectionName) const;
        std::string Selector::DisplayName(const std::string & selection) const;
        std::list<Effect> Effects(const std::string & selection) const;

        bool VerifyObject(
                std::stringstream * ss,
                const std::list<EnhancementTree> & trees,
                const std::list<Feat> & feats) const;
    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Selector_PROPERTIES(_) \
                DL_OPTIONAL_OBJECT(_, SelectorExclusions, Exclusions) \
                DL_OBJECT_LIST(_, EnhancementSelection, Selections)

        DL_DECLARE_ACCESS(Selector_PROPERTIES)
        DL_DECLARE_VARIABLES(Selector_PROPERTIES)

        friend class CEnhancementEditorDialog;
};
