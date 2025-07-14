#include "boost_gate.h"
#include <stdio.h>

// Note: This implementation assumes that your main C file (or a shared header like map_editor.h)
// provides the definition for the 'SelectedItem' struct and declares the following as extern:
//
// extern SelectedItem _activeItem;
// bool IsItemSelected(SelectedItem item);
//

// Public function to add a boost gate
void AddBoostGate(cJSON *boost_gates, Vector2 cameraOffset, float displayScale)
{
    if (!boost_gates) return;

    cJSON *new_pair = cJSON_CreateObject();

    // Calculate the center of the screen in world coordinates
    int center_x = (int)((GetScreenWidth() / 2 - cameraOffset.x) / displayScale);
    int center_y = -(int)((GetScreenHeight() / 2 - cameraOffset.y) / displayScale); // Flip y-axis for consistency

    // Create the two endpoints for the new gate
    cJSON *new_a = cJSON_CreateIntArray((int[]){center_x - 50, center_y}, 2);
    cJSON *new_b = cJSON_CreateIntArray((int[]){center_x + 50, center_y}, 2);

    cJSON_AddItemToObject(new_pair, "a", new_a);
    cJSON_AddItemToObject(new_pair, "b", new_b);
    cJSON_AddItemToArray(boost_gates, new_pair);

    printf("Added a new boost gate.\n");
}


// Public function to draw boost gates
void DrawBoostGates(cJSON *boost_gates, Vector2 cameraOffset, float *displayScale)
{
    if (!boost_gates) return;

    cJSON *point_pair = NULL;
    int gateIndex = 0;
    cJSON_ArrayForEach(point_pair, boost_gates)
    {
        cJSON *a = cJSON_GetObjectItem(point_pair, "a");
        cJSON *b = cJSON_GetObjectItem(point_pair, "b");

        Vector2 posA = {(cJSON_GetArrayItem(a, 0)->valueint) * *displayScale + cameraOffset.x, -(cJSON_GetArrayItem(a, 1)->valueint) * *displayScale + cameraOffset.y};
        Vector2 posB = {(cJSON_GetArrayItem(b, 0)->valueint) * *displayScale + cameraOffset.x, -(cJSON_GetArrayItem(b, 1)->valueint) * *displayScale + cameraOffset.y};

        DrawLineEx(posA, posB, 3, ORANGE);

        // Determine color for point A based on global selection state
        Color colorA = ORANGE;
        SelectedItem itemA = { gateIndex, ELEMENT_TYPE_BOOST_GATE_A };
        if (IsItemSelected(itemA)) colorA = RED;
        else if (_activeItem.index == itemA.index && _activeItem.type == itemA.type) colorA = YELLOW;

        // Determine color for point B based on global selection state
        Color colorB = ORANGE;
        SelectedItem itemB = { gateIndex, ELEMENT_TYPE_BOOST_GATE_B };
        if (IsItemSelected(itemB)) colorB = RED;
        else if (_activeItem.index == itemB.index && _activeItem.type == itemB.type) colorB = YELLOW;


        DrawCircleV(posA, 10, colorA);
        DrawCircleV(posB, 10, colorB);

        gateIndex++;
    }
}