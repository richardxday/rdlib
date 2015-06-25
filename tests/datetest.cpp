
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rdlib-0.1/rdlib/DateTime.h>

int main(int argc, char *argv[])
{
	int i;

	for (i = 1; i < argc; i++) {
		ADateTime dt;
		uint_t    specified = 0;

		dt.StrToDate(argv[i], false, &specified);
		printf("'%s' => '%s' (%u)\n", argv[i], dt.DateToStr().str(), specified);
	}

	return 0;
}
