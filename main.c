#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define MAX_SPLINE_POINTS 64

//------------------------------------------------------------------------------------
// Program Main Entry Point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 720;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Graph Project 4 - Output and Display");

    int n = 6, x = screenWidth/2, y = screenHeight/2, r = 200;
    Vector2 points[MAX_SPLINE_POINTS];
    for (int i = 0; i < n; i++)
        points[i] = (Vector2){x + r * cos(2 * PI * i / n), y + r * sin(2 * PI * i / n)};

    int pointCount = 6;
    int selectedPoint = -1;
    int focusedPoint = -1;

    // Spline config variables
    float splineThickness = 8.0f;
    int splineTypeActive = 0;
    bool splineTypeEditMode = false;
    bool debugInfoActive = true;

    // GuiLoadStyle("./resources/style/gp4_style.rgs");    // Load custom GUI style

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Spline points creation logic (at the end of spline)
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && (pointCount < MAX_SPLINE_POINTS))
        {
            points[pointCount] = GetMousePosition();
            pointCount++;
        }

        // Spline point focus and selection logic
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

        // Spline point movement logic
        if (selectedPoint >= 0)
        {
            points[selectedPoint] = GetMousePosition();
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) selectedPoint = -1;
        }

        // if (IsFileDropped())
        // {
        //     FilePathList droppedFiles = LoadDroppedFiles();

        //     if ((droppedFiles.count > 0) && IsFileExtension(droppedFiles.paths[0], ".csv"))
        //         printf(TextFormat("Check this: %s\n", droppedFiles.paths[0]));

        //     UnloadDroppedFiles(droppedFiles);    // Clear internal buffers
        // }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Draw Debug Information
            if (debugInfoActive)
            {
                DrawFPS(screenWidth - 80, 10);

                DrawLineEx(points[0], points[pointCount - 1], splineThickness, SKYBLUE);
                
                for (int i = 0; i < pointCount; i++)
                {
                    DrawText(TextFormat("(%.2f, %.2f)", points[i].x, points[i].y), points[i].x + 20, points[i].y + 20, 10, BLACK);
                    if (i < pointCount - 1) DrawLineEx(points[i], points[i + 1], splineThickness, SKYBLUE);
                }
            }

            for (int i = 0; i < pointCount; i++)
                DrawCircleV(points[i], (focusedPoint == i)? 30.0f : 24.0f, (focusedPoint == i)? GRAY: LIGHTGRAY);

            // Check all possible UI states that require controls lock
            if (splineTypeEditMode) GuiLock();

            // Draw Configs
            GuiLabel((Rectangle){ 12, 62, 140, 24 }, TextFormat("Spline thickness: %i", (int)splineThickness));
            GuiSliderBar((Rectangle){ 12, 60 + 24, 140, 16 }, NULL, NULL, &splineThickness, 1.0f, 40.0f);

            GuiCheckBox((Rectangle){ 12, 110, 20, 20 }, "Show Debug Info", &debugInfoActive);

            GuiUnlock();

            GuiLabel((Rectangle){ 12, 10, 140, 24 }, "Spline type:");
            if (GuiDropdownBox((Rectangle){ 12, 8 + 24, 160, 28 }, "LINEAR;BSPLINE;CATMULLROM;BEZIER", &splineTypeActive, splineTypeEditMode))
                splineTypeEditMode = !splineTypeEditMode;

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}