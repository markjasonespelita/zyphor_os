#include <stdio.h>
#include "helpers/headers/helperGUI.h"
#include "helpers/headers/helperLayout.h"
#include "helpers/headers/helperWidgets.h"

int main()
{
    GUI gui;
    if (!guiInit(&gui, "Zyphor GUI", 800, 600)) return 1;

    FontSet fonts;
    if (!widgetsInit(&fonts,
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 16))
    {
        return 1;
    }

    SDL_StartTextInput();

    // ── Colors ──────────────────────────────────────────────────────────────
    Color white       = {255, 255, 255, 255};
    Color lightGray   = {200, 200, 200, 255};
    Color inputBg     = {45,  45,  45,  255};
    Color borderNorm  = {80,  80,  80,  255};
    Color borderFocus = {100, 149, 237, 255};
    Color placeholder = {120, 120, 120, 255};
    Color btnBg       = {70,  130, 180, 255};
    Color btnHover    = {100, 160, 210, 255};
    Color btnActive   = {50,  100, 150, 255};

    // ── Layout ──────────────────────────────────────────────────────────────
    Container root = layoutContainer(0, 0, 800, 600, 30, 30);

    Cell headerRow[1]; int hs[] = {12};
    root.y = layoutRow(&root, hs, headerRow, 1, 40, 0) + 8;

    Cell inputRow[2]; int is[] = {6, 6};
    root.y = layoutRow(&root, is, inputRow, 2, 40, 12) + 12;

    Cell taRow[1]; int ts[] = {12};
    root.y = layoutRow(&root, ts, taRow, 1, 130, 0) + 12;

    Cell btnRow[2]; int bs[] = {9, 3};
    layoutRow(&root, bs, btnRow, 2, 40, 12);

    // ── Widgets ─────────────────────────────────────────────────────────────
    TextInput nameInput  = textInputCreate(inputRow[0], "Full name...",
        inputBg, borderNorm, borderFocus, white, placeholder);

    TextInput emailInput = textInputCreate(inputRow[1], "Email address...",
        inputBg, borderNorm, borderFocus, white, placeholder);

    TextArea msgArea = textAreaCreate(taRow[0], "Write your message here...", 22,
        inputBg, borderNorm, borderFocus, white, placeholder);

    Button submitBtn = buttonCreate(btnRow[1], "Submit",
        btnBg, btnHover, btnActive, white);

    // ── Event loop ───────────────────────────────────────────────────────────
    int mx = 0, my = 0, mouseDown = 0, mouseReleased = 0;

    while (gui.running) {
        mouseReleased = 0;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)        gui.running = 0;
            if (event.type == SDL_MOUSEMOTION) { mx = event.motion.x; my = event.motion.y; }
            if (event.type == SDL_MOUSEBUTTONDOWN) mouseDown = 1;
            if (event.type == SDL_MOUSEBUTTONUP)   { mouseDown = 0; mouseReleased = 1; }

            textInputHandleEvent(&nameInput,  &event);
            textInputHandleEvent(&emailInput, &event);
            textAreaHandleEvent (&msgArea,    &event);
        }

        buttonUpdate(&submitBtn, mx, my, mouseDown);
        if (buttonIsClicked(&submitBtn, mx, my, mouseReleased)) {
            printf("Name:    %s\n", nameInput.buffer);
            printf("Email:   %s\n", emailInput.buffer);
            printf("Message: %s\n", msgArea.buffer);
        }

        guiClear(&gui, 30, 30, 30, 255);

        drawHeader(&gui, &fonts, "Contact Form",
                   headerRow[0].x, headerRow[0].y, H1, white);

        drawHeader(&gui, &fonts, "Fill in all fields before submitting.",
                   btnRow[0].x, btnRow[0].y + 10, H3, lightGray);

        textInputDraw(&gui, &nameInput,  &fonts);
        textInputDraw(&gui, &emailInput, &fonts);
        textAreaDraw (&gui, &msgArea,    &fonts);
        buttonDraw   (&gui, &submitBtn,  &fonts);

        guiPresent(&gui);
    }

    SDL_StopTextInput();
    widgetsDestroy(&fonts);
    guiDestroy(&gui);
    return 0;
}