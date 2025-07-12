#ifndef PORTAL_H
#define PORTAL_H

#include "cJSON.h"
#include "raylib.h"

//------------------------------------------------------------------------------------
// Function Declarations
//------------------------------------------------------------------------------------

/**
 * @brief Updates the portal endpoint positions based on mouse input.
 * @param portals A cJSON array of portal location objects.
 * @param cameraOffset The current camera offset.
 * @param displayScale The current display scale (zoom).
 */
void UpdatePortals(cJSON *portals, Vector2 cameraOffset, float *displayScale);

/**
 * @brief Draws the portals on the screen as lines with endpoints.
 * @param portals A cJSON array of portal location objects.
 * @param cameraOffset The current camera offset.
 * @param displayScale The current display scale (zoom).
 */
void DrawPortals(cJSON *portals, Vector2 cameraOffset, float *displayScale);

/**
 * @brief Adds a new default portal to the array at the center of the view.
 * @param portals A cJSON array of portal location objects.
 * @param cameraOffset The current camera offset.
 * @param displayScale The current display scale (zoom).
 */
void AddPortal(cJSON *portals, Vector2 cameraOffset, float displayScale);

#endif // PORTAL_H
