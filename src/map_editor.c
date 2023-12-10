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

char *filePath = NULL;
bool fileDropped = false;

cJSON *configJson = NULL; // Global JSON object
cJSON *structures = NULL; // Global JSON array

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Wee Boats Map Editor");

    fileDropped = false;

    // Allocate space for the required file path
    filePath = (char *)RL_CALLOC(MAX_FILEPATH_SIZE, 1);

    SetTargetFPS(1);
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
        char *jsonData = (char *)malloc(dataSize + 1);
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
                char *spritePath = cJSON_GetObjectItem(structure, "sprite_path")->valuestring;
                cJSON *location = cJSON_GetObjectItem(structure, "location");
                if (location != NULL)
                {
                    int x = cJSON_GetArrayItem(location, 0)->valueint;
                    int y = cJSON_GetArrayItem(location, 1)->valueint;

                    // Use x and y here, for example, to position sprites
                    printf("Structure location: (%d, %d)\n", x, y);
                }
                // ... Access other properties as needed

                printf("Structure Name: %s, Sprite Path: %s\n", structureName, spritePath);
            }
        }
    }
}

void Draw()
{
    BeginDrawing();

    ClearBackground(RAYWHITE);

    if (fileDropped == false)
    {
        DrawText("Wee Boats Map Editor", 100, 40, 40, DARKGRAY);
        DrawText("Drop the structure config JSON file onto the window...", 100, 100, 20, DARKGRAY);
    }
    else
    {
        // A file has been dropped and we now need to parse the text
        DrawText(filePath, 120, 100 + 80, 10, GRAY);
        cJSON *a_structure = cJSON_GetArrayItem(structures, 0);

        // Add a copy of the first structure to the end of the array
        cJSON *copy = cJSON_Duplicate(a_structure, 1);
        cJSON_AddItemToArray(structures, copy);

        int structureCount = cJSON_GetArraySize(structures);
        DrawText(TextFormat("Structure Count: %d", structureCount), 120, 100 + 100, 10, GRAY);
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
