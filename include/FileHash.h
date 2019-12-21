
#ifndef __FILE_HASH__
#define __FILE_HASH__

#include "misc.h"
#include "ListNode.h"
#include "strsup.h"
#include "Recurse.h"

class AFileHash : public AListNode {
public:
    AFileHash(const AString& Line);
    AFileHash(const FILE_INFO *pFile);
    virtual ~AFileHash();

    LIST_FUNCTIONS(AFileHash);

    static AString ExtractFilename(const AString& Line);

    const AString& GetFilename()   const {return Filename;}
    const ADateTime& GetFileTime() const {return FileTime;}
    uint64_t GetHashValue() const {return HashValue;}

    AFileHash *Find(const AString& filename) {
        if (CompareNoCase(Filename, filename) == 0) return this;
        else if (Next())                            return Next()->Find(filename);
        else                                        return NULL;
    }

    enum {
        STATE_UNKNOWN = 0,
        STATE_UNCHANGED,
        STATE_CHANGED,
        STATE_CHANGED_BY_DATE,
        STATE_NEW,
        STATE_DELETED,
    };

    byte GetState() const {return State;}

    void CheckFile();
    void Output(FILE *fp);

protected:
    bool CalcHash(uint64_t *value);

protected:
    AString   Filename;
    ADateTime FileTime;
    uint64_t    HashValue;
    byte     State;
    bool      bHashCalculated;

    NODETYPE_DEFINE(AFileHash);
};

#endif
