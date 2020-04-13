
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>

#include "DataList.h"

/* end of includes */

/*------------------------------------------------------------
  Function: Standard Constructor
  ----------------------------------------------------------*/
ADataList::ADataList(void (*fn)(uptr_t item, void *context), void *context) : pDestructor(fn),
                                                                              pDestructorContext(context),
                                                                              bDuplication(true)
{
}

/*------------------------------------------------------------
  Function:
  ----------------------------------------------------------*/
ADataList::ADataList(const ADataList& List) : pDestructor(NULL),
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
    if (!data.empty()) {
        if (pDestructor) {
            size_t i;
            for (i = 0; i < data.size(); i++) {
                (*pDestructor)(data[i], pDestructorContext);
            }
        }
        data.resize(0);
    }
}

ADataList& ADataList::operator = (const ADataList& List)
{
    DeleteList();

    if (List.data.size()) {
        data = List.data;
    }

    pDestructor        = List.pDestructor;
    pDestructorContext = List.pDestructorContext;
    bDuplication       = List.bDuplication;

    return *this;
}

ADataList& ADataList::operator = (ADataList *pList)
{
    operator = (*pList);
    return *this;
}

sint_t ADataList::Add(uptr_t Item, sint_t Index)
{
    sint_t index = -1;

    if (bDuplication || ((index = Find(Item)) < 0)) {
        Index = std::max(Index, 0);
        Index = std::min(Index, (sint_t)data.size());

        data.insert(data.begin() + Index, Item);

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
            else if ((*fn)(Item, data[i], context) < 0) {
                if ((i == 0) || ((*fn)(Item, data[i - 1], context) >= 0)) break;
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
            else if ((*fn)(ptr, (void *)data[i], context) < 0) {
                if ((i == 0) || ((*fn)(ptr, (void *)data[i - 1], context) >= 0)) break;
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
        data.erase(data.begin() + index);
    }

    return index;
}

uptr_t ADataList::RemoveIndex(uint_t Index)
{
    uptr_t item = 0;

    if (Index < data.size()) {
        item = data[Index];
        data.erase(data.begin() + Index);
    }

    return item;
}

uint_t ADataList::Replace(uint_t Index, uptr_t Item, bool bAllowExpand)
{
    if ((Index >= data.size()) && bAllowExpand) {
        data.resize(Index + 1);
    }

    if (data.size()) {
        Index = std::min(Index, (uint_t)data.size() - 1);

        data[Index] = Item;
    }

    return Index;
}

bool ADataList::Generate(uint_t count, sint_t start, sint_t mul, sint_t div)
{
    uint_t n = data.size();
    uint_t i;

    data.resize(data.size() + count);

    for (i = 0; i < count; i++) data[n++] = muldivs(start + i, mul, div);

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
    std::vector<uptr_t>::const_iterator it;
    sint_t index = -1;

    Index = std::max(Index, 0);
    if ((it = std::find(data.begin() + Index, data.end(), Item)) != data.end()) {
        index = (uint_t)(it - data.begin());
    }

    return index;
}

void ADataList::SwapAndSort(uint_t Index, int (*fn)(uptr_t Item1, uptr_t Item2, void *pContext), void *pContext)
{
    SwapEx(Index, Index + 1);

    if ((Index > 0)                 && ((*fn)(data[Index - 1], data[Index],     pContext) > 0)) SwapAndSort(Index - 1, fn, pContext);
    if ((Index < (data.size() - 2)) && ((*fn)(data[Index + 1], data[Index + 2], pContext) > 0)) SwapAndSort(Index + 1, fn, pContext);
}

void ADataList::Sort(int (*fn)(uptr_t Item1, uptr_t Item2, void *pContext), void *pContext)
{
    if (data.size() > 1) {
        uint_t i, n = (uint_t)(data.size() - 1);
        bool   bSort;

        do {
            bSort = false;

            for (i = 0; i < n; i++) {
                if ((*fn)(data[i], data[i + 1], pContext) > 0) {
                    SwapAndSort(i, fn, pContext);
                    bSort = true;
                    break;
                }
            }
        } while (bSort);
    }
#if 0
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

    std::sort(data.begin(), data.end(), cmp);
#endif
}

uptr_t ADataList::Pop()
{
    uptr_t Item = 0;

    if (!data.empty()) {
        Item = data[0];
        data.erase(data.begin());
    }

    return Item;
}

uptr_t ADataList::EndPop()
{
    uptr_t Item = 0;

    if (!data.empty()) {
        Item = data[data.size() - 1];
        data.erase(data.begin() + data.size() - 1);
    }

    return Item;
}

bool ADataList::Traverse(bool (*fn)(uptr_t Item, void *Context), void *Context) const
{
    uint_t i;
    bool ok = true;

    for (i = 0; (i < data.size()) && ok; i++) ok = fn(data[i], Context);

    return ok;
}

void ADataList::Reverse()
{
    uint_t i, n = data.size() / 2;
    for (i = 0; i < n; i++) SwapEx(i, data.size() - 1 - i);
}

void ADataList::SwapEx(uint_t n1, uint_t n2)
{
    if (n1 != n2) {
        data[n1] ^= data[n2];
        data[n2] ^= data[n1];
        data[n1] ^= data[n2];
    }
}

bool ADataList::Swap(uint_t n1, uint_t n2)
{
    bool success = false;

    if ((n1 < data.size()) && (n2 < data.size())) {
        SwapEx(n1, n2);

        success = true;
    }

    return success;
}

bool ADataList::Merge(ADataList& List)
{
    uint_t i, n = List.Count();
    bool ok = true;

    for (i = 0; ok && (i < n); i++) ok = (Add(List.data[i]) >= 0);

    return ok;
}
