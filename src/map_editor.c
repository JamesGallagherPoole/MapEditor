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

const int SCREEN_WIDTH = 1080;
const int SCREEN_HEIGHT = 720;

char *_filePath = NULL;
bool _fileDropped = false;

cJSON *_configJson = NULL; // Global JSON object
cJSON *_structures = NULL; // Global JSON array

Vector2 _gridOffset = {0};

float _displayScale = 0.1f;

int _activeStructureIndex = -1;
int _selectedStructureIndex = -1;

Vector2 _cameraOffset;

// Settings
bool _showNames = true;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Wee Boats Map Editor");

    _fileDropped = false;

    // Allocate space for the required file path
    _filePath = (char *)RL_CALLOC(MAX_FILEPATH_SIZE, 1);

    _gridOffset.x = SCREEN_WIDTH % SQUARE_SIZE;
    _gridOffset.y = SCREEN_HEIGHT % SQUARE_SIZE;

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
    cJSON_ArrayForEach(structure, _structures)
    {
        cJSON *location = cJSON_GetObjectItem(structure, "location");
        if (location != NULL)
        {
            // Negative y to flip it to the expected location
            Vector2 structureLocation = {cJSON_GetArrayItem(location, 0)->valueint, cJSON_GetArrayItem(location, 1)->valueint};
            Vector2 structurePoint = {structureLocation.x * _displayScale, -structureLocation.y * _displayScale};

            // Transform the mouse position to take into acount the moved camera
            Vector2 transformedMouse = {mouse.x - _cameraOffset.x, mouse.y - _cameraOffset.y};

            // printf("Mouse: (%f, %f)\t Transformed Mouse: (%f, %f)\n", mouse.x, mouse.y, transformedMouse.x, transformedMouse.y);

            HandleExportButton((Rectangle){SCREEN_WIDTH - 200, 10, 190, 45});
            HandleAddStructureButton((Rectangle){SCREEN_WIDTH - 200, 60, 190, 45});

            // Set active structure if we hover over it
            if (CheckCollisionPointCircle(transformedMouse, structurePoint, 10.0f))
            {
                printf("Hovering over structure %d\n", structureIndex);
                _activeStructureIndex = structureIndex;
            }

            // Select structure
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                if (_activeStructureIndex != -1)
                {
                    _selectedStructureIndex = _activeStructureIndex;
                }
            }

            // Move selected structure if we are dragging it
            if (_selectedStructureIndex == structureIndex && _activeStructureIndex == structureIndex)
            {
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                {
                    printf("Dragging structure %d\n", structureIndex);
                    structurePoint = transformedMouse;
                    cJSON *new_location = cJSON_CreateIntArray((int[]){(int)(structurePoint.x / _displayScale), -(int)(structurePoint.y / _displayScale)}, 2); // Flipping y again to save it
                    if (!cJSON_ReplaceItemInObjectCaseSensitive(structure, "location", new_location))
                    {
                        printf("Error moving structure!");
                        return 0;
                    }
                }
            }

            // Cancel active structure if we hover outside of it
            if (_activeStructureIndex == structureIndex)
            {
                if (!CheckCollisionPointCircle(transformedMouse, structurePoint, 10.0f))
                {
                    _activeStructureIndex = -1;
                }
            }
        }
        structureIndex++;
    }
    ControlCamera();
    ToggleShowNames(_showNames);
}

int ToggleShowNames(int showNames)
{
    return showNames == 1 ? false : true;
}

void ExportCurrentStructuresToConfigFile()
{
    cJSON_ReplaceItemInObjectCaseSensitive(_configJson, "_structures", _structures);

    char *jsonString = cJSON_Print(_configJson);

    SaveFileText(_filePath, jsonString);
}

void AddStructure()
{
    cJSON *new_structure = cJSON_CreateObject();
    cJSON_AddItemToObject(new_structure, "name", cJSON_CreateString("New Empty Structure!"));
    cJSON_AddItemToObject(new_structure, "location", cJSON_CreateIntArray((int[]){0, 0}, 2));

    cJSON_AddItemToArray(_structures, new_structure);
}

void ControlCamera()
{
    // Pan Camera
    if (IsKeyDown(KEY_LEFT))
    {
        _cameraOffset.x += 10.0f;
    }
    else if (IsKeyDown(KEY_RIGHT))
    {
        _cameraOffset.x -= 10.0f;
    }
    if (IsKeyDown(KEY_UP))
    {
        _cameraOffset.y += 10.0f;
    }
    else if (IsKeyDown(KEY_DOWN))
    {
        _cameraOffset.y -= 10.0f;
    }

    // Zoom Camera
    if (IsKeyDown(KEY_I))
    {
        if (_displayScale < 1.0)
        {
            _displayScale += 0.001f;
        }
    }
    else if (IsKeyDown(KEY_O))
    {
        if (_displayScale > 0.05)
        {
            _displayScale -= 0.001f;
        }
    }

    // Toggle Show Names
    if (IsKeyPressed(KEY_N))
    {
        _showNames = ToggleShowNames(_showNames);
    }
}

void CheckForDroppedFile()
{
    if (_fileDropped == true)
    {
        return;
    }

    if (IsFileDropped())
    {
        FilePathList droppedFiles = LoadDroppedFiles();

        for (int i = 0; i < (int)droppedFiles.count; i++)
        {
            TextCopy(_filePath, droppedFiles.paths[i]);
            _fileDropped = true;
        }

        // Load the file data
        int dataSize = 0;
        unsigned char *fileData = LoadFileData(_filePath, &dataSize);

        // Free the old JSON data if it exists
        if (_configJson != NULL)
        {
            cJSON_Delete(_configJson);
            _configJson = NULL;
        }

        // Convert data to a null-terminated string
        char *jsonData = (char *)malloc(dataSize * 2); // Allocate double the size of the file data to allow for editing
        memcpy(jsonData, fileData, dataSize);
        jsonData[dataSize] = '\0';

        // Unload the file data
        UnloadFileData(fileData);
        UnloadDroppedFiles(droppedFiles); // Unload filepaths from memory

        // Parse the JSON data
        _configJson = cJSON_Parse(jsonData);
        if (_configJson == NULL)
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
            _structures = cJSON_GetObjectItem(_configJson, "structures");
            RL_FREE(jsonData);
        }

        // Iterate through the structures and parse them
        cJSON *_structures = cJSON_GetObjectItem(_configJson, "structures");
        if (_structures != NULL)
        {
            cJSON *structure = NULL;
            cJSON_ArrayForEach(structure, _structures)
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
    for (int i = 0; i < SCREEN_WIDTH / SQUARE_SIZE + 1; i++)
    {
        DrawLineV((Vector2){SQUARE_SIZE * i + _gridOffset.x / 2, _gridOffset.y / 2}, (Vector2){SQUARE_SIZE * i + _gridOffset.x / 2, SCREEN_WIDTH - _gridOffset.y / 2}, LIGHTGRAY);
    }

    for (int i = 0; i < SCREEN_WIDTH / SQUARE_SIZE + 1; i++)
    {
        DrawLineV((Vector2){_gridOffset.x / 2, SQUARE_SIZE * i + _gridOffset.y / 2}, (Vector2){SCREEN_WIDTH - _gridOffset.x / 2, SQUARE_SIZE * i + _gridOffset.y / 2}, LIGHTGRAY);
    }

    if (_fileDropped == false)
    {
        DrawText("Wee Boats Map Editor", 100, 40, 40, DARKGRAY);
        DrawText("Drop the structure config JSON file onto the window...", 100, 100, 20, DARKGRAY);
    }
    else
    {
        cJSON *structure = NULL;
        int structureIndex = 0;

        cJSON_ArrayForEach(structure, _structures)
        {
            cJSON *location = cJSON_GetObjectItem(structure, "location");
            if (location != NULL)
            {
                int x = cJSON_GetArrayItem(location, 0)->valueint;
                int y = cJSON_GetArrayItem(location, 1)->valueint;

                // Draw the structure
                if (_activeStructureIndex == structureIndex)
                {
                    DrawCircle((x * _displayScale) + _cameraOffset.x, -(y * _displayScale) + _cameraOffset.y, 10, RED);
                    if (_showNames == true)
                    {
                        DrawText(cJSON_GetObjectItem(structure, "name")->valuestring, (x * _displayScale) + _cameraOffset.x, -(y * _displayScale) + _cameraOffset.y, 20, DARKGRAY);
                    }
                }
                else
                {
                    DrawCircle((x * _displayScale) + _cameraOffset.x, -(y * _displayScale) + _cameraOffset.y, 10, GREEN);
                    if (_showNames == true)
                    {
                        DrawText(cJSON_GetObjectItem(structure, "name")->valuestring, (x * _displayScale) + _cameraOffset.x, -(y * _displayScale) + _cameraOffset.y, 20, DARKGRAY);
                    }
                }

                // Show info about the selected structure on bottom right of screen
                if (_selectedStructureIndex == structureIndex)
                {
                    DrawRectangle(SCREEN_WIDTH - 330, SCREEN_HEIGHT - 200, 290, 190, LIGHTGRAY);
                    DrawText(cJSON_GetObjectItem(structure, "name")->valuestring, SCREEN_WIDTH - 300, SCREEN_HEIGHT - 150, SELECTED_STRUCTURE_FONT_SIZE, DARKGRAY);
                    DrawText(TextFormat("Location: (%d, %d)", x, y), SCREEN_WIDTH - 300, SCREEN_HEIGHT - 110, SELECTED_STRUCTURE_FONT_SIZE, DARKGRAY);
                }
            }
            structureIndex++;
        }

        // Draw a button in the top right that says Export
        Rectangle exportButton = {SCREEN_WIDTH - 200, 10, 190, 45};
        DrawInteractiveButton(exportButton, "Export");

        // Draw a button that allows the user to add a structure
        Rectangle addStructureButton = {SCREEN_WIDTH - 200, 60, 190, 45};
        DrawInteractiveButton(addStructureButton, "Add Structure");

        // Draw the program commands in the bottom left
        DrawText("Commands:", 10, SCREEN_HEIGHT - 100, 20, DARKGRAY);
        DrawText("Move Camera: Arrow Keys", 10, SCREEN_HEIGHT - 80, 20, DARKGRAY);
        DrawText("Zoom Camera: I/O Keys", 10, SCREEN_HEIGHT - 60, 20, DARKGRAY);
        DrawText("Toggle Show Names: N Key", 10, SCREEN_HEIGHT - 40, 20, DARKGRAY);
    }

    EndDrawing();
}

void Cleanup()
{
    RL_FREE(_filePath); // Free allocated memory for all filepaths

    if (_configJson != NULL)
    {
        cJSON_Delete(_configJson);
        _configJson = NULL;
    }
}
