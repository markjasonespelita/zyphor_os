#include "headers/helperLayout.h"

Container layoutContainer(int x, int y, int w, int h, int paddingX, int paddingY) {
    Container c;
    c.x = x + paddingX;
    c.y = y + paddingY;
    c.w = w - paddingX * 2;
    c.h = h - paddingY * 2;
    c.paddingX = paddingX;
    c.paddingY = paddingY;
    return c;
}

// colSpans[] = e.g. {4, 8} means col-4 and col-8 (must sum to 12)
// Returns new Y after this row
int layoutRow(Container *parent, int colSpans[], Cell cellsOut[], int numCells, int rowHeight, int gapX) {
    int unitW = (parent->w - (gapX * (numCells - 1))) / MAX_COLS;
    int curX = parent->x;

    for (int i = 0; i < numCells; i++) {
        cellsOut[i].x = curX;
        cellsOut[i].y = parent->y;
        cellsOut[i].w = unitW * colSpans[i];
        cellsOut[i].h = rowHeight;
        curX += cellsOut[i].w + gapX;
    }

    return parent->y + rowHeight;
}

void layoutDrawCell(GUI *gui, Cell *cell, int r, int g, int b, int a) {
    guiDrawRect(gui, cell->x, cell->y, cell->w, cell->h, r, g, b, a);
}