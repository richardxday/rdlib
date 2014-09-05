
#ifndef __DATA_LIST__
#define __DATA_LIST__

/*
  ADataList object

  Provides a self-expanding list of data items (32-bit values or pointers).  The list is always contiguous
  in memory. Items can be added and removed.  Lists can be sorted, searched and merged.  Hooks can be run
  on lists.

*/

#include <vector>

#include "misc.h"

class ADataList {
public:
	ADataList();
	ADataList(const ADataList& List);
	virtual ~ADataList();

	void SetIncrement(uint_t inc) {assert(inc > 0); nIncItems = inc;}
	
	void SetDestructor(void (*fn)(uptr_t item, void *context), void *context = NULL) {pDestructor = fn; pDestructorContext = context;}

	// Clear, but do not delete memory allocated
	void Clear();

	// By default, items can appear more than once in the list
	// Disabling duplication prevents this
	void EnableDuplication(bool bEnable = true) {bDuplication = bEnable;}

	// overloaded operators
	ADataList& operator  = (const ADataList& List);
	ADataList& operator  = (ADataList *pList);
	ADataList& operator += (uptr_t Item) {Add(Item);    return *this;}
	ADataList& operator += (void *ptr)   {Add(ptr);     return *this;}
	ADataList& operator += (const ADataList& List);
	ADataList& operator -= (uptr_t Item) {Remove(Item); return *this;}
	ADataList& operator -= (void *ptr)   {Remove(ptr);  return *this;}
	ADataList& operator -= (const ADataList& List);

	// FIFO/LIFO type operations
	// Push adds to end, Pop removes first item
	bool Push(uptr_t Item) {return (Add(Item) >= 0);}
	bool Push(void   *ptr) {return (Add((uptr_t)ptr) >= 0);}
	uptr_t Pop();
	// StartPush pushes item on the start of the list
	bool StartPush(uptr_t Item) {return (Add(Item, 0) >= 0);}
	bool StartPush(void   *ptr) {return (Add((uptr_t)ptr, 0) >= 0);}
	// EndPop removes the last item on the list
	uptr_t EndPop();

	// First and last items
	uptr_t First() const {return (nItems && pData) ? pData[0]          : 0;}
	uptr_t Last()  const {return (nItems && pData) ? pData[nItems - 1] : 0;}

	// Add item (default is to end)
	sint_t Add(uptr_t Item, sint_t Index = MAX_SIGNED(sint_t));
	sint_t Add(void *ptr, sint_t Index = MAX_SIGNED(sint_t)) {return Add((uptr_t)ptr, Index);}
	sint_t Remove(uptr_t Item);
	sint_t Remove(void *ptr) {return Remove((uptr_t)ptr);}

	// Remove a specific index
	uptr_t RemoveIndex(uint_t Index);

	// Add a block of memory in 32-bit items to list
	sint_t AddBlock(void *ptr, uint_t BlockSize);

	// Replace item (with optional list expanding)
	uint_t Replace(uint_t Index, uptr_t Item, bool bAllowExpand = true);
	uint_t Replace(uint_t Index, void *ptr, bool bAllowExpand = true) {return Replace(Index, (uptr_t)ptr, bAllowExpand);}

	// Generate a sequence of values (ints)
	bool Generate(uint_t count, sint_t start = 0, sint_t mul = 1, sint_t div = 1);

	// Delete list, de-allocating any allocated memory
	void DeleteList();

	// Search list
	sint_t Find(uptr_t Item, sint_t Index = 0) const;
	sint_t Find(void *ptr, sint_t Index = 0) const {return Find((uptr_t)ptr, Index);}

	// Search list, assuming each item is a pointer to a block of memory
	// 'Offset' is offset into this block of memory
	sint_t FindData(uint32_t Offset, void *ptr, uint32_t size, sint_t Index = 0) const;
	sint_t FindData(uint32_t Offset, sint8_t Data, sint_t Index = 0) const;
	sint_t FindData(uint32_t Offset, uint8_t Data, sint_t Index = 0) const;
	sint_t FindData(uint32_t Offset, sint16_t Data, sint_t Index = 0) const;
	sint_t FindData(uint32_t Offset, uint16_t Data, sint_t Index = 0) const;
	sint_t FindData(uint32_t Offset, sint32_t Data, sint_t Index = 0) const;
	sint_t FindData(uint32_t Offset, uint32_t Data, sint_t Index = 0) const;
	sint_t FindData(uint32_t Offset, sint64_t Data, sint_t Index = 0) const;
	sint_t FindData(uint32_t Offset, uint64_t Data, sint_t Index = 0) const;
	sint_t FindData(uint32_t Offset, float Data, sint_t Index = 0) const;
	sint_t FindData(uint32_t Offset, double Data, sint_t Index = 0) const;
	sint_t FindData(uint32_t Offset, void *ptr, sint_t Index = 0) const {return FindData(Offset, (uptr_t)ptr, Index);}

	uint_t Count() const {return nItems;}

	// Return pointer to list
	uptr_t *List() const {return pData;}

	// Return item in list (or 0)
	uptr_t operator [](sint_t n) const {return ((n >= 0) && (n < (sint_t)nItems)) ? pData[n] : 0;}
	uptr_t operator [](uint_t n) const {return (n < nItems) ? pData[n] : 0;}

	// Sort list items using hook function
	void Sort(int (*fn)(uptr_t Item1, uptr_t Item2, void *pContext), void *pContext = NULL);

	// Execute hook funtion on each item in list until false is returned
	bool Traverse(bool (*fn)(uptr_t Item, void *pContext), void *pContext = NULL) const;

	// Reverse the order the list
	void Reverse();

	// Merge in a list
	bool Merge(ADataList& List);

	// Swap two indicies
	bool Swap(uint_t n1, uint_t n2);

	// Compare lists
	bool operator == (const ADataList& List) const;
	bool operator != (const ADataList& List) const;

protected:
	void SwapEx(uint_t n1, uint_t n2);
	void SwapAndSort(sint_t Index, int (*fn)(uptr_t Item1, uptr_t Item2, void *pContext), void *pContext);

protected:
	void (*pDestructor)(uptr_t item, void *context);
	void *pDestructorContext;
	std::vector<uptr_t> Data;
	bool bDuplication;
};

#endif
