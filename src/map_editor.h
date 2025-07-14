#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

#include "raylib.h" // For Vector2

// Shared type definitions for the entire project
typedef enum {
    ELEMENT_TYPE_NONE = -1,
    ELEMENT_TYPE_STRUCTURE,
    ELEMENT_TYPE_BOOST_GATE_A,
    ELEMENT_TYPE_BOOST_GATE_B
} SelectableElementType;

typedef struct {
    int index;
    SelectableElementType type;
    Vector2 dragStartPosition;
} SelectedItem;

// --- Extern declarations for Global Variables ---
// This tells other files like boost_gate.c that these variables exist
// and will be provided by another file (your main .c file).
extern SelectedItem _activeItem;

// --- Function Prototypes for Globally Used Functions ---
bool IsItemSelected(SelectedItem item);

#endif // MAP_EDITOR_H