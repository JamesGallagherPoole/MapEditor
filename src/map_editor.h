#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

void Update();
void ControlCamera();
void Draw();
void Cleanup();
void CheckForDroppedFile();

void UpdateBoostGates(cJSON *boost_gates, Vector2 cameraOffset, float* displayScale);
void DrawBoostGates(cJSON *boost_gates, Vector2 cameraOffset, float *displayScale);
void AddBoostGate(cJSON *snow_regions);

#endif // !MAP_EDITOR_H


