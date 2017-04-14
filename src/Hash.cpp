
#include <stdio.h>
#include <ctype.h>

#include "Hash.h"

/* end of includes */

AHash::AHash(uint_t size, void (*freefunc)(uptr_t value, void *context), void *context) : pBuckets(NULL),
																						  pCompFunc(&strcmp),
																						  pCompNFunc(&strncmp),
																						  pFreeFunc(NULL),
																						  pContext(NULL),
																						  Size(0),
																						  nTraverseCount(0),
																						  nItems(0)
{
	if (size) Create(size, freefunc, context);
}

AHash::AHash(const AHash& hash) : pBuckets(NULL),
								  pCompFunc(hash.pCompFunc),
								  pCompNFunc(hash.pCompNFunc),
								  pFreeFunc(NULL),
								  pContext(NULL),
								  Size(0),
								  nTraverseCount(0),
								  nItems(0)
{
	if (hash.Size) {
		Create(hash.Size, hash.pFreeFunc, hash.pContext);
		Copy(hash);
	}
}

AHash::~AHash()
{
	Delete();
}

bool AHash::Create(uint_t size, void (*freefunc)(uptr_t value, void *context), void *context)
{
	bool  ok = false;

	assert(!nTraverseCount);

	if (!pBuckets && size) {
		Size = size;

		if ((pBuckets = new BUCKET *[Size]) != NULL) {
			memset(pBuckets, 0, Size * sizeof(BUCKET *));

			ok = true;
		}

		pFreeFunc = freefunc;
		pContext  = context;
	}

	return ok;
}

void AHash::Delete()
{
	assert(!nTraverseCount);

	if (pBuckets) {
		uint_t i;
		for (i = 0; i < Size; i++) {
			if (pBuckets[i]) DeleteBucket(pBuckets[i]);
		}

		delete[] pBuckets;
		pBuckets = NULL;
	}
	pFreeFunc = NULL;
	pContext  = NULL;
	Size      = 0;
	nItems    = 0;
}

AHash::BUCKET *AHash::CreateBucket(const char *key, uptr_t value)
{
	BUCKET *p;

	if ((p = new BUCKET) != NULL) {
		memset(p, 0, sizeof(*p));

		p->Key   = strdup(key);
		p->Value = value;

		nItems++;
	}

	return p;
}

void AHash::DeleteBucket(BUCKET *p)
{
	while (p) {
		BUCKET *p1 = p->Next;

		if (p->Key)    free(p->Key);
		if (pFreeFunc) (*pFreeFunc)(p->Value, pContext);

		delete p;

		p = p1;

		nItems--;
	}
}

uint_t AHash::Hash(const char *p) const
{
	uint32_t val = 5381;
	uint_t i;

	if (pCompFunc == &stricmp) {
		for (i = 0; p[i]; i++) {
			val = (val * 33) + tolower(p[i]);
		}
	}
	else {
		for (i = 0; p[i]; i++) {
			val = (val * 33) + p[i];
		}
	}

	return (uint_t)(val % Size);
}

AHash::BUCKET *AHash::FindBucket(const char *key) const
{
	BUCKET *p = NULL;

	if (pBuckets && Size) {
		uint_t val = Hash(key);

		p = pBuckets[val];
		while (p && ((*pCompFunc)(p->Key, key) != 0)) p = p->Next;
	}

	return p;
}

AHash::BUCKET **AHash::FindBucketParent(const char *key) const
{
	BUCKET **p = NULL;

	if (pBuckets && Size) {
		uint_t val = Hash(key);

		p = pBuckets + val;
		
		if (p[0] && ((*pCompFunc)(p[0]->Key, key) != 0)) {
			while (p[0]->Next) {
				p = &p[0]->Next;
				if ((*pCompFunc)(p[0]->Key, key) == 0) break;
			}
		}
	}

	return p;
}

bool AHash::Insert(const char *key, uptr_t value)
{
	BUCKET **p = FindBucketParent(key);
	bool   ok  = false;

	assert(!nTraverseCount);

	if (p) {
		if (p[0] && ((*pCompFunc)(p[0]->Key, key) == 0)) {
			uptr_t oldvalue = p[0]->Value;
			
			p[0]->Value = value;
			
			if (pFreeFunc) (*pFreeFunc)(oldvalue, pContext);

			ok = true;
		}
		else {
			while (p[0]) p = &p[0]->Next;

			p[0] = CreateBucket(key, value);

			ok = (p[0] != NULL);
		}
	}

	return ok;
}

bool AHash::Remove(const char *key)
{
	BUCKET **p = FindBucketParent(key);
	bool   success = false;

	assert(!nTraverseCount);

	if (p && p[0] && ((*pCompFunc)(p[0]->Key, key) == 0)) {
		BUCKET *p1 = p[0];

		p[0] = p[0]->Next;
		p1->Next = NULL;

		DeleteBucket(p1);

		success = true;
	}

	return success;
}

bool AHash::Exists(const char *key) const
{
	return (FindBucket(key) != NULL);
}

uptr_t AHash::Read(const char *key) const
{
	BUCKET *p = FindBucket(key);
	return p ? p->Value : 0;
}

const char *AHash::GetKey(const char *key) const
{
	BUCKET *p = FindBucket(key);
	return p ? p->Key : NULL;
}

bool AHash::Resize(uint_t size)
{
	BUCKET **buckets = pBuckets, **buckets1;
	uint_t i, n = Size;
	bool   success = false;
	
	if ((buckets1 = new BUCKET *[size]) != NULL) {
		pBuckets = buckets1;
		Size     = size;

		for (i = 0; i < n; i++) {
			BUCKET *p = buckets[i];

			while (p) {
				BUCKET *p1 = p->Next;

				Insert(p->Key, p->Value);

				delete p;

				p = p1;
			}
		}

		delete[] buckets;

		success = true;
	}

	return success;
}

bool AHash::Traverse(bool (*fn)(const char *key, uptr_t value, void *context), void *context)
{
	bool success = false;

	if (pBuckets && Size) {
		uint_t i;

		nTraverseCount++;

		success = true;
		for (i = 0; i < Size; i++) {
			BUCKET *p = pBuckets[i];

			while (p && success) {
				success = (*fn)(p->Key, p->Value, context);
				p = p->Next;
			}

			if (!success) break;
		}

		nTraverseCount--;
	}

	return success;
}

bool AHash::TraverseCompare(const char *cmp, int l, bool (*fn)(const char *key, uptr_t value, void *context), void *context)
{
	bool success = false;

	if (pBuckets && Size) {
		uint_t i;

		nTraverseCount++;

		if (l <= 0) l = strlen(cmp);

		success = true;
		for (i = 0; i < Size; i++) {
			BUCKET *p = pBuckets[i];

			while (p && success) {
				if ((*pCompNFunc)(p->Key, cmp, l) == 0) {
					success = (*fn)(p->Key, p->Value, context);
				}
				p = p->Next;
			}

			if (!success) break;
		}

		nTraverseCount--;
	}

	return success;
}

bool AHash::__CopyKey(const char *key, uptr_t value, void *context)
{
	return ((AHash *)context)->Insert(key, value);
}

bool AHash::Copy(const AHash& src)
{
	return ((AHash&)src).Traverse(&__CopyKey, this);
}

bool AHash::CopyCompare(const AHash& src, const char *cmp, int l)
{
	return ((AHash&)src).TraverseCompare(cmp, l, &__CopyKey, this);
}
