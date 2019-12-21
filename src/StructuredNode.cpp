
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "StructuredNode.h"

NODETYPE_IMPLEMENT(AStructuredNode);

AStructuredNode::AStructuredNode() : AKeyValuePair(),
                                     pAttributes(NULL),
                                     pChildren(NULL),
                                     Type(0)
{
}

AStructuredNode::~AStructuredNode()
{
    DELETE_LIST(pAttributes);
    DELETE_LIST(pChildren);
}

const AKeyValuePair *AStructuredNode::FindAttribute(const AString& Name, const AKeyValuePair *pAttribute) const
{
    if (pAttribute) pAttribute = pAttribute->Next();
    else            pAttribute = GetAttributes();
    while (pAttribute && (pAttribute->Key != Name)) pAttribute = pAttribute->Next();
    return pAttribute;
}

AString AStructuredNode::GetAttribute(const AString& Name, const AKeyValuePair *pAttribute) const
{
    AString res;
    if ((pAttribute = FindAttribute(Name, pAttribute)) != NULL) res = pAttribute->Value;
    return res;
}

const AStructuredNode *AStructuredNode::FindChild(const AString& name, const AStructuredNode *pChild) const
{
    if (pChild) pChild = pChild->Next();
    else        pChild = GetChildren();
    while (pChild && (pChild->Key != name)) pChild = pChild->Next();
    return pChild;
}

AString AStructuredNode::GetChildValue(const AString& Name, const AStructuredNode *pChild) const
{
    AString res;
    if ((pChild = FindChild(Name, pChild)) != NULL) res = pChild->Value;
    return res;
}

void AStructuredNode::ToString(AString& str, uint_t level) const
{
    const AKeyValuePair   *pAttribute = pAttributes;
    const AStructuredNode *pChild     = pChildren;
    const AString         header      = AString("  ").Copies(level);

    str.printf("%sNode '%s' value '%s'", header.str(), Key.str(), Value.str());
    if (pAttribute) {
        bool first = true;

        str.printf(" (");
        while (pAttribute) {
            if (!first) str.printf(", ");
            first = false;

            str.printf("%s='%s'", pAttribute->Key.str(), pAttribute->Value.str());

            pAttribute = pAttribute->Next();
        }

        str.printf(")");
    }
    str.printf("\n");

    while (pChild) {
        pChild->ToString(str, level + 1);

        pChild = pChild->Next();
    }
}
