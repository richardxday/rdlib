
#include <stdio.h>
#include <ctype.h>

#include "Hash.h"

/* end of includes */

AHash::AHash(void (*fn)(uptr_t value, void *context), void *context) : std::map<AString, uptr_t>(),
																	   pDestructor(fn),
																	   pDestructorContext(context),
																	   nTraverseCount(0),
																	   bCaseInsensitive(false)
{
}

AHash::AHash(const AHash& hash) : std::map<AString, uptr_t>(),
								  pDestructor(hash.pDestructor),
								  pDestructorContext(hash.pDestructorContext),
								  nTraverseCount(0),
								  bCaseInsensitive(hash.bCaseInsensitive)
{
	Copy(hash);
}

AHash::~AHash()
{
	Delete();
}

void AHash::Delete()
{
	assert(!nTraverseCount);

	if (pDestructor) {
		AHash::iterator it;

		for (it = begin(); it != end(); ++it) {
			(*pDestructor)(it->second, pDestructorContext);
		}
	}
	
	clear();
}

void AHash::Insert(const AString& key, uptr_t value)
{
	AHash::iterator it;
	AString truekey = bCaseInsensitive ? key.ToLower() : key;

	assert(!nTraverseCount);

	if (pDestructor && ((it = find(truekey)) != end())) {
		(*pDestructor)(it->second, pDestructorContext);
	}
	
	(*(std::map<AString, uptr_t> *)this)[truekey] = value;
}

bool AHash::Remove(const AString& key)
{
	AHash::iterator it;
	AString truekey = bCaseInsensitive ? key.ToLower() : key;
	bool    success = false;
	
	assert(!nTraverseCount);

	if ((it = find(truekey)) != end()) {
		if (pDestructor) (*pDestructor)(it->second, pDestructorContext);

		erase(it);
		
		success = true;
	}

	return success;
}

bool AHash::Exists(const AString& key) const
{
	AString truekey = bCaseInsensitive ? key.ToLower() : key;
	return (find(truekey) != end());
}

uptr_t AHash::Read(const AString& key) const
{
	AHash::const_iterator it;
	AString truekey = bCaseInsensitive ? key.ToLower() : key;
	uptr_t  value   = 0;
	
	if ((it = find(truekey)) != end()) {
		value = it->second;
	}

	return value;		
}

bool AHash::Traverse(bool (*fn)(const AString& key, uptr_t value, void *context), void *context)
{
	AHash::iterator it;
	bool success = false;

	nTraverseCount++;

	success = true;
	for (it = begin(); success && (it != end()); ++it) {
		success = (*fn)(it->first, it->second, context);
	}

	nTraverseCount--;

	return success;
}

bool AHash::TraverseCompare(const AString& cmp, int l, bool (*fn)(const AString& key, uptr_t value, void *context), void *context)
{
	AHash::iterator it;
	AString truekey = bCaseInsensitive ? cmp.ToLower() : cmp;
	bool    success = false;

	nTraverseCount++;

	if (l < 0) l = truekey.len();
	
	success = true;
	for (it = begin(); success && (it != end()); ++it) {
		if (CompareCaseN(truekey, it->first, l) == 0) {
			success = (*fn)(it->first, it->second, context);
		}
	}

	nTraverseCount--;

	return success;
}

bool AHash::__CopyKey(const AString& key, uptr_t value, void *context)
{
	((AHash *)context)->Insert(key, value);
	return true;
}

bool AHash::Copy(const AHash& src)
{
	return ((AHash&)src).Traverse(&__CopyKey, this);
}

bool AHash::CopyCompare(const AHash& src, const AString& cmp, int l)
{
	return ((AHash&)src).TraverseCompare(cmp, l, &__CopyKey, this);
}
