
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>

#include "DataList.h"

/* end of includes */

/*------------------------------------------------------------
  Function: Standard Constructor
  ----------------------------------------------------------*/
ADataList::ADataList(void (*fn)(uptr_t item, void *context), void *context) : std::vector<uptr_t>(),
																			  pDestructor(fn),
																			  pDestructorContext(context),
																			  bDuplication(true)
{
}

/*------------------------------------------------------------
  Function: 
  ----------------------------------------------------------*/
ADataList::ADataList(const ADataList& List) : std::vector<uptr_t>(),
											  pDestructor(NULL),
											  pDestructorContext(NULL),
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
	if (size()) {
		if (pDestructor) {
			size_t i;
			for (i = 0; i < size(); i++) {
				(*pDestructor)(GetList()[i], pDestructorContext);
			}
		}
		resize(0);
	}
}

ADataList& ADataList::operator = (const ADataList& List)
{
	operator = ((ADataList *)&List);

	return *this;
}

ADataList& ADataList::operator = (ADataList *pList)
{
	DeleteList();
	
	if (pList->size()) {
		std::vector<uptr_t>::operator = (*pList);
	}
	
	pDestructor  	   = pList->pDestructor;
	pDestructorContext = pList->pDestructorContext;
	bDuplication 	   = pList->bDuplication;

	return *this;
}

sint_t ADataList::Add(uptr_t Item, sint_t Index)
{
	sint_t index = -1;

	if (bDuplication || ((index = Find(Item)) < 0)) {
		Index = std::max(Index, 0);
		Index = std::min(Index, (sint_t)size());
		
		insert(begin() + Index, Item);

		index = Index;
	}

	return index;
}

sint_t ADataList::Insert(uptr_t Item, int (*fn)(uptr_t Item1, uptr_t Item2, void *context), void *context)
{
	const uint_t n = Count();
	uint_t i = (n + 1) >> 1 , inc = (i + 1) >> 1;

	if (n) {
		while (true) {
			if (i >= n) i = SUBZ(i, inc);
			else if ((*fn)(Item, GetList()[i], context) < 0) {
				if ((i == 0) || ((*fn)(Item, GetList()[i - 1], context) >= 0)) break;
				i = SUBZ(i, inc);
			}
			else {
				i += inc;
				if ((i == n) && (inc == 1)) break;
			}

			inc = (inc + 1) >> 1;
		}
	}
	
	return Add(Item, i);
}

sint_t ADataList::Insert(void *ptr, int (*fn)(void *ptr1, void *ptr2, void *context), void *context)
{
	const uint_t n = Count();
	uint_t i = (n + 1) >> 1 , inc = (i + 1) >> 1;

	if (n) {
		while (true) {
			if (i >= n) i = SUBZ(i, inc);
			else if ((*fn)(ptr, (void *)GetList()[i], context) < 0) {
				if ((i == 0) || ((*fn)(ptr, (void *)GetList()[i - 1], context) >= 0)) break;
				i = SUBZ(i, inc);
			}
			else {
				i += inc;
				if ((i == n) && (inc == 1)) break;
			}

			inc = (inc + 1) >> 1;
		}
	}
	
	return Add(ptr, i);
}

sint_t ADataList::Remove(uptr_t Item)
{
	sint_t index = -1;

	if ((index = Find(Item)) >= 0) {
		erase(begin() + index);
	}

	return index;
}

uptr_t ADataList::RemoveIndex(uint_t Index)
{
	uptr_t item = 0;

	if (Index < size()) {
		item = GetList()[Index];
		erase(begin() + Index);
	}

	return item;
}

uint_t ADataList::Replace(uint_t Index, uptr_t Item, bool bAllowExpand)
{
	if ((Index >= size()) && bAllowExpand) {
		resize(Index + 1);
	}

	if (size()) {
		Index = std::min(Index, (uint_t)size() - 1);

		GetList()[Index] = Item;
	}

	return Index;
}

bool ADataList::Generate(uint_t count, sint_t start, sint_t mul, sint_t div)
{
	uint_t n = size();
	uint_t i;

	resize(size() + count);
	
	for (i = 0; i < count; i++) GetList()[n++] = muldivs(start + i, mul, div);

	return true;
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
	ADataList::const_iterator it;
	sint_t index = -1;

	Index = std::max(Index, 0);
	if ((it = std::find(begin() + Index, end(), Item)) != end()) {
		index = it - begin();
	}

	return index;
}

void ADataList::Sort(int (*fn)(uptr_t Item1, uptr_t Item2, void *pContext), void *pContext)
{
	struct {
		int (*fn)(uptr_t Item1, uptr_t Item2, void *pContext);
		void *pContext;
		
		bool operator()(uptr_t Item1, uptr_t Item2)
		{
			return ((*fn)(Item1, Item2, pContext) > 0);
		}
	} cmp;
	cmp.fn = fn;
	cmp.pContext = pContext;

	std::sort(begin(), end(), cmp);
}

uptr_t ADataList::Pop()
{
	uptr_t Item = 0;

	if (size()) {
		Item = GetList()[0];
		erase(begin());
	}
	
	return Item;
}

uptr_t ADataList::EndPop()
{
	uptr_t Item = 0;

	if (size()) {
		Item = GetList()[size() - 1];
		erase(begin() + size() - 1);
	}
	
	return Item;
}

bool ADataList::Traverse(bool (*fn)(uptr_t Item, void *Context), void *Context) const
{
	uint_t i;
	bool ok = true;

	for (i = 0; (i < size()) && ok; i++) ok = fn(GetList()[i], Context);

	return ok;
}

void ADataList::Reverse()
{
	uint_t i, n = size() / 2;
	for (i = 0; i < n; i++) SwapEx(i, size() - 1 - i);
}

void ADataList::SwapEx(uint_t n1, uint_t n2)
{
	if (n1 != n2) {
		GetList()[n1] ^= GetList()[n2];
		GetList()[n2] ^= GetList()[n1];
		GetList()[n1] ^= GetList()[n2];
	}
}

bool ADataList::Swap(uint_t n1, uint_t n2)
{
	bool success = false;

	if ((n1 < size()) && (n2 < size())) {
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
	return ((size() == List.size()) &&
			size() &&
			(memcmp(GetList(), List.GetList(), size() * sizeof(GetList()[0])) == 0));
}
