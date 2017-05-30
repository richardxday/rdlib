
#ifndef __HASH__
#define __HASH__

#include <map>

#include "StdData.h"
#include "strsup.h"

class AHash : public std::map<AString, uptr_t> {
public:
	AHash(void (*fn)(uptr_t item, void *context) = NULL, void *context = NULL);
	AHash(const AHash& hash);
	~AHash();

	void SetDestructor(void (*fn)(uptr_t item, void *context), void *context = NULL) {pDestructor = fn; pDestructorContext = context;}

	void EnableCaseInSensitive(bool enable) {bCaseInsensitive = enable;}
	bool CaseInSensitiveEnabled() const {return bCaseInsensitive;}

	void Delete();

	uint_t GetItems() const {return size();}
	
	void Insert(const AString& key, uptr_t value = 0);
	bool Remove(const AString& key);

	bool   Exists(const AString& key) const;
	uptr_t Read(const AString& key) const;

	bool Traverse(bool (*fn)(const AString& key, uptr_t value, void *context), void *context = NULL);
	bool TraverseCompare(const AString& cmp, int l, bool (*fn)(const AString& key, uptr_t value, void *context), void *context = NULL);

	bool Copy(const AHash& src);
	bool CopyCompare(const AHash& src, const AString& cmp, int l = -1);	uptr_t& operator [] (const AString& key);


	bool Write(AStdData& fp, bool (*fn)(AStdData& fp, const AString& key, uptr_t value, void *context), void *context = NULL);
	bool Read(AStdData& fp, bool (*fn)(AHash& hash, AStdData& fp, const AString& key, void *context), void *context = NULL);

protected:
	static bool __CopyKey(const AString& key, uptr_t value, void *context);
	
protected:
	void   (*pDestructor)(uptr_t item, void *context);
	void   *pDestructorContext;
	uint_t nTraverseCount;
	bool   bCaseInsensitive;
};

#endif
