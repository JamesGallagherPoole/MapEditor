#include "cJSON.h"
#include "raylib.h"
#ifndef snow_region__h
#define snow_region__h

void UpdateSnowRegions(cJSON *snow_regions, Vector2 cameraOffset, float *displayScale);
void DrawSnowRegions(cJSON *snow_regions, Vector2 cameraOffset, float *displayScale, char *headerText);
void AddSnowRegion(cJSON *snow_regions);

#endif
