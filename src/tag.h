
#ifndef __TAG__
#define __TAG__

#include "misc.h"

typedef struct {
	uint32_t Tag;
	uptr_t   Value;
} TAG;

enum {
	TAG_DONE = 0,
	TAG_SUB_LIST,
	TAG_LINKED_LIST,
	_TAG_START,
};

extern uint_t TagCount(const TAG  *tags);
extern bool   TagExists(const TAG *tags, uint32_t tag);
extern uptr_t TagValue(const TAG  *tags, uint32_t tag, uptr_t defval = 0);
extern bool   TagReplace(TAG      *tags, uint32_t tag, uptr_t value);

#endif
