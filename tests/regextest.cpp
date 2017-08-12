
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rdlib/Regex.h>

int main(void)
{
	AString pat = ParseRegex("{#[a-z]}{#[0-9]}");
	static struct {
		const char *str;
		bool match;
	} tests[] = {
		{"ggggfgdgd3543543", true},
		{"3543543ggggfgdgd", false},
		{"3543543ggggfgdgd4321", false},
	};
	uint_t i, npasses = 0, nfailures = 0;

	for (i = 0; i < NUMBEROF(tests); i++) {
		const REGEXREGION *reg;
		ADataList regionlist;
		
		if (MatchRegex(tests[i].str, pat, regionlist) == tests[i].match) {
			if (tests[i].match) {
				if (regionlist.Count() == 2) {
					if (((reg = (const REGEXREGION *)regionlist[0]) != NULL) && (reg->len > 0)) {
						if (((reg = (const REGEXREGION *)regionlist[1]) != NULL) && (reg->len > 0)) {
							npasses++;
						}
						else {
							printf("Test %u region 1 has zero length\n", i);
							nfailures++;
						}
					}
					else {
						printf("Test %u region 0 has zero length\n", i);
						nfailures++;
					}
				}
				else {
					printf("Test %u found %u regions, not 2\n", i, regionlist.Count());
					nfailures++;
				}
			}
			else npasses++;
		}
		else {
			printf("Test %u match failed\n", i);
			nfailures++;
		}
	}

	printf("%u passes, %u failures\n", npasses, nfailures);
	
	return 0;
}
