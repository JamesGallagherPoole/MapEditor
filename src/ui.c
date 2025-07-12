#include "raylib.h"

void HandleExportButton(Rectangle button)
{
    if (IsHoveringOverButton(button))
    {
        if (IsButtonPressed(button) == 1)
        {
            printf("Exporting!\n");
            ExportConfig();
        }
    }
}

void HandleAddStructureButton(Rectangle button)
{
    if (IsHoveringOverButton(button))
    {
        if (IsButtonPressed(button) == 1)
        {
            printf("Adding structure!\n");
            AddStructure();
        }
    }
}

int IsHoveringOverButton(Rectangle button)
{
    Vector2 mouse = GetMousePosition();

    if (CheckCollisionPointRec(mouse, button))
    {
        return 1;
    }

    return 0;
}

int IsButtonPressed(Rectangle button)
{
    Vector2 mouse = GetMousePosition();

    if (IsHoveringOverButton(button))
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            return 1;
        }
    }

    return 0;
}

// Drawing

void DrawInteractiveButton(Rectangle button, char *text)
{
    if (IsButtonPressed(button))
    {
        DrawRectangle(button.x, button.y, button.width, button.height, DARKBLUE);
        DrawText(text, button.x + 10, button.y + 10, 20, WHITE);
    }
    else if (IsHoveringOverButton(button))
    {
        DrawRectangle(button.x, button.y, button.width, button.height, SKYBLUE);
        DrawText(text, button.x + 10, button.y + 10, 20, DARKBLUE);
    }
    else
    {
        DrawRectangle(button.x, button.y, button.width, button.height, BLUE);
        DrawText(text, button.x + 10, button.y + 10, 20, WHITE);
    }
}