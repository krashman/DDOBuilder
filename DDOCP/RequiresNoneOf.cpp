// RequiresNoneOf.cpp
//
#include "StdAfx.h"
#include "RequiresNoneOf.h"
#include "XmlLib\SaxWriter.h"
#include "Requirement.h"

#define DL_ELEMENT RequiresNoneOf

namespace
{
    const wchar_t f_saxElementName[] = L"RequiresNoneOf";
    DL_DEFINE_NAMES(RequiresNoneOf_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

RequiresNoneOf::RequiresNoneOf() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(RequiresNoneOf_PROPERTIES)
}

DL_DEFINE_ACCESS(RequiresNoneOf_PROPERTIES)

XmlLib::SaxContentElementInterface * RequiresNoneOf::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(RequiresNoneOf_PROPERTIES)

    return subHandler;
}

void RequiresNoneOf::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(RequiresNoneOf_PROPERTIES)
}

void RequiresNoneOf::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(RequiresNoneOf_PROPERTIES)
    writer->EndElement();
}

bool RequiresNoneOf::CanTrainFeat(
        const Character & charData, 
        const std::vector<size_t> & classLevels,
        size_t totalLevel,
        const std::list<TrainedFeat> & currentFeats) const
{
    // one or more of the requirements must not be met
    bool canTrain = false;
    std::list<Requirement>::const_iterator it = m_Requirements.begin();
    while (it != m_Requirements.end())
    {
        canTrain |= (*it).CanTrainFeat(
                charData,
                classLevels,
                totalLevel,
                currentFeats);
        ++it;
    }
    return !canTrain;       // can't have any requirement met
}

bool RequiresNoneOf::CanTrainEnhancement(
        const Character & charData,
        size_t trainedRanks) const
{
    bool canTrain = false;
    std::list<Requirement>::const_iterator it = m_Requirements.begin();
    while (it != m_Requirements.end())
    {
        canTrain |= (*it).CanTrainEnhancement(
                charData,
                trainedRanks);
        ++it;
    }
    return !canTrain;
}

bool RequiresNoneOf::CanTrainTree(
        const Character & charData) const
{
    // one or more of the requirements must not be met
    bool canTrain = false;
    std::list<Requirement>::const_iterator it = m_Requirements.begin();
    while (it != m_Requirements.end())
    {
        canTrain |= (*it).CanTrainTree(charData);
        ++it;
    }
    return !canTrain;       // can't have any requirement met
}

void RequiresNoneOf::CreateRequirementStrings(
        const Character & charData,
        std::vector<CString> * requirements,
        std::vector<bool> * met) const
{
    std::list<Requirement>::const_iterator it = m_Requirements.begin();
    std::vector<CString> localRequirements;
    std::vector<bool> localMet;
    while (it != m_Requirements.end())
    {
        (*it).CreateRequirementStrings(charData, &localRequirements, &localMet);
        ++it;
    }
    // re-package the local requirements to a single line
    if (localRequirements.size() > 0)
    {
        CString description = "Requires none of: ";
        bool wasMet = false;
        for (int i = 0; i < (int)localRequirements.size(); ++i)
        {
            localRequirements[i].Replace("Requires: ", "");    // take of leading text
            description += localRequirements[i];
            if (i <= (int)localRequirements.size() - 3)
            {
                description += ", ";
            }
            else if (i == (int)localRequirements.size() - 2)
            {
                description += " or ";
            }
            wasMet |= localMet[i];
        }
        requirements->push_back(description);
        met->push_back(!wasMet);        // these are not required things
    }
}

bool RequiresNoneOf::VerifyObject(
        std::stringstream * ss,
        const std::list<Feat> & allFeats) const
{
    bool ok = true;
    // check all the individual requirements
    std::list<Requirement>::const_iterator it = m_Requirements.begin();
    while (it != m_Requirements.end())
    {
        ok &= (*it).VerifyObject(ss, allFeats);
        ++it;
    }
    return ok;
}
