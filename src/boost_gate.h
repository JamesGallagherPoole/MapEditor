#ifndef BOOST_GATE_H
#define BOOST_GATE_H

#include "raylib.h"
#include "cJSON.h"
#include "map_editor.h" // Make sure this defines SelectedItem and declares externs

/**
 * @brief Adds a new boost gate to the center of the current view.
 * * @param boost_gates The cJSON array of boost gates.
 * @param cameraOffset The current camera offset.
 * @param displayScale The current display scale.
 */
void AddBoostGate(cJSON *boost_gates, Vector2 cameraOffset, float displayScale);

/**
 * @brief Draws all boost gates with visual feedback for selection and hover.
 * * @param boost_gates The cJSON array of boost gates.
 * @param cameraOffset The current camera offset.
 * @param displayScale A pointer to the current display scale.
 */
void DrawBoostGates(cJSON *boost_gates, Vector2 cameraOffset, float *displayScale);

#endif // BOOST_GATE_H