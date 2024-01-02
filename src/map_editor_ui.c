/*******************************************************************************************
*
*   LayoutName v1.0.0 - Tool Description
*
*   LICENSE: Propietary License
*
*   Copyright (c) 2022 raylib technologies. All Rights Reserved.
*
*   Unauthorized copying of this file, via any medium is strictly prohibited
*   This project is proprietary and confidential unless the owner allows
*   usage in any other form by expresely written permission.
*
**********************************************************************************************/

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

//----------------------------------------------------------------------------------
// Controls Functions Declaration
//----------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    // Initialization
    //---------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "layout_name");

    // layout_name: controls initialization
    //----------------------------------------------------------------------------------
    Vector2 snow_region_control_anchor = { 0, 400 };
    
    bool ExportButtonPressed = false;
    bool AddStructuresButtonPressed = false;
    bool show_names_toggleChecked = false;
    bool show_audio_names_toggleChecked = false;
    bool show_region_text_toggleChecked = false;
    bool add_snow_region_buttonPressed = false;
    bool show_snow_region_toggleChecked = false;
    bool reload_config_buttonPressed = false;

    Rectangle layoutRecs[11] = {
        (Rectangle){ 16, 16, 120, 24 },
        (Rectangle){ 16, 56, 120, 24 },
        (Rectangle){ 0, 0, 152, 128 },
        (Rectangle){ 0, 168, 152, 208 },
        (Rectangle){ 16, 184, 24, 24 },
        (Rectangle){ 16, 232, 24, 24 },
        (Rectangle){ 16, 280, 24, 24 },
        (Rectangle){ snow_region_control_anchor.x + 0, snow_region_control_anchor.y + 0, 152, 72 },
        (Rectangle){ snow_region_control_anchor.x + 16, snow_region_control_anchor.y + 24, 120, 24 },
        (Rectangle){ 16, 328, 24, 24 },
        (Rectangle){ 16, 96, 120, 24 },
    };
    //----------------------------------------------------------------------------------

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Implement required update logic
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR))); 

            // raygui: controls drawing
            //----------------------------------------------------------------------------------
            ExportButtonPressed = GuiButton(layoutRecs[0], "Export"); 
            AddStructuresButtonPressed = GuiButton(layoutRecs[1], "Add Structure"); 
            GuiGroupBox(layoutRecs[2], "Structure Options");
            GuiGroupBox(layoutRecs[3], "Visual Controls");
            GuiCheckBox(layoutRecs[4], "Show Names", &show_names_toggleChecked);
            GuiCheckBox(layoutRecs[5], "Show Audio Names", &show_audio_names_toggleChecked);
            GuiCheckBox(layoutRecs[6], "Show Region Text", &show_region_text_toggleChecked);
            GuiGroupBox(layoutRecs[7], "Snow Region Controls");
            add_snow_region_buttonPressed = GuiButton(layoutRecs[8], "Add Snow Region"); 
            GuiCheckBox(layoutRecs[9], "Show Snow Regions", &show_snow_region_toggleChecked);
            reload_config_buttonPressed = GuiButton(layoutRecs[10], "Reload Config"); 
            //----------------------------------------------------------------------------------

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Controls Functions Definitions (local)
//------------------------------------------------------------------------------------

