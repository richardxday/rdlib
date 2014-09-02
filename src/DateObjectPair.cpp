
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DateObjectPair.h"

NODETYPE_IMPLEMENT(ADateObjectPair);

ADateObjectPair::ADateObjectPair(bool autodelete) : AListNode(),
													pObject(NULL),
													bAutoDelete(autodelete)
{
}

ADateObjectPair::ADateObjectPair(const ADateObjectPair& object, bool autodelete) : AListNode(),
																				   Date(object.Date),
																				   pObject(object.pObject ? object.pObject->Duplicate() : NULL),
																				   bAutoDelete(autodelete)
{
}

ADateObjectPair::~ADateObjectPair()
{
	if (pObject && bAutoDelete) delete pObject;
}

sint_t ADateObjectPair::CompareDate(const AListNode *pNode1, const AListNode *pNode2, void *pContext)
{
	const ADateObjectPair *pair1 = ADateObjectPair::Cast(pNode1);
	const ADateObjectPair *pair2 = ADateObjectPair::Cast(pNode2);
	const ADateTime *date1 = pair1 ? &pair1->Date : NULL;
	const ADateTime *date2 = pair2 ? &pair2->Date : NULL;
	sint_t cmp;

	if (!pair1 || !pair2) return -1;

	cmp = CompareDates(*date1, *date2);

	if (pContext && *(bool *)pContext) cmp = -cmp;

	return cmp;
}
