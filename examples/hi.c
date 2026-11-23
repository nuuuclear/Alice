#include <stdint.h>
#include <math.h>

#define ALICE_IMPLEMENTATION
#include "alice.h"

#define ALICE_TEXT_IMPLEMENTATION
#include "alice_text.h"

// build on windows (minGW)
// gcc examples/hi.c -O2 -flto -I. -luser32 -lgdi32 -mwindows -o hi.exe

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

void draw_wavey_string(
    alice_window *window,
    int x, int y,
    const char *text,
    const alice_text_style *style
);

const char* strings[] = {
    "Hi, I'm Alice!",
    "はい、アリスです！"
};

int main(void) {
    alice_config config = {
        .title = "Hi, I'm Alice!",
        .width      = 120,
        .height     = 40,
        .scale      = 2,
        .resizable  = true,
        .stretch    = false
    };

    alice_window *window = alice_create(&config);
    if (!window) return 1;

    alice_text_style style;
    alice_text_style_init(&style);
    style.color = 0xFFFFFF;
    style.font = alice_font_builtin();

    float hue = 0;

    bool running = true;
    while (running) {
        alice_event event;
        while (alice_poll_event(window, &event)) {
            if (event.type == ALICE_EVENT_QUIT) {
                running = false;
            }
        }

        if (!running) break;

        hue += 0.2;

        // drawing
        alice_clear(window, alice_darken(alice_hue((uint16_t)hue), 0.5));

        uint16_t current_y = 10;
        for (size_t i = 0; i < ARRAY_SIZE(strings); i++) {
            int width = 0;
            alice_measure_text(strings[i], &style, &width, 0);
            int x_pos = (config.width / 2) - (width / 2);

            style.color = 0x000000;
            draw_wavey_string(window, x_pos, current_y + 1, strings[i], &style);

            style.color = 0xFFFFFF;
            draw_wavey_string(window, x_pos, current_y, strings[i], &style);

            current_y += 10;
        }

        alice_present(window);

        alice_sleep(10);
    }
    
    alice_destroy(window);
    return 0;
}

int utf8_char_len(unsigned char c) {
    if ((c & 0x80) == 0x00) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}

void draw_wavey_string(
    alice_window *window,
    int x, int y,
    const char *text,
    const alice_text_style *style
) {
    float amp   = 2.0f;
    float speed = 6.0f;

    int cursor_x = 0;
    int char_index = 0;

    for (int i = 0; text[i] != '\0'; ) {
        int len = utf8_char_len((unsigned char)text[i]);

        char character[5] = {0}; 
        for (int b = 0; b < len && text[i + b] != '\0'; b++) {
            character[b] = text[i + b];
        }

        float offset_y = sinf((char_index * speed) + (alice_time() / 600.0f)) * amp;

        alice_draw_text(window, x + cursor_x, y + (int)roundf(offset_y), character, style);

        int width = 0;
        alice_measure_text(character, style, &width, 0);
        cursor_x += width;

        i += len;
        char_index++;
    }
}