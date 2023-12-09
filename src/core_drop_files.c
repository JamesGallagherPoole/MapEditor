/*******************************************************************************************
 *
 *   raylib [core] example - Windows drop files
 *
 *   NOTE: This example only works on platforms that support drag & drop (Windows, Linux, OSX, Html5?)
 *
 *   Example originally created with raylib 1.3, last time updated with raylib 4.2
 *
 *   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
 *   BSD-like license that allows static linking with closed source software
 *
 *   Copyright (c) 2015-2023 Ramon Santamaria (@raysan5)
 *
 ********************************************************************************************/

#include "raylib.h"

#include <stdlib.h> // Required for: calloc(), free()

#define MAX_FILEPATH_RECORDED 4096
#define MAX_FILEPATH_SIZE 2048

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

    bool fileDropped = false;
    char *filePaths[MAX_FILEPATH_RECORDED] = {0}; // We will register a maximum of filepaths

    // Allocate space for the required file paths
    for (int i = 0; i < MAX_FILEPATH_RECORDED; i++)
    {
        filePaths[i] = (char *)RL_CALLOC(MAX_FILEPATH_SIZE, 1);
    }

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsFileDropped())
        {
            FilePathList droppedFiles = LoadDroppedFiles();

            for (int i = 0; i < (int)droppedFiles.count; i++)
            {
                TextCopy(filePaths[i], droppedFiles.paths[i]);
                fileDropped = true;
            }

            UnloadDroppedFiles(droppedFiles); // Unload filepaths from memory
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
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
            DrawText(filePaths[0], 120, 100 + 80, 10, GRAY);
        }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    for (int i = 0; i < MAX_FILEPATH_RECORDED; i++)
    {
        RL_FREE(filePaths[i]); // Free allocated memory for all filepaths
    }

    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
