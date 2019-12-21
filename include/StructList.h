
#ifndef __STRUCT_LIST__
#define __STRUCT_LIST__

#include "misc.h"

typedef void (*STRUCT_FUNC)(void *p, void *context);
typedef bool (*STRUCT_TRAVERSE)(void *p, void *context);
typedef int  (*STRUCT_COMPARE)(const void *p1, const void *p2, void *context);

class AStructList {
public:
    AStructList(uint32_t itemSize = 0);
    AStructList(const AStructList& list);
    ~AStructList();

    void SetItemSize(uint32_t itemSize) {assert((ItemSize == 0) || (ItemSize && (ItemSize == itemSize))); ItemSize = itemSize;}

    void SetIncrement(uint_t inc) {assert(inc > 0); nIncItems = inc;}

    void Delete();
    void DeleteList();

    void *Add(const void *p = NULL, sint_t index = MAX_SIGNED(sint_t));
    void *Add(STRUCT_FUNC fn, void *context = NULL, sint_t index = MAX_SIGNED(sint_t));
    sint_t Remove(const void *p, STRUCT_COMPARE fn = NULL, void *context = NULL, uint_t start = 0, bool fromStart = true);
    bool RemoveIndex(uint_t index, void *p = NULL);
    bool RemoveRange(uint_t index, uint_t count = 1);

    void *Push(const void *p = NULL)                      {return Add(p, MAX_SIGNED(sint_t));}
    void *Push(STRUCT_FUNC fn, void *context = NULL)      {return Add(fn, context, MAX_SIGNED(sint_t));}

    void *StartPush(const void *p = NULL)                 {return Add(p, 0);}
    void *StartPush(STRUCT_FUNC fn, void *context = NULL) {return Add(fn, context, 0);}

    bool Pop(void *p);
    bool EndPop(void *p);

    void     *List()       const {return pData;}
    uint_t   Count()       const {return nItems;}
    uint32_t GetItemSize() const {return ItemSize;}

    bool Read(uint_t index, void *p) const;

    bool First(void *p) const;
    bool Last(void *p) const;

    void *Traverse(STRUCT_TRAVERSE fn, void *context = NULL, uint_t start = 0, uint_t count = MAX_UNSIGNED(uint_t));

    void   *Find(const void *p, STRUCT_COMPARE fn = NULL, void *context = NULL, uint_t start = 0, bool fromStart = true) const;
    sint_t FindIndex(const void *p, STRUCT_COMPARE fn = NULL, void *context = NULL, uint_t start = 0, bool fromStart = true) const;

    void *operator [](int n) const {
        return GetIndex(n);
    }

    sint_t IndexOf(const void *p) const;

protected:
    uint8_t *GetIndex(sint_t n) const {return ((n >= 0) && (n < (sint_t)nItems) && pData) ? pData + n * ItemSize : NULL;}
    uint8_t *Insert(uint_t index);

protected:
    uint8_t *pData;
    size_t  ItemSize;
    size_t  nItems, nMaxItems, nIncItems;
};

#endif
