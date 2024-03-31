#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../src/csv.h"
#include "../src/graph.h"

// CSV, Graph - Test Case 1
int test1()
{
    char *data[MAX_CSV_ROWS][MAX_CSV_COLS];
    int rows, cols;
    readCSV("test.csv", data, &rows, &cols);

    Graph g;

    graphInit(&g, rows-1, NULL);

    assert(g.n == rows-1);

    editGraph(&g, rows-1, data[0]+1, NULL);

    for (int i = 1; i < rows; i++) {
        assert(strcmp(g.labels[i-1], data[0][i]) == 0);
    }

    int adj[rows-1][rows-1];
    for (int i = 1; i < rows; i++)
    {
        for (int j = 1; j < rows; j++)
            adj[i-1][j-1] = strtol(data[i][j], NULL, 10);
    }

    editGraph(&g, rows-1, data[0]+1, adj);

    for (int i = 1; i < rows; i++)
    {
        for (int j = 1; j < rows; j++)
            assert(strtol(data[i][j], NULL, 10) == g.adj[i-1][j-1]);
    }

    graphDeinit(&g);
    return 1;
}

int main(void)
{
    printf("CSV, Graph - Test 1 %s\n", test1() ? "PASSED" : "FAILED");
    return 0;
}
