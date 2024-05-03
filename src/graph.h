#ifndef _GRAPH_H_
#define _GRAPH_H_

#include "priorityQueue.h"

#define MAX_VERTICES 64

typedef struct graph_t {
    int n;                               // Number of Vertices
    char *labels[MAX_VERTICES];          // Vertex Labels
    int adj[MAX_VERTICES][MAX_VERTICES]; // Adjacency Matrix
} Graph;

void graphInit(Graph *g, int n, char *labels[n]);
void graphDeinit(Graph *g);
int addVertex(Graph *g, char *label);
int editVertex(Graph *g, char *label, char *newLabel);
int addEdge(Graph *g, char *labelU, char *labelV);
int editEdge(Graph *g, char *labelU, char *labelV, int weight);
int removeEdge(Graph *g, char *labelU, char *labelV);
int editGraph(Graph *g, int n, char *labels[n], int newAdj[n][n]);

void dijkstra(Graph *g, char *label);
void djk(Graph *g, int u, int dist[], int prev[], priorityQueue *verticesWithDistance, int visited[]);

#endif
