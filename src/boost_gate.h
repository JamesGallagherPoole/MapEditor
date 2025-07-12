#ifndef BOOST_GATE_H
#define BOOST_GATE_H

#include "cJSON.h"
#include "raylib.h"

//------------------------------------------------------------------------------------
// Function Declarations
//------------------------------------------------------------------------------------

/**
 * @brief Updates the boost gate endpoint positions based on mouse input.
 * @param boost_gates A cJSON array of boost gate objects.
 * @param cameraOffset The current camera offset.
 * @param displayScale The current display scale (zoom).
 */
void UpdateBoostGates(cJSON *boost_gates, Vector2 cameraOffset, float *displayScale);

/**
 * @brief Draws the boost gates on the screen as lines with endpoints.
 * @param boost_gates A cJSON array of boost gate objects.
 * @param cameraOffset The current camera offset.
 * @param displayScale The current display scale (zoom).
 */
void DrawBoostGates(cJSON *boost_gates, Vector2 cameraOffset, float *displayScale);

/**
 * @brief Adds a new default boost gate to the array at the center of the view.
 * @param boost_gates A cJSON array of boost gate objects.
 * @param cameraOffset The current camera offset.
 * @param displayScale The current display scale (zoom).
 */
void AddBoostGate(cJSON *boost_gates, Vector2 cameraOffset, float displayScale);

#endif // BOOST_GATE_H
