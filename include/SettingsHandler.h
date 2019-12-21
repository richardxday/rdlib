
#ifndef __SETTINGS_HANDLER__
#define __SETTINGS_HANDLER__

#include "strsup.h"
#include "StdFile.h"
#include "Hash.h"
#include "DateTime.h"
#include "DataList.h"

class ASettingsHandler {
public:
    ASettingsHandler();
    ASettingsHandler(const AString& name, bool inhomedir = true, uint32_t iwritedelay = ~0);
    ASettingsHandler(const AString& path, const AString& name, uint32_t iwritedelay = ~0);
    ~ASettingsHandler();

    void EnableWrite(bool enabled = true) {writeenabled = enabled;}

    static void SetHomeDirectory(const char *dir) {homedir = dir;}

    void Read();
    bool CheckRead();
    void Write();
    void CheckWrite();
    bool HasFileChanged();

    bool HasChanged() const {return changed;}
    void ClearChanged() {changed = false;}

    AString Get(const AString& name, const AString& defval = "") const;
    void Set(const AString& name, const AString& value);
    bool Exists(const AString& name) const {return (Find(name) != NULL);}
    void Delete(const AString& name);

    void AddLine(const AString& str);

    enum {
        PairType_Value = 0,
        PairType_Other,
    };

    typedef AStringPairWithInt Value;
    const Value *GetFirst() const {return Value::Cast(list.First());}
    uint_t GetCount() const {return list.Count();}

    const AString& GetFilename() const {return filename;}

    uint_t GetAllLike(ADataList& list, const AString& str, bool regex = false) const;

protected:
    const Value *Find(const AString& name) const;

protected:
    AString   filename;
    ADateTime timestamp;
    AList     list;
    AHash     hash;
    uint32_t  readcheck_tick, changed_tick, write_tick;
    uint32_t  writedelay;
    bool      changed;
    bool      writeenabled;

    static AString homedir;
};

#endif
