/* textreader
Code by Chris Stoddard */

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>

#define MAX_LINES 10000
#define MAX_LINE_LEN 1024

char *lines[MAX_LINES];
int total_lines = 0;
char state_file_path[PATH_MAX];

void init_state_file_path(const char *filepath) {
    char path_copy[PATH_MAX];
    realpath(filepath, path_copy);
    snprintf(state_file_path, sizeof(state_file_path), "%s/.textreader.state", dirname(path_copy));
}

int load_last_position(const char *filepath) {
    init_state_file_path(filepath);
    char abspath[PATH_MAX];
    realpath(filepath, abspath);

    FILE *fp = fopen(state_file_path, "r");
    if (!fp) return 0;

    char file_entry[PATH_MAX];
    int pos = 0;
    while (fscanf(fp, "%s %d", file_entry, &pos) == 2) {
        if (strcmp(file_entry, abspath) == 0) {
            fclose(fp);
            return pos;
        }
    }

    fclose(fp);
    return 0;
}

void save_position(const char *filepath, int pos) {
    init_state_file_path(filepath);
    char abspath[PATH_MAX];
    realpath(filepath, abspath);

    FILE *fp = fopen(state_file_path, "r");
    FILE *tmp = tmpfile();
    char buffer[PATH_MAX + 16];
    int updated = 0;

    if (fp) {
        while (fgets(buffer, sizeof(buffer), fp)) {
            char file_entry[PATH_MAX];
            int old_pos;
            if (sscanf(buffer, "%s %d", file_entry, &old_pos) == 2) {
                if (strcmp(file_entry, abspath) == 0) {
                    fprintf(tmp, "%s %d\n", abspath, pos);
                    updated = 1;
                } else {
                    fputs(buffer, tmp);
                }
            }
        }
        fclose(fp);
    }

    if (!updated) fprintf(tmp, "%s %d\n", abspath, pos);

    FILE *out = fopen(state_file_path, "w");
    if (!out) return;
    rewind(tmp);
    while (fgets(buffer, sizeof(buffer), tmp)) fputs(buffer, out);
    fclose(out);
    fclose(tmp);
}

void load_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) { perror("fopen"); exit(1); }

    char buffer[MAX_LINE_LEN];
    while (fgets(buffer, sizeof(buffer), fp) && total_lines < MAX_LINES) {
        lines[total_lines++] = strdup(buffer);
    }

    fclose(fp);
}

void free_lines() {
    for (int i = 0; i < total_lines; i++) {
        free(lines[i]);
    }
}

void draw_screen(int start, int height) {
    clear();
    for (int i = 0; i < height && (start + i) < total_lines; i++) {
        mvprintw(i, 0, "%s", lines[start + i]);
    }
    refresh();
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <textfile>\n", argv[0]);
        return 1;
    }

    load_file(argv[1]);

    initscr(); cbreak(); noecho(); keypad(stdscr, TRUE);
    int ch, start = load_last_position(argv[1]), h = LINES;

    draw_screen(start, h);

    while ((ch = getch()) != 'q') {
        switch (ch) {
            case KEY_UP: if (start > 0) start--; break;
            case KEY_DOWN: if (start + h < total_lines) start++; break;
            case ' ': start += h; if (start > total_lines - h) start = total_lines - h; break;
        }
        draw_screen(start, h);
    }

    endwin();
    save_position(argv[1], start);
    free_lines();
    return 0;
}

