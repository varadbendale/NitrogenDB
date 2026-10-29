#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdarg.h>
#include <unistd.h>

#include "Parser/Parser.h"
#include "Storage_engine/storage_engine.h"
#include "raylib.h"

#define DB_FILE "nitrogen.db"
#define HIST_MAX 2000
#define LINE_MAX_LEN 512

typedef struct {
    char *data;
    int size;
    bool query;
} Row;

struct {
    Row *ri;
    int row_length;
    int query_lines;
} edit = {0};

static pagezero pg;
parser_data proper_data;

static char hist[HIST_MAX][LINE_MAX_LEN];
static int nhist = 0;

static void hist_add(const char *s) {
    if (nhist == HIST_MAX) {
        memmove(hist[0], hist[1], (HIST_MAX - 1) * LINE_MAX_LEN);
        nhist--;
    }

    snprintf(hist[nhist++], LINE_MAX_LEN, "%s", s);
}

static void out(const char *fmt, ...) {
    char tmp[2048];

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);

    char *save = NULL;
    char *ln = strtok_r(tmp, "\n", &save);

    while (ln != NULL) {
        hist_add(ln);
        ln = strtok_r(NULL, "\n", &save);
    }
}

int seperator_new(int c) {
    return c == '\0' || strchr(",()+-/*=~%<>[];", c) != NULL;
}

static const char *KW[] = {
    "CREATE", "TABLE", "PRIMARY", "KEY", "FOREIGN", "REFERENCES",
    "NOT", "NULL", "UNIQUE", "DEFAULT", "INT", "BIGINT", "SMALLINT",
    "TINYINT", "FLOAT", "DOUBLE", "DECIMAL", "CHAR", "VARCHAR",
    "BOOL", "DATE", "TIMESTAMP", "AS", "SELECT", "INSERT", "DELETE",
    "INTO", "VALUES", "FROM", "WHERE", NULL
};

static void upper_if_keyword(char *s) {
    for (int k = 0; KW[k] != NULL; k++) {
        if (strcasecmp(s, KW[k]) == 0) {
            for (char *p = s; *p; p++) {
                *p = toupper((unsigned char)*p);
            }

            return;
        }
    }
}

static char **tokenize_line(const char *buf) {
    char **temp = malloc(300 * sizeof(char *));
    char te[300];

    int j = 0;
    int m = 0;
    int len = strlen(buf);

    for (int k = 0; k < len; k++) {
        if (isspace((unsigned char)buf[k])) {
            if (j > 0) {
                te[j] = '\0';
                temp[m] = strdup(te);
                upper_if_keyword(temp[m++]);
                j = 0;
            }

            continue;
        }

        if (seperator_new(buf[k])) {
            if (j > 0) {
                te[j] = '\0';
                temp[m] = strdup(te);
                upper_if_keyword(temp[m++]);
                j = 0;
            }

            char t[2] = {buf[k], '\0'};
            temp[m++] = strdup(t);
        } else if (j < 299) {
            te[j++] = buf[k];
        }
    }

    if (j > 0) {
        te[j] = '\0';
        temp[m] = strdup(te);
        upper_if_keyword(temp[m++]);
    }

    temp[m] = NULL;

    return temp;
}

static char ***build_rows(void) {
    char ***rows = malloc((edit.query_lines + 2) * sizeof(char **));
    int n = 0;

    for (int i = 0; i < edit.query_lines; i++) {
        char **t = tokenize_line(edit.ri[i].data);

        if (t[0] == NULL) {
            free(t);
            continue;
        }

        rows[n++] = t;
    }

    rows[n] = calloc(1, sizeof(char *));
    rows[n + 1] = NULL;

    return rows;
}

static void free_rows(char ***rows) {
    for (int i = 0; rows[i] != NULL; i++) {
        for (int j = 0; rows[i][j] != NULL; j++) {
            free(rows[i][j]);
        }

        free(rows[i]);
    }

    free(rows);
}

static void clear_query(void) {
    for (int i = 0; i < edit.query_lines; i++) {
        free(edit.ri[i].data);
    }

    free(edit.ri);

    edit.ri = NULL;
    edit.query_lines = 0;
    edit.row_length = 0;
}

static void add_line(const char *s) {
    edit.ri = realloc(edit.ri, (edit.query_lines + 1) * sizeof(Row));

    edit.ri[edit.query_lines].data = strdup(s);
    edit.ri[edit.query_lines].size = (int)strlen(s);
    edit.ri[edit.query_lines].query = true;

    edit.query_lines++;
    edit.row_length = edit.query_lines;
}

static void run_query(void) {
    char ***rows = build_rows();

    if (rows[0][0] == NULL) {
        free_rows(rows);
        return;
    }

    if (strcmp(rows[0][0], "CREATE") == 0) {
        proper_data.query = rows;
        main_create_parser(&pg);
        write_pagezero(&pg, DB_FILE);
        out("OK");
    } else if (strcmp(rows[0][0], "INSERT") == 0) {
        out("INSERT not connected yet");
    } else if (strcmp(rows[0][0], "SELECT") == 0) {
        out("SELECT not connected yet");
    } else if (strcmp(rows[0][0], "DELETE") == 0) {
        out("DELETE not connected yet");
    } else {
        out("Unknown command: %s", rows[0][0]);
    }

    free_rows(rows);
}

tree *select_query(int row, int col, int check, int end_row, int end_col, int pain) {
    return NULL;
}

int main(void) {
    if (access(DB_FILE, F_OK) == 0) {
        read_pagezero(&pg, DB_FILE);
    } else {
        default_pagezero(&pg);
        write_pagezero(&pg, DB_FILE);
    }

    SetConfigFlags(
        FLAG_WINDOW_RESIZABLE |
        FLAG_WINDOW_HIGHDPI |
        FLAG_MSAA_4X_HINT
    );

    InitWindow(960, 600, "NitrogenDB");
    SetExitKey(0);
    SetTargetFPS(60);

    const float FS = 18;
    const float SP = 0;

    const char *fonts[] = {
        "/System/Library/Fonts/Supplemental/Andale Mono.ttf",
        "/System/Library/Fonts/Supplemental/Courier New.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        NULL
    };

    Font font = GetFontDefault();

    for (int f = 0; fonts[f] != NULL; f++) {
        if (access(fonts[f], F_OK) != 0) {
            continue;
        }

        font = LoadFontEx(fonts[f], 36, NULL, 0);

        if (font.texture.id != 0) {
            break;
        }
    }

    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

    const Color BG = {12, 12, 12, 255};
    const Color FG = {210, 210, 210, 255};
    const Color DIM = {130, 130, 130, 255};

    const int PAD = 16;
    const int LH = 24;

    char input[LINE_MAX_LEN] = "";

    int cur = 0;
    int continuing = 0;

    bool quit = false;

    while (!WindowShouldClose() && !quit) {
        int len = (int)strlen(input);
        int c;

        while ((c = GetCharPressed()) > 0) {
            if (c >= 32 && c < 127 && len < LINE_MAX_LEN - 2) {
                memmove(input + cur + 1, input + cur, len - cur + 1);
                input[cur++] = (char)c;
                len++;
            }
        }

        bool mod = IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_LEFT_CONTROL);

        if (mod && IsKeyPressed(KEY_V)) {
            const char *clip = GetClipboardText();

            for (int k = 0; clip && clip[k]; k++) {
                if (clip[k] >= 32 && clip[k] < 127 && len < LINE_MAX_LEN - 2) {
                    memmove(input + cur + 1, input + cur, len - cur + 1);
                    input[cur++] = clip[k];
                    len++;
                }
            }
        }

        if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && cur > 0) {
            memmove(input + cur - 1, input + cur, len - cur + 1);
            cur--;
            len--;
        }

        if ((IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) && cur > 0) {
            cur--;
        }

        if ((IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) && cur < len) {
            cur++;
        }

        if (IsKeyPressed(KEY_ENTER)) {
            char shown[LINE_MAX_LEN + 20];

            snprintf(
                shown,
                sizeof(shown),
                "%s%s",
                continuing ? "          > " : "nitrogendb> ",
                input
            );

            hist_add(shown);

            if (!continuing && strcmp(input, ".exit") == 0) {
                quit = true;
            } else if (input[0] != '\0' || continuing) {
                add_line(input);

                if (strchr(input, ';')) {
                    run_query();
                    clear_query();
                    continuing = 0;
                } else {
                    continuing = 1;
                }
            }

            input[0] = '\0';
            cur = 0;
        }

        BeginDrawing();
        ClearBackground(BG);

        int h = GetScreenHeight();
        int rows_fit = (h - 2 * PAD) / LH;
        int total = nhist + 1;

        int first = total > rows_fit ? total - rows_fit : 0;
        int y = PAD;

        for (int i = first; i < nhist; i++, y += LH) {
            DrawTextEx(font, hist[i], (Vector2){PAD, y}, FS, SP, FG);
        }

        const char *prompt = continuing ? "          > " : "nitrogendb> ";

        DrawTextEx(font, prompt, (Vector2){PAD, y}, FS, SP, DIM);

        float px = PAD + MeasureTextEx(font, prompt, FS, SP).x;

        DrawTextEx(font, input, (Vector2){px, y}, FS, SP, FG);

        char pre[LINE_MAX_LEN];

        memcpy(pre, input, cur);
        pre[cur] = '\0';

        float cx = px + MeasureTextEx(font, pre, FS, SP).x;

        if (((int)(GetTime() * 2)) % 2 == 0) {
            DrawRectangle((int)cx, y + 2, 2, (int)FS, FG);
        }

        EndDrawing();
    }

    CloseWindow();
    clear_query();

    return 0;
}
