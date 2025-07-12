#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

#include "raylib.h"
#include "cJSON.h"

//------------------------------------------------------------------------------------
// Function Declarations for map_editor.c
//------------------------------------------------------------------------------------
void Update(void);
void Draw(void);
void Cleanup(void);
void LoadJsonData(void);
void CheckForDroppedFile(void);
void ControlCamera(void);
void AddStructure(void);
void ExportConfig(void);

#endif // MAP_EDITOR_H
