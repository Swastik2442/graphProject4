#include "utils.h"

#include <math.h>
#include <unistd.h>

// Create a Polygon of Points
//----------------------------------------------------------------------------------
void createPointPolygon(Vector2 *points, int pointCount, Vector2 center, int radius)
{
    int arrSize = (pointCount <= MAX_VERTICES) ? pointCount : MAX_VERTICES;
    for (int i = 0; i < arrSize; i++)
        points[i] = (Vector2){center.x + radius * cos(2 * PI * i / arrSize), center.y + radius * sin(2 * PI * i / arrSize)};
}
//----------------------------------------------------------------------------------

// Draw an Arrow between the two Points
//----------------------------------------------------------------------------------
void drawArrow(Vector2 pointA, Vector2 pointB, Color color)
{
    Vector2 distPoint = {pointB.x - pointA.x, pointB.y - pointA.y};
    float angle = atan2f(distPoint.y, distPoint.x);
    Vector2 arrowEndPoint = {pointB.x - (22)*cosf(angle), pointB.y - (22)*sinf(angle)};
    Vector2 arrowBasePoint = {arrowEndPoint.x - (22)*cosf(angle), arrowEndPoint.y - (22)*sinf(angle)};
    float side1 = arrowEndPoint.x - pointA.x;
    float side2 = arrowEndPoint.y - pointA.y;
    Vector2 endPoint1 = {arrowBasePoint.x - side2 / 24, arrowBasePoint.y + side1 / 24};
    Vector2 endPoint2 = {arrowBasePoint.x + side2 / 24, arrowBasePoint.y - side1 / 24};
    DrawTriangle(endPoint2, endPoint1, arrowEndPoint, color);
}
//----------------------------------------------------------------------------------

// Cause Delay in Execution
//----------------------------------------------------------------------------------
void delay(int intensity)
{
    sleep(intensity);
}
//----------------------------------------------------------------------------------
