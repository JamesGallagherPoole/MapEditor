/*******************************************************************************************
 *
 * Wee Boats Map Editor
 *
 ********************************************************************************************/

#include "raylib.h"
#include "cJSON.h"
#include "map_editor.h"

// Include headers for all editable element types
#include "snow_region.h"
#include "world_area.h"
#include "boost_gate.h"
#include "portal.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "raymath.h" // Required for Vector2Distance

#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <math.h> // Required for fabsf

#define MAX_FILEPATH_SIZE 2048
#define SELECTED_STRUCTURE_FONT_SIZE 20
#define MAX_SELECTED_ITEMS 512 // For multi-select

//------------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------------
const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

// File Dropping
FilePathList _droppedFiles;
char *_filePath = NULL;
bool _fileDropped = false;

// JSON Data
cJSON *_configJson = NULL;
cJSON *_structures = NULL;
cJSON *_regions = NULL;
cJSON *_snow_regions = NULL;
cJSON *_rain_regions = NULL;
cJSON *_star_regions = NULL;
cJSON *_boost_gates = NULL;
cJSON *_portals = NULL;
cJSON *_ocean_world_area = NULL;
cJSON *_space_world_area = NULL;

// Camera and Display
float _displayScale = 0.5f;
Vector2 _cameraOffset;

// State & Selection
int _activeStructureIndex = -1; // For hover
int _infoPanelIndex = -1;      // For displaying info in the bottom right panel

// Multi-Select & Dragging
int _selectedStructureIndices[MAX_SELECTED_ITEMS];
Vector2 _dragStartPositions[MAX_SELECTED_ITEMS]; // Store original positions for dragging
int _selectedItemCount = 0;
bool _isMarqueeSelecting = false;
bool _isDraggingGroup = false;
bool _potentialDrag = false;    // Flag to check if a drag should start
Vector2 _marqueeStartPos = { 0 };
Vector2 _mouseDownWorldPos = { 0 }; // Position where mouse was pressed, in world coords
Rectangle _selectionMarquee = { 0 }; 

// Settings
bool _showNames = true;
bool _showRegionNames = true;
bool _showSnowRegions = true;
bool _showRainRegions = true;
bool _showStarRegions = true;
bool _showBoostGates = true;
bool _showPortals = true;
bool _showOceanWorldArea = true;
bool _showSpaceWorldArea = true;

//------------------------------------------------------------------------------------
// Helper function declarations
//------------------------------------------------------------------------------------
bool IsStructureSelected(int index);
void ClearSelection(void);
void AddToSelection(int index);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Wee Boats Map Editor");
    _filePath = (char *)RL_CALLOC(MAX_FILEPATH_SIZE, 1);
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        Update();
        Draw();
    }

    Cleanup();
    CloseWindow();
    return 0;
}

//------------------------------------------------------------------------------------
// Update and Draw
//------------------------------------------------------------------------------------
void Update()
{
    CheckForDroppedFile();
    if (!_fileDropped) return;

    Vector2 mousePos = GetMousePosition();
    Vector2 worldMousePos = {(mousePos.x - _cameraOffset.x) / _displayScale, (mousePos.y - _cameraOffset.y) / _displayScale};

    // Reset hover index before checking
    _activeStructureIndex = -1;

    // Check for hover over structures (only if not currently dragging something else)
    if (!_isDraggingGroup && !_isMarqueeSelecting)
    {
        cJSON *structure = NULL;
        int structureIndex = 0;
        cJSON_ArrayForEach(structure, _structures)
        {
            cJSON *location = cJSON_GetObjectItem(structure, "location");
            if (location)
            {
                Vector2 structureWorldPos = {cJSON_GetArrayItem(location, 0)->valuedouble, -cJSON_GetArrayItem(location, 1)->valuedouble};
                if (CheckCollisionPointCircle(worldMousePos, structureWorldPos, 10.0f / _displayScale))
                {
                    _activeStructureIndex = structureIndex;
                    break;
                }
            }
            structureIndex++;
        }
    }

    // Handle Mouse Input for Selection and Dragging
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        _mouseDownWorldPos = worldMousePos; // Store world position on click

        if (_activeStructureIndex != -1) // Clicked on a structure
        {
            // If the clicked item isn't already selected, clear the old selection.
            if (!IsStructureSelected(_activeStructureIndex))
            {
                ClearSelection();
                AddToSelection(_activeStructureIndex);
                _infoPanelIndex = _activeStructureIndex;
            }
            // If it was clicked, it's a potential drag, regardless of prior selection.
            _potentialDrag = true;
        }
        else // Clicked on empty space
        {
            _isMarqueeSelecting = true;
            _marqueeStartPos = mousePos;
            ClearSelection();
        }
    }

    // Handle dragging
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        // Check if a potential drag should become a real drag
        if (_potentialDrag && !_isDraggingGroup)
        {
            float dragThreshold = 5.0f / _displayScale; // 5 pixels in screen space
            if (Vector2Distance(worldMousePos, _mouseDownWorldPos) > dragThreshold)
            {
                _isDraggingGroup = true;
                // Store original positions of all selected items at the moment drag starts
                for (int i = 0; i < _selectedItemCount; i++)
                {
                    cJSON* selStructure = cJSON_GetArrayItem(_structures, _selectedStructureIndices[i]);
                    cJSON* loc = cJSON_GetObjectItem(selStructure, "location");
                    _dragStartPositions[i].x = cJSON_GetArrayItem(loc, 0)->valuedouble;
                    _dragStartPositions[i].y = cJSON_GetArrayItem(loc, 1)->valuedouble;
                }
            }
        }

        if (_isDraggingGroup)
        {
            Vector2 dragDelta = {worldMousePos.x - _mouseDownWorldPos.x, worldMousePos.y - _mouseDownWorldPos.y};
            for (int i = 0; i < _selectedItemCount; i++)
            {
                int idx = _selectedStructureIndices[i];
                cJSON* selStructure = cJSON_GetArrayItem(_structures, idx);
                
                float newX = _dragStartPositions[i].x + dragDelta.x;
                float newY = _dragStartPositions[i].y - dragDelta.y; // Y is flipped in world space

                cJSON *new_location = cJSON_CreateIntArray((int[]){(int)newX, (int)newY}, 2);
                cJSON_ReplaceItemInObjectCaseSensitive(selStructure, "location", new_location);
            }
        }
        else if (_isMarqueeSelecting)
        {
            _selectionMarquee.x = fminf(_marqueeStartPos.x, mousePos.x);
            _selectionMarquee.y = fminf(_marqueeStartPos.y, mousePos.y);
            _selectionMarquee.width = fabsf(_marqueeStartPos.x - mousePos.x);
            _selectionMarquee.height = fabsf(_marqueeStartPos.y - mousePos.y);
        }
    }

    // Handle mouse release
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        if (_isMarqueeSelecting)
        {
            _isMarqueeSelecting = false;
            ClearSelection();
            
            int structureIndex = 0;
            cJSON* structure = NULL;
            cJSON_ArrayForEach(structure, _structures)
            {
                cJSON* location = cJSON_GetObjectItem(structure, "location");
                if (location)
                {
                    Vector2 screenPos = { (cJSON_GetArrayItem(location, 0)->valuedouble * _displayScale) + _cameraOffset.x, -(cJSON_GetArrayItem(location, 1)->valuedouble * _displayScale) + _cameraOffset.y };
                    if (CheckCollisionPointRec(screenPos, _selectionMarquee))
                    {
                        AddToSelection(structureIndex);
                    }
                }
                structureIndex++;
            }
            // Reset marquee visually
            _selectionMarquee = (Rectangle){0,0,0,0};
        }
        _isDraggingGroup = false;
        _potentialDrag = false; // Reset potential drag flag
    }

    // Update all other editable elements if they are visible
    if (_showSnowRegions) UpdateSnowRegions(_snow_regions, _cameraOffset, &_displayScale);
    if (_showRainRegions) UpdateSnowRegions(_rain_regions, _cameraOffset, &_displayScale);
    if (_showStarRegions) UpdateSnowRegions(_star_regions, _cameraOffset, &_displayScale);
    if (_showBoostGates) UpdateBoostGates(_boost_gates, _cameraOffset, &_displayScale);
    if (_showPortals) UpdatePortals(_portals, _cameraOffset, &_displayScale);
    if (_showOceanWorldArea) UpdateWorldArea(_ocean_world_area, _cameraOffset, &_displayScale);
    if (_showSpaceWorldArea) UpdateWorldArea(_space_world_area, _cameraOffset, &_displayScale);

    ControlCamera();
}

void Draw()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    if (!_fileDropped)
    {
        DrawText("Wee Boats Map Editor", 100, 40, 40, DARKGRAY);
        DrawText("Drop the structure config JSON file onto the window...", 100, 100, 20, DARKGRAY);
    }
    else
    {
        // Draw grid lines and all editable elements
        DrawLineEx((Vector2){_cameraOffset.x, 0}, (Vector2){_cameraOffset.x, SCREEN_HEIGHT}, 2, LIGHTGRAY);
        DrawLineEx((Vector2){0, _cameraOffset.y}, (Vector2){SCREEN_WIDTH, _cameraOffset.y}, 2, LIGHTGRAY);
        if (_showOceanWorldArea) DrawWorldArea(_ocean_world_area, _cameraOffset, &_displayScale, "Ocean World Area", (Color){0, 117, 117, 150});
        if (_showSpaceWorldArea) DrawWorldArea(_space_world_area, _cameraOffset, &_displayScale, "Space World Area", (Color){75, 0, 130, 150});
        if (_showSnowRegions) DrawSnowRegions(_snow_regions, _cameraOffset, &_displayScale, "Snow Region");
        if (_showRainRegions) DrawSnowRegions(_rain_regions, _cameraOffset, &_displayScale, "Rain Region");
        if (_showStarRegions) DrawSnowRegions(_star_regions, _cameraOffset, &_displayScale, "Star Region");
        if (_showBoostGates) DrawBoostGates(_boost_gates, _cameraOffset, &_displayScale);
        if (_showPortals) DrawPortals(_portals, _cameraOffset, &_displayScale);

        // Draw Structures
        cJSON *structure = NULL;
        int structureIndex = 0;
        Color regionColors[] = {PINK, ORANGE, SKYBLUE, PURPLE, BROWN, BEIGE, VIOLET, GOLD, LIME};
        cJSON_ArrayForEach(structure, _structures)
        {
            cJSON *location = cJSON_GetObjectItem(structure, "location");
            if (location)
            {
                int x = cJSON_GetArrayItem(location, 0)->valueint;
                int y = cJSON_GetArrayItem(location, 1)->valueint;
                Vector2 pos = {(x * _displayScale) + _cameraOffset.x, -(y * _displayScale) + _cameraOffset.y};

                Color structureColor = GREEN;
                char *regionName = "No Region";
                if (cJSON_HasObjectItem(structure, "region_id"))
                {
                    int regionId = cJSON_GetObjectItem(structure, "region_id")->valueint;
                    if (regionId < (sizeof(regionColors) / sizeof(regionColors[0]))) structureColor = regionColors[regionId];
                    cJSON *region = cJSON_GetArrayItem(_regions, regionId);
                    if (region) regionName = cJSON_GetObjectItem(region, "name")->valuestring;
                }

                // Determine draw color based on selection/hover state
                Color drawColor = structureColor;
                if (IsStructureSelected(structureIndex)) drawColor = RED;
                else if (structureIndex == _activeStructureIndex) drawColor = YELLOW;

                DrawCircleV(pos, 10, drawColor);
                if (_showNames) DrawText(cJSON_GetObjectItem(structure, "name")->valuestring, pos.x + 15, pos.y, 15, DARKGRAY);
                if (_showRegionNames) DrawText(regionName, pos.x + 15, pos.y + 20, 15, structureColor);

                // Info panel shows the last single-clicked item
                if (_infoPanelIndex == structureIndex)
                {
                    DrawRectangle(SCREEN_WIDTH - 330, SCREEN_HEIGHT - 200, 320, 190, Fade(LIGHTGRAY, 0.8f));
                    DrawText(cJSON_GetObjectItem(structure, "name")->valuestring, SCREEN_WIDTH - 320, SCREEN_HEIGHT - 180, SELECTED_STRUCTURE_FONT_SIZE, DARKGRAY);
                    DrawText(TextFormat("Location: (%d, %d)", x, y), SCREEN_WIDTH - 320, SCREEN_HEIGHT - 150, SELECTED_STRUCTURE_FONT_SIZE, DARKGRAY);
                    DrawText(TextFormat("Region: %s", regionName), SCREEN_WIDTH - 320, SCREEN_HEIGHT - 120, SELECTED_STRUCTURE_FONT_SIZE, DARKGRAY);
                }
            }
            structureIndex++;
        }
        
        // Draw selection marquee
        if (_isMarqueeSelecting)
        {
            DrawRectangleRec(_selectionMarquee, Fade(BLUE, 0.25f));
            DrawRectangleLinesEx(_selectionMarquee, 1, BLUE);
        }

        // Draw GUI Controls
        float panelX = SCREEN_WIDTH - 200;
        float panelY = 20;
        float panelWidth = 180;
        GuiGroupBox((Rectangle){panelX, panelY, panelWidth, 120}, "File Options");
        if (GuiButton((Rectangle){panelX + 10, panelY + 20, 160, 25}, "Export Config")) ExportConfig();
        if (GuiButton((Rectangle){panelX + 10, panelY + 50, 160, 25}, "Add Structure")) AddStructure();
        if (GuiButton((Rectangle){panelX + 10, panelY + 80, 160, 25}, "Reload Config")) LoadJsonData();

        panelY += 130;
        GuiGroupBox((Rectangle){panelX, panelY, panelWidth, 240}, "Visibility Controls");
        GuiCheckBox((Rectangle){panelX + 10, panelY + 20, 20, 20}, "Names", &_showNames);
        GuiCheckBox((Rectangle){panelX + 10, panelY + 45, 20, 20}, "Region Names", &_showRegionNames);
        GuiCheckBox((Rectangle){panelX + 10, panelY + 70, 20, 20}, "Snow Regions", &_showSnowRegions);
        GuiCheckBox((Rectangle){panelX + 10, panelY + 95, 20, 20}, "Rain Regions", &_showRainRegions);
        GuiCheckBox((Rectangle){panelX + 10, panelY + 120, 20, 20}, "Star Regions", &_showStarRegions);
        GuiCheckBox((Rectangle){panelX + 10, panelY + 145, 20, 20}, "Boost Gates", &_showBoostGates);
        GuiCheckBox((Rectangle){panelX + 10, panelY + 170, 20, 20}, "Portals", &_showPortals);
        GuiCheckBox((Rectangle){panelX + 10, panelY + 195, 20, 20}, "Ocean Area", &_showOceanWorldArea);
        GuiCheckBox((Rectangle){panelX + 10, panelY + 215, 20, 20}, "Space Area", &_showSpaceWorldArea);

        panelY += 250;
        if (_showSnowRegions) { GuiGroupBox((Rectangle){panelX, panelY, panelWidth, 60}, "Snow Region"); if (GuiButton((Rectangle){panelX + 10, panelY + 20, 160, 25}, "Add Snow Region")) AddSnowRegion(_snow_regions); panelY += 70; }
        if (_showRainRegions) { GuiGroupBox((Rectangle){panelX, panelY, panelWidth, 60}, "Rain Region"); if (GuiButton((Rectangle){panelX + 10, panelY + 20, 160, 25}, "Add Rain Region")) AddSnowRegion(_rain_regions); panelY += 70; }
        if (_showStarRegions) { GuiGroupBox((Rectangle){panelX, panelY, panelWidth, 60}, "Star Region"); if (GuiButton((Rectangle){panelX + 10, panelY + 20, 160, 25}, "Add Star Region")) AddSnowRegion(_star_regions); panelY += 70; }
        if (_showBoostGates) { GuiGroupBox((Rectangle){panelX, panelY, panelWidth, 60}, "Boost Gate"); if (GuiButton((Rectangle){panelX + 10, panelY + 20, 160, 25}, "Add Boost Gate")) AddBoostGate(_boost_gates, _cameraOffset, _displayScale); panelY += 70; }
        if (_showPortals) { GuiGroupBox((Rectangle){panelX, panelY, panelWidth, 60}, "Portal"); if (GuiButton((Rectangle){panelX + 10, panelY + 20, 160, 25}, "Add Portal")) AddPortal(_portals, _cameraOffset, _displayScale); }

        // Draw Help Text
        DrawText("Commands: Move Camera: Arrow Keys, Zoom: Mouse Wheel/I-O", 10, SCREEN_HEIGHT - 30, 20, DARKGRAY);
    }
    EndDrawing();
}


//------------------------------------------------------------------------------------
// Core Functions
//------------------------------------------------------------------------------------
void Cleanup()
{
    RL_FREE(_filePath);
    if (_configJson != NULL) cJSON_Delete(_configJson);
}

void LoadJsonData()
{
    if (_configJson != NULL) cJSON_Delete(_configJson);

    char *jsonString = LoadFileText(_filePath);
    if (jsonString == NULL) return;

    _configJson = cJSON_Parse(jsonString);
    UnloadFileText(jsonString);

    if (_configJson == NULL) { printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr()); return; }

    _structures = cJSON_GetObjectItem(_configJson, "structures");
    _regions = cJSON_GetObjectItem(_configJson, "regions");
    _snow_regions = cJSON_GetObjectItem(_configJson, "snow_regions");
    _rain_regions = cJSON_GetObjectItem(_configJson, "rain_regions");
    _star_regions = cJSON_GetObjectItem(_configJson, "star_regions");
    _boost_gates = cJSON_GetObjectItem(_configJson, "boost_gates");
    _ocean_world_area = cJSON_GetObjectItem(_configJson, "ocean_world_area");
    _space_world_area = cJSON_GetObjectItem(_configJson, "space_world_area");

    cJSON *portals_obj = cJSON_GetObjectItem(_configJson, "portals");
    if (portals_obj) _portals = cJSON_GetObjectItem(portals_obj, "locations");

    printf("JSON data loaded successfully.\n");
}

void CheckForDroppedFile()
{
    if (IsFileDropped())
    {
        FilePathList dropped = LoadDroppedFiles();
        if (dropped.count > 0)
        {
            TextCopy(_filePath, dropped.paths[0]);
            _fileDropped = true;
            LoadJsonData();
        }
        UnloadDroppedFiles(dropped);
    }
}

void ExportConfig()
{
    // Use cJSON_PrintBuffered to get a formatted (indented) string
    // The second argument is a buffer size guess, 0 is fine to let cJSON calculate.
    // The third argument (1) enables formatting.
    char *jsonString = cJSON_PrintBuffered(_configJson, 0, 1);
    if (jsonString)
    {
        if (SaveFileText(_filePath, jsonString)) printf("Configuration exported to %s\n", _filePath);
        else printf("ERROR: Failed to save file.\n");
        free(jsonString);
    }
    else printf("ERROR: Failed to generate JSON string.\n");
}

void AddStructure()
{
    cJSON *new_structure = cJSON_CreateObject();
    cJSON_AddItemToObject(new_structure, "name", cJSON_CreateString("New Structure"));
    cJSON_AddItemToObject(new_structure, "location", cJSON_CreateIntArray((int[]){0, 0}, 2));
    cJSON_AddItemToArray(_structures, new_structure);
}

void ControlCamera()
{
    // Pan with arrow keys only if not dragging a group or selecting
    if (!_isDraggingGroup && !_isMarqueeSelecting)
    {
        if (IsKeyDown(KEY_LEFT)) _cameraOffset.x += 10.0f;
        if (IsKeyDown(KEY_RIGHT)) _cameraOffset.x -= 10.0f;
        if (IsKeyDown(KEY_UP)) _cameraOffset.y += 10.0f;
        if (IsKeyDown(KEY_DOWN)) _cameraOffset.y -= 10.0f;
    }

    // Zoom with mouse wheel or keys
    float wheel = GetMouseWheelMove();
    if (wheel != 0) _displayScale += wheel * 0.05f;
    if (IsKeyDown(KEY_I)) _displayScale += 0.01f;
    if (IsKeyDown(KEY_O)) _displayScale -= 0.01f;
    if (_displayScale < 0.05f) _displayScale = 0.05f;
    if (_displayScale > 2.0f) _displayScale = 2.0f;
}

//------------------------------------------------------------------------------------
// Selection Helper Functions
//------------------------------------------------------------------------------------
bool IsStructureSelected(int index)
{
    for (int i = 0; i < _selectedItemCount; i++)
    {
        if (_selectedStructureIndices[i] == index) return true;
    }
    return false;
}

void ClearSelection(void)
{
    _selectedItemCount = 0;
    _infoPanelIndex = -1;
}

void AddToSelection(int index)
{
    if (_selectedItemCount < MAX_SELECTED_ITEMS && !IsStructureSelected(index))
    {
        _selectedStructureIndices[_selectedItemCount] = index;
        _selectedItemCount++;
    }
}
