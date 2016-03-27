
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DataList.h"

/* end of includes */

/*------------------------------------------------------------
  Function: Standard Constructor
  ----------------------------------------------------------*/
ADataList::ADataList() : pDestructor(NULL),
						 pDestructorContext(NULL),
						 pData(NULL),
						 nItems(0),
						 nMaxItems(0),
						 nIncItems(16),
						 bDuplication(true)
{
}

/*------------------------------------------------------------
  Function: 
  ----------------------------------------------------------*/
ADataList::ADataList(const ADataList& List) : pDestructor(NULL),
											  pDestructorContext(NULL),
											  pData(NULL),
											  nItems(0),
											  nMaxItems(0),
											  nIncItems(16),
											  bDuplication(true)
{
	operator = (List);
}

ADataList::~ADataList()
{
	DeleteList();
}

void ADataList::DeleteList()
{
	if (pData) {
		if (pDestructor) {
			uint_t i;
			for (i = 0; i < nItems; i++) {
				(*pDestructor)(pData[i], pDestructorContext);
			}
		}
		delete[] (uint8_t *)pData;
		pData  = NULL;
		nItems = nMaxItems = 0;
		nIncItems = 16;
	}
}

void ADataList::Clear()
{
	if (pDestructor) {
		uint_t i;
		for (i = 0; i < nItems; i++) {
			(*pDestructor)(pData[i], pDestructorContext);
		}
	}
	nItems = 0;
}

ADataList& ADataList::operator = (const ADataList& List)
{
	operator = ((ADataList *)&List);

	return *this;
}

ADataList& ADataList::operator = (ADataList *pList)
{
	if (pList->nItems) {
		if (nItems == pList->nItems) {
			if (pDestructor) {
				uint_t i;
				for (i = 0; i < nItems; i++) {
					(*pDestructor)(pData[i], pDestructorContext);
				}
			}
			memcpy(pData, pList->pData, nItems * sizeof(uptr_t));
		}
		else {
			DeleteList();

			nItems = pList->nItems;
			if ((pData = (uptr_t *)::Allocate(pData, nItems + 2, nMaxItems, sizeof(uptr_t), nIncItems)) != NULL) {
				memcpy(pData, pList->pData, nItems * sizeof(uptr_t));
			}
			else nItems = 0;
		}

		bDuplication 	   = pList->bDuplication;
		pDestructor  	   = pList->pDestructor;
		pDestructorContext = pList->pDestructorContext;
	}
	else DeleteList();

	return *this;
}

sint_t ADataList::Add(uptr_t Item, sint_t Index)
{
	sint_t index = -1;

	if (bDuplication || ((index = Find(Item)) < 0)) {
		if ((pData = (uptr_t *)::Allocate(pData, nItems + 2, nMaxItems, sizeof(uptr_t), nIncItems)) != NULL) {
			Index = MAX(Index, 0);
			Index = MIN(Index, (sint_t)nItems);
			
			if (Index < (sint_t)nItems) memmove(pData + Index + 1, pData + Index, (nItems - Index) * sizeof(*pData));

			pData[Index] = Item;
			nItems++;
			
			index = Index;
		}
	}

	return index;
}

sint_t ADataList::Insert(uptr_t Item, int (*fn)(uptr_t Item1, uptr_t Item2, void *context), void *context)
{
	const uint_t n = Count() - 1;
	uint_t i = (n + 1) >> 1 , inc = (i + 1) >> 1;

	if (Count()) {
		while (true) {
			if (i && ((*fn)(Item, pData[MIN(i, n)], context) < 0)) {
				i   = SUBZ(i, inc);
				inc = (inc + 1) >> 1;
			}
			else if ((i < n) && ((*fn)(Item, pData[MIN(i + 1, n)], context) >= 0)) {
				i  += inc;
				inc = (inc + 1) >> 1;
			}
			else break;
		}
	}
	
	return Add(Item, i);
}

sint_t ADataList::Insert(void *ptr, int (*fn)(void *ptr1, void *ptr2, void *context), void *context)
{
	const uint_t n = Count() - 1;
	uint_t i = (n + 1) >> 1 , inc = (i + 1) >> 1;

	if (Count()) {
		while (true) {
			if (i && ((*fn)(ptr, (void *)pData[MIN(i, n)], context) < 0)) {
				i   = SUBZ(i, inc);
				inc = (inc + 1) >> 1;
			}
			else if ((i < n) && ((*fn)(ptr, (void *)pData[MIN(i + 1, n)], context) >= 0)) {
				i  += inc;
				inc = (inc + 1) >> 1;
			}
			else break;
		}
	}
	
	return Add(ptr, i);
}

sint_t ADataList::Remove(uptr_t Item)
{
	sint_t index = -1;

	if ((index = Find(Item)) >= 0) {
		if ((index + 1) < (sint_t)nItems) memmove(pData + index, pData + index + 1, (nItems - index - 1) * sizeof(*pData));
		nItems--;
		pData[nItems] = 0;
	}

	return index;
}

uptr_t ADataList::RemoveIndex(uint_t Index)
{
	uptr_t item = 0;

	if (Index < nItems) {
		item = pData[Index];
		if ((Index + 1) < nItems) memmove(pData + Index, pData + Index + 1, (nItems - Index - 1) * sizeof(*pData));
		nItems--;
		pData[nItems] = 0;
	}

	return item;
}

sint_t ADataList::AddBlock(void *ptr, uint_t BlockSize)
{
	uptr_t *p = (uptr_t *)ptr;
	uint_t i, n = BlockSize / sizeof(uptr_t);
	sint_t index;

	index = Add(p[0]);
	for (i = 1; (i < n) && (index >= 0); i++) {
		if (Add(p[i]) < 0) index = -1;
	}

	return index;
}

uint_t ADataList::Replace(uint_t Index, uptr_t Item, bool bAllowExpand)
{
	if ((Index >= nItems) && bAllowExpand) {
		nItems = Index + 1;
		if ((pData = (uptr_t *)::Allocate(pData, nItems + 2, nMaxItems, sizeof(uptr_t), nIncItems)) == NULL) {
			nItems = nMaxItems = 0;
		}
	}

	if (nItems) {
		Index = MIN(Index, nItems - 1);

		pData[Index] = Item;
	}

	return Index;
}

bool ADataList::Generate(uint_t count, sint_t start, sint_t mul, sint_t div)
{
	bool success = false;

	if ((count > 0) && ((pData = (uptr_t *)Allocate(pData, nItems + count + 2, nMaxItems, sizeof(uptr_t))) != NULL)) {
		uint_t i;

		for (i = 0; i < count; i++) pData[nItems++] = muldivs(start + i, mul, div);

		success = true;
	}

	return success;
}

ADataList& ADataList::operator += (const ADataList& List)
{
	uint_t i;

	for (i = 0; i < List.Count(); i++) Push(List[i]);

	return *this;
}

ADataList& ADataList::operator -= (const ADataList& List)
{
	uint_t i;

	for (i = 0; i < List.Count(); i++) Remove(List[i]);

	return *this;
}

sint_t ADataList::Find(uptr_t Item, sint_t Index) const
{
	sint_t i;

	Index = MAX(Index, 0);

	for (i = Index; (i < (sint_t)nItems) && (pData[i] != Item); i++) ;

	if (i >= (sint_t)nItems) i = -1;

	return i;
}

sint_t ADataList::FindData(uint32_t Offset, void *ptr, uint32_t size, sint_t Index) const
{
	sint_t i;

	Index = MAX(Index, 0);

	for (i = Index; i < (sint_t)nItems; i++) {
		if (memcmp(ptr, (void *)(pData[i] + Offset), size) == 0) break;
	}

	if (i >= (sint_t)nItems) i = -1;

	return i;
}

#define FINDDATA(type)													\
	sint_t ADataList::FindData(uint32_t Offset, type Data, sint_t Index) const \
	{																	\
		sint_t i;														\
																		\
		Index = MAX(Index, 0);											\
																		\
		for (i = Index; i < (sint_t)nItems; i++) {						\
			if (Data == *((type *)(pData[i] + Offset))) break;			\
		}																\
																		\
		if (i >= (sint_t)nItems) i = -1;								\
																		\
		return i;														\
	}

FINDDATA(sint8_t);
FINDDATA(uint8_t);
FINDDATA(sint16_t);
FINDDATA(uint16_t);
FINDDATA(sint32_t);
FINDDATA(uint32_t);
FINDDATA(sint64_t);
FINDDATA(uint64_t);
FINDDATA(float);
FINDDATA(double);

void ADataList::SwapAndSort(sint_t Index, int (*fn)(uptr_t Item1, uptr_t Item2, void *pContext), void *pContext)
{
	SwapEx(Index, Index + 1);

	if ((Index > 0)					 && ((*fn)(pData[Index - 1], pData[Index],     pContext) > 0)) SwapAndSort(Index - 1, fn, pContext);
	if ((Index < (sint_t)(nItems - 2)) && ((*fn)(pData[Index + 1], pData[Index + 2], pContext) > 0)) SwapAndSort(Index + 1, fn, pContext);
}

void ADataList::Sort(int (*fn)(uptr_t Item1, uptr_t Item2, void *pContext), void *pContext)
{
	if (pData && (nItems > 1)) {
		bool bSort;
		uint_t i, n = nItems - 1;

		do {
			bSort = false;

			for (i = 0; i < n; i++) {
				if ((*fn)(pData[i], pData[i + 1], pContext) > 0) {
					SwapAndSort(i, fn, pContext);
					bSort = true;
					break;
				}
			}

		} while (bSort);
	}
}

uptr_t ADataList::Pop()
{
	uptr_t Item = 0;

	if (nItems) {
		Item = pData[0];
		RemoveIndex(0);
	}
	
	return Item;
}

uptr_t ADataList::EndPop()
{
	uptr_t Item = 0;

	if (nItems) {
		Item = pData[nItems - 1];
		RemoveIndex(nItems - 1);
	}
	
	return Item;
}

bool ADataList::Traverse(bool (*fn)(uptr_t Item, void *Context), void *Context) const
{
	bool ok = true;

	uint_t i;
	for (i = 0; (i < nItems) && ok; i++) ok = fn(pData[i], Context);

	return ok;
}

void ADataList::Reverse()
{
	uint_t i, n = nItems / 2;
	for (i = 0; i < n; i++) SwapEx(i, nItems - 1 - i);
}

void ADataList::SwapEx(uint_t n1, uint_t n2)
{
	if (n1 != n2) {
		pData[n1] ^= pData[n2];
		pData[n2] ^= pData[n1];
		pData[n1] ^= pData[n2];
	}
}

bool ADataList::Swap(uint_t n1, uint_t n2)
{
	bool success = false;

	if ((n1 < nItems) && (n2 < nItems)) {
		SwapEx(n1, n2);

		success = true;
	}

	return success;
}

bool ADataList::Merge(ADataList& List)
{
	uint_t i, n = List.Count();
	bool ok = true;

	for (i = 0; ok && (i < n); i++) ok = (Add(List[i]) >= 0);

	return ok;
}

bool ADataList::operator == (const ADataList& List) const
{
	bool diff = ((nItems == List.nItems) &&
				 (memcmp(pData, List.pData, nItems * sizeof(pData[0])) == 0));

	return diff;
}

bool ADataList::operator != (const ADataList& List) const
{
	bool diff = ((nItems != List.nItems) ||
				 (memcmp(pData, List.pData, nItems * sizeof(pData[0])) != 0));

	return diff;
}
