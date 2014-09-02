
#include <stdio.h>

#include "ListNode.h"

/* end of includes */

NODETYPE_IMPLEMENT(AListNode);

AListNode::AListNode() : ANodeType(),
						 pPrev(NULL),
						 pNext(NULL)
{
}

AListNode::~AListNode()
{
	DetachEx();
}

AListNode *AListNode::DeleteAll()
{
	AListNode *pNode = First();
	while (pNode) {
		AListNode *pNode1 = pNode->pNext;

		if (pNode != this) delete pNode;

		pNode = pNode1;
	}

	DetachEx();

	return this;
}

AListNode *AListNode::First() const
{
	const AListNode *pNode = this;
	while (pNode && pNode->pPrev) pNode = pNode->pPrev;
	
	return (AListNode *)pNode;
}

AListNode *AListNode::Last() const
{
	const AListNode *pNode = this;
	while (pNode && pNode->pNext) pNode = pNode->pNext;
	
	return (AListNode *)pNode;
}

AListNode *AListNode::Next(int n) const
{
	const AListNode *pNode = this;
	while ((n < 0) && pNode) {
		pNode = pNode->pPrev;
		n++;
	}
	while ((n > 0) && pNode) {
		pNode = pNode->pNext;
		n--;
	}
	
	return (n == 0) ? (AListNode *)pNode : NULL;
}

AListNode *AListNode::Prev(int n) const
{
	const AListNode *pNode = this;
	while ((n < 0) && pNode) {
		pNode = pNode->pNext;
		n++;
	}
	while ((n > 0) && pNode) {
		pNode = pNode->pPrev;
		n--;
	}

	return (AListNode *)pNode;
}

AListNode *AListNode::InsertBefore(AListNode *pNode)
{
	DetachEx();

	if (pNode) Insert(pNode->pPrev, pNode);

	return First();
}

AListNode *AListNode::InsertAfter(AListNode *pNode)
{
	DetachEx();

	if (pNode) Insert(pNode, pNode->pNext);

	return First();
}

AListNode *AListNode::Attach(AListNode *pNode)
{
	DetachEx();

	AListNode *pFirst = pNode ? pNode->First() : this;
	if (pNode) pFirst = AttachEx(pFirst, pNode);

	return pFirst;
}

AListNode *AListNode::Attach(AListNode *pNode, COMPARE_FUNC fn, void *pContext)
{
	DetachEx();

	AListNode *pFirst = pNode ? pNode->First() : this;
	if (pNode) pFirst = AttachEx(pFirst, pNode, fn, pContext);

	return pFirst;
}

AListNode *AListNode::AttachEx(AListNode *pFirst, AListNode *pNode)
{
	AListNode *pNode1, *pNode2;

	pNext = pPrev = NULL;

	pNode1 = pNode;
	pNode2 = pNode ? pNode->pNext : NULL;
	while (pNode1 && (Compare(this, pNode1) <  0)) {
		pNode2 = pNode1;
		pNode1 = pNode1->pPrev;
	}
	while (pNode2 && (Compare(this, pNode2) >= 0)) {
		pNode1 = pNode2;
		pNode2 = pNode2->pNext;
	}

	Insert(pNode1, pNode2);

	return pNode1 ? pFirst : this;
}

AListNode *AListNode::AttachEx(AListNode *pFirst, AListNode *pNode, COMPARE_FUNC fn, void *pContext)
{
	AListNode *pNode1, *pNode2;

	pNext = pPrev = NULL;

	pNode1 = pNode;
	pNode2 = pNode ? pNode->pNext : NULL;
	while (pNode1 && (fn(this, pNode1, pContext) <  0)) {
		pNode2 = pNode1;
		pNode1 = pNode1->pPrev;
	}
	while (pNode2 && (fn(this, pNode2, pContext) >= 0)) {
		pNode1 = pNode2;
		pNode2 = pNode2->pNext;
	}

	Insert(pNode1, pNode2);

	return pNode1 ? pFirst : this;
}

void AListNode::Insert(AListNode *pNode1, AListNode *pNode2)
{
	assert((pNode1 == NULL) || (pNode1->pNext == pNode2));
	assert((pNode2 == NULL) || (pNode2->pPrev == pNode1));

	if (pNode1) pNode1->pNext = this;
	if (pNode2) pNode2->pPrev = this;

	pPrev = pNode1;
	pNext = pNode2;
}

AListNode *AListNode::Detach()
{
	AListNode *pNode = pNext ? pNext : pPrev;
	DetachEx();

	return pNode ? pNode->First() : NULL;
}

void AListNode::DetachEx()
{
	if (pNext) pNext->pPrev = pPrev;
	if (pPrev) pPrev->pNext = pNext;
	pPrev = pNext = NULL;
}

AListNode *AListNode::Sort()
{
	AListNode *pNode  = First();
	AListNode *pFirst = NULL;

	while (pNode) {
		AListNode *pNode1 = pNode->pNext;
		pNode->DetachEx();

		pFirst = pNode->AttachEx(pFirst, pFirst);
		pNode  = pNode1;
	}

	return pFirst;
}

AListNode *AListNode::Sort(COMPARE_FUNC fn, void *pContext)
{
	AListNode *pNode  = First();
	AListNode *pFirst = NULL;

	while (pNode) {
		AListNode *pNode1 = pNode->pNext;
		pNode->DetachEx();

		pFirst = pNode->AttachEx(pFirst, pFirst, fn, pContext);
		pNode  = pNode1;
	}

	return pFirst;
}

int AListNode::Count(bool bTotal)
{
	int n = 0;

	if (pPrev && bTotal) n = First()->Count();
	else {
		AListNode *pNode;
		forptr (pNode, this) {
			n++;
		}
	}

	return n;
}

AListNode *AListNode::MergeList(AListNode *pNode)
{
	AListNode *pFirst = NULL;

	while (pNode) {
		AListNode *pNode1 = pNode->pNext;
		pNode->Detach();

		pFirst = pNode->AttachEx(pFirst, pFirst);
		pNode  = pNode1;
	}

	return pFirst;
}

AListNode *AListNode::MergeList(AListNode *pNode, COMPARE_FUNC fn, void *pContext)
{
	AListNode *pFirst = NULL;

	while (pNode) {
		AListNode *pNode1 = pNode->pNext;
		pNode->Detach();

		pFirst = pNode->AttachEx(pFirst, pFirst, fn, pContext);
		pNode  = pNode1;
	}

	return pFirst;
}

AListNode *AListNode::Traverse(bool (*fn)(AListNode *pNode, void *pContext), void *pContext)
{
	AListNode *pNode = this;
	
	while (pNode && fn(pNode, pContext)) pNode = pNode->Next();

	return pNode;
}

/*----------------------------------------------------------------------------------------------------*/

AList::AList() : pFirst(NULL),
				 pLast(NULL),
				 nNodes(0),
				 bAutoDelete(true)
{
}

AList::AList(const AList& object) : pFirst(NULL),
									pLast(NULL),
									nNodes(0),
									bAutoDelete(true)
{
	Copy(object);
}

AList::~AList()
{
	Delete();
}

void AList::Copy(const AList& object)
{
	const AListNode *pNode;

	if (bAutoDelete) DeleteAll();
	else			 RemoveAll();

	bAutoDelete = object.bAutoDelete;

	for (pNode = object.First(); pNode; pNode = pNode->Next()) {
		Add(pNode->Duplicate());
	}
}

void AList::Add(AListNode *pNode)
{
	pNode->Detach();
	pFirst = pNode->Attach(pLast);
	pLast  = pNode->Last();
	nNodes++;
	MarkAsChanged();
}

void AList::Add(AListNode *pNode, COMPARE_FUNC fn, void *pContext)
{
	pNode->Detach();
	pFirst = pNode->Attach(pLast, fn, pContext);
	pLast  = pNode->Last();
	nNodes++;
	MarkAsChanged();
}

bool AList::InsertBefore(AListNode *pNode, AListNode *pMemberNode)
{
	if (!IsMember(pMemberNode)) return false;

	pNode->Detach();
	pFirst = pNode->InsertBefore(pMemberNode);
	pLast  = pNode->Last();
	nNodes++;
	MarkAsChanged();

	return true;
}

bool AList::InsertAfter(AListNode *pNode, AListNode *pMemberNode)
{
	if (!IsMember(pMemberNode)) return false;

	pNode->Detach();
	pFirst = pNode->InsertAfter(pMemberNode);
	pLast  = pNode->Last();
	nNodes++;
	MarkAsChanged();

	return true;
}

bool AList::Replace(AListNode *pNode, AListNode *pMemberNode)
{
	if (!IsMember(pMemberNode)) return false;

	pNode->Detach();
	pNode->InsertAfter(pMemberNode);
	pFirst = pMemberNode->Detach();
	pLast  = pNode->Last();

	MarkAsChanged();

	return true;
}

bool AList::Remove(AListNode *pNode)
{
	if (!IsMember(pNode)) return false;

	pFirst = pNode->Detach();
	pLast  = pFirst ? pFirst->Last() : NULL;
	nNodes--;
	MarkAsChanged();
	
	return true;
}

bool AList::Delete(AListNode *pNode)
{
	if (!IsMember(pNode)) return false;

	pFirst = pNode->Detach();
	pLast  = pFirst ? pFirst->Last() : NULL;
	nNodes--;
	MarkAsChanged();

	delete pNode;

	return true;
}

void AList::RemoveAll()
{
	AListNode *pNode = pFirst;
	while (pNode) {
		AListNode *pNextNode = pNode->Next();

		pNode->Detach();

		pNode = pNextNode;
	}

	pFirst = pLast = NULL;
	nNodes = 0;
	MarkAsChanged();
}

void AList::DeleteAll()
{
	AListNode *pNode = pFirst;
	while (pNode) {
		AListNode *pNextNode = pNode->Next();

		pNode->Detach();
		delete pNode;

		pNode = pNextNode;
	}

	pFirst = pLast = NULL;
	nNodes = 0;
	MarkAsChanged();
}

void AList::Push(AListNode *pNode)
{
	pNode->Detach();
	pFirst = pNode->InsertAfter(pLast);
	pLast  = pNode->Last();
	nNodes++;
	MarkAsChanged();
}

void AList::StartPush(AListNode *pNode)
{
	pNode->Detach();
	pFirst = pNode->InsertBefore(pFirst);
	pLast  = pNode->Last();
	nNodes++;
	MarkAsChanged();
}

AListNode *AList::Pop()
{
	AListNode *pNode = pFirst;
	if (pNode) {
		pFirst = pNode->Detach();
		pLast  = pFirst ? pLast : NULL;
		nNodes--;
		MarkAsChanged();
	}

	return pNode;
}

AListNode *AList::EndPop()
{
	AListNode *pNode = pLast;
	if (pNode) {
		pFirst = pNode->Detach();
		pLast  = pFirst ? pFirst->Last() : NULL;
		nNodes--;
		MarkAsChanged();
	}

	return pNode;
}

void AList::Sort()
{
	if (pFirst) {
		pFirst = pFirst->Sort();
		pLast  = pFirst ? pFirst->Last() : NULL;
		MarkAsChanged();
	}
}

void AList::Sort(COMPARE_FUNC fn, void *pContext)
{
	if (pFirst) {
		pFirst = pFirst->Sort(fn, pContext);
		pLast  = pFirst ? pFirst->Last() : NULL;
		MarkAsChanged();
	}
}

void AList::Delete()
{
	if (bAutoDelete) {
		DELETE_LIST(pFirst);
		pFirst = pLast = NULL;
		nNodes = 0;
		MarkAsChanged();
	}
}

bool AList::Find(const AListNode *pNode) const
{
	return (pFirst && pNode) ? (pNode->First() == pFirst) : false;
}

int AList::Member(const AListNode *pNode) const
{
	int index = -1;

	if (pNode && Find(pNode)) {
		const AListNode *pNode1 = pFirst;
		
		index = 0;
		while (pNode1 && (pNode1 != pNode)) {
			pNode1 = pNode1->Next(); index++;
		}

		if (!pNode1) index = -1;
	}

	return index;
}

AListNode *AList::Find(const AListNode *pCmpNode, bool bFromStart, COMPARE_FUNC fn, void *pContext) const
{
	const AListNode *pNode = bFromStart ? pFirst : pLast;
	if (bFromStart) {
		while (pNode && ((*fn)(pNode, pCmpNode, pContext) != 0)) pNode = pNode->Next();
	}
	else {
		while (pNode && ((*fn)(pNode, pCmpNode, pContext) != 0)) pNode = pNode->Prev();
	}

	return (AListNode *)pNode;
}

AList *AList::Duplicate(AList *pList) const
{
	if (!pList) pList = new AList;

	if (pList) {
		const AListNode *pNode = pFirst;
		while (pNode) {
			AListNode *pNode1 = pNode->Duplicate();

			if (pNode1) {
				if (!pList->pLast) pList->pFirst = pList->pLast = pNode1;
				else {
					pList->pLast->pNext = pNode1;
					pNode1->pPrev = pList->pLast;
				}
			}
			else {
				delete pList;
				pList = NULL;
				break;
			}

			pNode = pNode->Next();
		}

		pList->MarkAsChanged();
	}

	return pList;
}

uint_t AList::Limit(uint_t count, bool fromEnd)
{
	uint_t n = 0;

	if (fromEnd) {
		while (pLast && (nNodes > count)) {
			AListNode *pNode = pLast;
			Remove(pNode);
			delete pNode;
			n++;
		}
	}
	else {
		while (pFirst && (nNodes > count)) {
			AListNode *pNode = pFirst;
			Remove(pNode);
			delete pNode;
			n++;
		}
	}

	return n;
}

/*----------------------------------------------------------------------------------------------------*/

NODETYPE_IMPLEMENT(AListListNode);

AListListNode::AListListNode() : AListNode()
{
}

AListListNode::AListListNode(const AListListNode& object) : AListNode(),
															List(object.List)
{
}

AListListNode::~AListListNode()
{
}
