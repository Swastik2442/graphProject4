#include <stdlib.h>

#include "graph.h"

void graphInit(Graph *g, int n)
{
    g->n = n;
    g->adj = (int **)malloc(n*n * sizeof(int *));
}

void graphDeinit(Graph *g)
{
    free(g->adj);
}

void addEdge(Graph *g, int u, int v);
void removeEdge(Graph *g, int u, int v);
int hasEdge(Graph *g, int u, int v);
