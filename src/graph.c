#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graph.h"

// Initialize a Graph with n vertices
void graphInit(Graph *g, int n, char *labels[n])
{
    if (n > MAX_VERTICES)
        return;

    g->n = n;

    if (labels == NULL) {
        char label[3];
        for (int i = 0; i < g->n; i++)
        {
            snprintf(label, 3, "%d", i);
            g->labels[i] = strdup(label);
        }
    }
    else {
        for (int i = 0; i < g->n; i++)
            g->labels[i] = strdup(labels[i]);
    }

    for (int i = 0; i < MAX_VERTICES; i++)
        for (int j = 0; j < MAX_VERTICES; j++)
            g->adj[i][j] = 0;
}

// Deinitialize a Graph
void graphDeinit(Graph *g)
{
    for (int i = 0; i < g->n; i++)
        free(g->labels[i]);
    g->n = 0;
}

// Add a Vertex to the Graph
int addVertex(Graph *g, char *label)
{
    if (g->n == MAX_VERTICES)
        return -1;

    g->labels[g->n] = strdup(label);
    g->n++;
    return 0;
}

// Edit a Vertex in the Graph
int editVertex(Graph *g, char *label, char *newLabel)
{
    for (int i = 0; i < g->n; i++)
        if (strcmp(g->labels[i], label) == 0)
        {
            free(g->labels[i]);
            g->labels[i] = strdup(newLabel);
            return 0;
        }
    return -1;
}

// Add an Edge to the Graph
int addEdge(Graph *g, char *labelU, char *labelV)
{
    return editEdge(g, labelU, labelV, 1);
}

// Edit an Edge in the Graph
int editEdge(Graph *g, char *labelU, char *labelV, int weight)
{
    if (strcmp(labelU, labelV) == 0)
        return -1;

    int u = -1, v = -1;
    for (int i = 0; i < g->n; i++)
    {
        if (strcmp(g->labels[i], labelU) == 0)
            u = i;
        if (strcmp(g->labels[i], labelV) == 0)
            v = i;
    }

    if (u == -1 || v == -1)
        return -1;

    g->adj[u][v] = weight;

    return 0;
}

// Remove an Edge from the Graph
int removeEdge(Graph *g, char *labelU, char *labelV)
{
    return editEdge(g, labelU, labelV, 0);
}

// Edit the Graph
int editGraph(Graph *g, int n, char *labels[n], int newAdj[n][n])
{
    graphDeinit(g);
    graphInit(g, n, labels);

    if (newAdj != NULL)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                g->adj[i][j] = newAdj[i][j];

    return 0;
}
