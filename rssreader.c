/* rssreader.c
Code by Chris Stoddard */

#include <ncurses.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>

#define MAX_ITEMS 100
#define MAX_TITLE 256
#define MAX_LINK 512
#define SCREEN_ROWS 38
#define SCREEN_COLS 80
#define DISPLAY_LIMIT 10

struct FeedItem {
    char title[MAX_TITLE];
    char link[MAX_LINK];
};

struct Buffer {
    char *data;
    size_t size;
};

struct FeedItem items[MAX_ITEMS];
int total_items = 0;

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct Buffer *mem = (struct Buffer *)userp;

    mem->data = realloc(mem->data, mem->size + realsize + 1);
    if (!mem->data) return 0;

    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;

    return realsize;
}

void parse_rss(const char *xml) {
    const char *p = xml;
    total_items = 0;

    while ((p = strstr(p, "<item>")) && total_items < DISPLAY_LIMIT) {
        struct FeedItem *item = &items[total_items];
        const char *start, *end;

        // Title
        start = strstr(p, "<title>");
        end = strstr(p, "</title>");
        if (start && end && end > start) {
            start += 7;
            int len = (int)(end - start);
            if (strncmp(start, "<![CDATA[", 9) == 0 && len > 12) {
                start += 9;
                len -= 12;
            }
            snprintf(item->title, MAX_TITLE, "%.*s", len, start);
        }

        // Link
        start = strstr(p, "<link>");
        end = strstr(p, "</link>");
        if (start && end && end > start) {
            start += 6;
            int len = (int)(end - start);
            if (strncmp(start, "<![CDATA[", 9) == 0 && len > 12) {
                start += 9;
                len -= 12;
            }
            snprintf(item->link, MAX_LINK, "%.*s", len, start);
        }

        total_items++;
        p += 6; // move past <item>
    }
}

int fetch_rss(const char *url, struct Buffer *buf) {
    CURL *curl = curl_easy_init();
    if (!curl) return 0;

    buf->data = malloc(1);
    buf->size = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)buf);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "rssreader/1.0");

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}

void draw_screen(int offset, int selected) {
    clear();
    int row = 0;

    for (int i = offset; i < total_items && (row + 2) < SCREEN_ROWS; i++) {
        if (i == selected) attron(A_REVERSE);
        mvprintw(row++, 0, "%s", items[i].title);
        if (i == selected) attroff(A_REVERSE);
        if (row < SCREEN_ROWS) mvprintw(row++, 0, "");
        if (row < SCREEN_ROWS) mvprintw(row++, 0, "");
    }

    mvprintw(SCREEN_ROWS, 0, "[Enter] Open  [q] Quit  [%d/%d] %s",
             selected + 1, total_items, items[selected].link);
    clrtoeol();
    refresh();
}

int main(int argc, char *argv[]) {
    char feed_url[512] = {0};
    char *home = getenv("HOME");
    char rssfile[PATH_MAX];
    snprintf(rssfile, sizeof(rssfile), "%s/.rssfeeds", home ? home : ".");

    // Ensure ~/.rssfeeds exists
    FILE *fp = fopen(rssfile, "a+");
    if (!fp) {
        fprintf(stderr, "Could not open or create %s\n", rssfile);
        return 1;
    }
    fseek(fp, 0, SEEK_SET);  // rewind to read

    if (argc == 2) {
        strncpy(feed_url, argv[1], sizeof(feed_url) - 1);

        // Check if URL is already present
        char line[512];
        int found = 0;
        while (fgets(line, sizeof(line), fp)) {
            line[strcspn(line, "\n")] = 0;
            if (strstr(line, feed_url)) {
                found = 1;
                break;
            }
        }

        if (!found) {
            fprintf(fp, "%s\n", feed_url);
        }

    } else {
        // Interactive selection from ~/.rssfeeds
        char lines[20][512];
        int count = 0;

        printf("Select a feed:\n");
        while (fgets(lines[count], sizeof(lines[count]), fp) && count < 20) {
            lines[count][strcspn(lines[count], "\n")] = 0;
            printf("%2d: %s\n", count + 1, lines[count]);
            count++;
        }

        if (count == 0) {
            fprintf(stderr, "No feeds in %s\n", rssfile);
            fclose(fp);
            return 1;
        }

        printf("Enter choice [1-%d]: ", count);
        int choice = 0;
        scanf("%d", &choice);
        if (choice < 1 || choice > count) {
            fprintf(stderr, "Invalid choice.\n");
            fclose(fp);
            return 1;
        }

        // Extract last token as URL
        char *line = lines[choice - 1];
        char *last_space = strrchr(line, ' ');
        if (last_space && *(last_space + 1)) {
            strncpy(feed_url, last_space + 1, sizeof(feed_url) - 1);
        } else {
            strncpy(feed_url, line, sizeof(feed_url) - 1);
        }
    }

    fclose(fp);

    // Fetch and parse
    struct Buffer buf = {0};
    if (!fetch_rss(feed_url, &buf)) {
        fprintf(stderr, "Failed to fetch RSS feed.\n");
        return 1;
    }

    parse_rss(buf.data);
    free(buf.data);

    // UI
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    int ch, selected = 0, offset = 0;

    draw_screen(offset, selected);
    while ((ch = getch()) != 'q') {
        switch (ch) {
            case KEY_DOWN:
                if (selected + 1 < total_items) selected++;
                if (selected >= offset + (SCREEN_ROWS / 3)) offset++;
                break;
            case KEY_UP:
                if (selected > 0) selected--;
                if (selected < offset) offset--;
                break;
            case ' ':
                offset += SCREEN_ROWS;
                if (offset + 1 >= total_items) offset = total_items - 1;
                selected = offset;
                break;
            case '\n': {
                endwin();
                char *url = items[selected].link;
                if (url && strlen(url) > 0) {
                    char command[MAX_LINK + 16];
                    snprintf(command, sizeof(command), "w3m '%s'", url);
                    system(command);
                } else {
                    printf("No URL to open.\n");
                    sleep(2);
                }
                initscr();
                cbreak();
                noecho();
                keypad(stdscr, TRUE);
                draw_screen(offset, selected);
                break;
            }
        }
        draw_screen(offset, selected);
    }

    endwin();
    return 0;
}

