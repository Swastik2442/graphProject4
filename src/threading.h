#ifndef _THREADING_H_
#define _THREADING_H_

#include "raylib.h"

#include "csv.h"
#include "graph.h"
#include "priorityQueue.h"

// Thread Status
typedef enum threadStatus {
    NOT_STARTED = 0, // Thread has not started yet
    IN_PROGRESS,     // Thread is in progress
    COMPLETED        // Thread has completed execution
} ThreadStatus;

//------------------------------------------------------------------------------------
// Thread Data Structures

typedef struct csvThreadData
{
    ThreadStatus *status;  // Thread Status
    char *csvFilePath;     // CSV File Path
    Vector2 *points;       // Points Array
    int *pointCount;       // Point Count
    Vector2 polygonCenter; // Polygon Center
    int polygonRadius;     // Polygon Radius
    Graph *theGraph;       // Graph
    char *srcLabel;        // Source Label
    char *destLabel;       // Destination Label
} csvThreadData;

typedef struct dijkstraThreadData
{
    ThreadStatus *status;  // Thread Status
    Graph *theGraph;       // Graph
    char *srcLabel;        // Source Label
    char *destLabel;       // Destination Label
    int *current;          // Current Vertex
    int *src;              // Source Vertex
    int *dest;             // Destination Vertex
    int *children;         // Children Array
    int *childrenTop;      // Children Top Index
    int *distance;         // Distance Array
    int *previous;         // Previous Array
    priorityQueue *pQueue; // Priority Queue
    bool *visited;         // Visited Array
    int *path;             // Path Array
    int *pathHead;         // Path Head Index
    bool *animationActive; // Animation Active Flag
} dijkstraThreadData;

void *csvThread(void *arg);
void *dijkstraThread(void *arg);

#endif
