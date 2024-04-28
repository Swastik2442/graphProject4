#ifndef _UTILS_H_
#define _UTILS_H_

#include "raylib.h"

void createPointPolygon(
    Vector2 *points, // Points Array
    int pointCount,  // Point Count
    Vector2 center,  // Polygon Center
    int radius       // Polygon Radius
);
void drawArrow(
    Vector2 pointA, // Source Point
    Vector2 pointB, // Destination Point
    Color color     // Color
);
void delay(
    int intensity // Delay Intensity
);

#endif
