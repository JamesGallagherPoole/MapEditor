/*******************************************************************************************
 *
 *   Wee Boats Map Editor
 *
 ********************************************************************************************/

#include "raylib.h"
#include "cJSON.h"

#include <stdlib.h> // Required for: calloc(), free()
#include <stdio.h>
#include <string.h>

#define MAX_FILEPATH_SIZE 2048
#define SQUARE_SIZE 31
#define SELECTED_STRUCTURE_FONT_SIZE 20

char *filePath = NULL;
bool fileDropped = false;

cJSON *configJson = NULL; // Global JSON object
cJSON *structures = NULL; // Global JSON array

const int screenWidth = 800;
const int screenHeight = 450;

Vector2 offset = {0};

float displayScale = 0.1f;

int activeStructureIndex = -1;
int selectedStructureIndex = -1;

Vector2 cameraOffset;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------

    InitWindow(screenWidth, screenHeight, "Wee Boats Map Editor");

    fileDropped = false;

    // Allocate space for the required file path
    filePath = (char *)RL_CALLOC(MAX_FILEPATH_SIZE, 1);

    offset.x = screenWidth % SQUARE_SIZE;
    offset.y = screenHeight % SQUARE_SIZE;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        Update();

        Draw();
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    Cleanup();

    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

void Update()
{
    CheckForDroppedFile();

    Vector2 mouse = GetMousePosition();

    cJSON *structure = NULL;
    int structureIndex = 0;
    cJSON_ArrayForEach(structure, structures)
    {
        cJSON *location = cJSON_GetObjectItem(structure, "location");
        if (location != NULL)
        {
            // Negative y to flip it to the expected location
            Vector2 structureLocation = {cJSON_GetArrayItem(location, 0)->valueint, cJSON_GetArrayItem(location, 1)->valueint};
            Vector2 structurePoint = {structureLocation.x * displayScale, -structureLocation.y * displayScale};

            // Transform the mouse position to take into acount the moved camera
            Vector2 transformedMouse = {mouse.x - cameraOffset.x, mouse.y - cameraOffset.y};

            printf("Mouse: (%f, %f)\t Transformed Mouse: (%f, %f)\n", mouse.x, mouse.y, transformedMouse.x, transformedMouse.y);

            // Set active structure if we hover over it
            if (CheckCollisionPointCircle(transformedMouse, structurePoint, 10.0f))
            {
                printf("Hovering over structure %d\n", structureIndex);
                activeStructureIndex = structureIndex;
            }

            // Select structure
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                if (activeStructureIndex != -1)
                {
                    selectedStructureIndex = activeStructureIndex;
                }
            }

            // Move selected structure if we are dragging it
            if (selectedStructureIndex == structureIndex && activeStructureIndex == structureIndex)
            {
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                {
                    printf("Dragging structure %d\n", structureIndex);
                    structurePoint = transformedMouse;
                    cJSON *new_location = cJSON_CreateIntArray((int[]){(int)(structurePoint.x / displayScale), -(int)(structurePoint.y / displayScale)}, 2); // Flipping y again to save it
                    if (!cJSON_ReplaceItemInObjectCaseSensitive(structure, "location", new_location))
                    {
                        printf("Error moving structure!");
                        return 0;
                    }
                }
            }

            // Cancel active structure if we hover outside of it
            if (activeStructureIndex == structureIndex)
            {
                if (!CheckCollisionPointCircle(transformedMouse, structurePoint, 10.0f))
                {
                    activeStructureIndex = -1;
                }
            }
        }
        structureIndex++;
    }
    ControlCamera();
}

void ControlCamera()
{
    if (IsKeyDown(KEY_LEFT))
    {
        cameraOffset.x += 10.0f;
    }
    else if (IsKeyDown(KEY_RIGHT))
    {
        cameraOffset.x -= 10.0f;
    }
    if (IsKeyDown(KEY_UP))
    {
        cameraOffset.y += 10.0f;
    }
    else if (IsKeyDown(KEY_DOWN))
    {
        cameraOffset.y -= 10.0f;
    }
}

void CheckForDroppedFile()
{
    if (fileDropped == true)
    {
        return;
    }

    if (IsFileDropped())
    {
        FilePathList droppedFiles = LoadDroppedFiles();

        for (int i = 0; i < (int)droppedFiles.count; i++)
        {
            TextCopy(filePath, droppedFiles.paths[i]);
            fileDropped = true;
        }

        // Load the file data
        int dataSize = 0;
        unsigned char *fileData = LoadFileData(filePath, &dataSize);

        // Free the old JSON data if it exists
        if (configJson != NULL)
        {
            cJSON_Delete(configJson);
            configJson = NULL;
        }

        // Convert data to a null-terminated string
        char *jsonData = (char *)malloc(dataSize * 2); // Allocate double the size of the file data to allow for editing
        memcpy(jsonData, fileData, dataSize);
        jsonData[dataSize] = '\0';

        // Unload the file data
        UnloadFileData(fileData);
        UnloadDroppedFiles(droppedFiles); // Unload filepaths from memory

        // Parse the JSON data
        configJson = cJSON_Parse(jsonData);
        if (configJson == NULL)
        {
            const char *error_ptr = cJSON_GetErrorPtr();
            if (error_ptr != NULL)
            {
                fprintf(stderr, "Error before: %s\n", error_ptr);
            }
            RL_FREE(jsonData);
            return -1;
        }
        else
        {
            structures = cJSON_GetObjectItem(configJson, "structures");
            RL_FREE(jsonData);
        }

        // Iterate through the structures and parse them
        cJSON *structures = cJSON_GetObjectItem(configJson, "structures");
        if (structures != NULL)
        {
            cJSON *structure = NULL;
            cJSON_ArrayForEach(structure, structures)
            {
                char *structureName = cJSON_GetObjectItem(structure, "name")->valuestring;
                cJSON *location = cJSON_GetObjectItem(structure, "location");
                if (location != NULL)
                {
                    int x = cJSON_GetArrayItem(location, 0)->valueint;
                    int y = cJSON_GetArrayItem(location, 1)->valueint;
                }

                printf("Structure Name: %s\n", structureName);
            }
        }
    }
}

void Draw()
{
    BeginDrawing();

    ClearBackground(RAYWHITE);

    // Draw grid lines
    for (int i = 0; i < screenWidth / SQUARE_SIZE + 1; i++)
    {
        DrawLineV((Vector2){SQUARE_SIZE * i + offset.x / 2, offset.y / 2}, (Vector2){SQUARE_SIZE * i + offset.x / 2, screenHeight - offset.y / 2}, LIGHTGRAY);
    }

    for (int i = 0; i < screenHeight / SQUARE_SIZE + 1; i++)
    {
        DrawLineV((Vector2){offset.x / 2, SQUARE_SIZE * i + offset.y / 2}, (Vector2){screenWidth - offset.x / 2, SQUARE_SIZE * i + offset.y / 2}, LIGHTGRAY);
    }

    if (fileDropped == false)
    {
        DrawText("Wee Boats Map Editor", 100, 40, 40, DARKGRAY);
        DrawText("Drop the structure config JSON file onto the window...", 100, 100, 20, DARKGRAY);
    }
    else
    {
        cJSON *structure = NULL;
        int structureIndex = 0;

        cJSON_ArrayForEach(structure, structures)
        {
            cJSON *location = cJSON_GetObjectItem(structure, "location");
            if (location != NULL)
            {
                int x = cJSON_GetArrayItem(location, 0)->valueint;
                int y = cJSON_GetArrayItem(location, 1)->valueint;

                if (selectedStructureIndex == structureIndex)
                {
                    // Show info about the selected structure on bottom right of screen
                    DrawText(cJSON_GetObjectItem(structure, "name")->valuestring, screenWidth - 300, screenHeight - 150, SELECTED_STRUCTURE_FONT_SIZE, DARKGRAY);
                    DrawText(cJSON_GetObjectItem(structure, "sprite_path")->valuestring, screenWidth - 300, screenHeight - 130, SELECTED_STRUCTURE_FONT_SIZE, DARKGRAY);
                    DrawText(TextFormat("Location: (%d, %d)", x, y), screenWidth - 300, screenHeight - 110, SELECTED_STRUCTURE_FONT_SIZE, DARKGRAY);
                }

                if (activeStructureIndex == structureIndex)
                {
                    DrawCircle((x * displayScale) + cameraOffset.x, -(y * displayScale) + cameraOffset.y, 10, RED);
                }
                else
                {
                    DrawCircle((x * displayScale) + cameraOffset.x, -(y * displayScale) + cameraOffset.y, 10, GREEN);
                }
            }
            structureIndex++;
        }

        int structureCount = cJSON_GetArraySize(structures);
    }

    EndDrawing();
}

void Cleanup()
{
    RL_FREE(filePath); // Free allocated memory for all filepaths

    if (configJson != NULL)
    {
        cJSON_Delete(configJson);
        configJson = NULL;
    }
}
