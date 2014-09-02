
#ifndef __SETTINGS_HANDLER__
#define __SETTINGS_HANDLER__

#include "strsup.h"
#include "StdFile.h"
#include "Hash.h"

class ASettingsHandler {
public:
	ASettingsHandler(const AString& name, bool inhomedir = true, uint32_t iwritedelay = ~0);
	ASettingsHandler(const AString& path, const AString& name, uint32_t iwritedelay = ~0);
	~ASettingsHandler();

	static void SetHomeDirectory(const char *dir) {homedir = dir;}

	void Read();
	void Write();
	void CheckWrite();

	const AString& Get(const AString& name, const AString& defval = "") const;
	void Set(const AString& name, const AString& value);
	bool Exists(const AString& name) const {return (Find(name) != NULL);}
	void Delete(const AString& name);

	void AddLine(const AString& str);

	enum {
		PairType_Value = 0,
		PairType_Other,
	};

	const AStringPairWithInt *GetFirst() const {return AStringPairWithInt::Cast(list.First());}
	uint_t GetCount() const {return list.Count();}

	const AString& GetFilename() const {return filename;}

protected:
	const AStringPairWithInt *Find(const AString& name) const;

protected:
	AString   filename;
	AList 	  list;
	AHash     hash;
	uint32_t  changed_tick, write_tick;
	uint32_t  writedelay;
	bool  	  changed;
	bool	  autowrite;

	static AString homedir;
};

#endif
