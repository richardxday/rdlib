
#ifndef __STRING_LIST__
#define __STRING_LIST__

#include "strsup.h"
#include "DataList.h"

class AStringList {
public:
    AStringList();
    virtual ~AStringList();

    virtual AString *AddString(const AString& str = "", bool locked = false);
    virtual void    ModifyString(const AString *str, bool force = false) {UNUSED(str); UNUSED(force);}
    virtual bool    RemoveString(AString *str);
    virtual bool    UnLockString(const AString *str) {ModifyString(str); return (LockedStringList.Remove((uptr_t)str) >= 0);}

    virtual uint_t    LimitStrings(uint_t n);

    bool IsStringLocked(const AString *str) const {return (LockedStringList.Find((uptr_t)str) >= 0);}

    int  GetStringCount() const {return StringList.Count();}

    const AList& GetStringList() const {return StringList;}

    const AString *FirstString() const {return (const AString *)StringList.First();}
    const AString *LastString()  const {return (const AString *)StringList.Last();}

protected:
    AList     StringList;
    ADataList LockedStringList;
};

#endif
