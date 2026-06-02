#ifndef LAYOUT_H
#define LAYOUT_H

#include "helperGUI.h"

#define MAX_COLS 12  // Bootstrap-style 12-column grid

typedef struct {
    int x, y, w, h;
    int paddingX, paddingY;
} Container;

typedef struct {
    int x, y, w, h;
} Cell;

// Create a container (like <div class="container">)
Container layoutContainer(int x, int y, int w, int h, int paddingX, int paddingY);

// Create a row inside a container (like <div class="row">)
// Returns the y-offset for the next row
int layoutRow(Container *parent, int colSpans[], Cell cellsOut[], int numCells, int rowHeight, int gapX);

// Draw a cell background (like a styled div)
void layoutDrawCell(GUI *gui, Cell *cell, int r, int g, int b, int a);

#endif