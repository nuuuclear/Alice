#include <stdint.h>

#define ALICE_IMPLEMENTATION
#include "alice.h"

#define WIDTH  1000
#define HEIGHT 1000

uint32_t canvas[WIDTH * HEIGHT];

int main(void) {
    alice_config config = {
        .title      = "Paint",
        .width      = WIDTH,
        .height     = HEIGHT,
        .scale      = 1,
        .resizable  = true,
        .stretch    = false
    };

    alice_window *window = alice_create(&config);
    if (!window) return 1;

    uint32_t background   = 0xFF000000;
    uint32_t paint_colour = 0xFFFFFFFF; // 0xFFFF0780
    uint32_t tool_colour  = 0xFFFFFFFF;

    int brush_x = 0;
    int brush_y = 0;
    int brush_last_x = 0;
    int brush_last_y = 0;

    int brush_radius = 16;
    uint32_t brush_colour = 0xFFFFFFFF;

    float hue = 0;
    bool use_hue = false;

    bool erase = false;

    bool running = true;
    while (running) {
        alice_event event;
        while (alice_poll_event(window, &event)) {
            if (event.type == ALICE_EVENT_QUIT) {
                running = false;
            }
            if (event.type == ALICE_EVENT_MOUSE_WHEEL) {
                brush_radius += event.wheel;
                if (brush_radius < 1) brush_radius = 1;
            }
        }
        if (!running) break;

        hue += 0.2;

        alice_get_mouse(window, &brush_x, &brush_y);

        if (alice_mouse_down(window, ALICE_MOUSE_RIGHT)) {
            erase = true;
            brush_colour = background;
        } else {
            erase = false;
            brush_colour = use_hue ? alice_hue((uint16_t)hue) : paint_colour;
        }

        if (alice_mouse_down(window, ALICE_MOUSE_LEFT)
        ||  erase
        ) {
            int64_t first, last, y;

            first = (int64_t)brush_y - brush_radius;
            last  = (int64_t)brush_y + brush_radius;
            
            for (y = first; y <= last; ++y) {
                uint32_t *row;

                int64_t reach = alice_circle_reach(brush_radius, y - brush_y);
                
                int64_t start_x = brush_x - reach;
                int64_t end_x   = brush_x + reach;

                if (y < 0 || y >= HEIGHT)
                    continue;
                
                row = canvas + (size_t)y * (size_t)WIDTH;
                for (; start_x <= end_x; ++start_x) {
                    if (start_x < 0) continue;
                    
                    row[start_x] = brush_colour;
                }
            }

            // TODO: draw full stroke path (the gaps between the points)
        }

        if (alice_key_down(window, 'R')) {
            use_hue = !use_hue;
        }

        // drawing
        alice_clear(window, background);

        alice_blit(window, 0, 0, canvas, WIDTH, HEIGHT, 0);

        uint32_t under = alice_get_pixel(window, brush_x, brush_y);
        tool_colour = alice_invert(under);

        alice_draw_circle_outline(window, brush_x, brush_y, brush_radius, tool_colour);

        // stroke path
        alice_draw_line(window, brush_last_x, brush_last_y, brush_x, brush_y, tool_colour);

        brush_last_x = brush_x;
        brush_last_y = brush_y;

        alice_present(window);

        alice_sleep(10);
    }
    
    alice_destroy(window);
    return 0;
}