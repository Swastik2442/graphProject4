#include "csv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Read CSV File and store records in a 2D Array of Strings
void readCSV(const char *filename, char *data[MAX_CSV_ROWS][MAX_CSV_COLS], int *rows, int *cols)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        return;
    }

    char line[1024];
    int i = 0;
    while (fgets(line, 1024, fp) && i < MAX_CSV_ROWS)
    {
        int j = 0;
        char *tmp = strdup(line);
        for (char *tok = strtok(line, ","); tok && *tok && j < MAX_CSV_COLS; tok = strtok(NULL, ",\n"))
        {
            data[i][j] = strdup(tok);
            j++;
        }
        free(tmp);
        i++;
    }
    *rows = i;
    *cols = i > 0 ? i : 0;
    fclose(fp);
}
