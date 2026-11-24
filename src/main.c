#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/stat.h>
#include "storage_engine.h"
#include "buffer.h"
#include "wal.h"
#include <unistd.h>
#include "raylib.h"

#define HIST_MAX 2000
#define LINE_MAX_LEN 4096

#define WIN_W 960
#define WIN_H 600
#define MARGIN 20
#define PAD 14
#define LH 24
#define FS 18.0f
#define SP 0.0f
#define WRAP_CAP 4200
#define VROWS_CAP 128

frames *bp = NULL;

void buffer_pool_init(void) {
    bp = calloc(1, sizeof(frames));
    if (bp == NULL) {
        return;
    }
    bp->mp = calloc(1, sizeof(map_out_frames));
    bp->stat = calloc(1, sizeof(stats));
    if (bp->mp == NULL || bp->stat == NULL) {
        return;
    }
    for (int i = 0; i < frames_in_buf; i++) {
        bp->dt[i] = calloc(1, sizeof(details));
        bp->dt[i]->page_id = INVALID_PAGE_ID;
    }
    /* frame_page_hash is now [max_tables][max_pages]; -1 = all bits set */
    memset(bp->mp->frame_page_hash, -1, sizeof(bp->mp->frame_page_hash));
    bp->mp->free_bits = frames_in_buf;
    for_the_locks_init(bp);

    pthread_t writer_thread;
    pthread_create(&writer_thread, NULL, background_dirty_writer, bp);
    pthread_detach(writer_thread);
}

void buffer_pool_flush(void) {
    if (bp == NULL) {
        return;
    }
    for (int i = 0; i < frames_in_buf; i++) {
        if (bp->dt[i]->valid == 1 && bp->dt[i]->dirty == 1) {
            insert_the_thing(i, bp);
        }
    }
}

void end_transaction(int id) {
    for (int i = 0; i < 256; i++) {
        if (dat->running_transaction[i] == id) {
            dat->running_transaction[i] = -1;
            dat->running_first_lsn[i] = -1;
            dat->running_last_lsn[i] = -1;
            dat->previous_offset[i] = -1;
            dat->num_of_run_transactions--;
            return;
        }
    }
}

void init_page_counter(void) {
    struct stat file_info;
    if (stat(data_filename, &file_info) == 0 && file_info.st_size >= PAGE_SIZE) {
        number_of_pages = (int)(file_info.st_size / PAGE_SIZE) - 1;
    }
}

typedef struct { char *data; int size; bool query; } Row;
struct { Row *ri; int row_length; int query_lines; } edit = {0};

static pagezero pg;
parser_data proper_data;

static char hist[HIST_MAX][LINE_MAX_LEN];
static int  nhist = 0;

static void hist_add(const char *s) {
    if (nhist == HIST_MAX) {
        memmove(hist[0], hist[1], (HIST_MAX - 1) * LINE_MAX_LEN);
        nhist--;
    }
    snprintf(hist[nhist++], LINE_MAX_LEN, "%s", s);
}

static void out(const char *fmt, ...) {
    char tmp[2048];
    va_list ap; va_start(ap, fmt);
    vsnprintf(tmp, sizeof tmp, fmt, ap);
    va_end(ap);
    char *save = NULL;
    for (char *ln = strtok_r(tmp, "\n", &save); ln; ln = strtok_r(NULL, "\n", &save))
        hist_add(ln);
}

int seperator_new(int c) {
    return c == '\0' || strchr(",()+-/*=~%<>[];", c) != NULL;
}

static const char *KW[] = {"CREATE","TABLE","PRIMARY","KEY","FOREIGN","REFERENCES",
    "NOT","NULL","UNIQUE","DEFAULT","INT","BIGINT","SMALLINT","TINYINT","FLOAT",
    "DOUBLE","DECIMAL","CHAR","VARCHAR","BOOL","DATE","TIMESTAMP","AS",
    "SELECT","INSERT","DELETE","INTO","VALUES","FROM","WHERE",
    "REPLACE","IGNORE","ON","DUPLICATE","UPDATE","RETURNING", NULL};

static void upper_if_keyword(char *s) {
    for (int k = 0; KW[k]; k++)
        if (strcasecmp(s, KW[k]) == 0) {
            for (char *p = s; *p; p++) *p = toupper((unsigned char)*p);
            return;
        }
}

static char **tokenize_line(const char *buf) {
    char **temp = malloc(300 * sizeof(char *));
    char te[4096];
    int j = 0, m = 0, len = strlen(buf);
    for (int k = 0; k < len; k++) {
        if (isspace((unsigned char)buf[k])) {
            if (j > 0) { te[j] = '\0'; temp[m] = strdup(te); upper_if_keyword(temp[m++]); j = 0; }
            continue;
        }
        if (seperator_new(buf[k])) {
            if (j > 0) { te[j] = '\0'; temp[m] = strdup(te); upper_if_keyword(temp[m++]); j = 0; }
            char t[2] = {buf[k], '\0'};
            temp[m++] = strdup(t);
        } else if (j < 4095) te[j++] = buf[k];
    }
    if (j > 0) { te[j] = '\0'; temp[m] = strdup(te); upper_if_keyword(temp[m++]); }
    temp[m] = NULL;
    return temp;
}

static char ***build_rows(void) {
    char ***rows = malloc((edit.query_lines + 2) * sizeof(char **));
    int n = 0;
    for (int i = 0; i < edit.query_lines; i++) {
        char **t = tokenize_line(edit.ri[i].data);
        if (t[0] == NULL) { free(t); continue; }
        rows[n++] = t;
    }
    rows[n] = calloc(1, sizeof(char *));
    rows[n + 1] = NULL;
    return rows;
}

static void free_rows(char ***rows) {
    for (int i = 0; rows[i]; i++) {
        for (int j = 0; rows[i][j]; j++) free(rows[i][j]);
        free(rows[i]);
    }
    free(rows);
}

static void clear_query(void) {
    for (int i = 0; i < edit.query_lines; i++) free(edit.ri[i].data);
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

extern char insert_message[200];

static void run_query(void) {
    char ***rows = build_rows();
    if (rows[0][0] == NULL) { free_rows(rows); return; }

    if (strcmp(rows[0][0], "CREATE") == 0) {
        if (rows[0][1] && rows[0][2] && access(rows[0][2], F_OK) == 0) {
            out("Table already exists");
        } 
        else {
            proper_data.query = rows;
            main_create_parser(&pg);
            write_pagezero(&pg, data_filename);
            out("OK");
        }
    }

    else if (strcmp(rows[0][0], "INSERT") == 0 || strcmp(rows[0][0], "REPLACE") == 0) {
        proper_data.query = rows;
        itree *t = insert_parser();
        if (t == NULL) out("Insert syntax error");
        else {
            struct_for_insert(t);
            read_pagezero(&pg, t->table );
            out("%s", insert_message);
        }
    }
    else if (strcmp(rows[0][0], "SELECT") == 0) out("SELECT not connected yet");
    else if (strcmp(rows[0][0], "DELETE") == 0) out("DELETE not connected yet");
    else out("Unknown command: %s", rows[0][0]);

    free_rows(rows);
}

tree *select_query(int row, int col, int check, int end_row, int end_col, int pain) {
    return NULL;
}

static Font g_font;

static float glyph_w(char c) {
    char s[2] = {c, '\0'};
    return MeasureTextEx(g_font, s, FS, SP).x;
}

static int wrap_text(const char *s, int len, float maxw, int *st, int cap) {
    int n = 0, start = 0, lastsp = -1;
    float w = 0;
    st[n++] = 0;
    for (int i = 0; i < len; i++) {
        float cw = glyph_w(s[i]);
        if (w + cw > maxw && i > start) {
            int brk = (lastsp > start) ? lastsp + 1 : i;
            if (n < cap - 1) st[n++] = brk;
            start = brk;
            w = 0;
            lastsp = -1;
            for (int j = start; j < i; j++) {
                w += glyph_w(s[j]);
                if (s[j] == ' ') lastsp = j;
            }
        }
        w += cw;
        if (s[i] == ' ') lastsp = i;
    }
    st[n] = len;
    return n;
}

static void draw_seg(const char *s, int a, int b, float x, float y, Color col) {
    char buf[LINE_MAX_LEN];
    int n = b - a;
    if (n <= 0) return;
    memcpy(buf, s + a, n);
    buf[n] = '\0';
    DrawTextEx(g_font, buf, (Vector2){x, y}, FS, SP, col);
}

typedef struct { int idx, a, b; } VRow;

int main(void) {
    default_pagezero(&pg);

    start_the_wal();
    buffer_pool_init();
    init_page_counter();

    SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT);
    InitWindow(WIN_W, WIN_H, "NitrogenDB");
    SetExitKey(0);
    SetTargetFPS(60);

    const char *fonts[] = {
        "/System/Library/Fonts/Supplemental/Andale Mono.ttf",
        "/System/Library/Fonts/Supplemental/Courier New.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        NULL
    };
    g_font = GetFontDefault();
    for (int f = 0; fonts[f]; f++) {
        if (access(fonts[f], F_OK) != 0) continue;
        g_font = LoadFontEx(fonts[f], 36, NULL, 0);
        if (g_font.texture.id != 0) break;
    }
    SetTextureFilter(g_font.texture, TEXTURE_FILTER_BILINEAR);

    const Color BG     = {12, 12, 12, 255};
    const Color PANEL  = {20, 20, 20, 255};
    const Color BORDER = {70, 70, 70, 255};
    const Color FG     = {210, 210, 210, 255};
    const Color DIM    = {130, 130, 130, 255};

    const Rectangle panel = {MARGIN, MARGIN, WIN_W - 2 * MARGIN, WIN_H - 2 * MARGIN};
    const float text_x = panel.x + PAD;
    const float text_y = panel.y + PAD;
    const float text_w = panel.width - 2 * PAD;
    int rows_fit = (int)((panel.height - 2 * PAD) / LH);
    if (rows_fit > VROWS_CAP) rows_fit = VROWS_CAP;

    static int in_st[WRAP_CAP];
    static int hs[WRAP_CAP];
    VRow vrows[VROWS_CAP];

    char input[LINE_MAX_LEN] = "";
    int cur = 0, continuing = 0;
    bool quit = false;

    while (!WindowShouldClose() && !quit) {
        int len = (int)strlen(input);
        int c;
        while ((c = GetCharPressed()) > 0)
            if (c >= 32 && c < 127 && len < LINE_MAX_LEN - 2) {
                memmove(input + cur + 1, input + cur, len - cur + 1);
                input[cur++] = (char)c; len++;
            }

        bool mod = IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_LEFT_CONTROL);
        if (mod && IsKeyPressed(KEY_V)) {
            const char *clip = GetClipboardText();
            for (int k = 0; clip && clip[k]; k++)
                if (clip[k] >= 32 && clip[k] < 127 && len < LINE_MAX_LEN - 2) {
                    memmove(input + cur + 1, input + cur, len - cur + 1);
                    input[cur++] = clip[k]; len++;
                }
        }
        if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && cur > 0) {
            memmove(input + cur - 1, input + cur, len - cur + 1); cur--; len--;
        }
        if ((IsKeyPressed(KEY_LEFT)  || IsKeyPressedRepeat(KEY_LEFT))  && cur > 0)   cur--;
        if ((IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) && cur < len) cur++;

        if (IsKeyPressed(KEY_ENTER)) {
            char shown[LINE_MAX_LEN + 20];
            snprintf(shown, sizeof shown, "%s%s", continuing ? "          > " : "nitrogendb> ", input);
            hist_add(shown);

            if (!continuing && strcmp(input, ".exit") == 0) quit = true;
            else if (input[0] != '\0' || continuing) {
                add_line(input);
                if (strchr(input, ';')) { run_query(); clear_query(); continuing = 0; }
                else continuing = 1;
            }
            input[0] = '\0'; cur = 0;
        }

        len = (int)strlen(input);
        const char *prompt = continuing ? "          > " : "nitrogendb> ";
        float prompt_w = MeasureTextEx(g_font, prompt, FS, SP).x;
        float input_x = text_x + prompt_w;

        int n_in = wrap_text(input, len, text_w - prompt_w, in_st, WRAP_CAP);
        int in_first = n_in > rows_fit ? n_in - rows_fit : 0;
        int in_shown = n_in - in_first;
        int avail = rows_fit - in_shown;

        int cnt = 0;
        for (int i = nhist - 1; i >= 0 && cnt < avail; i--) {
            int hl = (int)strlen(hist[i]);
            int n = wrap_text(hist[i], hl, text_w, hs, WRAP_CAP);
            for (int k = n - 1; k >= 0 && cnt < avail; k--) {
                vrows[cnt].idx = i;
                vrows[cnt].a = hs[k];
                vrows[cnt].b = hs[k + 1];
                cnt++;
            }
        }

        BeginDrawing();
        ClearBackground(BG);
        DrawRectangleRec(panel, PANEL);
        DrawRectangleLinesEx(panel, 1, BORDER);

        float y = text_y;
        for (int r = cnt - 1; r >= 0; r--, y += LH)
            draw_seg(hist[vrows[r].idx], vrows[r].a, vrows[r].b, text_x, y, FG);

        DrawTextEx(g_font, prompt, (Vector2){text_x, y}, FS, SP, DIM);
        for (int k = in_first; k < n_in; k++)
            draw_seg(input, in_st[k], in_st[k + 1], input_x, y + (k - in_first) * LH, FG);

        int ck = 0;
        for (int k = 0; k < n_in; k++)
            if (in_st[k] <= cur) ck = k;
        if (ck >= in_first) {
            char pre[LINE_MAX_LEN];
            int pn = cur - in_st[ck];
            memcpy(pre, input + in_st[ck], pn);
            pre[pn] = '\0';
            float cx = input_x + MeasureTextEx(g_font, pre, FS, SP).x;
            if (((int)(GetTime() * 2)) % 2 == 0)
                DrawRectangle((int)cx, (int)(y + (ck - in_first) * LH + 2), 2, (int)FS, FG);
        }

        EndDrawing();
    }

    buffer_pool_flush();
    CloseWindow();
    clear_query();
    return 0;
}