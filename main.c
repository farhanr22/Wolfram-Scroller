#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "font/font.h"
#include "tigr/tigr.h"
#include "utils/utils.h"

#define SCR_SZ 240
#define HEADER_H 22
#define EXPORT_SCALE 3
#define DEFAULT_RULE 110

enum Mode { RUNNING, PAUSED, HELP, QUIT };

enum GridSize { SMALL = 60, MEDIUM = 120, LARGE = 240 };

typedef struct colorPair {
    unsigned char bg[3];
    unsigned char fg[3];
} colorPair;

colorPair colors[] = {{{255, 230, 230}, {180, 70, 70}},    // RED = 0
                      {{240, 255, 240}, {45, 140, 85}},    // GREEN = 1
                      {{230, 240, 255}, {70, 115, 180}}};  // BLUE = 2

typedef struct {
    int rule;
    enum GridSize width;
    enum Mode mode;
    int color_idx;

    int** cells;
    int* buffer;
    int cell_size_px;
    int fill_counter;
    int start_row_idx;

    int should_save_img;

    TigrFont* font;
    TPixel bg_color;
    TPixel fg_color;
} AppState;

void set_initial_rule(int* rule, int argc, char* argv[]);
void set_color_scheme(AppState* state);

void handle_input(Tigr* screen, AppState* state);
void update_state(AppState* state);
void render_frame(Tigr* screen, AppState* state);
void draw_header(Tigr* screen, AppState* state);
void draw_grid(Tigr* screen, AppState* state);
void draw_help_screen(Tigr* screen, AppState* state);
void export_as_png(Tigr* screen, int rule);

int main(int argc, char* argv[]) {
    srand(time(NULL));

    Tigr* screen =
        tigrWindow(SCR_SZ, SCR_SZ + HEADER_H, "Wolfram CA Scroller", TIGR_2X);
    Tigr* my_font_bmp = tigrLoadImageMem(png_data, sizeof(png_data));

    AppState state = {0};
    state.font = tigrLoadFont(my_font_bmp, TCP_1252);
    state.width = MEDIUM;
    state.color_idx = 0;
    state.mode = RUNNING;
    state.should_save_img = 0;

    // Initialize buffer and grid arrays with maximum possible size
    // actual portion of them in use is dictated by state.width
    state.cells = new_grid(LARGE);
    state.buffer = malloc(sizeof(int) * LARGE);

    set_initial_rule(&state.rule, argc, argv);
    DEBUG_PRINT("Starting with rule: %d\n", state.rule);
    set_color_scheme(&state);

    state.cell_size_px = SCR_SZ / state.width;  // size in pixels of each cell
    state.fill_counter = 0;
    state.start_row_idx = 0;
    init_middle(state.buffer, state.width);

    // Manually copy the first line in to make it visible
    memcpy(state.cells[0], state.buffer, state.width * sizeof(int));
    state.fill_counter++;

    // Main application loop, runs until the user closes the window or hits Esc
    while (!tigrClosed(screen) && state.mode != QUIT) {
        handle_input(screen, &state);
        update_state(&state);
        render_frame(screen, &state);
        tigrUpdate(screen);
    }

    DEBUG_PRINT("Exiting main loop, cleaning up resources.\n");
    tigrFreeFont(state.font);
    tigrFree(screen);
    free(state.buffer);
    free_grid(state.cells, LARGE);
}

// check for a command line argument and use it as the rule if it's valid
void set_initial_rule(int* rule, int argc, char* argv[]) {
    *rule = DEFAULT_RULE;

    if (argc == 2) {
        int valid = 1;
        for (int i = 0; argv[1][i] != '\0'; i++) {
            if (!isdigit(argv[1][i])) {
                valid = 0;
                break;
            }
        }

        if (valid) {
            int num = atoi(argv[1]);
            if (num >= 0 && num <= 255) {
                *rule = num;
                return;
            }
        }
    }

    if (argc != 1) {
        printf("Usage: %s [optional rule: 0-255]\n", argv[0]);
        printf("Invalid argument, using default rule (%d)\n", *rule);
    }
}

// set the foreground and background colors from the colors array
void set_color_scheme(AppState* state) {
    state->bg_color =
        tigrRGB(colors[state->color_idx].bg[0], colors[state->color_idx].bg[1],
                colors[state->color_idx].bg[2]);
    state->fg_color =
        tigrRGB(colors[state->color_idx].fg[0], colors[state->color_idx].fg[1],
                colors[state->color_idx].fg[2]);
}

// handles all the keyboard presses
void handle_input(Tigr* screen, AppState* state) {
    if (tigrKeyDown(screen, TK_ESCAPE)) {
        if (state->mode == HELP) {
            state->mode = RUNNING;
        } else {
            state->mode = QUIT;
        }
        DEBUG_PRINT("Mode changed to %d\n", state->mode);
    }

    if (tigrKeyDown(screen, 'H')) {
        state->mode = (state->mode != HELP) ? HELP : RUNNING;
        DEBUG_PRINT("Mode changed to %d\n", state->mode);
    }

    if (tigrKeyDown(screen, 'C')) {
        state->color_idx =
            (state->color_idx + 1) % (sizeof(colors) / sizeof(colorPair));
        set_color_scheme(state);
        DEBUG_PRINT("Color scheme changed to index %d\n", state->color_idx);
    }
    
    if (state->mode != HELP && state->mode != QUIT) {
        if (tigrKeyDown(screen, 'E')) {
            state->should_save_img = 1;
            DEBUG_PRINT("Image export requested.\n");
        }

        if (tigrKeyDown(screen, TK_SPACE)) {
            state->mode = (state->mode == RUNNING) ? PAUSED : RUNNING;
            DEBUG_PRINT("Mode changed to %d (PAUSED/RUNNING)\n", state->mode);
        }

        if (tigrKeyDown(screen, 'R')) {
            randomize_array(state->buffer, state->width);
            DEBUG_PRINT("Grid randomized.\n");
        }

        if (tigrKeyDown(screen, 'M')) {
            init_middle(state->buffer, state->width);
            DEBUG_PRINT("Grid reset to middle.\n");
        }

        if (tigrKeyDown(screen, 'S')) {
            if (state->width == SMALL) {
                state->width = MEDIUM;
            } else if (state->width == MEDIUM) {
                state->width = LARGE;
            } else {
                state->width = SMALL;
            }
            DEBUG_PRINT("Grid size changed to %d\n", state->width);

            // have to reset everything when size changes
            state->cell_size_px = SCR_SZ / state->width;
            state->fill_counter = 0;
            state->start_row_idx = 0;
            init_middle(state->buffer, state->width);
        }

        if (tigrKeyDown(screen, TK_UP)) {
            state->rule += tigrKeyHeld(screen, TK_SHIFT) ? 10 : 1;
            if (state->rule > 255) state->rule = 0;
            DEBUG_PRINT("Rule changed to %d\n", state->rule);
        }

        if (tigrKeyDown(screen, TK_DOWN)) {
            state->rule -= tigrKeyHeld(screen, TK_SHIFT) ? 10 : 1;
            if (state->rule < 0) state->rule = 255;
            DEBUG_PRINT("Rule changed to %d\n", state->rule);
        }
    }
}

// calculates the next generation and scrolls the grid if needed
void update_state(AppState* state) {
    if (state->mode == RUNNING) {
        next_gen(state->buffer, state->width, state->rule);

        if (state->fill_counter < state->width) {
            // Initial phase : Filling
            // The screen is not full yet, write to the next available line.
            memcpy(state->cells[state->fill_counter], state->buffer,
                   state->width * sizeof(int));
            state->fill_counter++;
        } else {
            // Stable phase: Scrolling
            // Overwrite the oldest row
            memcpy(state->cells[state->start_row_idx], state->buffer,
                   state->width * sizeof(int));
            // Move the start pointer to the next row,
            // wrapping around so that the rows are drawn in the correct order
            // Essentially use the entire matrix like a ring buffer of rows
            state->start_row_idx = (state->start_row_idx + 1) % state->width;
        }
        DEBUG_PRINT("fill_counter: %d, start_row_idx: %d\n",
                    state->fill_counter, state->start_row_idx);
    }
}

// main drawing function, decides what screen to show
void render_frame(Tigr* screen, AppState* state) {
    tigrClear(screen, state->bg_color);

    if (state->mode == HELP) {
        draw_help_screen(screen, state);
    } else if (state->mode != QUIT) {
        draw_header(screen, state);
        draw_grid(screen, state);
    }

    if (state->should_save_img) {
        export_as_png(screen, state->rule);
        state->should_save_img = 0;
    }
}

void draw_header(Tigr* screen, AppState* state) {
    tigrPrint(screen, state->font, 4, 6, state->fg_color,
              "Wolfram CA Scroller:%3d          H:Help", state->rule);
    tigrRect(screen, 0, 0, SCR_SZ, HEADER_H, state->fg_color);
}

void draw_grid(Tigr* screen, AppState* state) {
    int c_sz = state->cell_size_px;
    int rows_to_draw = (state->fill_counter < state->width)
                           ? state->fill_counter
                           : state->width;

    for (int i = 0; i < rows_to_draw; i++) {
        int source_row = (state->start_row_idx + i) % state->width;

        for (int j = 0; j < state->width; j++) {
            if (state->cells[source_row][j]) {
                tigrFill(screen, j * c_sz, HEADER_H + i * c_sz, c_sz, c_sz,
                         state->fg_color);
            }
        }
    }
}

void draw_help_screen(Tigr* screen, AppState* state) {
    tigrPrint(screen, state->font, 4, 6, state->fg_color,
              "Wolfram CA Scroller:Help");
    tigrRect(screen, 0, 0, SCR_SZ, HEADER_H, state->fg_color);
    tigrRect(screen, 0, HEADER_H - 1, SCR_SZ, SCR_SZ + 1, state->fg_color);

    const char* const strings[] = {
        "R       » Randomize cell values",
        "M       » Set middle cell to 1",
        "          and the rest to 0",
        "C       » Cycle through colors",
        "S       » Cycle through sizes",
        "E       » Export as PNG",
        "Esc     » Close help / Quit",
        "Space   » Pause / Play",
        "Up/Down » Change rule",
        "(Shift) » Change rule by 10",
        "",
        "",
        "* github.com/",
        "  farhanr22/",
        "  Wolfram-Scroller",
    };

    int num_lines = sizeof(strings) / sizeof(strings[0]);
    for (int i = 0; i < num_lines; i++) {
        tigrPrint(screen, state->font, 5, 1.5 * HEADER_H + (i * 13),
                  state->fg_color, strings[i]);
    }

    // design thing for the corner
    for (int i = 0; i < 21; i++) {
        int pos_y = SCR_SZ + HEADER_H;
        int st_pos_x = 70;
        int st_pos_y_2 = SCR_SZ + HEADER_H;
        int pos_x_2 = SCR_SZ;
        tigrLine(screen, st_pos_x + (i * 8), pos_y, pos_x_2,
                 st_pos_y_2 - (i * 15), state->fg_color);
    }
}

void export_as_png(Tigr* screen, int rule) {
    int sc = EXPORT_SCALE;
    // temporary bitmap for constructing export
    Tigr* bmp = tigrBitmap(SCR_SZ * sc, SCR_SZ * sc);

    char time_str[20];               // buffer as time string
    time_t now = time(NULL);         // seconds since epoch
    struct tm* t = localtime(&now);  // convert into broken down struct

    // format to human readable
    strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H-%M-%S", t);

    char filename[40];  // actual filename string buffer
    sprintf(filename, "rule%03d_%s.png", rule, time_str);  // combine them

    DEBUG_PRINT("Saving image as %s\n", filename);

    // Copy data from the main screen to the export bitmap, scaling up.
    for (int i = 0; i < SCR_SZ; i++) {
        for (int j = 0; j < SCR_SZ; j++) {
            tigrRect(bmp, i * sc, j * sc, sc, sc,
                     tigrGet(screen, i, HEADER_H + j));
            if (sc > 2) {
                tigrFillRect(bmp, i * sc, j * sc, sc, sc,
                             tigrGet(screen, i, HEADER_H + j));
            }
        }
    }

    // save to png and free temporary bitmap
    tigrSaveImage(filename, bmp);
    tigrFree(bmp);
}