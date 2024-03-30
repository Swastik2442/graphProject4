#ifndef _GRAPH_H_
#define _GRAPH_H_

typedef struct graph_t {
    int n;
    int **adj;
} Graph;

// Initialize a Graph with n vertices
void graphInit(Graph *g, int n);

// Deinitialize a Graph
void graphDeinit(Graph *g);

// Add an Edge between two Vertices
void addEdge(Graph *g, int u, int v);

// Remove an Edge between two Vertices
void removeEdge(Graph *g, int u, int v);

// Check if there is an Edge between two Vertices
int hasEdge(Graph *g, int u, int v);

#endif
