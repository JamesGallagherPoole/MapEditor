#include "cJSON.h"
#include "raylib.h"

void UpdateSnowRegions(cJSON *snow_regions, Vector2 cameraOffset, float *displayScale)
{
    Vector2 mouse = GetMousePosition();

    // Transform the mouse position to take into acount the moved camera
    Vector2 transformedMouse = {mouse.x - cameraOffset.x, mouse.y - cameraOffset.y};

    cJSON *snow_region = NULL;
    cJSON_ArrayForEach(snow_region, snow_regions)
    {
        cJSON *bounds = cJSON_GetObjectItem(snow_region, "bounds");
        if (bounds != NULL)
        {
            cJSON *min = cJSON_GetObjectItem(bounds, "min");
            cJSON *max = cJSON_GetObjectItem(bounds, "max");

            int min_x = cJSON_GetArrayItem(min, 0)->valueint;
            int min_y = cJSON_GetArrayItem(min, 1)->valueint;

            int max_x = cJSON_GetArrayItem(max, 0)->valueint;
            int max_y = cJSON_GetArrayItem(max, 1)->valueint;

            Vector2 topLeft = {min_x * *displayScale, -(max_y * *displayScale)};
            Vector2 topRight = {max_x * *displayScale, -(max_y * *displayScale)};
            Vector2 bottomLeft = {min_x * *displayScale, -(min_y * *displayScale)};
            Vector2 bottomRight = {max_x * *displayScale, -(min_y * *displayScale)};

            // Transform the mouse position to take into acount the moved camera
            Vector2 transformedMouse = {mouse.x - cameraOffset.x, mouse.y - cameraOffset.y};

            // Set active structure if we hover over it
            if (CheckCollisionPointCircle(transformedMouse, topLeft, 15.0f))
            {
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                {
                    topLeft = transformedMouse;
                    min_x = (int)(topLeft.x / *displayScale);
                    max_y = -(int)(topLeft.y / *displayScale);
                }
            }

            if (CheckCollisionPointCircle(transformedMouse, topRight, 15.0f))
            {
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                {
                    topRight = transformedMouse;
                    max_x = (int)(topRight.x / *displayScale);
                    max_y = -(int)(topRight.y / *displayScale);
                }
            }

            if (CheckCollisionPointCircle(transformedMouse, bottomLeft, 15.0f))
            {
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                {
                    bottomLeft = transformedMouse;
                    min_x = (int)(bottomLeft.x / *displayScale);
                    min_y = -(int)(bottomLeft.y / *displayScale);
                }
            }

            if (CheckCollisionPointCircle(transformedMouse, bottomRight, 15.0f))
            {
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                {
                    bottomRight = transformedMouse;
                    max_x = (int)(bottomRight.x / *displayScale);
                    min_y = -(int)(bottomRight.y / *displayScale);
                }
            }

            cJSON *new_min = cJSON_CreateIntArray((int[]){min_x, min_y}, 2);
            cJSON *new_max = cJSON_CreateIntArray((int[]){max_x, max_y}, 2);
            if (!cJSON_ReplaceItemInObjectCaseSensitive(bounds, "min", new_min))
            {
                printf("Error moving snow region point!");
                return;
            }
            if (!cJSON_ReplaceItemInObjectCaseSensitive(bounds, "max", new_max))
            {
                printf("Error moving snow region point!");
                return;
            }
        }
    }
}

void DrawSnowRegions(cJSON *snow_regions, Vector2 _cameraOffset, float _displayScale)
{
    // Draw the snow regions
    cJSON *snow_region = NULL;
    cJSON_ArrayForEach(snow_region, snow_regions)
    {
        cJSON *bounds = cJSON_GetObjectItem(snow_region, "bounds");
        if (bounds != NULL)
        {
            cJSON *min = cJSON_GetObjectItem(bounds, "min");
            cJSON *max = cJSON_GetObjectItem(bounds, "max");

            int min_x = cJSON_GetArrayItem(min, 0)->valueint;
            int min_y = cJSON_GetArrayItem(min, 1)->valueint;

            int max_x = cJSON_GetArrayItem(max, 0)->valueint;
            int max_y = cJSON_GetArrayItem(max, 1)->valueint;

            printf("Snow Region: (%d, %d) to (%d, %d)\n", min_x, min_y, max_x, max_y);

            // Snow Region Label
            DrawText("Snow Region", (min_x + 20) * _displayScale + _cameraOffset.x, -((max_y - 15) * _displayScale) + _cameraOffset.y, 20, BLUE);
            DrawRectangleLinesEx((Rectangle){min_x * _displayScale + _cameraOffset.x, -(max_y * _displayScale) + _cameraOffset.y, (max_x - min_x) * _displayScale, (max_y - min_y) * _displayScale}, 2, BLUE);
            DrawCircle((min_x + 8) * _displayScale + _cameraOffset.x, -((max_y - 5) * _displayScale) + _cameraOffset.y, 10, BLUE);
            DrawCircle((min_x + 8) * _displayScale + _cameraOffset.x, -((min_y - 5) * _displayScale) + _cameraOffset.y, 10, BLUE);
            DrawCircle((max_x - 8) * _displayScale + _cameraOffset.x, -((max_y - 5) * _displayScale) + _cameraOffset.y, 10, BLUE);
            DrawCircle((max_x - 8) * _displayScale + _cameraOffset.x, -((min_y - 5) * _displayScale) + _cameraOffset.y, 10, BLUE);
        }
    }
}