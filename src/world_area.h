#ifndef WORLD_AREA_H
#define WORLD_AREA_H

#include "cJSON.h"
#include "raylib.h"

//------------------------------------------------------------------------------------
// Function Declarations
//------------------------------------------------------------------------------------

/**
 * @brief Updates the position of a single world area based on mouse input.
 * @param world_area A cJSON object representing the world area.
 * @param cameraOffset The current camera offset.
 * @param displayScale The current display scale (zoom).
 */
void UpdateWorldArea(cJSON *world_area, Vector2 cameraOffset, float *displayScale);

/**
 * @brief Draws a single world area on the screen.
 * @param world_area A cJSON object representing the world area.
 * @param cameraOffset The current camera offset.
 * @param displayScale The current display scale (zoom).
 * @param headerText The text to display for the area.
 * @param color The color to use for drawing the area.
 */
void DrawWorldArea(cJSON *world_area, Vector2 cameraOffset, float *displayScale, const char *headerText, Color color);

#endif // WORLD_AREA_H
