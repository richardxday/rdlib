#ifndef __PRINT_TABLE__
#define __PRINT_TABLE__

#include <vector>

#include "strsup.h"

typedef std::vector<AString> TABLEROW;
typedef struct {
	std::vector<uint8_t>  justify;
	std::vector<int>      widths;
	std::vector<TABLEROW> rows;
	bool				  headerscentred;
} TABLE;

extern void UpdateTable(TABLE& table);
extern void PrintTable(AStdData& fp, TABLE& table);
extern void PrintTable(AStdData *fp, TABLE& table);
extern void PrintTable(TABLE& table);

#endif
