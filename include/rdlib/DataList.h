
#ifndef __DATA_LIST__
#define __DATA_LIST__

#include <vector>

/*
  ADataList object

  Provides a self-expanding list of data items (32-bit values or pointers).  The list is always contiguous
  in memory. Items can be added and removed.  Lists can be sorted, searched and merged.  Hooks can be run
  on lists.

*/

#include "misc.h"

class ADataList : public std::vector<uptr_t> {
public:
	ADataList(void (*fn)(uptr_t item, void *context) = NULL, void *context = NULL);
	ADataList(const ADataList& List);
	virtual ~ADataList();

	void SetDestructor(void (*fn)(uptr_t item, void *context), void *context = NULL) {pDestructor = fn; pDestructorContext = context;}

	// Clear, but do not delete memory allocated
	void Clear();

	// By default, items can appear more than once in the list
	// Disabling duplication prevents this
	void EnableDuplication(bool bEnable = true) {bDuplication = bEnable;}

	// overloaded operators
	ADataList& operator = (const ADataList& List);
	ADataList& operator = (ADataList *pList);
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
	uptr_t First() const {return size() ? List()[0]          : 0;}
	uptr_t Last()  const {return size() ? List()[size() - 1] : 0;}

	// Add item (default is to end)
	sint_t Add(uptr_t Item, sint_t Index = MAX_SIGNED(sint_t));
	sint_t Add(void *ptr, sint_t Index = MAX_SIGNED(sint_t)) {return Add((uptr_t)ptr, Index);}
	sint_t Remove(uptr_t Item);
	sint_t Remove(void *ptr) {return Remove((uptr_t)ptr);}
	sint_t Insert(uptr_t Item, int (*fn)(uptr_t Item1, uptr_t Item2, void *context), void *context = NULL);
	sint_t Insert(void *ptr, int (*fn)(void *ptr1, void *ptr2, void *context), void *context = NULL);
	
	// Remove a specific index
	uptr_t RemoveIndex(uint_t Index);

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

	uint_t Count() const {return (uint_t)size();}

	// Return pointer to list
	uptr_t *List() const {return size() ? GetList() : NULL;}

	// Return item in list (or 0)
	uptr_t operator [](sint_t n) const {return ((n >= 0) && (n < (sint_t)size())) ? GetList()[(size_t)n] : 0;}
	uptr_t operator [](uint_t n) const {return (n < size()) ? GetList()[n] : 0;}

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
	bool operator != (const ADataList& List) const {return !operator == (List);}

protected:
	void SwapEx(uint_t n1, uint_t n2);

	uptr_t *GetList() const {return _M_impl._M_start;}
	
protected:
	void (*pDestructor)(uptr_t item, void *context);
	void *pDestructorContext;
	bool bDuplication;
};

#endif
