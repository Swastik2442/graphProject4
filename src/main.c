#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <pthread.h>
#include <signal.h>

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "graph.h"
#include "csv.h"

#define MAX_POINTS 64

// Screen Size

const int screenWidth = 1280;
const int screenHeight = 720;
const int halfScreenWidth = screenWidth / 2;
const int halfScreenHeight = screenHeight / 2;

// Constants

const int polygonRadius = 200;
const int bfsOffset = halfScreenHeight - 12;
const int dfsOffset = halfScreenHeight + 24;
const int dijkstraOffset = halfScreenHeight - 48;

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
    ThreadStatus *status; // Thread Status
    char *csvFilePath;    // CSV File Path
    Vector2 *points;      // Points Array
    int *pointCount;      // Point Count
    Graph *theGraph;      // Graph
    char *bfsLabel;       // BFS Label
    char *dfsLabel;       // DFS Label
    char *dijkstraLabel;  // Dijkstra Label
} csvThreadData;

typedef struct bfsThreadData
{
    ThreadStatus *status;  // Thread Status
    Graph *theGraph;       // Graph
    char *startLabel;      // Starting Label
    bool *visited;         // Visited Array
    int *queue;            // BFS Queue
    int *qFront;           // Queue Front Index
    int *qRear;            // Queue Rear Index
    bool *animationActive; // Animation Active Flag
} bfsThreadData;

typedef struct dfsThreadData
{
    ThreadStatus *status;  // Thread Status
    Graph *theGraph;       // Graph
    char *startLabel;      // Starting Label
    bool *visited;         // Visited Array
    int *stack;            // DFS Stack
    int *sTop;             // Stack Top Index
    bool *animationActive; // Animation Active Flag
} dfsThreadData;

//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// Utility Functions

void createPointPolygon(
    Vector2 *points, // Points Array
    int pointCount,  // Point Count
    int radius       // Polygon Radius
);

void *csvThread(void *arg);
void *bfsThread(void *arg);
void *dfsThread(void *arg);
void *dijkstraThread(void *arg);
//------------------------------------------------------------------------------------

// Scene Types
typedef enum sceneType {
    START_MENU = 0,      // Start Menu
    MAIN_SCENE,          // Main Scene
} SceneType;

//------------------------------------------------------------------------------------
// All the Scenes

void startMenu(
    SceneType *currentScene, // Current Scene
    bool *exitWindow         // Exit Window Flag
);
void mainScene(
    Vector2 *points,                   // Points Array
    Graph *theGraph,                   // Graph
    int pointCount,                    // Point Count
    int *focusedPoint,                 // Focused Point
    SceneType *currentScene,           // Current Scene
    bool *bfsActive,                   // BFS Active Flag
    bool *dfsActive,                   // DFS Active Flag
    bool *dijkstraActive,              // Dijkstra Active Flag
    char *bfsLabel,                    // BFS Label
    char *dfsLabel,                    // DFS Label
    char *dijkstraLabel,               // Dijkstra Label
    bfsThreadData *bfsData,            // BFS Thread Data
    dfsThreadData *dfsData,            // DFS Thread Data
    bool *adjacencyMatrixWindowActive, // Adjacency Matrix Window Active Flag
    float *edgeThickness,              // Edge Thickness
    bool debugInfoActive               // Debug Information Active Flag
);
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// Program Main Entry Point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Graph Project 4 - Output and Display");

    // Scene Configuration

    SceneType currentScene = START_MENU;
    bool exitWindow = false;
    bool debugInfoActive = false;

    // Points Configuration

    int pointCount = 6;
    int selectedPoint = -1;
    int focusedPoint = -1;

    Vector2 points[MAX_POINTS];
    createPointPolygon(points, pointCount, polygonRadius);

    // Graph Configuration

    Graph theGraph;
    {
        char *labels[] = {"A", "B", "C", "D", "E", "F"};
        graphInit(&theGraph, 6, labels);
        addEdge(&theGraph, "A", "B");
        addEdge(&theGraph, "B", "C");
        addEdge(&theGraph, "C", "D");
        addEdge(&theGraph, "D", "E");
        addEdge(&theGraph, "E", "F");
        addEdge(&theGraph, "F", "A");
    }
    float edgeThickness = 4.0f;
    bool visitedVertices[MAX_POINTS];
    int dsArray[MAX_POINTS];
    int qFront, qRear, stackTop;

    char *csvFilePath = (char *)RL_CALLOC(4096, 1);
    char *bfsLabel = (char *)RL_CALLOC(4096, 1);
    char *dfsLabel = (char *)RL_CALLOC(4096, 1);
    char *dijkstraLabel = (char *)RL_CALLOC(4096, 1);

    TextCopy(csvFilePath, "Drop a CSV File Here");
    TextCopy(bfsLabel, theGraph.labels[0]);
    TextCopy(dfsLabel, theGraph.labels[0]);
    TextCopy(dijkstraLabel, theGraph.labels[0]);

    // Windows Configuration

    bool adjacencyMatrixWindowActive = false;
    bool bfsActive = false;
    bool dfsActive = false;
    bool dijkstraActive = false;

    // Threading Configuration

    pthread_t csvThreadID = PTHREAD_ONCE_INIT;
    pthread_t bfsThreadID = PTHREAD_ONCE_INIT;
    pthread_t dfsThreadID = PTHREAD_ONCE_INIT;
    pthread_t dijkstraThreadID = PTHREAD_ONCE_INIT;

    ThreadStatus csvStatus = NOT_STARTED;
    ThreadStatus bfsStatus = NOT_STARTED;
    ThreadStatus dfsStatus = NOT_STARTED;

    csvThreadData csvData = {&csvStatus, csvFilePath, points, &pointCount, &theGraph, bfsLabel, dfsLabel, dijkstraLabel};
    bfsThreadData bfsData = {&bfsStatus, &theGraph, bfsLabel, visitedVertices, dsArray, &qFront, &qRear, &bfsActive};
    dfsThreadData dfsData = {&dfsStatus, &theGraph, dfsLabel, visitedVertices, dsArray, &stackTop, &dfsActive};

    // Set custom GUI Style

    GuiSetStyle(DEFAULT, TEXT_SIZE, 14);
    GuiSetStyle(DEFAULT, TEXT_SPACING, 2);

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main Loop
    while (!exitWindow)    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        
        if (WindowShouldClose()) exitWindow = true;

        switch (currentScene)
        {
            case START_MENU:
            {
                if (IsKeyPressed(KEY_ENTER))
                    currentScene = MAIN_SCENE;
                break;
            }
            case MAIN_SCENE:
            {
                // Point Creation Logic
                if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && (pointCount < MAX_POINTS) && (focusedPoint == -1) && (!adjacencyMatrixWindowActive))
                {
                    if (addVertex(&theGraph, (char *)TextFormat("%c", 65 + pointCount)) == 0)
                    {
                        points[pointCount] = GetMousePosition();
                        pointCount++;
                    }
                }

                // Point Focus and Selection Logic
                for (int i = 0; i < pointCount; i++)
                {
                    if (CheckCollisionPointCircle(GetMousePosition(), points[i], 24.0f))
                    {
                        focusedPoint = i;
                        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) selectedPoint = i; 
                        break;
                    }
                    else focusedPoint = -1;
                }

                // Point Movement Logic
                if (selectedPoint >= 0)
                {
                    points[selectedPoint] = GetMousePosition();
                    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) selectedPoint = -1;
                }

                // File Drop Logic
                if (IsFileDropped())
                {
                    FilePathList droppedFiles = LoadDroppedFiles();

                    if ((droppedFiles.count > 0) && IsFileExtension(droppedFiles.paths[0], ".csv"))
                    {
                        TextCopy(csvFilePath, droppedFiles.paths[0]);
                        csvStatus = NOT_STARTED;
                        pthread_create(&csvThreadID, NULL, csvThread, (void *)&csvData);                            
                    }

                    UnloadDroppedFiles(droppedFiles);
                }

                // BFS Logic
                if (bfsActive && !dfsActive && !dijkstraActive && bfsStatus != IN_PROGRESS)
                {
                    bfsStatus = NOT_STARTED;
                    pthread_create(&bfsThreadID, NULL, bfsThread, (void *)&bfsData);
                }

                // DFS Logic
                if (!bfsActive && dfsActive && !dijkstraActive && dfsStatus != IN_PROGRESS)
                {
                    dfsStatus = NOT_STARTED;
                    pthread_create(&dfsThreadID, NULL, dfsThread, (void *)&dfsData);
                }

                break;
            }
            default:
                break;
        }

        // Toggle Debug Information
        if (IsKeyPressed(KEY_F1))
            debugInfoActive = !debugInfoActive;

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            switch (currentScene)
            {
                case START_MENU:
                {
                    startMenu(&currentScene, &exitWindow);
                    break;
                }
                case MAIN_SCENE:
                {
                    mainScene(
                        points, &theGraph, pointCount, &focusedPoint, &currentScene,
                        &bfsActive, &dfsActive, &dijkstraActive,
                        bfsLabel, dfsLabel, dijkstraLabel,
                        &bfsData, &dfsData,
                        &adjacencyMatrixWindowActive, &edgeThickness, debugInfoActive
                    );
                    break;
                }
                default:
                    break;
            }

            // Draw Debug Information
            if (debugInfoActive)
            {
                DrawFPS(screenWidth - 80, 10);

                Vector2 mousePos = GetMousePosition();
                DrawText(TextFormat("(%.2f, %.2f)", mousePos.x, mousePos.y), mousePos.x + 15, mousePos.y + 15, 12, BLACK);
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    pthread_kill(csvThreadID, SIGTERM);
    pthread_kill(bfsThreadID, SIGTERM);
    pthread_kill(dfsThreadID, SIGTERM);
    pthread_kill(dijkstraThreadID, SIGTERM);

    pthread_join(csvThreadID, NULL);
    pthread_join(bfsThreadID, NULL);
    pthread_join(dfsThreadID, NULL);
    pthread_join(dijkstraThreadID, NULL);

    graphDeinit(&theGraph);

    RL_FREE(csvFilePath);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

// Draw Start Menu
//----------------------------------------------------------------------------------
void startMenu(SceneType *currentScene, bool *exitWindow)
{

    // Draw Menu Options
    GuiGroupBox((Rectangle){halfScreenWidth - 250, halfScreenHeight - 150, 500, 300}, "Graph Output and Display Project");
    GuiLabel((Rectangle){halfScreenWidth - 210, halfScreenHeight + 110, 450, 30}, "A Project by Group 4 in CS1105 2022-26 @ IET, JKLU");
    
    if (GuiButton((Rectangle){halfScreenWidth - 60, halfScreenHeight - 30, 120, 30}, "Start"))
        *currentScene = MAIN_SCENE;
    if (GuiButton((Rectangle){halfScreenWidth - 50, halfScreenHeight + 10, 100, 30}, "Exit"))
        *exitWindow = true;
}
//----------------------------------------------------------------------------------

// Draw Main Scene
//----------------------------------------------------------------------------------
void mainScene(Vector2 *points, Graph *theGraph, int pointCount, int *focusedPoint, SceneType *currentScene,
               bool *bfsActive, bool *dfsActive, bool *dijkstraActive,
               char *bfsLabel, char *dfsLabel, char *dijkstraLabel,
               bfsThreadData *bfsData, dfsThreadData *dfsData,
               bool *adjacencyMatrixWindowActive, float *edgeThickness, bool debugInfoActive)
{
    // Draw Edges
    for (int i = 0; i < pointCount; i++) {
        for (int j = 0; j < pointCount; j++)
        {
            if ((*theGraph).adj[i][j] >= 1)
            {
                // Draw Edge
                DrawLineEx(points[i], points[j], *edgeThickness, SKYBLUE);

                // Draw Arrow // TODO: Refactor this into a Better Function
                Vector2 distPoint = {points[j].x - points[i].x, points[j].y - points[i].y};
                float angle = atan2f(distPoint.y, distPoint.x);
                Vector2 arrowEndPoint = {points[j].x - (22)*cosf(angle), points[j].y - (22)*sinf(angle)};
                Vector2 arrowBasePoint = {arrowEndPoint.x - (22)*cosf(angle), arrowEndPoint.y - (22)*sinf(angle)};
                float side1 = arrowEndPoint.x - points[i].x;
                float side2 = arrowEndPoint.y - points[i].y;
                Vector2 endPoint1 = {arrowBasePoint.x - side2 / 24, arrowBasePoint.y + side1 / 24};
                Vector2 endPoint2 = {arrowBasePoint.x + side2 / 24, arrowBasePoint.y - side1 / 24};
                DrawTriangle(endPoint2, endPoint1, arrowEndPoint, SKYBLUE);

                // Draw Edge Weights
                Vector2 midPoint = {(points[i].x + points[j].x) / 2, (points[i].y + points[j].y) / 2};
                DrawText(TextFormat("%d", (*theGraph).adj[i][j]), midPoint.x, midPoint.y + 20, 10, BLACK);
            }
        }
    }

    // Draw Vertices and Labels
    for (int i = 0; i < pointCount; i++)
    {
        DrawCircleV(points[i], (*focusedPoint == i)? 30.0f : 24.0f, (*focusedPoint == i)? GRAY: LIGHTGRAY);
        DrawText(TextFormat("%s", (*theGraph).labels[i]), points[i].x - 5, points[i].y - 5, 15, BLACK);
    }

    // Draw Configs
    if (GuiButton((Rectangle){12, 12, 24, 24}, "#185#"))
        *currentScene = START_MENU;
    if (GuiButton((Rectangle){48, 12, 24, 24}, "#75#"))
        createPointPolygon(points, pointCount, polygonRadius);
    if (GuiButton((Rectangle){12, 48, 200, 24}, "Show Adjacency Matrix"))
        *adjacencyMatrixWindowActive = !(*adjacencyMatrixWindowActive);

    // Draw Algorithm Buttons
    if (GuiButton((Rectangle){84, bfsOffset, 200, 24}, "Breadth-First Search") && !(*bfsActive))
    {
        *bfsActive = true;
        *dfsActive = false;
        *dijkstraActive = false;
    }
    if (GuiButton((Rectangle){84, dfsOffset, 200, 24}, "Depth-First Search") && !(*dfsActive))
    {
        *bfsActive = false;
        *dfsActive = true;
        *dijkstraActive = false;
    }
    if (GuiButton((Rectangle){84, dijkstraOffset, 200, 24}, "Dijkstra's Algorithm") && !(*dijkstraActive))
    {
        *bfsActive = false;
        *dfsActive = false;
        *dijkstraActive = true;
    }

    // Draw Algorithm Start Labels
    GuiTextBox((Rectangle){12, bfsOffset, 60, 24}, bfsLabel, 4096, 
        (!(*bfsActive) && CheckCollisionPointRec(GetMousePosition(), (Rectangle){12, bfsOffset, 60, 24})) ? true : false
    );
    GuiTextBox((Rectangle){12, dfsOffset, 60, 24}, dfsLabel, 4096,
        (!(*dfsActive) && CheckCollisionPointRec(GetMousePosition(), (Rectangle){12, dfsOffset, 60, 24})) ? true : false
    );
    GuiTextBox((Rectangle){12, dijkstraOffset, 60, 24}, dijkstraLabel, 4096,
        (!(*dijkstraActive) && CheckCollisionPointRec(GetMousePosition(), (Rectangle){12, dijkstraOffset, 60, 24})) ? true : false
    );

    // Draw Algorithm Results
    if (*bfsActive && !(*dfsActive) && !(*dijkstraActive))
    {
        int visitedX = halfScreenWidth + halfScreenWidth / 2;
        int visitedY = halfScreenHeight - 12 * pointCount;
        int queueX = visitedX - 12 * pointCount + 25;
        int queueY = visitedY + 24 * pointCount + 48;
        GuiLabel((Rectangle){visitedX - 1, queueY - 24, 50, 24}, "Queue");
        GuiLabel((Rectangle){visitedX - 1, visitedY - 24, 50, 24}, "Visited");

        for (int i = 0; i < pointCount; i++)
        {
            GuiTextBox((Rectangle){queueX + 24 * i, queueY, 24, 24},
                ((*(bfsData->qRear)) - (*(bfsData->qFront)) > i) ? theGraph->labels[bfsData->queue[(*(bfsData->qFront)) + i]] : "",
                0, false
            );

            int currY = visitedY + 24 * i;
            GuiLabel((Rectangle){visitedX, currY, 24, 24}, theGraph->labels[i]);
            GuiTextBox((Rectangle){visitedX + 24, currY, 24, 24},
                (dfsData->visited[i]) ? "#112#" : "#113#",
                0, false
            );
        }
    }
    else if (!(*bfsActive) && *dfsActive && !(*dijkstraActive))
    {
        int stackX = halfScreenWidth + halfScreenWidth / 2 + 13;
        int visitedX = stackX + 63;
        int dsY = halfScreenHeight - 12 * pointCount;
        GuiLabel((Rectangle){stackX - 13, dsY - 24, 50, 24}, "Stack");
        GuiLabel((Rectangle){visitedX - 1, dsY - 24, 50, 24}, "Visited");

        for (int i = 0; i < pointCount; i++)
        {
            int currY = dsY + 24 * i;
            GuiTextBox((Rectangle){stackX, currY, 24, 24},
                (*(dfsData->sTop) > pointCount - i - 1) ? theGraph->labels[dfsData->stack[pointCount - i - 1]] : "",
                0, false
            );
            
            GuiLabel((Rectangle){visitedX, currY, 24, 24}, theGraph->labels[i]);
            GuiTextBox((Rectangle){visitedX + 24, currY, 24, 24},
                (dfsData->visited[i]) ? "#112#" : "#113#",
                0, false
            );
        }
    }

    // Draw Adjacency Matrix Window
    if (*adjacencyMatrixWindowActive)
    {
        int windowSize = 48 * (pointCount < 4 ? 6 : pointCount + 2);
        int windowX = screenWidth - windowSize - 12;
        int windowY = 12;
        *adjacencyMatrixWindowActive = !GuiWindowBox((Rectangle){windowX, windowY, windowSize - 16, windowSize}, "Adjacency Matrix");
        for (int i = 0; i < pointCount; i++)
        {
            for (int j = 0; j < pointCount; j++)
            {
                if (
                    CheckCollisionPointRec(GetMousePosition(), (Rectangle){windowX + 40 + 50 * j, 72 + 50 * i, 40, 40}) ||
                    CheckCollisionPointRec(GetMousePosition(), (Rectangle){windowX + 40 + 50 * i, 72 + 50 * j, 40, 40})
                )
                {
                    GuiValueBox((Rectangle){windowX + 40 + 50 * j, 72 + 50 * i, 40, 40}, "", &(*theGraph).adj[i][j], 0, INT_MAX, true);
                    DrawRing(points[i], 24.0f, 26.0f, 0, 360, 90, RED);
                    DrawRing(points[j], 24.0f, 26.0f, 0, 360, 90, LIME);
                }
                else
                    GuiValueBox((Rectangle){windowX + 40 + 50 * j, 72 + 50 * i, 40, 40}, "", &(*theGraph).adj[i][j], 0, INT_MAX, false);
            }
            GuiLabel((Rectangle){windowX + 16, 72 + 50 * i, 40, 40}, (*theGraph).labels[i]);
            GuiLabel((Rectangle){windowX + 56 + 50 * i, 36, 40, 40}, (*theGraph).labels[i]);
        }
    }

    // Draw Debug Information
    if (debugInfoActive)
    {
        GuiLabel((Rectangle){ 12, screenHeight - 60, 160, 24 }, TextFormat("Edge Thickness: %i", (int)*edgeThickness));
        GuiSliderBar((Rectangle){ 12, screenHeight - 36, 160, 24 }, NULL, NULL, edgeThickness, 1.0f, 40.0f);
        GuiLabel((Rectangle){ screenWidth - 150, screenHeight - 36, 150, 24 }, TextFormat("Focused Point: %i", (int)*focusedPoint));

        for (int i = 0; i < pointCount; i++)
            DrawText(TextFormat("(%.2f, %.2f)", points[i].x, points[i].y), points[i].x + 20, points[i].y + 20, 10, BLACK);
    }
}
//----------------------------------------------------------------------------------

// Create a Polygon of Points
//----------------------------------------------------------------------------------
void createPointPolygon(Vector2 *points, int pointCount, int radius)
{
    int arrSize = (pointCount <= MAX_POINTS) ? pointCount : MAX_POINTS;
    for (int i = 0; i < arrSize; i++)
        points[i] = (Vector2){halfScreenWidth + radius * cos(2 * PI * i / arrSize), halfScreenHeight + radius * sin(2 * PI * i / arrSize)};
}
//----------------------------------------------------------------------------------

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
    int adj[csvN][csvN];
    for (int i = 1; i < csvN; i++)
        for (int j = 1; j < csvN; j++)
            adj[i-1][j-1] = strtol(csvData[i][j], NULL, 10);

    *(data->pointCount) = csvN-1;
    editGraph(data->theGraph, *(data->pointCount), csvData[0]+1, adj);
    createPointPolygon(data->points, *(data->pointCount), polygonRadius);

    TextCopy(data->bfsLabel, data->theGraph->labels[0]);
    TextCopy(data->dfsLabel, data->theGraph->labels[0]);
    TextCopy(data->dijkstraLabel, data->theGraph->labels[0]);

    *(data->status) = COMPLETED;
    return NULL;
}
//----------------------------------------------------------------------------------

// Thread Function to render Breadth-First Search
//----------------------------------------------------------------------------------
void *bfsThread(void *arg)
{
    bfsThreadData *data = (bfsThreadData *)arg;
    *(data->status) = IN_PROGRESS;

    for (int i = 0; i < data->theGraph->n; i++)
        data->visited[i] = false;

    int u;
    for (u = 0; u < data->theGraph->n; u++)
        if (strcmp(data->theGraph->labels[u], data->startLabel) == 0)
            break;

    if (u == data->theGraph->n)
    {
        *(data->status) = COMPLETED;
        *(data->animationActive) = false;
        return NULL;
    }

    *(data->qFront) = 0;
    *(data->qRear) = 0;

    data->visited[u] = true;
    // printf("%s ", g->labels[u]);
    data->queue[(*(data->qRear))++] = u;

    for (int i = 0; i < 1000000000; i++); // Delay

    while (*(data->qFront) < *(data->qRear)) {
        u = data->queue[(*(data->qFront))++];

        for (int v = 0; v < data->theGraph->n; v++)
            if (data->theGraph->adj[u][v] && !data->visited[v]) {
                data->visited[v] = true;
                // printf("%s ", g->labels[v]);
                data->queue[(*(data->qRear))++] = v;
            }

        for (int i = 0; i < 1000000000; i++); // Delay
    }

    for (int i = 0; i < INT_MAX; i++); // Delay

    *(data->status) = COMPLETED;
    *(data->animationActive) = false;
    return NULL;
}
//----------------------------------------------------------------------------------

// Thread Function to render Depth-First Search
//----------------------------------------------------------------------------------
void *dfsThread(void *arg)
{
    dfsThreadData *data = (dfsThreadData *)arg;
    *(data->status) = IN_PROGRESS;

    for (int i = 0; i < data->theGraph->n; i++)
        data->visited[i] = false;

    int u;
    for (u = 0; u < data->theGraph->n; u++)
        if (strcmp(data->theGraph->labels[u], data->startLabel) == 0)
            break;

    if (u == data->theGraph->n)
    {
        *(data->status) = COMPLETED;
        *(data->animationActive) = false;
        return NULL;
    }

    *(data->sTop) = 0;
    data->visited[u] = true;
    data->stack[(*(data->sTop))++] = u;

    for (int i = 0; i < 1000000000; i++); // Delay

    while (*(data->sTop) > 0) {
        u = data->stack[--(*(data->sTop))];
        // printf("%s ", g->labels[u]);

        for (int v = 0; v < data->theGraph->n; v++)
        {
            if (data->theGraph->adj[u][v] && !data->visited[v]) {
                data->visited[v] = 1;
                data->stack[(*(data->sTop))++] = v;
            }
        }
        for (int i = 0; i < 1000000000; i++); // Delay
    }

    for (int i = 0; i < INT_MAX; i++); // Delay

    *(data->status) = COMPLETED;
    *(data->animationActive) = false;
    return NULL;
}
//----------------------------------------------------------------------------------

// Thread Function to render Dijkstra's Algorithm
//----------------------------------------------------------------------------------
void *dijkstraThread(void *arg)
{
    return NULL;
}
//----------------------------------------------------------------------------------
