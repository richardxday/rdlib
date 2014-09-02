
#include <stdio.h>

#include "StringList.h"

/* end of includes */

AStringList::AStringList()
{
	LockedStringList.EnableDuplication(true);
}

AStringList::~AStringList()
{
}

AString *AStringList::AddString(const AString& str, bool locked)
{
	AString *pstr = new AString(str);

	if (pstr) {
		StringList.Add(pstr);
		if (locked) LockedStringList.Add(pstr);

		ModifyString(pstr);
	}

	return pstr;
}

bool AStringList::RemoveString(AString *str)
{
	bool member = StringList.IsMember(str);
	
	if (member) {
		ModifyString(str);
		LockedStringList.Remove(str);
		StringList.Remove(str);
	}
	
	return member;
}

uint_t AStringList::LimitStrings(uint_t n)
{
	uint_t nremoved = 0;

	AString *str = (AString *)StringList.First();
	while (str && (StringList.Count() > (sint_t)n)) {
		while (str && (LockedStringList.Find(str) >= 0)) str = str->Next();

		if (!str) break;

		AString *str1 = str->Next();
		StringList.Remove(str);

		nremoved++;
		str = str1;
	}

	return nremoved;
}
