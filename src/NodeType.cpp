
#include <stdio.h>

#include "NodeType.h"

/* end of includes */

NODETYPE_IMPLEMENT(ANodeType);

#ifdef _DEBUG
#include "DataList.h"

static ADataList *pNodeTypeNameList = NULL;

uint_t ANodeType::AllocateNodeType(const char *type)
{
    static ADataList NameList;

    if (!pNodeTypeNameList) pNodeTypeNameList = &NameList;

    uint_t n = 0;
    if (pNodeTypeNameList) {
        n = pNodeTypeNameList->Count();

        pNodeTypeNameList->Add((uint32_t)type);
    }
    return n;
}

const char *ANodeType::GetNodeTypeName(uint_t type)
{
    return pNodeTypeNameList ? (const char *)(*pNodeTypeNameList)[type] : NULL;
}

#else

uint_t ANodeType::AllocateNodeType(const char *type)
{
    static uint_t NodeTypeCount = 0;
    UNUSED(type);
    return NodeTypeCount++;
}

const char *ANodeType::GetNodeTypeName(uint_t type)
{
    UNUSED(type);
    return "<unknown>";
}

#endif
