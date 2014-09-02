
#ifndef __HASH__
#define __HASH__

#include "StdData.h"

class AHash {
public:
	AHash(uint_t size = 0, void (*freefunc)(uptr_t value, void *context) = NULL, void *context = NULL);
	AHash(const AHash& hash);
	~AHash();

	void EnableCaseInSensitive(bool enable) {pCompFunc = enable ? &stricmp : &strcmp; pCompNFunc = enable ? &strnicmp : &strncmp;}
	bool CaseInSensitiveEnabled() const {return (pCompFunc == &stricmp);}

	bool Create(uint_t size, void (*freefunc)(uptr_t value, void *context) = NULL, void *context = NULL);
	void Delete();
	bool Valid() const {return (pBuckets && Size);}

	bool Resize(uint_t size);

	uint_t GetSize()  const {return Size;}
	uint_t GetItems() const {return nItems;}

	bool Insert(const char *key, uptr_t value = 0);
	bool Remove(const char *key);

	bool   Exists(const char *key) const;
	uptr_t Read(const char *key) const;

	const char *GetKey(const char *key) const;

	bool Traverse(bool (*fn)(const char *key, uptr_t value, void *context), void *context = NULL);
	bool TraverseCompare(const char *cmp, int l, bool (*fn)(const char *key, uptr_t value, void *context), void *context = NULL);

	bool Copy(const AHash& src);
	bool CopyCompare(const AHash& src, const char *cmp, int l = -1);

	bool Write(AStdData& fp, bool (*fn)(AStdData& fp, const char *key, uptr_t value, void *context), void *context = NULL);
	bool Read(AStdData& fp, bool (*fn)(AHash& hash, AStdData& fp, const char *key, void *context), void *context = NULL);

protected:
	typedef struct _BUCKET {
		char  *Key;
		uptr_t Value;
		struct _BUCKET *Next;
	} BUCKET;

	BUCKET *CreateBucket(const char *key, uptr_t value);
	void DeleteBucket(BUCKET *p);

	uint_t    Hash(const char *p) const;
	BUCKET *FindBucket(const char *key) const;
	BUCKET **FindBucketParent(const char *key) const;

	static bool __CopyKey(const char *key, uptr_t value, void *context);

protected:
	BUCKET **pBuckets;
	int    (*pCompFunc)(const char *str1, const char *str2);
	int    (*pCompNFunc)(const char *str1, const char *str2, size_t n);
	void   (*pFreeFunc)(uptr_t value, void *context);
	void   *pContext;
	uint_t   Size;
	uint_t   nTraverseCount;
	uint_t   nItems;
};

class AIntHash {
public:
	AIntHash(uint_t size = 0, void (*freefunc)(uptr_t value, void *context) = NULL, void *context = NULL);
	~AIntHash();

	bool Create(uint_t size, void (*freefunc)(uptr_t value, void *context) = NULL, void *context = NULL);
	void Delete();
	bool Valid() const {return (pBuckets && Size);}

	bool Resize(uint_t size);

	uint_t GetSize()  const {return Size;}
	uint_t GetItems() const {return nItems;}

	bool Insert(uint32_t key, uptr_t value = 0);
	bool Remove(uint32_t key);

	bool   Exists(uint32_t key) const;
	uptr_t Read(uint32_t key) const;

	bool Traverse(bool (*fn)(uint32_t key, uptr_t value, void *context), void *context = NULL);
	bool TraverseMatch(uint32_t cmp, bool (*fn)(uint32_t key, uptr_t value, void *context), void *context = NULL);

protected:
	typedef struct _BUCKET {
		uint32_t Key;
		uptr_t Value;
		struct _BUCKET *Next;
	} BUCKET;

	BUCKET *CreateBucket(uint32_t key, uptr_t value);
	void DeleteBucket(BUCKET *p);

	uint_t Hash(uint32_t p) const {return (uint_t)(p % Size);}
	BUCKET *FindBucket(uint32_t key) const;
	BUCKET **FindBucketParent(uint32_t key) const;

protected:
	BUCKET **pBuckets;
	void   (*pFreeFunc)(uptr_t value, void *context);
	void   *pContext;
	uint_t   Size;
	uint_t   nTraverseCount;
	uint_t   nItems;
};

class AStructHash {
public:
	AStructHash(uint_t structSize = 0, uint_t size = 0, void (*freefunc)(uptr_t value, void *context) = NULL, void *context = NULL);
	~AStructHash();

	bool Create(uint_t structSize, uint_t size, void (*freefunc)(uptr_t value, void *context) = NULL, void *context = NULL);
	void Delete();
	bool Valid() const {return (pBuckets && Size);}

	bool Resize(uint_t size);

	uint_t GetStructSize() const {return StructSize;}

	uint_t GetSize()  const {return Size;}
	uint_t GetItems() const {return nItems;}

	bool Insert(const void *key, uptr_t value = 0);
	bool Remove(const void *key);

	bool   Exists(const void *key) const;
	uptr_t Read(const void *key) const;

	const void *GetKey(const void *key) const;

	bool Traverse(bool (*fn)(const void *key, uptr_t value, void *context), void *context = NULL);
	bool TraverseMatch(const void *cmp, bool (*fn)(const void *key, uptr_t value, void *context), void *context = NULL);

protected:
	typedef struct _BUCKET {
		uint8_t *Key;
		uptr_t  Value;
		struct _BUCKET *Next;
	} BUCKET;

	BUCKET *CreateBucket(const void *key, uptr_t value);
	void DeleteBucket(BUCKET *p);

	uint_t Hash(const void *key) const;
	BUCKET *FindBucket(const void *key) const;
	BUCKET **FindBucketParent(const void *key) const;

protected:
	BUCKET **pBuckets;
	void   (*pFreeFunc)(uptr_t value, void *context);
	void   *pContext;
	uint_t StructSize;
	uint_t Size;
	uint_t nTraverseCount;
	uint_t nItems;
};


#endif
