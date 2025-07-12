#include "boost_gate.h"
#include <stdio.h>

// Static helper function to add a new a-b point pair
static void AddPairedPoint(cJSON *point_array, Vector2 cameraOffset, float displayScale)
{
    cJSON *new_pair = cJSON_CreateObject();
    
    // Calculate the center of the screen in world coordinates
    int center_x = (int)((GetScreenWidth() / 2 - cameraOffset.x) / displayScale);
    int center_y = -(int)((GetScreenHeight() / 2 - cameraOffset.y) / displayScale); // Flip y-axis for consistency

    // Create the two endpoints for the new gate
    cJSON *new_a = cJSON_CreateIntArray((int[]){center_x - 50, center_y}, 2);
    cJSON *new_b = cJSON_CreateIntArray((int[]){center_x + 50, center_y}, 2);
    
    cJSON_AddItemToObject(new_pair, "a", new_a);
    cJSON_AddItemToObject(new_pair, "b", new_b);
    cJSON_AddItemToArray(point_array, new_pair);
}

// Public function to add a boost gate
void AddBoostGate(cJSON *boost_gates, Vector2 cameraOffset, float displayScale)
{
    if (boost_gates)
    {
        AddPairedPoint(boost_gates, cameraOffset, displayScale);
    }
}

// Public function to update boost gate positions
void UpdateBoostGates(cJSON *boost_gates, Vector2 cameraOffset, float *displayScale)
{
    if (!IsMouseButtonDown(MOUSE_LEFT_BUTTON) || !boost_gates) return;
    
    Vector2 mouse = GetMousePosition();
    Vector2 transformedMouse = {mouse.x - cameraOffset.x, mouse.y - cameraOffset.y};

    cJSON *point_pair = NULL;
    cJSON_ArrayForEach(point_pair, boost_gates)
    {
        cJSON *a = cJSON_GetObjectItem(point_pair, "a");
        cJSON *b = cJSON_GetObjectItem(point_pair, "b");

        Vector2 a_vec = {(cJSON_GetArrayItem(a, 0)->valueint) * *displayScale, -(cJSON_GetArrayItem(a, 1)->valueint) * *displayScale};
        Vector2 b_vec = {(cJSON_GetArrayItem(b, 0)->valueint) * *displayScale, -(cJSON_GetArrayItem(b, 1)->valueint) * *displayScale};

        if (CheckCollisionPointCircle(transformedMouse, a_vec, 10.0f))
        {
            cJSON *new_a = cJSON_CreateIntArray((int[]){transformedMouse.x / *displayScale, -transformedMouse.y / *displayScale}, 2);
            cJSON_ReplaceItemInObjectCaseSensitive(point_pair, "a", new_a);
        }
        else if (CheckCollisionPointCircle(transformedMouse, b_vec, 10.0f))
        {
            cJSON *new_b = cJSON_CreateIntArray((int[]){transformedMouse.x / *displayScale, -transformedMouse.y / *displayScale}, 2);
            cJSON_ReplaceItemInObjectCaseSensitive(point_pair, "b", new_b);
        }
    }
}

// Public function to draw boost gates
void DrawBoostGates(cJSON *boost_gates, Vector2 cameraOffset, float *displayScale)
{
    if (!boost_gates) return;
    
    cJSON *point_pair = NULL;
    cJSON_ArrayForEach(point_pair, boost_gates)
    {
        cJSON *a = cJSON_GetObjectItem(point_pair, "a");
        cJSON *b = cJSON_GetObjectItem(point_pair, "b");
        
        Vector2 posA = {(cJSON_GetArrayItem(a, 0)->valueint) * *displayScale + cameraOffset.x, -(cJSON_GetArrayItem(a, 1)->valueint) * *displayScale + cameraOffset.y};
        Vector2 posB = {(cJSON_GetArrayItem(b, 0)->valueint) * *displayScale + cameraOffset.x, -(cJSON_GetArrayItem(b, 1)->valueint) * *displayScale + cameraOffset.y};

        DrawLineEx(posA, posB, 3, ORANGE);
        DrawCircleV(posA, 10, ORANGE);
        DrawCircleV(posB, 10, ORANGE);
    }
}
