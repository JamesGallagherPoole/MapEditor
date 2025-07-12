#include "world_area.h"
#include <stdio.h>

void UpdateWorldArea(cJSON *world_area, Vector2 cameraOffset, float *displayScale)
{
    if (world_area == NULL || !IsMouseButtonDown(MOUSE_LEFT_BUTTON)) return;

    Vector2 mouse = GetMousePosition();
    Vector2 transformedMouse = {mouse.x - cameraOffset.x, mouse.y - cameraOffset.y};

    cJSON *bounds = cJSON_GetObjectItem(world_area, "bounds");
    if (bounds != NULL)
    {
        cJSON *min = cJSON_GetObjectItem(bounds, "min");
        cJSON *max = cJSON_GetObjectItem(bounds, "max");

        if (min == NULL || max == NULL) return;

        int min_x = cJSON_GetArrayItem(min, 0)->valueint;
        int min_y = cJSON_GetArrayItem(min, 1)->valueint;
        int max_x = cJSON_GetArrayItem(max, 0)->valueint;
        int max_y = cJSON_GetArrayItem(max, 1)->valueint;

        // Calculate screen coordinates of corners
        float rect_x = min_x * *displayScale;
        float rect_y = -(max_y * *displayScale);
        float rect_width = (max_x - min_x) * *displayScale;
        float rect_height = (max_y - min_y) * *displayScale;

        Vector2 topLeft = {rect_x, rect_y};
        Vector2 topRight = {rect_x + rect_width, rect_y};
        Vector2 bottomLeft = {rect_x, rect_y + rect_height};
        Vector2 bottomRight = {rect_x + rect_width, rect_y + rect_height};

        bool updated = false;
        if (CheckCollisionPointCircle(transformedMouse, topLeft, 15.0f))
        {
            min_x = (int)(transformedMouse.x / *displayScale);
            max_y = -(int)(transformedMouse.y / *displayScale);
            updated = true;
        }
        else if (CheckCollisionPointCircle(transformedMouse, topRight, 15.0f))
        {
            max_x = (int)(transformedMouse.x / *displayScale);
            max_y = -(int)(transformedMouse.y / *displayScale);
            updated = true;
        }
        else if (CheckCollisionPointCircle(transformedMouse, bottomLeft, 15.0f))
        {
            min_x = (int)(transformedMouse.x / *displayScale);
            min_y = -(int)(transformedMouse.y / *displayScale);
            updated = true;
        }
        else if (CheckCollisionPointCircle(transformedMouse, bottomRight, 15.0f))
        {
            max_x = (int)(transformedMouse.x / *displayScale);
            min_y = -(int)(transformedMouse.y / *displayScale);
            updated = true;
        }

        if (updated)
        {
            cJSON *new_min = cJSON_CreateIntArray((int[]){min_x, min_y}, 2);
            cJSON *new_max = cJSON_CreateIntArray((int[]){max_x, max_y}, 2);
            cJSON_ReplaceItemInObjectCaseSensitive(bounds, "min", new_min);
            cJSON_ReplaceItemInObjectCaseSensitive(bounds, "max", new_max);
        }
    }
}

void DrawWorldArea(cJSON *world_area, Vector2 cameraOffset, float *displayScale, const char *headerText, Color color)
{
    if (world_area == NULL) return;

    cJSON *bounds = cJSON_GetObjectItem(world_area, "bounds");
    if (bounds != NULL)
    {
        cJSON *min = cJSON_GetObjectItem(bounds, "min");
        cJSON *max = cJSON_GetObjectItem(bounds, "max");

        if (min == NULL || max == NULL) return;

        int min_x = cJSON_GetArrayItem(min, 0)->valueint;
        int min_y = cJSON_GetArrayItem(min, 1)->valueint;
        int max_x = cJSON_GetArrayItem(max, 0)->valueint;
        int max_y = cJSON_GetArrayItem(max, 1)->valueint;

        float rect_x = min_x * *displayScale + cameraOffset.x;
        float rect_y = -(max_y * *displayScale) + cameraOffset.y;
        float rect_width = (max_x - min_x) * *displayScale;
        float rect_height = (max_y - min_y) * *displayScale;

        DrawRectangleRec((Rectangle){rect_x, rect_y, rect_width, rect_height}, Fade(color, 0.3f));
        DrawRectangleLinesEx((Rectangle){rect_x, rect_y, rect_width, rect_height}, 2, color);
        DrawText(headerText, rect_x + 10, rect_y + 10, 20, color);

        // Draw draggable corners
        DrawCircle(rect_x, rect_y, 10, color);
        DrawCircle(rect_x + rect_width, rect_y, 10, color);
        DrawCircle(rect_x, rect_y + rect_height, 10, color);
        DrawCircle(rect_x + rect_width, rect_y + rect_height, 10, color);
    }
}
