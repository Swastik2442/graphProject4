#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "graph.h"

#define MAX_POINTS 64

// Screen Size
const int screenWidth = 1280;
const int screenHeight = 720;
const int halfScreenWidth = screenWidth / 2;
const int halfScreenHeight = screenHeight / 2;

// Scene Types
typedef enum {
    START_MENU = 0,      // Start Menu
    MAIN_SCENE,          // Main Scene
} SceneType;

//------------------------------------------------------------------------------------
// All the Scenes
void startMenu(SceneType *currentScene, bool *debugInfoActive, bool *exitWindow);
void mainScene(Vector2 *points, int pointCount, int focusedPoint, char *csvFilePath,
               bool *adjacencyMatrixWindowActive, float *edgeThickness, bool *debugInfoActive);
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
    int n = 6, x = halfScreenWidth, y = halfScreenHeight, r = 200;
    Vector2 points[MAX_POINTS];
    for (int i = 0; i < n; i++)
        points[i] = (Vector2){x + r * cos(2 * PI * i / n), y + r * sin(2 * PI * i / n)};

    int pointCount = 6;
    int selectedPoint = -1;
    int focusedPoint = -1;


    // Graph Configuration
    char *csvFilePath = (char *)RL_CALLOC(4096, 1);
    TextCopy(csvFilePath, "Drop a CSV File Here");
    // Graph g;
    // graphInit(&g, n);

    // Edge Config Variables
    float edgeThickness = 8.0f;

    bool adjacencyMatrixWindowActive = false;

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
                    if (IsKeyPressed(KEY_F1))
                        debugInfoActive = !debugInfoActive;
                    break;
                }
            case MAIN_SCENE:
                {
                    if (IsKeyPressed(KEY_F1))
                        debugInfoActive = !debugInfoActive;

                    // Point Creation Logic
                    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && (pointCount < MAX_POINTS) && (focusedPoint == -1) && (!adjacencyMatrixWindowActive))
                    {
                        points[pointCount] = GetMousePosition();
                        pointCount++;
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
                            TextCopy(csvFilePath, droppedFiles.paths[0]);

                        UnloadDroppedFiles(droppedFiles);
                    }

                    break;
                }
            default:
                break;
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            switch (currentScene)
            {
                case START_MENU:
                {
                    startMenu(&currentScene, &debugInfoActive, &exitWindow);
                    break;
                }
                case MAIN_SCENE:
                {
                    mainScene(points, pointCount, focusedPoint, csvFilePath, &adjacencyMatrixWindowActive, &edgeThickness, &debugInfoActive);
                    break;
                }
                default:
                    break;
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

// Draw Start Menu
//----------------------------------------------------------------------------------
void startMenu(SceneType *currentScene, bool *debugInfoActive, bool *exitWindow)
{
    // Draw Debug Information
    if (*debugInfoActive)
    {
        DrawFPS(screenWidth - 80, 10);
    }

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
void mainScene(Vector2 *points, int pointCount, int focusedPoint, char *csvFilePath,
               bool *adjacencyMatrixWindowActive, float *edgeThickness, bool *debugInfoActive)
{
    // Draw Debug Information
    if (*debugInfoActive)
    {
        DrawFPS(screenWidth - 80, 10);

        DrawLineEx(points[0], points[pointCount - 1], *edgeThickness, SKYBLUE);
        
        for (int i = 0; i < pointCount; i++)
        {
            DrawText(TextFormat("(%.2f, %.2f)", points[i].x, points[i].y), points[i].x + 20, points[i].y + 20, 10, BLACK);
            if (i < pointCount - 1) DrawLineEx(points[i], points[i + 1], *edgeThickness, SKYBLUE);
        }
    }

    for (int i = 0; i < pointCount; i++)
        DrawCircleV(points[i], (focusedPoint == i)? 30.0f : 24.0f, (focusedPoint == i)? GRAY: LIGHTGRAY);

    // Draw Configs
    if (GuiButton((Rectangle){12, 12, 200, 24}, "Show Adjacency Matrix"))
        *adjacencyMatrixWindowActive = !(*adjacencyMatrixWindowActive);
    GuiLabel((Rectangle){ 12, 62, 140, 24 }, TextFormat("Edge Thickness: %i", (int)*edgeThickness));
    GuiSliderBar((Rectangle){ 12, 84, 140, 16 }, NULL, NULL, edgeThickness, 1.0f, 40.0f);
    DrawText(csvFilePath, 12, 110, 12, GRAY);

    // Draw Adjacency Matrix Window
    if (*adjacencyMatrixWindowActive)
    {
        *adjacencyMatrixWindowActive = !GuiWindowBox((Rectangle){screenWidth - 312, 12, 300, 300}, "Adjacency Matrix");
    }
}
//----------------------------------------------------------------------------------
