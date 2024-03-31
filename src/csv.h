#ifndef _CSV_H_
#define _CSV_H_

#define MAX_CSV_ROWS 128
#define MAX_CSV_COLS 128

void readCSV(const char *filename, char *data[MAX_CSV_ROWS][MAX_CSV_COLS], int *rows, int *cols);

#endif
