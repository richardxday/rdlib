
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rdlib/StdFile.h>
#include <rdlib/DateTime.h>

int main(int argc, char *argv[])
{
	const AString format = "%d %Y-%M-%D %h:%m:%s.%S";
	AStdFile fp;
	int errors = 0, passes = 0;
	
	if (fp.open("tests.txt")) {
		ADateTime a("min"), b("min");
		AString line;

		while (line.ReadLn(fp) >= 0) {
			int p;

			if ((p = line.Pos("=")) >= 0) {
				a.StrToDate(line.Left(p),    ADateTime::Time_Existing);
				b.StrToDate(line.Mid(p + 1), ADateTime::Time_Existing);

				if (a != b) {
					printf("%s != %s ('%s') ****\n", a.DateFormat(format).str(), b.DateFormat(format).str(), line.str());
					errors++;
				}
				else {
					printf("%s == %s ('%s')\n", a.DateFormat(format).str(), b.DateFormat(format).str(), line.str());
					passes++;
				}
			}
		}

		fp.close();
	}

	printf("%d passes, %d errors\n", passes, errors);

	ADateTime dt("min");

	ADateTime::EnableDebugStrToDate();
	
	int i;
	for (i = 1; i < argc; i++) {
		dt.StrToDate(argv[i], ADateTime::Time_Existing);
		printf("'%s' == %s\n", argv[i], dt.DateFormat(format).str());
	}

	return errors;
}
