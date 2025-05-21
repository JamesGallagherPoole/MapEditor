/*******************************************************************************************
 *
 *   Wee Boats Map Editor
 *
 ********************************************************************************************/

#include "raylib.h"
#include "cJSON.h"
#include "map_editor.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <stdlib.h> // Required for: calloc(), free()
#include <stdio.h>
#include <string.h>

#define MAX_FILEPATH_SIZE 2048
#define SQUARE_SIZE 31
#define SELECTED_STRUCTURE_FONT_SIZE 20

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

FilePathList _droppedFiles;
char *_filePath = NULL;
bool _fileDropped = false;

cJSON *_configJson = NULL; // Global JSON object
cJSON *_structures = NULL; // Global JSON array
cJSON *_regions = NULL;    // Global JSON array
cJSON *_snow_regions = NULL;
cJSON *_rain_regions = NULL;
cJSON *_star_regions = NULL;
cJSON* _boost_gates = NULL;

float _displayScale = 0.5f;
Vector2 _cameraOffset;

int _activeStructureIndex = -1;
int _selectedStructureIndex = -1;

// Settings
bool _showNames = true;
bool _showAudio = true;
bool _showRegionNames = true;
bool _showSnowRegions = true;
bool _showRainRegions = true;
bool _showStarRegions = true;
bool _showBoostGates = true;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
   // SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Wee Boats Map Editor");

    _fileDropped = false;

    // Allocate space for the required file path
    _filePath = (char *)RL_CALLOC(MAX_FILEPATH_SIZE, 1);

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

            // Set active structure if we hover over it
            if (CheckCollisionPointCircle(transformedMouse, structurePoint, 10.0f))
            {
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

    // Move snow region points
    if (_showSnowRegions == true)
    {
        UpdateSnowRegions(_snow_regions, _cameraOffset, &_displayScale);
    }

    if (_showRainRegions == true)
    {
        UpdateSnowRegions(_rain_regions, _cameraOffset, &_displayScale);
    }

    if (_showStarRegions == true)
    {
        UpdateSnowRegions(_star_regions, _cameraOffset, &_displayScale);
    }

    if (_showBoostGates == true)
    {
        UpdateBoostGates(_boost_gates, _cameraOffset, &_displayScale);
    }

    ControlCamera();
}

void ToggleShowNames()
{
    _showNames = !_showNames;
}

void ExportCurrentStructuresToConfigFile()
{
    cJSON_ReplaceItemInObjectCaseSensitive(_configJson, "structures", _structures);

    char *jsonString = cJSON_Print(_configJson);

    SaveFileText(_filePath, jsonString);
}

void ExportCurrentSnowRegionsToConfigFile()
{
    cJSON_ReplaceItemInObjectCaseSensitive(_configJson, "snow_regions", _snow_regions);

    char *jsonString = cJSON_Print(_configJson);

    SaveFileText(_filePath, jsonString);
}

void AddStructure()
{
    printf("Adding new structure!\n");

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
        ToggleShowNames();
    }
}

void LoadJsonData()
{
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
    // UnloadDroppedFiles(_droppedFiles); // Unload filepaths from memory

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
        _regions = cJSON_GetObjectItem(_configJson, "regions");
        _snow_regions = cJSON_GetObjectItem(_configJson, "snow_regions");
        _rain_regions = cJSON_GetObjectItem(_configJson, "rain_regions");
        _star_regions = cJSON_GetObjectItem(_configJson, "star_regions");
        _boost_gates = cJSON_GetObjectItem(_configJson, "boost_gates");
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

void CheckForDroppedFile()
{
    if (_fileDropped == true)
    {
        return;
    }

    if (IsFileDropped())
    {
        _droppedFiles = LoadDroppedFiles();

        for (int i = 0; i < (int)_droppedFiles.count; i++)
        {
            TextCopy(_filePath, _droppedFiles.paths[i]);
            _fileDropped = true;
        }

        LoadJsonData();
    }
}

void UpdateBoostGates(cJSON *boost_gates, Vector2 cameraOffset, float *displayScale)
{
    Vector2 mouse = GetMousePosition();

    // Transform the mouse position to take into acount the moved camera
    Vector2 transformedMouse = {mouse.x - cameraOffset.x, mouse.y - cameraOffset.y};

    cJSON *boost_gate = NULL;
    cJSON_ArrayForEach(boost_gate, boost_gates)
    {
		cJSON *a = cJSON_GetObjectItem(boost_gate, "a");
		cJSON *b = cJSON_GetObjectItem(boost_gate, "b");

		int a_x = cJSON_GetArrayItem(a, 0)->valueint;
		int a_y = cJSON_GetArrayItem(a, 1)->valueint;

		int b_x = cJSON_GetArrayItem(b, 0)->valueint;
		int b_y = cJSON_GetArrayItem(b, 1)->valueint;

		Vector2 a_vec = {a_x * *displayScale, -(a_y * *displayScale)};
		Vector2 b_vec = {b_x * *displayScale, -(b_y * *displayScale)};

		// Transform the mouse position to take into acount the moved camera
		Vector2 transformedMouse = {mouse.x - cameraOffset.x, mouse.y - cameraOffset.y};

		// Set active structure if we hover over it
		if (CheckCollisionPointCircle(transformedMouse, a_vec, 20.0f))
		{
			if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
			{
				a_vec = transformedMouse;
				a_x = (int)(a_vec.x / *displayScale);
				a_y = -(int)(a_vec.y / *displayScale);
			}
		}

		if (CheckCollisionPointCircle(transformedMouse, b_vec, 20.0f))
		{
			if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
			{
				b_vec = transformedMouse;
				b_x = (int)(b_vec.x / *displayScale);
				b_y = -(int)(b_vec.y / *displayScale);
			}
		}

		cJSON *new_a = cJSON_CreateIntArray((int[]){a_x, a_y}, 2);
		cJSON *new_b = cJSON_CreateIntArray((int[]){b_x, b_y}, 2);
		if (!cJSON_ReplaceItemInObjectCaseSensitive(boost_gate, "a", new_a))
		{
			printf("Error moving snow region point!");
			return;
		}
		if (!cJSON_ReplaceItemInObjectCaseSensitive(boost_gate, "b", new_b))
		{
			printf("Error moving snow region point!");
			return;
		}
    }
}

void DrawBoostGates(cJSON *boost_gates, Vector2 _cameraOffset, float *_displayScale)
{
    // Draw the snow regions
    cJSON *boost_gate = NULL;
    cJSON_ArrayForEach(boost_gate, boost_gates)
    {
		cJSON *a = cJSON_GetObjectItem(boost_gate, "a");
		cJSON *b = cJSON_GetObjectItem(boost_gate, "b");

		int a_x = cJSON_GetArrayItem(a, 0)->valueint;
		int a_y = cJSON_GetArrayItem(a, 1)->valueint;

		int b_x = cJSON_GetArrayItem(b, 0)->valueint;
		int b_y = cJSON_GetArrayItem(b, 1)->valueint;

		// Snow Region Label
        DrawLine(
            a_x * *_displayScale + _cameraOffset.x,
            -a_y * *_displayScale + _cameraOffset.y,
            b_x * *_displayScale + _cameraOffset.x,
            -b_y * *_displayScale + _cameraOffset.y,
            ORANGE
        );
		DrawCircle((a_x + 8)** _displayScale + _cameraOffset.x, -((a_y - 5) * *_displayScale) + _cameraOffset.y, 10, ORANGE);
		DrawCircle((b_x + 8) * *_displayScale + _cameraOffset.x, -((b_y - 5) * *_displayScale) + _cameraOffset.y, 10, ORANGE);
    }
}

void AddBoostGate(cJSON *boost_gates)
{
    cJSON *new_boost_gate = cJSON_CreateObject();

    // Create a new min and max array at the center of the screen
    cJSON *new_a = cJSON_CreateIntArray((int[]){-50, -50}, 2);
    cJSON *new_b = cJSON_CreateIntArray((int[]){100, 100}, 2);

    cJSON_AddItemToObject(new_boost_gate, "a", new_a);
    cJSON_AddItemToObject(new_boost_gate, "b", new_b);

    cJSON_AddItemToArray(boost_gates, new_boost_gate);
}

void Draw()
{
    BeginDrawing();

    ClearBackground(RAYWHITE);

    if (_fileDropped == false)
    {
        Vector2 _gridOffset = {0};
        _gridOffset.x = SCREEN_WIDTH % SQUARE_SIZE;
        _gridOffset.y = SCREEN_HEIGHT % SQUARE_SIZE;

        // Draw grid lines
        for (int i = 0; i < SCREEN_WIDTH / SQUARE_SIZE + 1; i++)
        {
            DrawLineV((Vector2){SQUARE_SIZE * i + _gridOffset.x / 2, _gridOffset.y / 2}, (Vector2){SQUARE_SIZE * i + _gridOffset.x / 2, SCREEN_WIDTH - _gridOffset.y / 2}, LIGHTGRAY);
        }

        for (int i = 0; i < SCREEN_WIDTH / SQUARE_SIZE + 1; i++)
        {
            DrawLineV((Vector2){_gridOffset.x / 2, SQUARE_SIZE * i + _gridOffset.y / 2}, (Vector2){SCREEN_WIDTH - _gridOffset.x / 2, SQUARE_SIZE * i + _gridOffset.y / 2}, LIGHTGRAY);
        }
        DrawText("Wee Boats Map Editor", 100, 40, 40, DARKGRAY);
        DrawText("Drop the structure config JSON file onto the window...", 100, 100, 20, DARKGRAY);
    }
    else
    {
        cJSON *structure = NULL;
        int structureIndex = 0;

        // Draw grid lines
        DrawLineEx((Vector2){_cameraOffset.x, 0}, (Vector2){_cameraOffset.x, SCREEN_HEIGHT}, 2, RED);
        DrawLineEx((Vector2){0, _cameraOffset.y}, (Vector2){SCREEN_WIDTH, _cameraOffset.y}, 2, RED);

        cJSON_ArrayForEach(structure, _structures)
        {
            cJSON *location = cJSON_GetObjectItem(structure, "location");
            if (location != NULL)
            {
                int x = cJSON_GetArrayItem(location, 0)->valueint;
                int y = cJSON_GetArrayItem(location, 1)->valueint;

                // Region Colours and Names
                Color structureColor = GREEN;
                Color activeColor = RED;
                char* regionName = NULL;

                // An array of colours for each region
                Color _regionColors[] = { PINK, ORANGE, SKYBLUE, PURPLE, BROWN, BEIGE, VIOLET, GOLD, LIME };

                int hasRegionId = cJSON_HasObjectItem(structure, "region_id");
                if (hasRegionId == true)
                {
                    int regionId = cJSON_GetObjectItem(structure, "region_id")->valueint;

                    if (regionId > sizeof(_regionColors) - 1)
                    {
                        printf("Region ID %d is out of bounds!\nWe need to add the region to the Map Editor!\n", regionId);
                    }

                    structureColor = _regionColors[regionId];

                    cJSON *region = cJSON_GetArrayItem(_regions, regionId);
                    regionName = cJSON_GetObjectItem(region, "name")->valuestring;
                }

                // Draw the structure
                if (_activeStructureIndex == structureIndex)
                {
                    DrawCircle((x * _displayScale) + _cameraOffset.x, -(y * _displayScale) + _cameraOffset.y, 10, activeColor);
                }
                else
                {
                    DrawCircle((x * _displayScale) + _cameraOffset.x, -(y * _displayScale) + _cameraOffset.y, 10, structureColor);
                }

                if (_showNames == true)
                {
                    DrawText(cJSON_GetObjectItem(structure, "name")->valuestring, (x * _displayScale) + _cameraOffset.x, -(y * _displayScale) + _cameraOffset.y, 20, DARKGRAY);
                }
                if (_showAudio == true)
                {
                    cJSON *audio_sources = cJSON_GetObjectItem(structure, "audio_sources");
                    if (audio_sources != NULL)
                    {
                        cJSON *audio = NULL;
                        cJSON_ArrayForEach(audio, audio_sources)
                        {
                            DrawCircleLines((x * _displayScale) + _cameraOffset.x, -(y * _displayScale) + _cameraOffset.y, 20, BLUE);
                            DrawText(cJSON_GetObjectItem(audio, "file_path")->valuestring, (x * _displayScale) + _cameraOffset.x, -(y * _displayScale) + _cameraOffset.y - 20, 20, BLUE);
                        }
                    }
                }
                if (_showRegionNames == true)
                {
                    DrawText(regionName, (x * _displayScale) + _cameraOffset.x, -(y * _displayScale) + _cameraOffset.y + 20, 20, structureColor);
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

        if (_showSnowRegions == true)
        {
            DrawSnowRegions(_snow_regions, _cameraOffset, &_displayScale, "Snow Region");
        }

        if (_showRainRegions == true)
        {
            DrawSnowRegions(_rain_regions, _cameraOffset, &_displayScale, "Rain Region");
        }

        if (_showStarRegions == true)
        {
            DrawSnowRegions(_star_regions, _cameraOffset, &_displayScale, "Star Region");
        }

        if (_showBoostGates == true)
        {
            DrawBoostGates(_boost_gates, _cameraOffset, &_displayScale);
        }

        // Structure Options
        Vector2 right_panel_anchor = {SCREEN_WIDTH - 200, 20};
        Vector2 snow_region_control_anchor = {right_panel_anchor.x, right_panel_anchor.y + 550};
        Vector2 rain_region_control_anchor = {right_panel_anchor.x, right_panel_anchor.y + 650};
        Vector2 star_region_control_anchor = {right_panel_anchor.x, right_panel_anchor.y + 750};
        Vector2 boost_gates_control_anchor = {right_panel_anchor.x, right_panel_anchor.y + 850};

        Rectangle exportButton = {right_panel_anchor.x + 16, right_panel_anchor.y + 16, 120, 24};
        Rectangle addStructureButton = {right_panel_anchor.x + 16, right_panel_anchor.y + 56, 120, 24};
        Rectangle reloadConfigButton = {right_panel_anchor.x + 16, right_panel_anchor.y + 96, 120, 24};
        Rectangle structureOptionsBox = {right_panel_anchor.x, right_panel_anchor.y, 152, 128};

        GuiGroupBox(structureOptionsBox, "Structure Options");
        if (GuiButton(exportButton, "Export"))
        {
            ExportCurrentStructuresToConfigFile();
            ExportCurrentSnowRegionsToConfigFile();
        }
        if (GuiButton(addStructureButton, "Add Structure"))
        {
            AddStructure();
        }
        if (GuiButton(reloadConfigButton, "Reload Config"))
        {
            LoadJsonData();
        }

        // Visual Controls
        Rectangle visualControlsBox = {right_panel_anchor.x + 0, right_panel_anchor.y + 158, 152, 300};
        Rectangle showNamesToggle = {right_panel_anchor.x + 24, right_panel_anchor.y + 184, 24, 24};
        Rectangle showAudioNamesToggle = {right_panel_anchor.x + 24, right_panel_anchor.y + 232, 24, 24};
        Rectangle showRegionNamesToggle = {right_panel_anchor.x + 24, right_panel_anchor.y + 280, 24, 24};
        Rectangle showSnowRegionToggle = {right_panel_anchor.x + 24, right_panel_anchor.y + 328, 24, 24};
        Rectangle showRainRegionToggle = {right_panel_anchor.x + 24, right_panel_anchor.y + 376, 24, 24};
        Rectangle showStarRegionToggle = {right_panel_anchor.x + 24, right_panel_anchor.y + 424, 24, 24};

        GuiGroupBox(visualControlsBox, "Visual Controls");
        GuiCheckBox(showNamesToggle, "Show Names", &_showNames);
        GuiCheckBox(showAudioNamesToggle, "Show Audio Names", &_showAudio);
        GuiCheckBox(showRegionNamesToggle, "Show Region Text", &_showRegionNames);
        GuiCheckBox(showSnowRegionToggle, "Show Snow Regions", &_showSnowRegions);
        GuiCheckBox(showRainRegionToggle, "Show Rain Regions", &_showRainRegions);
        GuiCheckBox(showStarRegionToggle, "Show Star Regions", &_showStarRegions);

        if (_showSnowRegions)
        {
            GuiGroupBox((Rectangle){snow_region_control_anchor.x + 0, snow_region_control_anchor.y + 0, 152, 72}, "Snow Region Controls");
            if (GuiButton((Rectangle){snow_region_control_anchor.x + 16, snow_region_control_anchor.y + 24, 120, 24}, "Add Snow Region"))
            {
                AddSnowRegion(_snow_regions);
            }
        }

        if (_showRainRegions)
        {
            GuiGroupBox((Rectangle){rain_region_control_anchor.x + 0, rain_region_control_anchor.y + 0, 152, 72}, "Rain Region Controls");
            if (GuiButton((Rectangle){rain_region_control_anchor.x + 16, rain_region_control_anchor.y + 24, 120, 24}, "Add Rain Region"))
            {
                AddSnowRegion(_rain_regions);
            }
        }

        if (_showStarRegions)
        {
            GuiGroupBox((Rectangle){star_region_control_anchor.x + 0, star_region_control_anchor.y + 0, 152, 72}, "Star Region Controls");
            if (GuiButton((Rectangle){star_region_control_anchor.x + 16, star_region_control_anchor.y + 24, 120, 24}, "Add Star Region"))
            {
                AddSnowRegion(_star_regions);
            }
        }

        if (_showBoostGates) {
            GuiGroupBox((Rectangle){boost_gates_control_anchor.x + 0, boost_gates_control_anchor.y + 0, 152, 72}, "Boost Gate Controls");
            if (GuiButton((Rectangle){boost_gates_control_anchor.x + 16, boost_gates_control_anchor.y + 24, 120, 24}, "Add Boost Gate"))
            {
                AddBoostGate(_boost_gates);
            }
        }

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

    UnloadDroppedFiles(_droppedFiles); // Unload filepaths from memory
}
