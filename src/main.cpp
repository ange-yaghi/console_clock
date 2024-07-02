#include <assert.h>
#include <conio.h>
#include <iostream>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <time.h>

#include <Windows.h>

#include "../include/file_manager.h"
#include <console_clock.h>

static const DWORD default_color = FOREGROUND_BLUE | FOREGROUND_GREEN |
                                   FOREGROUND_RED | FOREGROUND_INTENSITY;

struct screen_buffer {
    char *c;
    DWORD *attributes;
    COORD size;
};

struct line_information {
    line_count lc;
    std::mutex lc_lock;
    std::string settings_path;
    std::atomic_bool kill;
};

static screen_buffer *c_0 = NULL, *c_1 = NULL, *c_2 = NULL, *c_3 = NULL,
                     *c_4 = NULL, *c_5 = NULL, *c_6 = NULL, *c_7 = NULL,
                     *c_8 = NULL, *c_9 = NULL, *c_colon = NULL;

struct screen_buffer *get_glyph(char c) {
    switch (c) {
        case '0':
            return c_0;
        case '1':
            return c_1;
        case '2':
            return c_2;
        case '3':
            return c_3;
        case '4':
            return c_4;
        case '5':
            return c_5;
        case '6':
            return c_6;
        case '7':
            return c_7;
        case '8':
            return c_8;
        case '9':
            return c_9;
        case ':':
            return c_colon;
        default:
            return nullptr;
    }
}

struct timer_context {
    char *profile;
    unsigned int seconds_elapsed;
    unsigned int last_lc_write;
};

void init_buffer(screen_buffer *buffer, SHORT width, SHORT height) {
    buffer->c = (char *) malloc(sizeof(char) * width * height);
    buffer->attributes = (DWORD *) malloc(sizeof(DWORD) * width * height);
    buffer->size = {width, height};
}

void free_buffer(screen_buffer *buffer) {
    free(buffer->c);
    free(buffer->attributes);
}

void clear_buffer(screen_buffer *buffer, char c) {
    const SHORT width = buffer->size.X;
    const SHORT height = buffer->size.Y;

    for (SHORT y = 0; y < height; ++y) {
        for (SHORT x = 0; x < width; ++x) {
            buffer->c[y * buffer->size.X + x] = c;
            buffer->attributes[y * buffer->size.X + x] = default_color;
        }
    }
}

size_t stripped_len(const char *s) {
    for (size_t length = 0; s[length] != NULL; ++length) {
        if (s[length] == '\r' || s[length] == '\n') { return length; }
    }

    return 0;
}

int load_character(const char *root_path, const char *fname,
                   screen_buffer *buffer) {
    char full_path[1024];
    sprintf(full_path, "%s/%s", root_path, fname);

    FILE *fp = fopen(full_path, "r");
    size_t read;
    size_t len = 0;
    char line[256];
    int width = 0, height = 0;

    if (fp == NULL) {
        buffer->c = NULL;
        buffer->size = {0, 0};
        return 0;
    }

    while (fgets(line, 256, fp) != NULL) {
        width = max(width, stripped_len(line));
        ++height;
    }

    init_buffer(buffer, width, height);
    clear_buffer(buffer, 'X');

    fseek(fp, 0, SEEK_SET);

    int current_line = 0;
    while (fgets(line, 256, fp) != NULL) {
        const size_t line_width = stripped_len(line);
        memcpy(&buffer->c[current_line++ * width], line,
               sizeof(char) * line_width);
    }

    fclose(fp);

    return 1;
}

void load_characters(const char *root_path) {
    c_0 = (struct screen_buffer *) malloc(sizeof(struct screen_buffer));
    c_1 = (struct screen_buffer *) malloc(sizeof(struct screen_buffer));
    c_2 = (struct screen_buffer *) malloc(sizeof(struct screen_buffer));
    c_3 = (struct screen_buffer *) malloc(sizeof(struct screen_buffer));
    c_4 = (struct screen_buffer *) malloc(sizeof(struct screen_buffer));
    c_5 = (struct screen_buffer *) malloc(sizeof(struct screen_buffer));
    c_6 = (struct screen_buffer *) malloc(sizeof(struct screen_buffer));
    c_7 = (struct screen_buffer *) malloc(sizeof(struct screen_buffer));
    c_8 = (struct screen_buffer *) malloc(sizeof(struct screen_buffer));
    c_9 = (struct screen_buffer *) malloc(sizeof(struct screen_buffer));
    c_colon = (struct screen_buffer *) malloc(sizeof(struct screen_buffer));

    load_character(root_path, "resources/big_money_nw/0.txt", c_0);
    load_character(root_path, "resources/big_money_nw/1.txt", c_1);
    load_character(root_path, "resources/big_money_nw/2.txt", c_2);
    load_character(root_path, "resources/big_money_nw/3.txt", c_3);
    load_character(root_path, "resources/big_money_nw/4.txt", c_4);
    load_character(root_path, "resources/big_money_nw/5.txt", c_5);
    load_character(root_path, "resources/big_money_nw/6.txt", c_6);
    load_character(root_path, "resources/big_money_nw/7.txt", c_7);
    load_character(root_path, "resources/big_money_nw/8.txt", c_8);
    load_character(root_path, "resources/big_money_nw/9.txt", c_9);
    load_character(root_path, "resources/big_money_nw/colon.txt", c_colon);
}

void update_screen(HANDLE console_handle, screen_buffer *back_buffer,
                   screen_buffer *front_buffer, bool force) {
    SHORT width = min(back_buffer->size.X, front_buffer->size.X);
    SHORT height = min(back_buffer->size.Y, front_buffer->size.Y);

    for (SHORT y = 0; y < height; ++y) {
        for (SHORT x = 0; x < width; ++x) {
            const char f_c = front_buffer->c[y * width + x];
            const char b_c = back_buffer->c[y * width + x];

            const char f_attributes = front_buffer->attributes[y * width + x];
            const char b_attributes = back_buffer->attributes[y * width + x];

            if (force || f_c != b_c || f_attributes != b_attributes) {
                SetConsoleTextAttribute(console_handle, f_attributes);
                SetConsoleCursorPosition(console_handle, {x, y});
                WriteConsoleA(console_handle, &front_buffer->c[y * width + x],
                              1, NULL, NULL);
            }
        }
    }

    SetConsoleTextAttribute(console_handle, default_color);
}

void draw_text(const char *text, screen_buffer *buffer, COORD pos,
               DWORD attributes, int *right_edge = NULL) {
    if (pos.Y < 0 || pos.Y >= buffer->size.Y) return;

    for (SHORT i = 0; text[i] != 0; ++pos.X, ++i) {
        if (pos.X < 0 || pos.X >= buffer->size.X) continue;
        buffer->c[pos.Y * buffer->size.X + pos.X] = text[i];
        buffer->attributes[pos.Y * buffer->size.X + pos.X] = attributes;
    }

    if (right_edge != NULL) { *right_edge = pos.X; }
}

void blit(screen_buffer *target, screen_buffer *buffer, COORD pos) {
    const SHORT width = buffer->size.X;
    const SHORT height = buffer->size.Y;

    const SHORT t_width = target->size.X;
    const SHORT t_height = target->size.Y;

    for (SHORT y = 0; y < height; ++y) {
        for (SHORT x = 0; x < width; ++x) {
            const int x_f = x + pos.X;
            const int y_f = y + pos.Y;

            if (x_f < 0 || x_f >= t_width) continue;
            if (y_f < 0 || y_f >= t_height) continue;

            target->c[y_f * t_width + x_f] = buffer->c[y * width + x];
            target->attributes[y_f * t_width + x_f] =
                    buffer->attributes[y * width + x];
        }
    }
}

void draw_large_text(const char *text, screen_buffer *target, COORD pos,
                     int *right_edge) {
    for (int i = 0; text[i] != NULL; ++i) {
        const char c = text[i];
        struct screen_buffer *glyph = get_glyph(c);

        if (glyph == NULL) continue;

        const int width = glyph->size.X;
        blit(target, glyph, pos);

        pos.X += width;
    }

    *right_edge = pos.X;
}

void show_cursor(HANDLE console_handle, bool show = true) {
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = show ? TRUE : FALSE;
    SetConsoleCursorInfo(console_handle, &info);
}

void load_timer_context(struct timer_context *context) {
    char fname[256];
    sprintf(fname, "./workspace/timer_%s.txt", context->profile);

    FILE *fp = fopen(fname, "r");

    if (fp == NULL) {
        context->seconds_elapsed = 0;
    } else {
        fscanf(fp, "%u", &context->seconds_elapsed);
        fclose(fp);
    }

    context->last_lc_write = context->seconds_elapsed;
}

void save_timer_context(struct timer_context *context, line_count *lc,
                        bool lc_valid) {
    char fname[256];
    sprintf(fname, "./workspace/timer_%s.txt", context->profile);

    FILE *fp = fopen(fname, "w");
    if (fp == NULL) { return; }

    fprintf(fp, "%u", context->seconds_elapsed);
    fclose(fp);

    if (context->seconds_elapsed - context->last_lc_write >= 10 && lc_valid) {
        context->last_lc_write = context->seconds_elapsed;
        sprintf(fname, "./workspace/line_count_%s.txt", context->profile);
        fp = fopen(fname, "a");
        fprintf(fp, "%u\t%d\t%d\n", context->seconds_elapsed, lc->line_count,
                lc->raw_line_count);
        fclose(fp);
    }
}

void get_screen_size(HANDLE console_handle, int *width, int *height) {
    CONSOLE_SCREEN_BUFFER_INFO screen_info;
    GetConsoleScreenBufferInfo(console_handle, &screen_info);
    *width = screen_info.srWindow.Right - screen_info.srWindow.Left + 1;
    *height = screen_info.srWindow.Bottom - screen_info.srWindow.Top + 1;
}

bool is_inactive() {
    for (int i = 0; i < 255; ++i) {
        if ((GetAsyncKeyState(i) & 0x1) != 0) { return false; }
    }

    return true;
}

const char *get_month(int m) {
    switch (m) {
        case 0:
            return "JAN";
        case 1:
            return "FEB";
        case 2:
            return "MAR";
        case 3:
            return "APR";
        case 4:
            return "MAY";
        case 5:
            return "JUN";
        case 6:
            return "JUL";
        case 7:
            return "AUG";
        case 8:
            return "SEP";
        case 9:
            return "OCT";
        case 10:
            return "NOV";
        case 11:
            return "DEC";
        default:
            return "XXX";
    }
}

const char *get_weekday(int i) {
    switch (i) {
        case 0:
            return "SUN";
        case 1:
            return "MON";
        case 2:
            return "TUE";
        case 3:
            return "WED";
        case 4:
            return "THU";
        case 5:
            return "FRI";
        case 6:
            return "SAT";
        default:
            return "XXX";
    }
}

void draw_status_indicators(struct screen_buffer *target, bool paused,
                            bool active, bool single_line, line_count *lc,
                            COORD pos) {
    static const char *status_paused = " PAUSED         ";
    static const char *status_active = " ACTIVE         ";
    static const char *status_idle = " IDLE           ";
    static const char *status_empty = "                ";

    const COORD pos_0_0 = pos;
    const COORD pos_0_1 = {pos.X, (SHORT) (pos.Y + 1)};
    const COORD pos_1_0 = {(SHORT) (pos.X + 32), pos.Y};
    const COORD pos_2_0 = {(SHORT) (pos.X + 32 + 16), pos.Y};
    const COORD pos_0_2 = {pos.X, (SHORT) (pos.Y + 2)};
    const COORD pos_0_3 = {pos.X, (SHORT) (pos.Y + 3)};
    const COORD pos_3_0 = {(SHORT) (pos.X + 32 + 32)};

    const COORD pos_0 = pos;
    const COORD pos_1 = single_line ? pos_1_0 : pos_0_1;
    const COORD pos_2 = single_line ? pos_2_0 : pos_0_2;
    const COORD pos_3 = single_line ? pos_3_0 : pos_0_3;

    if (paused) { ::draw_text(status_paused, target, pos_2, BACKGROUND_RED); }

    if (active) {
        ::draw_text(status_active, target, pos_1, BACKGROUND_BLUE);
    } else {
        ::draw_text(status_idle, target, pos_1,
                    BACKGROUND_RED | BACKGROUND_GREEN);
    }

    time_t t = time(NULL);
    struct tm time_data = *localtime(&t);
    char current_time[256];

    const char *am_pm = (time_data.tm_hour >= 12) ? "PM" : "AM";
    const int hour =
            (time_data.tm_hour % 12 == 0) ? 12 : time_data.tm_hour % 12;

    sprintf(current_time, "%s %s %d, %d | %d:%02d %s",
            get_weekday(time_data.tm_wday), get_month(time_data.tm_mon),
            time_data.tm_mday, time_data.tm_year + 1900, hour, time_data.tm_min,
            am_pm);

    ::draw_text(current_time, target, pos_0, default_color);

    sprintf(current_time, "[%d/%d]", lc->line_count, lc->raw_line_count);
    ::draw_text(current_time, target, pos_3, default_color);
}

void on_exit(HANDLE console_handle, SHORT width, SHORT height) {
    for (SHORT y = 0; y < height; ++y) {
        for (SHORT x = 0; x < width; ++x) {
            SetConsoleCursorPosition(console_handle, {x, y});
            WriteConsoleA(console_handle, " ", 1, NULL, NULL);
        }
    }

    SetConsoleCursorPosition(console_handle, {0, 0});
    show_cursor(console_handle, true);
}

void line_count_thread(line_information *info) {
    while (!info->kill) {
        lc_settings settings;
        read_settings(&settings, info->settings_path.c_str());

        line_count lc;
        count_all(&lc, &settings);

        {
            std::unique_lock<std::mutex> lock(info->lc_lock);
            info->lc = lc;
        }

        Sleep(1000);
    }
}

int main(int argc, char *argv[]) {
    char profile[256];
    char root_path[256];

    if (argc >= 2) {
        ::strcpy(root_path, argv[1]);
    } else {
        ::strcpy(root_path, "../");
    }

    if (argc >= 3) {
        ::strcpy(profile, argv[2]);
    } else {
        ::strcpy(profile, "default");
    }

    line_information li;
    li.lc.line_count = li.lc.raw_line_count = -1;
    li.kill = false;
    li.settings_path = "./workspace/lc.txt";

    std::thread *lc_thread = new std::thread(&line_count_thread, &li);

    const int timeout_period_ms = 5 * 60 * 1000;
    const int refresh_period_ms = 100;

    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    show_cursor(console_handle, false);

    int screen_width, screen_height;
    get_screen_size(console_handle, &screen_width, &screen_height);

    screen_buffer buffer_0;
    screen_buffer buffer_1;

    load_characters(root_path);

    init_buffer(&buffer_0, screen_width, screen_height);
    init_buffer(&buffer_1, screen_width, screen_height);

    clear_buffer(&buffer_0, ' ');
    clear_buffer(&buffer_1, 'X');

    struct timer_context context;
    context.profile = profile;
    load_timer_context(&context);

    struct screen_buffer *back = &buffer_0;
    struct screen_buffer *front = &buffer_1;

    unsigned long seconds_elapsed_start = context.seconds_elapsed;
    unsigned long t_unix_start = time(NULL);

    bool manual_pause = false;
    bool running = true;
    bool paused = false;
    bool is_inline = false;
    bool prev_paused = false;
    bool show_seconds = true;

    int inactivity_counter = 0;

    while (running) {
        // Retrieve line count information
        line_count lc;
        {
            std::unique_lock<std::mutex> lck(li.lc_lock);
            lc = li.lc;
        }

        bool force_update = false;
        int new_screen_width, new_screen_height;
        get_screen_size(console_handle, &new_screen_width, &new_screen_height);

        if (is_inactive()) {
            inactivity_counter += refresh_period_ms;
        } else {
            inactivity_counter = 0;
        }

        if (_kbhit()) {
            const char c = _getch();
            if (c == ' ') {
                manual_pause = !manual_pause;
            } else if (c == 27) {
                running = false;
            } else if (c == 's') {
                show_seconds = !show_seconds;
            } else if (c == 'i') {
                is_inline = !is_inline;
            }
        }

        if (inactivity_counter >= timeout_period_ms || manual_pause) {
            paused = true;
            inactivity_counter = min(inactivity_counter, timeout_period_ms);
        } else {
            paused = false;
        }

        if (new_screen_width != screen_width ||
            new_screen_height != screen_height) {
            screen_width = new_screen_width;
            screen_height = new_screen_height;

            free_buffer(&buffer_0);
            free_buffer(&buffer_1);

            init_buffer(&buffer_0, screen_width, screen_height);
            init_buffer(&buffer_1, screen_width, screen_height);

            force_update = true;
        }

        struct screen_buffer *temp = back;
        back = front;
        front = temp;

        clear_buffer(front, ' ');

        unsigned long t_unix = time(NULL);
        unsigned long elapsed = t_unix - t_unix_start;

        if (paused && !prev_paused) {
            seconds_elapsed_start += elapsed;
            elapsed = 0;
        } else if (!paused && prev_paused) {
            t_unix_start = t_unix;
            elapsed = 0;
        } else if (paused) {
            elapsed = 0;
        }

        prev_paused = paused;

        context.seconds_elapsed = seconds_elapsed_start + elapsed;

        char buffer[256];
        if (show_seconds) {
            sprintf(buffer, "%02u:%02u:%02u", context.seconds_elapsed / 3600,
                    (context.seconds_elapsed % 3600) / 60,
                    context.seconds_elapsed % 60);
        } else {
            sprintf(buffer, "%02u:%02u", context.seconds_elapsed / 3600,
                    (context.seconds_elapsed % 3600) / 60);
        }

        int right_edge;
        if (!is_inline) {
            draw_large_text(buffer, front, {3, 3}, &right_edge);
        } else {
            draw_text(buffer, front, {1, 0}, default_color, &right_edge);
        }

        draw_status_indicators(front, paused, inactivity_counter < 5000,
                               is_inline, &lc,
                               {(SHORT) (right_edge + 1),
                                (is_inline) ? (SHORT) 0 : (SHORT) 3});

        update_screen(console_handle, back, front, force_update);

        save_timer_context(&context, &lc, lc.line_count != -1);

        Sleep(refresh_period_ms);
    }

    on_exit(console_handle, screen_width, screen_height);

    li.kill = true;
    lc_thread->join();
    delete lc_thread;

    return 0;
}
