
#include <stdio.h>

#include "tag.h"

/* end of includes */

static const TAG *TagFind(const TAG *tags, uint32_t tag)
{
    while (tags && (tags->Tag != TAG_DONE) && (tags->Tag != tag)) {
        switch (tags->Tag) {
            case TAG_SUB_LIST:
                tags = TagFind((const TAG *)tags->Value, tag);
                break;
            case TAG_LINKED_LIST:
                tags = (const TAG *)tags->Value;
                break;
            default:
                tags++;
                break;
        }
    }
    return (tags && (tags->Tag == tag)) ? tags : NULL;
}

uint_t TagCount(const TAG *tags)
{
    uint_t n = 0;
    while (tags && (tags->Tag != TAG_DONE)) {
        switch (tags->Tag) {
            case TAG_SUB_LIST:
                n += TagCount((const TAG *)tags->Value);
                break;
            case TAG_LINKED_LIST:
                tags = (const TAG *)tags->Value;
                break;
            default:
                tags++;
                n++;
                break;
        }
    }
    return n + 1;
}
bool TagExists(const TAG *tags, uint32_t tag)
{
    return (TagFind(tags, tag) != NULL);
}

uptr_t TagValue(const TAG *tags, uint32_t tag, uptr_t defval)
{
    const TAG *tagp = TagFind(tags, tag);
    return tagp ? tagp->Value : defval;
}

bool TagReplace(TAG *tags, uint32_t tag, uptr_t value)
{
    TAG *tagp = (TAG *)TagFind(tags, tag);
    if (tagp) tagp->Value = value;
    return (tagp != NULL);
}
