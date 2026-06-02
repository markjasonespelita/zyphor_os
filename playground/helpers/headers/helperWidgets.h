#ifndef WIDGETS_H
#define WIDGETS_H

#include "helperGUI.h"
#include "helperLayout.h"
#include <SDL2/SDL_ttf.h>

// ─── Color ───────────────────────────────────────────────────────────────────

typedef struct { int r, g, b, a; } Color;

// ─── FontSet ─────────────────────────────────────────────────────────────────

typedef struct {
    TTF_Font *regular;
    TTF_Font *bold;
} FontSet;

int  widgetsInit(FontSet *fonts, const char *regularPath,
                 const char *boldPath, int size);
void widgetsDestroy(FontSet *fonts);

// ─── Button ──────────────────────────────────────────────────────────────────

typedef enum { BTN_NORMAL, BTN_HOVER, BTN_ACTIVE } ButtonState;

typedef struct {
    Cell        cell;
    char        label[128];
    Color       bgNormal;
    Color       bgHover;
    Color       bgActive;
    Color       textColor;
    ButtonState state;
} Button;

Button buttonCreate(Cell cell, const char *label,
                    Color bgNormal, Color bgHover, Color bgActive,
                    Color textColor);
void   buttonUpdate(Button *btn, int mouseX, int mouseY, int mouseDown);
int    buttonIsClicked(Button *btn, int mouseX, int mouseY, int mouseReleased);
void   buttonDraw(GUI *gui, Button *btn, FontSet *fonts);

// ─── Header ──────────────────────────────────────────────────────────────────

typedef enum { H1, H2, H3 } HeaderLevel;

void drawHeader(GUI *gui, FontSet *fonts, const char *text,
                int x, int y, HeaderLevel level, Color color);

// ─── Paragraph ───────────────────────────────────────────────────────────────

void drawParagraph(GUI *gui, FontSet *fonts, const char *text,
                   int x, int y, int maxWidth, int lineHeight, Color color);

// ─── TextInput ───────────────────────────────────────────────────────────────

#define INPUT_BUF 256

typedef struct {
    Cell   cell;
    char   buffer[INPUT_BUF];
    int    bufLen;
    int    focused;
    int    cursorVisible;
    Uint32 lastBlink;
    Color  bgColor;
    Color  borderNormal;
    Color  borderFocused;
    Color  textColor;
    Color  placeholderColor;
    char   placeholder[128];
} TextInput;

TextInput textInputCreate(Cell cell, const char *placeholder,
                          Color bgColor, Color borderNormal, Color borderFocused,
                          Color textColor, Color placeholderColor);
void textInputHandleEvent(TextInput *inp, SDL_Event *event);
void textInputDraw(GUI *gui, TextInput *inp, FontSet *fonts);

// ─── TextArea ────────────────────────────────────────────────────────────────

#define TEXTAREA_BUF 2048

typedef struct {
    Cell   cell;
    char   buffer[TEXTAREA_BUF];
    int    bufLen;
    int    focused;
    int    cursorVisible;
    Uint32 lastBlink;
    int    lineHeight;
    Color  bgColor;
    Color  borderNormal;
    Color  borderFocused;
    Color  textColor;
    Color  placeholderColor;
    char   placeholder[128];
} TextArea;

TextArea textAreaCreate(Cell cell, const char *placeholder, int lineHeight,
                        Color bgColor, Color borderNormal, Color borderFocused,
                        Color textColor, Color placeholderColor);
void textAreaHandleEvent(TextArea *ta, SDL_Event *event);
void textAreaDraw(GUI *gui, TextArea *ta, FontSet *fonts);

#endif