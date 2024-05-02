#include <stdio.h>
#include <pthread.h>

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "csv.h"
#include "graph.h"
#include "priorityQueue.h"
#include "threading.h"
#include "utils.h"

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

// Static Variables

static int bfsTitleOffset;
static int dfsTitleOffset;
static int dijkstraTitleOffset;

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
    bfsThreadData *bfsData,            // BFS Thread Data
    dfsThreadData *dfsData,            // DFS Thread Data
    dijkstraThreadData *dijkstraData,  // Dijkstra Thread Data
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

    // Geometry Configuration

    int pointCount = 6;
    int selectedPoint = -1;
    int focusedPoint = -1;
    float edgeThickness = 4.0f;

    Vector2 points[MAX_VERTICES];
    createPointPolygon(points, pointCount, (Vector2){halfScreenWidth, halfScreenHeight}, polygonRadius);

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
    priorityQueue pQueue;
    bool visitedVertices[MAX_VERTICES];
    int dsArray1[MAX_VERTICES], dsArray2[MAX_VERTICES], dsArray3[MAX_VERTICES], dsArray4[MAX_VERTICES];
    int dsVar1, dsVar2;
    int currentV, srcV, destV, childTop;

    char *csvFilePath = (char *)RL_CALLOC(4096, 1);
    char *bfsSrcLabel = (char *)RL_CALLOC(4096, 1);
    char *dfsSrcLabel = (char *)RL_CALLOC(4096, 1);
    char *dijkstraSrcLabel = (char *)RL_CALLOC(4096, 1);
    char *bfsDestLabel = (char *)RL_CALLOC(4096, 1);
    char *dfsDestLabel = (char *)RL_CALLOC(4096, 1);
    char *dijkstraDestLabel = (char *)RL_CALLOC(4096, 1);

    TextCopy(csvFilePath, "Drop a CSV File Here");
    TextCopy(bfsSrcLabel, theGraph.labels[0]);
    TextCopy(dfsSrcLabel, theGraph.labels[0]);
    TextCopy(dijkstraSrcLabel, theGraph.labels[0]);
    TextCopy(bfsDestLabel, theGraph.labels[theGraph.n - 1]);
    TextCopy(dfsDestLabel, theGraph.labels[theGraph.n - 1]);
    TextCopy(dijkstraDestLabel, theGraph.labels[theGraph.n - 1]);

    // Title Configuration

    bfsTitleOffset = halfScreenWidth - (MeasureText("Breadth-First Search", 18) / 2);
    dfsTitleOffset = halfScreenWidth - (MeasureText("Depth-First Search", 18) / 2);
    dijkstraTitleOffset = halfScreenWidth - (MeasureText("Dijkstra's Algorithm", 18) / 2);

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
    ThreadStatus dijkstraStatus = NOT_STARTED;

    csvThreadData csvData = {&csvStatus, csvFilePath, points, &pointCount, (Vector2){halfScreenWidth, halfScreenHeight}, polygonRadius, &theGraph, bfsSrcLabel, dfsSrcLabel, dijkstraSrcLabel, bfsDestLabel, dfsDestLabel, dijkstraDestLabel};
    bfsThreadData bfsData = {&bfsStatus, &theGraph, bfsSrcLabel, bfsDestLabel, &currentV, &srcV, &destV, dsArray1, &childTop, visitedVertices, dsArray2, &dsVar1, &dsVar2, &bfsActive};
    dfsThreadData dfsData = {&dfsStatus, &theGraph, dfsSrcLabel, dfsDestLabel, &currentV, &srcV, &destV, dsArray1, &childTop, visitedVertices, dsArray2, &dsVar1, &dfsActive};
    dijkstraThreadData dijkstraData = {&dijkstraStatus, &theGraph, dijkstraSrcLabel, dijkstraDestLabel, &currentV, &srcV, &destV, dsArray1, &childTop, dsArray2, dsArray3, &pQueue, visitedVertices, dsArray4, &dsVar1, &dijkstraActive};

    // Set custom GUI Style

    GuiSetStyle(DEFAULT, TEXT_SIZE, 14);
    GuiSetStyle(DEFAULT, TEXT_SPACING, 2);

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main Loop
    while (!exitWindow)
    {
        // Update
        //----------------------------------------------------------------------------------

        // Detect Window Close button or ESC key
        if (WindowShouldClose())
            exitWindow = true;

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
                if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && (pointCount < MAX_VERTICES) && (focusedPoint == -1) && (!adjacencyMatrixWindowActive))
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

                    if (
                        (droppedFiles.count > 0) && IsFileExtension(droppedFiles.paths[0], ".csv") &&
                        !bfsActive && !dfsActive && !dijkstraActive && csvStatus != IN_PROGRESS
                    )
                    {
                        TextCopy(csvFilePath, droppedFiles.paths[0]);
                        csvStatus = NOT_STARTED;
                        pthread_create(&csvThreadID, NULL, csvThread, (void *)&csvData);
                    }

                    UnloadDroppedFiles(droppedFiles);
                }

                // Animation Logic
                if (csvStatus == IN_PROGRESS)
                {
                    bfsActive = false;
                    dfsActive = false;
                    dijkstraActive = false;
                }
                if (bfsActive && !dfsActive && !dijkstraActive && bfsStatus != IN_PROGRESS)
                {
                    bfsStatus = NOT_STARTED;
                    pthread_create(&bfsThreadID, NULL, bfsThread, (void *)&bfsData);
                }
                else if (!bfsActive && dfsActive && !dijkstraActive && dfsStatus != IN_PROGRESS)
                {
                    dfsStatus = NOT_STARTED;
                    pthread_create(&dfsThreadID, NULL, dfsThread, (void *)&dfsData);
                }
                else if (!bfsActive && !dfsActive && dijkstraActive && dijkstraStatus != IN_PROGRESS)
                {
                    dijkstraStatus = NOT_STARTED;
                    pthread_create(&dijkstraThreadID, NULL, dijkstraThread, (void *)&dijkstraData);
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
                        &bfsData, &dfsData, &dijkstraData,
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
    CloseWindow();        // Close window and OpenGL context

    graphDeinit(&theGraph);
    pQueueDeinit(&pQueue);

    RL_FREE(csvFilePath);
    RL_FREE(bfsSrcLabel);
    RL_FREE(dfsSrcLabel);
    RL_FREE(dijkstraSrcLabel);
    RL_FREE(bfsDestLabel);
    RL_FREE(dfsDestLabel);
    RL_FREE(dijkstraDestLabel);

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
               bfsThreadData *bfsData, dfsThreadData *dfsData, dijkstraThreadData *dijkstraData,
               bool *adjacencyMatrixWindowActive, float *edgeThickness, bool debugInfoActive)
{
    // Draw Edges
    for (int i = 0; i < pointCount; i++)
    {
        for (int j = 0; j < pointCount; j++)
        {
            if ((*theGraph).adj[i][j] >= 1)
            {
                // Draw Edge
                DrawLineEx(points[i], points[j], *edgeThickness, SKYBLUE);

                // Draw Arrow
                drawArrow(points[i], points[j], SKYBLUE);

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
        createPointPolygon(points, pointCount, (Vector2){halfScreenWidth, halfScreenHeight}, polygonRadius);
    if (GuiButton((Rectangle){84, 12, 128, 24}, "Randomize") && !(*(bfsData->animationActive)) && !(*(dfsData->animationActive)) && !(*(dijkstraData->animationActive)))
        changeWeights(theGraph);
    if (GuiButton((Rectangle){12, 48, 200, 24}, "Show Adjacency Matrix"))
        *adjacencyMatrixWindowActive = !(*adjacencyMatrixWindowActive);

    // Draw Algorithm Source Labels
    GuiTextBox((Rectangle){12, bfsOffset, 60, 24}, bfsData->srcLabel, 4096, 
        (!(*(bfsData->animationActive)) && CheckCollisionPointRec(GetMousePosition(), (Rectangle){12, bfsOffset, 60, 24})) ? true : false
    );
    GuiTextBox((Rectangle){12, dfsOffset, 60, 24}, dfsData->srcLabel, 4096,
        (!(*(dfsData->animationActive)) && CheckCollisionPointRec(GetMousePosition(), (Rectangle){12, dfsOffset, 60, 24})) ? true : false
    );
    GuiTextBox((Rectangle){12, dijkstraOffset, 60, 24}, dijkstraData->srcLabel, 4096,
        (!(*(dijkstraData->animationActive)) && CheckCollisionPointRec(GetMousePosition(), (Rectangle){12, dijkstraOffset, 60, 24})) ? true : false
    );

    // Draw Algorithm Destination Labels
    GuiTextBox((Rectangle){78, bfsOffset, 60, 24}, bfsData->destLabel, 4096,
        (!(*(bfsData->animationActive)) && CheckCollisionPointRec(GetMousePosition(), (Rectangle){78, bfsOffset, 60, 24})) ? true : false
    );
    GuiTextBox((Rectangle){78, dfsOffset, 60, 24}, dfsData->destLabel, 4096,
        (!(*(dfsData->animationActive)) && CheckCollisionPointRec(GetMousePosition(), (Rectangle){78, dfsOffset, 60, 24})) ? true : false
    );
    GuiTextBox((Rectangle){78, dijkstraOffset, 60, 24}, dijkstraData->destLabel, 4096,
        (!(*(dijkstraData->animationActive)) && CheckCollisionPointRec(GetMousePosition(), (Rectangle){78, dijkstraOffset, 60, 24})) ? true : false
    );

    // Draw Algorithm Buttons
    if (
        GuiButton((Rectangle){144, bfsOffset, 200, 24}, "Breadth-First Search") &&
        !(*(bfsData->animationActive)) && !(*(dfsData->animationActive)) && !(*(dijkstraData->animationActive))
    )
    {
        *(bfsData->animationActive) = true;
        *(dfsData->animationActive) = false;
        *(dijkstraData->animationActive) = false;
    }
    if (
        GuiButton((Rectangle){144, dfsOffset, 200, 24}, "Depth-First Search") &&
        !(*(bfsData->animationActive)) && !(*(dfsData->animationActive)) && !(*(dijkstraData->animationActive))
    )
    {
        *(bfsData->animationActive) = false;
        *(dfsData->animationActive) = true;
        *(dijkstraData->animationActive) = false;
    }
    if (
        GuiButton((Rectangle){144, dijkstraOffset, 200, 24}, "Dijkstra's Algorithm") &&
        !(*(bfsData->animationActive)) && !(*(dfsData->animationActive)) && !(*(dijkstraData->animationActive))
    )
    {
        *(bfsData->animationActive) = false;
        *(dfsData->animationActive) = false;
        *(dijkstraData->animationActive) = true;
    }

    // Draw Algorithm Results
    if (*(bfsData->animationActive) && !(*(dfsData->animationActive)) && !(*(dijkstraData->animationActive)) && *(bfsData->status) == IN_PROGRESS)
    {
        // Draw Title
        DrawText("Breadth-First Search", bfsTitleOffset, 12, 18, BLACK);

        // Position Calculation
        int visitedX = halfScreenWidth + halfScreenWidth / 2;
        int visitedY = halfScreenHeight - 12 * pointCount;
        int queueX = visitedX - 12 * pointCount + 25;
        int queueY = visitedY + 24 * pointCount + 48;

        // Draw Labels
        GuiLabel((Rectangle){visitedX - 1, queueY - 24, 50, 24}, "Queue");
        GuiLabel((Rectangle){visitedX - 1, visitedY - 24, 50, 24}, "Visited");

        // Draw Text Structures
        for (int i = 0; i < pointCount; i++)
        {
            int currY = visitedY + 24 * i;

            // Draw Queue Elements
            GuiTextBox((Rectangle){queueX + 24 * i, queueY, 24, 24},
                ((*(bfsData->qRear)) - (*(bfsData->qFront)) > i) ? theGraph->labels[bfsData->queue[(*(bfsData->qFront)) + i]] : "",
                0, false
            );

            // Draw Visited Elements
            GuiLabel((Rectangle){visitedX, currY, 24, 24}, theGraph->labels[i]);
            GuiTextBox((Rectangle){visitedX + 24, currY, 24, 24},
                (dfsData->visited[i]) ? "#112#" : "#113#",
                0, false
            );

            // Draw Visited Vertices
            if (bfsData->visited[i])
                DrawRing(points[i], 24.0f, 26.0f, 0, 360, 90, BLACK);
        }

        // Draw Queue Vertices
        for (int i = *(bfsData->qFront); i < *(bfsData->qRear); i++)
            DrawRing(points[bfsData->queue[i]], 24.0f, 26.0f, 0, 360, 90, YELLOW);

        // Draw Children Vertices
        for (int i = 0; i < *(bfsData->childrenTop); i++)
            DrawRing(points[bfsData->children[i]], 24.0f, 26.0f, 0, 360, 90, BLUE);

        // Draw Current, Source and Destination Vertices
        if (*(bfsData->current) != -1)
            DrawRing(points[*(bfsData->current)], 24.0f, 26.0f, 0, 360, 90, ORANGE);
        if (*(bfsData->src) != -1)
            DrawRing(points[*(bfsData->src)], 24.0f, 26.0f, 0, 360, 90, RED);
        if (*(bfsData->dest) != -1)
            DrawRing(points[*(bfsData->dest)], 24.0f, 26.0f, 0, 360, 90, GREEN);
    }
    else if (!(*(bfsData->animationActive)) && *(dfsData->animationActive) && !(*(dijkstraData->animationActive)) && *(dfsData->status) == IN_PROGRESS)
    {
        // Draw Title
        DrawText("Depth-First Search", dfsTitleOffset, 12, 18, BLACK);

        // Position Calculation
        int stackX = halfScreenWidth + halfScreenWidth / 2 + 13;
        int visitedX = stackX + 63;
        int dsY = halfScreenHeight - 12 * pointCount;

        // Draw Labels
        GuiLabel((Rectangle){stackX - 13, dsY - 24, 50, 24}, "Stack");
        GuiLabel((Rectangle){visitedX - 1, dsY - 24, 50, 24}, "Visited");

        // Draw Text Structures
        for (int i = 0; i < pointCount; i++)
        {
            int currY = dsY + 24 * i;

            // Draw Stack Elements
            GuiTextBox((Rectangle){stackX, currY, 24, 24},
                (*(dfsData->sTop) > pointCount - i - 1) ? theGraph->labels[dfsData->stack[pointCount - i - 1]] : "",
                0, false
            );

            // Draw Visited Elements
            GuiLabel((Rectangle){visitedX, currY, 24, 24}, theGraph->labels[i]);
            GuiTextBox((Rectangle){visitedX + 24, currY, 24, 24},
                (dfsData->visited[i]) ? "#112#" : "#113#",
                0, false
            );

            // Draw Visited Vertices
            if (dfsData->visited[i])
                DrawRing(points[i], 24.0f, 26.0f, 0, 360, 90, BLACK);
        }

        // Draw Stack Vertices
        for (int i = 0; i < *(dfsData->sTop); i++)
            DrawRing(points[dfsData->stack[i]], 24.0f, 26.0f, 0, 360, 90, YELLOW);

        // Draw Children Vertices
        for (int i = 0; i < *(dfsData->childrenTop); i++)
            DrawRing(points[dfsData->children[i]], 24.0f, 26.0f, 0, 360, 90, BLUE);

        // Draw Current, Source and Destination Vertices
        if (*(dfsData->current) != -1)
            DrawRing(points[*(dfsData->current)], 24.0f, 26.0f, 0, 360, 90, ORANGE);
        if (*(dfsData->src) != -1)
            DrawRing(points[*(dfsData->src)], 24.0f, 26.0f, 0, 360, 90, RED);
        if (*(dfsData->dest) != -1)
            DrawRing(points[*(dfsData->dest)], 24.0f, 26.0f, 0, 360, 90, GREEN);
    }
    else if (!(*(bfsData->animationActive)) && !(*(dfsData->animationActive)) && *(dijkstraData->animationActive) && *(dijkstraData->status) == IN_PROGRESS)
    {
        // Draw Title
        DrawText("Dijkstra's Algorithm", dijkstraTitleOffset, 12, 18, BLACK);

        // Position Calculation
        int labelX = halfScreenWidth + halfScreenWidth / 2 - 24;
        int parentX = labelX + 37;
        int distanceX = parentX + 62;
        int dsY = halfScreenHeight - 12 * pointCount;

        // Draw Labels
        GuiLabel((Rectangle){parentX - 13, dsY - 24, 50, 24}, "Parent");
        GuiLabel((Rectangle){distanceX - 1, dsY - 24, 60, 24}, "Distance");

        // Draw Text Structures
        for (int i = 0; i < pointCount; i++)
        {
            int currY = dsY + 24 * i;

            // Draw Parent Elements
            GuiLabel((Rectangle){labelX, currY, 24, 24}, theGraph->labels[i]);
            GuiTextBox((Rectangle){parentX, currY, 24, 24},
                dijkstraData->previous[i] == -1 ? "" : theGraph->labels[dijkstraData->previous[i]],
                0, false
            );

            // Draw Distance Elements
            GuiTextBox((Rectangle){distanceX + 24, currY, 24, 24},
                (dijkstraData->distance[i] != INT_MAX && dijkstraData->distance[i] != INT_MIN) ? (char*)TextFormat("%d", dijkstraData->distance[i]) : "#219#",
                0, false
            );

            // Draw Vertex Distances
            if (dijkstraData->distance[i] != INT_MAX && dijkstraData->distance[i] != INT_MIN)
            {
                DrawText(TextFormat("%d", dijkstraData->distance[i]), points[i].x + 24, points[i].y + 24, 12, BLACK);

                // Draw Vertex's Parent // TODO: Refactor this to reduce Redundancy
                if (dijkstraData->previous[i] != -1)
                {
                    DrawLineEx(points[dijkstraData->previous[i]], points[i], *edgeThickness, GRAY);
                    drawArrow(points[dijkstraData->previous[i]], points[i], GRAY);
                    DrawCircleV(points[i], (*focusedPoint == i)? 30.0f : 24.0f, (*focusedPoint == i)? GRAY: LIGHTGRAY);
                    DrawText(TextFormat("%s", (*theGraph).labels[i]), points[i].x - 5, points[i].y - 5, 15, BLACK);
                    DrawCircleV(points[dijkstraData->previous[i]], (*focusedPoint == dijkstraData->previous[i])? 30.0f : 24.0f, (*focusedPoint == dijkstraData->previous[i])? GRAY: LIGHTGRAY);
                    DrawText(TextFormat("%s", (*theGraph).labels[dijkstraData->previous[i]]), points[dijkstraData->previous[i]].x - 5, points[dijkstraData->previous[i]].y - 5, 15, BLACK);
                }
            }
        }

        // Draw Children Vertices
        for (int i = 0; i < *(dijkstraData->childrenTop); i++)
            DrawRing(points[dijkstraData->children[i]], 24.0f, 26.0f, 0, 360, 90, BLUE);

        // Draw Current, Source and Destination Vertices
        if (*(dijkstraData->current) != -1)
            DrawRing(points[*(dijkstraData->current)], 24.0f, 26.0f, 0, 360, 90, ORANGE);
        if (*(dijkstraData->src) != -1)
            DrawRing(points[*(dijkstraData->src)], 24.0f, 26.0f, 0, 360, 90, RED);
        if (*(dijkstraData->dest) != -1)
            DrawRing(points[*(dijkstraData->dest)], 24.0f, 26.0f, 0, 360, 90, GREEN);

        // Draw Path // TODO: Refactor this to reduce Redundancy
        if (*(dijkstraData->pathHead) != MAX_VERTICES && dijkstraData->path[*(dijkstraData->pathHead)] == *(dijkstraData->src))
        {
            int parent, child, pathLength = 0;
            for (int i = *(dijkstraData->pathHead); i < MAX_VERTICES - 1; i++)
            {
                parent = dijkstraData->path[i];
                child = dijkstraData->path[i + 1];
                pathLength += (*theGraph).adj[parent][child];
                DrawLineEx(points[parent], points[child], *edgeThickness, BLACK);
                drawArrow(points[parent], points[child], BLACK);
                DrawCircleV(points[parent], (*focusedPoint == parent)? 30.0f : 24.0f, (*focusedPoint == parent)? GRAY: LIGHTGRAY);
                DrawText(TextFormat("%s", (*theGraph).labels[parent]), points[parent].x - 5, points[parent].y - 5, 15, BLACK);
            }
            DrawCircleV(points[child], (*focusedPoint == child)? 30.0f : 24.0f, (*focusedPoint == child)? GRAY: LIGHTGRAY);
            DrawText(TextFormat("%s", (*theGraph).labels[child]), points[child].x - 5, points[child].y - 5, 15, BLACK);
            GuiLabel((Rectangle){parentX, dsY + 24 * (pointCount + 2), 120, 24}, TextFormat("Path Length: %d", pathLength));
        }
        else
            GuiLabel((Rectangle){parentX, dsY + 24 * (pointCount + 2), 150, 24}, "No Path Found");
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
                    CheckCollisionPointRec(GetMousePosition(), (Rectangle){windowX + 40 + 50 * j, 72 + 50 * i, 40, 40}) &&
                    !(*(bfsData->animationActive)) && !(*(dfsData->animationActive)) && !(*(dijkstraData->animationActive))
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
