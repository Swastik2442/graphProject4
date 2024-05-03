#include "threading.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "utils.h"

// Thread Function to read Graph CSV File
//----------------------------------------------------------------------------------
void *csvThread(void *arg)
{
    // TODO: Add Error Handling
    csvThreadData *data = (csvThreadData *)arg;
    *(data->status) = IN_PROGRESS;

    char *csvData[MAX_CSV_ROWS][MAX_CSV_COLS];
    int csvRows, csvCols, csvN;
    readCSV(data->csvFilePath, csvData, &csvRows, &csvCols);

    csvN = (csvRows <= csvCols) ? csvRows : csvCols;
    int adj[csvN-1][csvN-1];
    for (int i = 1; i < csvN; i++)
        for (int j = 1; j < csvN; j++)
            adj[i-1][j-1] = strtol(csvData[i][j], NULL, 10);

    *(data->pointCount) = csvN-1;
    editGraph(data->theGraph, *(data->pointCount), csvData[0]+1, adj);
    createPointPolygon(data->points, *(data->pointCount), data->polygonCenter, data->polygonRadius);

    TextCopy(data->srcLabel, data->theGraph->labels[0]);
    TextCopy(data->destLabel, data->theGraph->labels[data->theGraph->n - 1]);

    *(data->status) = COMPLETED;
    return NULL;
}
//----------------------------------------------------------------------------------

// Thread Function to perform Dijkstra's Algorithm
//----------------------------------------------------------------------------------
void *dijkstraThread(void *arg)
{
    dijkstraThreadData *data = (dijkstraThreadData *)arg;
    *(data->status) = IN_PROGRESS;

    // Search for Source and Destination Vertices
    *(data->current) = -1;
    *(data->src) = -1;
    *(data->dest) = -1;
    for (int i = 0; i < data->theGraph->n; i++)
    {
        if (strcmp(data->theGraph->labels[i], data->srcLabel) == 0)
            *(data->src) = i;
        if (strcmp(data->theGraph->labels[i], data->destLabel) == 0)
            *(data->dest) = i;
        if (*(data->src) != -1 && *(data->dest) != -1)
            break;
    }

    // If Source or Destination Vertex not Found
    if (*(data->src) == -1 || *(data->dest) == -1)
    {
        *(data->status) = COMPLETED;
        *(data->animationActive) = false;
        return NULL;
    }

    // Initialize all Variables
    pQueueInit(data->pQueue);
    *(data->pathHead) = MAX_VERTICES;

    data->distance[*(data->src)] = 0;
    data->previous[*(data->src)] = -1;
    pQueueInsert(data->pQueue, *(data->src), 0);
    for (int i = 0; i < data->theGraph->n; i++)
    {
        data->visited[i] = false;
        if (i != *(data->src))
        {
            data->distance[i] = INT_MAX;
            data->previous[i] = -1;
            pQueueInsert(data->pQueue, i, INT_MAX);
        }
    }
    int distCheck;

    delay(2);

    // Algorithm Starts
    // Search all Vertices for Shortest Path
    while (data->pQueue->filled > 0)
    {
        *(data->current) = pQueueExtractMin(data->pQueue);
        data->visited[*(data->current)] = true;
        *(data->childrenTop) = 0;

        for (int v = 0; v < data->theGraph->n; v++)
        {
            distCheck = data->distance[*(data->current)] + data->theGraph->adj[*(data->current)][v];
            if (!data->visited[v] && data->theGraph->adj[*(data->current)][v] && data->distance[v] > distCheck)
            {
                data->distance[v] = distCheck;
                data->previous[v] = *(data->current);
                data->children[(*(data->childrenTop))++] = v;
                pQueueDecreaseKey(data->pQueue, v, data->distance[v]);
            }
        }

        delay(1);
    }

    delay(1);

    // Find Shortest Path
    *(data->current) = *(data->dest);
    data->path[--(*(data->pathHead))] = *(data->dest);
    while (*(data->current) != *(data->src))
    {
        *(data->current) = data->previous[*(data->current)];
        if (*(data->current) == -1)
            break;

        data->path[--(*(data->pathHead))] = *(data->current);
    }

    if (*(data->current) == *(data->src))
        delay(3);
    // Algorithm Ends

    delay(2);
    pQueueDeinit(data->pQueue);

    *(data->status) = COMPLETED;
    *(data->animationActive) = false;
    return NULL;
}
//----------------------------------------------------------------------------------
