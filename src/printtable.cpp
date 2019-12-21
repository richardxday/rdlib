
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "printtable.h"

void UpdateTable(TABLE& table)
{
    size_t col, row, maxcol = 0;

    for (row = 0; row < table.rows.size(); row++) {
        maxcol = std::max(maxcol, table.rows[row].size());
    }

    table.widths.resize(maxcol);

    for (col = 0; col < maxcol; col++) {
        table.widths[col] = 0;
    }

    for (row = 0; row < table.rows.size(); row++) {
        for (col = 0; col < table.rows[row].size(); col++) {
            table.widths[col] = std::max(table.widths[col], table.rows[row][col].len());
        }
    }
}

void PrintTable(AStdData& fp, TABLE& table)
{
    size_t col, row;

    UpdateTable(table);

    table.justify.resize(table.widths.size());

    for (row = 0; row < table.rows.size(); row++) {
        for (col = 0; col < table.widths.size(); col++) {
            if (col < table.rows[row].size()) {
                int justify = (table.headerscentred && (row == 0)) ? 1 : (int)table.justify[col];
                int left  = ((table.widths[col] - table.rows[row][col].len()) * justify) / 2;
                int right = table.widths[col] - table.rows[row][col].len() - left;

                fp.printf("| %s ", (AString(" ").Copies(left) + table.rows[row][col] + AString(" ").Copies(right)).str());
            }
            else {
                fp.printf("| %s ", AString(" ").Copies(table.widths[col]).str());
            }
        }
        fp.printf("|\n");
    }
}

void PrintTable(AStdData *fp, TABLE& table)
{
    PrintTable(*fp, table);
}

void PrintTable(TABLE& table)
{
    PrintTable(*Stdout, table);
}
