
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rdlib/StdFile.h>

int main(void)
{
	AStdFile fp;

	if (fp.open("formattest.txt")) {
		AString line;
		uint_t passes = 0, failures = 0.0;
		uint_t ln = 1;
		
		while (line.ReadLn(fp) >= 0) {
			if (line.Words(0).Valid()) {
				AString type   = line.Word(0);
				AString val    = line.Word(1);
				AString format = line.Words(2).Line(0, "=", 0);
				AString result = line.Words(2).Line(1, "=", 0);
				AString res;
			
				if		(type == "s") res = format.Arg((sint64_t)val);
				else if (type == "u") res = format.Arg((uint64_t)val);
				else if (type == "f") res = format.Arg((double)val);
				
				if (res == result) passes++;
				else {
					printf("'%s' != '%s' (line %u)\n", res.str(), result.str(), ln);
					failures++;
				}
			}

			ln++;
		}
		
		fp.close();

		printf("%u passes, %u failures\n", passes, failures);
	}
	
	return 0;
}
