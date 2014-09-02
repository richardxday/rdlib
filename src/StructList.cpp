
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "StructList.h"

/* end of includes */

AStructList::AStructList(uint32_t itemSize) : pData(NULL),
											ItemSize(0),
											nItems(0),
											nMaxItems(0),
											nIncItems(16)
{
	if (itemSize) SetItemSize(itemSize);
}

AStructList::AStructList(const AStructList& list) : pData(NULL),
													ItemSize(list.ItemSize),
													nItems(0),
													nMaxItems(0),
													nIncItems(16)
{
}

AStructList::~AStructList()
{
	Delete();
}

void AStructList::Delete()
{
	DeleteList();

	if (pData) {
		delete[] pData;
		pData = NULL;
	}

	nMaxItems = 0;
	ItemSize  = 0;
	nIncItems = 16;
}

void AStructList::DeleteList()
{
	if (pData) memset(pData, 0, nMaxItems * ItemSize);

	nItems = 0;
}

uint8_t *AStructList::Insert(uint_t index)
{
	uint_t n = MAX(index, nItems) + 1;

	pData  = (uint8_t *)::Allocate(pData, n, nMaxItems, ItemSize, nIncItems);

	if (pData) nItems = n;
	else	   nItems = nMaxItems = 0;

	uint8_t *p = GetIndex(index);
	if (p) {
		memmove(p + ItemSize, p, (nItems - index - 1) * ItemSize);
		memset(p, 0, ItemSize);
	}

	return p;
}

void *AStructList::Traverse(STRUCT_TRAVERSE fn, void *context, uint_t start, uint_t count)
{
	start = MAX(start, 0);
	start = MIN(start, nItems);
	count = MIN(count, nItems - start);

	uint8_t *p = GetIndex(start);
	uint_t  i;	
	for (i = 0; i < count; i++, p += ItemSize) {
		if (!(*fn)(p, context)) break;
	}

	return p;
}

void *AStructList::Add(const void *p, sint_t index)
{
	index = MAX(index, 0);
	index = MIN(index, (sint_t)nItems);

	uint8_t *p1 = NULL;
	if (ItemSize && ((p1 = Insert(index)) != NULL)) memcpy(p1, p, ItemSize);

	return p1;
}

void *AStructList::Add(STRUCT_FUNC fn, void *context, sint_t index)
{
	index = MAX(index, 0);
	index = MIN(index, (sint_t)nItems);

	uint8_t *p1 = NULL;
	if (ItemSize && ((p1 = Insert(index)) != NULL)) (*fn)(p1, context);

	return p1;
}

sint_t AStructList::Remove(const void *p, STRUCT_COMPARE fn, void *context, uint_t start, bool fromStart)
{
	sint_t index = FindIndex(p, fn, context, start, fromStart);
	if ((index >= 0) && (index < (sint_t)nItems)) RemoveIndex(index);

	return index;
}

bool AStructList::RemoveIndex(uint_t index, void *p)
{
	uint8_t *p1	  = GetIndex(index);
	bool  success = false;

	if (p1) {
		if (p) memcpy(p, p1, ItemSize);

		nItems--;

		if (index < nItems) memmove(p1, p1 + ItemSize, (nItems - index) * ItemSize);
		memset(pData + nItems * ItemSize, 0, ItemSize);

		success = true;
	}

	return success;
}

bool AStructList::RemoveRange(uint_t index, uint_t count)
{
	bool success = false;
	if (pData && (count > 0) && ((index + count) <= nItems)) {
		uint8_t *p1 = GetIndex(index);
		uint8_t *p2 = GetIndex(index + count);
		sint_t   n  = nItems - index - count;

		if (n > 0) memmove(p1, p2, n * ItemSize);

		nItems -= count;

		success = true;
	}

	return success;
}

bool AStructList::Pop(void *p)
{
	return RemoveIndex(0, p);
}

bool AStructList::EndPop(void *p)
{
	return RemoveIndex(nItems - 1, p);
}

bool AStructList::Read(uint_t index, void *p) const
{
	uint8_t *p1 = GetIndex(index);
	if (p && p1) memcpy(p, p1, ItemSize);
	
	return (p1 != NULL);
}

bool AStructList::First(void *p) const
{
	uint8_t *p1 = GetIndex(0);
	if (p && p1) memcpy(p, p1, ItemSize);
	
	return (p1 != NULL);
}

bool AStructList::Last(void *p) const
{
	uint8_t *p1 = (nItems > 0) ? GetIndex(nItems - 1)  : NULL;
	if (p && p1) memcpy(p, p1, ItemSize);
	
	return (p1 != NULL);
}

sint_t AStructList::IndexOf(const void *p) const
{
	sint_t offset = (sint_t)((uint8_t *)p - pData);
	sint_t index  = -1;

	if (pData &&
		((offset % ItemSize) == 0) &&
		(offset >= 0) &&
		(offset < (sint_t)(nItems * ItemSize))) {
		index = (sint_t)(offset / ItemSize);
	}

	return index;
}

void *AStructList::Find(const void *p, STRUCT_COMPARE fn, void *context, uint_t start, bool fromStart) const
{
	sint_t index = FindIndex(p, fn, context, start, fromStart);
	void *p1 = NULL;

	if ((index >= 0) && (index < (sint_t)nItems)) p1 = (void *)GetIndex(index);

	return p1;
}

sint_t AStructList::FindIndex(const void *p, STRUCT_COMPARE fn, void *context, uint_t start, bool fromStart) const
{
	sint_t index = IndexOf(p);

	if (index >= 0) return index;

	if (pData && nItems) {
		start = MIN(start, nItems - 1);
		
		if (fromStart) {
			uint8_t *p1 = GetIndex(start);
			uint_t  i;
			
			for (i = start; i < nItems; i++, p1 += ItemSize) {
				if (fn) {
					if ((*fn)(p, p1, context) == 0) {
						index = (sint_t)i;
						break;
					}
				}
				else if (memcmp(p, p1, ItemSize) == 0) {
					index = (sint_t)i;
					break;
				}
			}
		}
		else {
			uint8_t *p1 = GetIndex(nItems - 1 - start);
			uint_t  i;
			
			for (i = start; i < nItems; i++, p1 -= ItemSize) {
				if (fn) {
					if ((*fn)(p, p1, context) == 0) {
						index = (sint_t)(nItems - 1 - i);
						break;
					}
				}
				else if (memcmp(p, p1, ItemSize) == 0) {
					index = (sint_t)(nItems - 1 - i);
					break;
				}
			}
		}
	}

	return index;
}
