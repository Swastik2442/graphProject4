#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

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

const int graphRadius = 200;

//------------------------------------------------------------------------------------
// Thread Data Structures

typedef struct csvData
{
    char *csvFilePath;
    Vector2 *points;
    Graph *theGraph;
    int *pointCount;
} csvData;

//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// Utility Functions

void createPointPolygon(Vector2 *points, int pointCount, int radius);
void *csvThread(void *arg);
//------------------------------------------------------------------------------------

// Scene Types
typedef enum {
    START_MENU = 0,      // Start Menu
    MAIN_SCENE,          // Main Scene
} SceneType;

//------------------------------------------------------------------------------------
// All the Scenes

void startMenu(SceneType *currentScene, bool *exitWindow);
void mainScene(Vector2 *points, Graph *theGraph, int pointCount, int *focusedPoint, SceneType *currentScene,
               bool *adjacencyMatrixWindowActive, float *edgeThickness, bool debugInfoActive);
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
    createPointPolygon(points, pointCount, graphRadius);

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

    char *csvFilePath = (char *)RL_CALLOC(4096, 1);
    TextCopy(csvFilePath, "Drop a CSV File Here");

    // Windows Configuration

    bool adjacencyMatrixWindowActive = false;

    // Threading Configuration

    pthread_t csvThreadID = PTHREAD_ONCE_INIT;

    // Set custom GUI Style

    GuiSetStyle(DEFAULT, TEXT_SIZE, 14);
    GuiSetStyle(DEFAULT, TEXT_SPACING, 2);

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
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
                            csvData data = {csvFilePath, points, &theGraph, &pointCount};
                            pthread_create(&csvThreadID, NULL, csvThread, &data);                            
                        }

                        UnloadDroppedFiles(droppedFiles);
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
                        &adjacencyMatrixWindowActive, &edgeThickness, debugInfoActive
                    );
                    break;
                }
                default:
                    break;
            }

            // Draw Debug Information
            if (debugInfoActive)
                DrawFPS(screenWidth - 80, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }


    // De-Initialization
    //--------------------------------------------------------------------------------------
    pthread_join(csvThreadID, NULL);

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
    if (GuiButton((Rectangle){12, 12, 24, 24}, "#56#"))
        *currentScene = START_MENU;
    if (GuiButton((Rectangle){48, 12, 24, 24}, "#185#"))
        createPointPolygon(points, pointCount, graphRadius);
    if (GuiButton((Rectangle){84, 12, 200, 24}, "Show Adjacency Matrix"))
        *adjacencyMatrixWindowActive = !(*adjacencyMatrixWindowActive);

    // Draw Algorithm Buttons
    // GuiTextBox((Rectangle){12, 48, 60, 24}, "A", 4096, false);
    // GuiButton((Rectangle){84, 48, 200, 24}, "Breadth First Search");
    // GuiTextBox((Rectangle){12, 84, 60, 24}, "B", 4096, false);
    // GuiButton((Rectangle){84, 84, 200, 24}, "Depth First Search");

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
                if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){windowX + 40 + 50 * j, 72 + 50 * i, 40, 40}))
                {
                    GuiValueBox((Rectangle){windowX + 40 + 50 * j, 72 + 50 * i, 40, 40}, "", &(*theGraph).adj[i][j], 0, INT_MAX, true);
                    DrawCircleLinesV(points[i], 24.0f, RED);
                    DrawCircleLinesV(points[i], 25.0f, RED);
                    DrawCircleLinesV(points[i], 26.0f, RED);
                    DrawCircleLinesV(points[j], 24.0f, LIME);
                    DrawCircleLinesV(points[j], 25.0f, LIME);
                    DrawCircleLinesV(points[j], 26.0f, LIME);
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
        GuiLabel((Rectangle){ 12, screenHeight - 96, 160, 24 }, TextFormat("Edge Thickness: %i", (int)*edgeThickness));
        GuiSliderBar((Rectangle){ 12, screenHeight - 72, 140, 24 }, NULL, NULL, edgeThickness, 1.0f, 40.0f);
        GuiLabel((Rectangle){ 12, screenHeight - 36, 160, 24 }, TextFormat("Focused Point: %i", (int)*focusedPoint));

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
    csvData *data = (csvData *)arg;

    char *csvData[MAX_CSV_ROWS][MAX_CSV_COLS];
    int csvRows, csvCols, csvN;
    readCSV(data->csvFilePath, csvData, &csvRows, &csvCols);

    csvN = (csvRows <= csvCols) ? csvRows : csvCols;
    int adj[csvN][csvN];
    for (int i = 1; i < csvN; i++)
    {
        for (int j = 1; j < csvN; j++)
            adj[i-1][j-1] = strtol(csvData[i][j], NULL, 10);
    }
    *(data->pointCount) = csvN-1;
    editGraph(data->theGraph, *(data->pointCount), csvData[0]+1, adj);
    createPointPolygon(data->points, *(data->pointCount), graphRadius);

    return NULL;
}
//----------------------------------------------------------------------------------
