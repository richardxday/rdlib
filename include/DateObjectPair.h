
#ifndef __DATE_OBJECT_PAIR__
#define __DATE_OBJECT_PAIR__

#include "ListNode.h"
#include "DateTime.h"

class ADateObjectPair : public AListNode {
public:
    ADateObjectPair(bool autodelete = true);
    ADateObjectPair(const ADateObjectPair& object, bool autodelete = true);
    virtual ~ADateObjectPair();

    LIST_FUNCTIONS(ADateObjectPair);

    LISTNODE_DUPLICATE(ADateObjectPair);

    ADateTime Date;
    AListNode *pObject;

    static sint_t CompareDate(const AListNode *pNode1, const AListNode *pNode2, void *pContext);

protected:
    bool bAutoDelete;

    NODETYPE_DEFINE(ADateObjectPair);
};

#endif
