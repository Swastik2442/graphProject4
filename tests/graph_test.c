#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../src/graph.h"

Graph g;

// Graph - Test Case 1
int test1()
{
    char *labels[] = {"A", "B", "C", "D", "E", "F"};
    graphInit(&g, 6, labels);

    assert(g.n == 6);
    for (int i = 0; i < g.n; i++)
        assert(strcmp(g.labels[i], labels[i]) == 0);

    addVertex(&g, "G");
    addVertex(&g, "H");
    addVertex(&g, "I");

    assert(g.n == 9);

    for (int i = 0; i < g.n-3; i++)
        assert(strcmp(g.labels[i], labels[i]) == 0);

    assert(strcmp(g.labels[6], "G") == 0);
    assert(strcmp(g.labels[7], "H") == 0);
    assert(strcmp(g.labels[8], "I") == 0);
    
    editVertex(&g, "G", "G1");
    editVertex(&g, "H", "H1");
    editVertex(&g, "I", "I1");

    assert(strcmp(g.labels[6], "G1") == 0);
    assert(strcmp(g.labels[7], "H1") == 0);
    assert(strcmp(g.labels[8], "I1") == 0);

    removeVertex(&g, "G1");
    removeVertex(&g, "H1");
    removeVertex(&g, "I1");

    assert(g.n == 6);

    graphDeinit(&g);

    assert(g.n == 0);
    for (int i = 0; i < g.n; i++)
        assert(g.labels[i] == NULL);

    return 1;
}

// Graph - Test Case 2
int test2()
{
    graphInit(&g, 5, NULL);

    assert(g.n == 5);

    addEdge(&g, "0", "1");
    addEdge(&g, "1", "4");
    addEdge(&g, "4", "3");

    assert(g.adj[0][1] == 1);
    assert(g.adj[1][0] == 0);
    assert(g.adj[1][4] == 1);
    assert(g.adj[4][1] == 0);
    assert(g.adj[4][3] == 1);
    assert(g.adj[3][4] == 0);

    editEdge(&g, "0", "1", 5);
    editEdge(&g, "1", "4", 4);
    editEdge(&g, "4", "3", 3);

    assert(g.adj[0][1] == 5);
    assert(g.adj[1][0] == 0);
    assert(g.adj[1][4] == 4);
    assert(g.adj[4][1] == 0);
    assert(g.adj[4][3] == 3);
    assert(g.adj[3][4] == 0);

    removeEdge(&g, "0", "1");
    removeEdge(&g, "1", "4");
    removeEdge(&g, "4", "3");

    assert(g.adj[0][1] == 0);
    assert(g.adj[1][0] == 0);
    assert(g.adj[1][4] == 0);
    assert(g.adj[4][1] == 0);
    assert(g.adj[4][3] == 0);
    assert(g.adj[3][4] == 0);

    graphDeinit(&g);
    return 1;
}

int test3()
{
    graphInit(&g, 3, NULL);

    assert(g.n == 3);

    int adj[5][5] = {
        {0, 1, 0, 1, 0},
        {1, 0, 1, 0, 1},
        {0, 1, 0, 1, 0},
        {1, 0, 1, 0, 1},
        {0, 1, 0, 1, 0}
    };

    editGraph(&g, 5, NULL, adj);

    assert(g.n == 5);

    for (int i = 0; i < g.n; i++)
    {
        for (int j = 0; j < g.n; j++)
            assert(g.adj[i][j] == adj[i][j]);
    }

    graphDeinit(&g);
    return 1;
}

int main(void)
{
    printf("Graph - Test 1 %s\n", test1() ? "PASSED" : "FAILED");
    printf("Graph - Test 2 %s\n", test2() ? "PASSED" : "FAILED");
    printf("Graph - Test 3 %s\n", test3() ? "PASSED" : "FAILED");
    return 0;
}
