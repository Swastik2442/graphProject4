#include <stdio.h>
#include <string.h>

#include "../src/csv.h"

// CSV - Test Case 1
int test1()
{
    char *data[MAX_CSV_ROWS][MAX_CSV_COLS];
    int rows, cols;
    readCSV("test.csv", data, &rows, &cols);

    char *correctData[6][6] = {
        {"+", "A", "B", "C", "D", "E"},
        {"A", "0", "1", "0", "1", "0"},
        {"B", "1", "0", "1", "0", "1"},
        {"C", "0", "1", "0", "1", "0"},
        {"D", "1", "0", "1", "0", "1"},
        {"E", "0", "1", "0", "1", "0"}
    };

    int correct = 1;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (strcmp(data[i][j], correctData[i][j]) != 0)
            {
                correct = 0;
                break;
            }
        }
    }
    return correct;
}

int main(void)
{
    printf("CSV - Test 1 %s\n", test1() ? "PASSED" : "FAILED");
    return 0;
}
