#include "headers/helperWidgets.h"
#include <string.h>
#include <stdlib.h>

// ─── Internal helpers ────────────────────────────────────────────────────────

static int inCell(Cell *c, int mx, int my)
{
    return mx >= c->x && mx <= c->x + c->w &&
           my >= c->y && my <= c->y + c->h;
}

static void renderText(GUI *gui, TTF_Font *font, const char *text,
                       int x, int y, Color col)
{
    SDL_Color sc = {col.r, col.g, col.b, col.a};
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, text, sc);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(gui->renderer, surf);
    SDL_Rect dst = {x, y, surf->w, surf->h};
    SDL_FreeSurface(surf);
    if (!tex) return;
    SDL_RenderCopy(gui->renderer, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
}

static void renderTextCentered(GUI *gui, TTF_Font *font, const char *text,
                                int cx, int cy, Color col)
{
    int w, h;
    TTF_SizeUTF8(font, text, &w, &h);
    renderText(gui, font, text, cx - w / 2, cy - h / 2, col);
}

static void drawBorder(GUI *gui, Cell *cell, Color col, int thickness)
{
    SDL_SetRenderDrawColor(gui->renderer, col.r, col.g, col.b, col.a);
    for (int i = 0; i < thickness; i++) {
        SDL_Rect r = {
            cell->x + i,
            cell->y + i,
            cell->w - i * 2,
            cell->h - i * 2
        };
        SDL_RenderDrawRect(gui->renderer, &r);
    }
}

// ─── FontSet ─────────────────────────────────────────────────────────────────

int widgetsInit(FontSet *fonts, const char *regularPath,
                const char *boldPath, int size)
{
    if (TTF_Init() != 0) return 0;
    fonts->regular = TTF_OpenFont(regularPath, size);
    fonts->bold    = TTF_OpenFont(boldPath,    size);
    return (fonts->regular != NULL && fonts->bold != NULL);
}

void widgetsDestroy(FontSet *fonts)
{
    if (fonts->regular) TTF_CloseFont(fonts->regular);
    if (fonts->bold)    TTF_CloseFont(fonts->bold);
    TTF_Quit();
}

// ─── Button ──────────────────────────────────────────────────────────────────

Button buttonCreate(Cell cell, const char *label,
                    Color bgNormal, Color bgHover, Color bgActive,
                    Color textColor)
{
    Button btn  = {0};
    btn.cell    = cell;
    btn.bgNormal = bgNormal;
    btn.bgHover  = bgHover;
    btn.bgActive = bgActive;
    btn.textColor = textColor;
    btn.state   = BTN_NORMAL;
    strncpy(btn.label, label, sizeof(btn.label) - 1);
    return btn;
}

void buttonUpdate(Button *btn, int mx, int my, int mouseDown)
{
    if (inCell(&btn->cell, mx, my))
        btn->state = mouseDown ? BTN_ACTIVE : BTN_HOVER;
    else
        btn->state = BTN_NORMAL;
}

int buttonIsClicked(Button *btn, int mx, int my, int mouseReleased)
{
    return mouseReleased && inCell(&btn->cell, mx, my);
}

void buttonDraw(GUI *gui, Button *btn, FontSet *fonts)
{
    Color col = btn->state == BTN_ACTIVE ? btn->bgActive
              : btn->state == BTN_HOVER  ? btn->bgHover
              : btn->bgNormal;

    guiDrawRect(gui, btn->cell.x, btn->cell.y,
                     btn->cell.w, btn->cell.h,
                     col.r, col.g, col.b, col.a);

    int cx = btn->cell.x + btn->cell.w / 2;
    int cy = btn->cell.y + btn->cell.h / 2;
    renderTextCentered(gui, fonts->bold, btn->label, cx, cy, btn->textColor);
}

// ─── Header ──────────────────────────────────────────────────────────────────

void drawHeader(GUI *gui, FontSet *fonts, const char *text,
                int x, int y, HeaderLevel level, Color color)
{
    if (level == H1) {
        Color shadow = {0, 0, 0, 80};
        renderText(gui, fonts->bold, text, x + 2, y + 2, shadow);
    }
    TTF_Font *font = (level == H3) ? fonts->regular : fonts->bold;
    renderText(gui, font, text, x, y, color);
}

// ─── Paragraph ───────────────────────────────────────────────────────────────

void drawParagraph(GUI *gui, FontSet *fonts, const char *text,
                   int x, int y, int maxWidth, int lineHeight, Color color)
{
    char buf[2048];
    strncpy(buf, text, sizeof(buf) - 1);

    char  line[512] = "";
    char  word[256];
    int   curY = y;
    char *ptr  = buf;

    while (*ptr) {
        int wi = 0;
        while (*ptr && *ptr != ' ' && *ptr != '\n')
            word[wi++] = *ptr++;
        word[wi] = '\0';
        if (*ptr) ptr++;

        char test[512];
        snprintf(test, sizeof(test), "%s%s%s",
                 line, line[0] ? " " : "", word);

        int w, h;
        TTF_SizeUTF8(fonts->regular, test, &w, &h);

        if (w > maxWidth && line[0]) {
            renderText(gui, fonts->regular, line, x, curY, color);
            curY += lineHeight;
            strncpy(line, word, sizeof(line) - 1);
        } else {
            strncpy(line, test, sizeof(line) - 1);
        }
    }
    if (line[0])
        renderText(gui, fonts->regular, line, x, curY, color);
}

// ─── TextInput ───────────────────────────────────────────────────────────────

TextInput textInputCreate(Cell cell, const char *placeholder,
                          Color bgColor, Color borderNormal, Color borderFocused,
                          Color textColor, Color placeholderColor)
{
    TextInput inp      = {0};
    inp.cell           = cell;
    inp.bgColor        = bgColor;
    inp.borderNormal   = borderNormal;
    inp.borderFocused  = borderFocused;
    inp.textColor      = textColor;
    inp.placeholderColor = placeholderColor;
    inp.lastBlink      = SDL_GetTicks();
    strncpy(inp.placeholder, placeholder, sizeof(inp.placeholder) - 1);
    return inp;
}

void textInputHandleEvent(TextInput *inp, SDL_Event *e)
{
    if (e->type == SDL_MOUSEBUTTONDOWN)
        inp->focused = inCell(&inp->cell, e->button.x, e->button.y);

    if (!inp->focused) return;

    if (e->type == SDL_KEYDOWN &&
        e->key.keysym.sym == SDLK_BACKSPACE &&
        inp->bufLen > 0)
    {
        inp->buffer[--inp->bufLen] = '\0';
    }

    if (e->type == SDL_TEXTINPUT) {
        int addLen = (int)strlen(e->text.text);
        if (inp->bufLen + addLen < INPUT_BUF - 1) {
            strcat(inp->buffer, e->text.text);
            inp->bufLen += addLen;
        }
    }
}

void textInputDraw(GUI *gui, TextInput *inp, FontSet *fonts)
{
    guiDrawRect(gui, inp->cell.x, inp->cell.y,
                     inp->cell.w, inp->cell.h,
                     inp->bgColor.r, inp->bgColor.g,
                     inp->bgColor.b, inp->bgColor.a);

    Color border = inp->focused ? inp->borderFocused : inp->borderNormal;
    drawBorder(gui, &inp->cell, border, 2);

    Uint32 now = SDL_GetTicks();
    if (now - inp->lastBlink > 500) {
        inp->cursorVisible = !inp->cursorVisible;
        inp->lastBlink = now;
    }

    int padX = 8;
    int padY = (inp->cell.h - TTF_FontHeight(fonts->regular)) / 2;

    if (inp->bufLen == 0) {
        renderText(gui, fonts->regular, inp->placeholder,
                   inp->cell.x + padX, inp->cell.y + padY,
                   inp->placeholderColor);
    } else {
        renderText(gui, fonts->regular, inp->buffer,
                   inp->cell.x + padX, inp->cell.y + padY,
                   inp->textColor);
    }

    if (inp->focused && inp->cursorVisible) {
        int tw = 0, th = 0;
        if (inp->bufLen > 0)
            TTF_SizeUTF8(fonts->regular, inp->buffer, &tw, &th);
        SDL_SetRenderDrawColor(gui->renderer,
            inp->textColor.r, inp->textColor.g,
            inp->textColor.b, inp->textColor.a);
        SDL_Rect cur = {
            inp->cell.x + padX + tw,
            inp->cell.y + padY,
            2, TTF_FontHeight(fonts->regular)
        };
        SDL_RenderFillRect(gui->renderer, &cur);
    }
}

// ─── TextArea ────────────────────────────────────────────────────────────────

TextArea textAreaCreate(Cell cell, const char *placeholder, int lineHeight,
                        Color bgColor, Color borderNormal, Color borderFocused,
                        Color textColor, Color placeholderColor)
{
    TextArea ta        = {0};
    ta.cell            = cell;
    ta.lineHeight      = lineHeight;
    ta.bgColor         = bgColor;
    ta.borderNormal    = borderNormal;
    ta.borderFocused   = borderFocused;
    ta.textColor       = textColor;
    ta.placeholderColor = placeholderColor;
    ta.lastBlink       = SDL_GetTicks();
    strncpy(ta.placeholder, placeholder, sizeof(ta.placeholder) - 1);
    return ta;
}

void textAreaHandleEvent(TextArea *ta, SDL_Event *e)
{
    if (e->type == SDL_MOUSEBUTTONDOWN)
        ta->focused = inCell(&ta->cell, e->button.x, e->button.y);

    if (!ta->focused) return;

    if (e->type == SDL_KEYDOWN) {
        SDL_Keycode k = e->key.keysym.sym;
        if (k == SDLK_BACKSPACE && ta->bufLen > 0) {
            ta->buffer[--ta->bufLen] = '\0';
        } else if (k == SDLK_RETURN && ta->bufLen < TEXTAREA_BUF - 2) {
            ta->buffer[ta->bufLen++] = '\n';
            ta->buffer[ta->bufLen]   = '\0';
        }
    }

    if (e->type == SDL_TEXTINPUT) {
        int addLen = (int)strlen(e->text.text);
        if (ta->bufLen + addLen < TEXTAREA_BUF - 1) {
            strcat(ta->buffer, e->text.text);
            ta->bufLen += addLen;
        }
    }
}

void textAreaDraw(GUI *gui, TextArea *ta, FontSet *fonts)
{
    guiDrawRect(gui, ta->cell.x, ta->cell.y,
                     ta->cell.w, ta->cell.h,
                     ta->bgColor.r, ta->bgColor.g,
                     ta->bgColor.b, ta->bgColor.a);

    Color border = ta->focused ? ta->borderFocused : ta->borderNormal;
    drawBorder(gui, &ta->cell, border, 2);

    Uint32 now = SDL_GetTicks();
    if (now - ta->lastBlink > 500) {
        ta->cursorVisible = !ta->cursorVisible;
        ta->lastBlink = now;
    }

    int padX = 8, padY = 8;

    if (ta->bufLen == 0) {
        renderText(gui, fonts->regular, ta->placeholder,
                   ta->cell.x + padX, ta->cell.y + padY,
                   ta->placeholderColor);
        return;
    }

    // Render word-wrapped lines, splitting on \n
    char  copy[TEXTAREA_BUF];
    strncpy(copy, ta->buffer, sizeof(copy) - 1);

    char *line = strtok(copy, "\n");
    int   curY = ta->cell.y + padY;

    while (line) {
        char  wbuf[512] = "";
        char  word[256];
        char *p     = line;
        int   wrapY = curY;

        while (*p) {
            int wi = 0;
            while (*p && *p != ' ') word[wi++] = *p++;
            word[wi] = '\0';
            if (*p) p++;

            char test[512];
            snprintf(test, sizeof(test), "%s%s%s",
                     wbuf, wbuf[0] ? " " : "", word);

            int w, h;
            TTF_SizeUTF8(fonts->regular, test, &w, &h);

            if (w > ta->cell.w - padX * 2 && wbuf[0]) {
                renderText(gui, fonts->regular, wbuf,
                           ta->cell.x + padX, wrapY, ta->textColor);
                wrapY += ta->lineHeight;
                strncpy(wbuf, word, sizeof(wbuf) - 1);
            } else {
                strncpy(wbuf, test, sizeof(wbuf) - 1);
            }
        }
        if (wbuf[0])
            renderText(gui, fonts->regular, wbuf,
                       ta->cell.x + padX, wrapY, ta->textColor);

        curY = wrapY + ta->lineHeight;
        line = strtok(NULL, "\n");
    }

    // Draw cursor at end of last line
    if (ta->focused && ta->cursorVisible) {
        char tmp[TEXTAREA_BUF];
        strncpy(tmp, ta->buffer, sizeof(tmp) - 1);
        char *nl      = strrchr(tmp, '\n');
        char *lastSeg = nl ? nl + 1 : tmp;

        int tw = 0;
        TTF_SizeUTF8(fonts->regular, lastSeg, &tw, NULL);

        SDL_SetRenderDrawColor(gui->renderer,
            ta->textColor.r, ta->textColor.g,
            ta->textColor.b, ta->textColor.a);
        SDL_Rect cur = {
            ta->cell.x + padX + tw,
            curY - ta->lineHeight,
            2, TTF_FontHeight(fonts->regular)
        };
        SDL_RenderFillRect(gui->renderer, &cur);
    }
}