
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rdlib/XMLDecode.h>

int main(int argc, char *argv[])
{
	int i;

	for (i = 1; i < argc; i++) {
		AString str = AString::ReadFile(argv[i]);

		debug("Decoding...\n");
		AStructuredNode xml;

		if (DecodeXML(xml, str)) {
			debug("Decode successful!\n");
		}
		else debug("Failed!\n");
	}
	
	return 0;
}
