#ifndef _GRAPH_H_
#define _GRAPH_H_

#define MAX_VERTICES 64

typedef struct graph_t {
    int n;
    char *labels[MAX_VERTICES];
    int adj[MAX_VERTICES][MAX_VERTICES];
} Graph;

void graphInit(Graph *g, int n, char *labels[n]);
void graphDeinit(Graph *g);
int addVertex(Graph *g, char *label);
int editVertex(Graph *g, char *label, char *newLabel);
int addEdge(Graph *g, char *labelU, char *labelV);
int editEdge(Graph *g, char *labelU, char *labelV, int weight);
int removeEdge(Graph *g, char *labelU, char *labelV);
int editGraph(Graph *g, int n, char *labels[n], int newAdj[n][n]);

void breadthFirstSearch(Graph *g, char *label);
void bfs(Graph *g, int u, int visited[]);
void depthFirstSearch(Graph *g, char *label);
void dfs(Graph *g, int u, int visited[]);

#endif
