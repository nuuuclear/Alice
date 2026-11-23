/* alice.h - a small library for your input and graphical needs

In exactly ONE source file:
    #define ALICE_IMPLEMENTATION
    #include "alice.h"

Backends (automatically selected for you):
    Windows (Win32/GDI)
        link -luser32 -lgdi32 -lshell32
    Linux (BSD X11)
        link -lX11
    anything else (SDL2)
        #define ALICE_USE_SDL2

        link SDL2
        
        This is here to support whatever sdl2 supports (mostly...)
        Use this for Mac and Wayland
    headless
        #define ALICE_NO_BACKEND

        Has no window, you can still render to the framebuffer and inject inputs
        Usefull for testing. also this is good for implementing custom backends

Optional macros (before the implementation include):
    ALICE_STATIC
        make all functions static
    ALICE_MALLOC/CALLOC/FREE
        override the allocator
    ALICE_X11_SHM
        For X11, send frames through shared memory (MIT-SHM) instead of the X socket
        This should perform faster!. with automatic fallback for remote displays

        link -lXext
    ALICE_WAIT_SPIN_NS
        alice_wait_until() sleeps for most of the wait and busy waits
        for the last stretch to wake up on time.
        length in nanoseconds (default 2000000 or 2ms)
*/

#if defined(ALICE_IMPLEMENTATION) && defined(__STRICT_ANSI__) \
    && !defined(_WIN32) && !defined(__APPLE__) \
    && !defined(_POSIX_C_SOURCE) && !defined(_GNU_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#ifndef ALICE_H
#define ALICE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef ALICE_API
    #ifdef ALICE_STATIC
        #define ALICE_API static
    #else
        #define ALICE_API extern
    #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////
//   Types   ///////////////////////////////////////
////////////////////////////////////////////

typedef struct alice_window alice_window;

typedef struct alice_config {
    const char *title;      // a window title in UTF-8
    int width;              // framebuffer width in pixels
    int height;             // framebuffer height in pixels
    int scale;              // initial window size multiplier (of the framebuffer's size)
    bool resizable;
    bool stretch;           // fill the whole window with a non-integer stretch
                            // (uneven pixel widths) instead of integer scaling
} alice_config;

typedef struct alice_framebuffer {
    uint32_t *pixels;       // 0x00RRGGBB, row[y] major, starts in top row
    int width;
    int height;
    int pitch;              // bytes per row
} alice_framebuffer;

enum {
    ALICE_KEY_UNKNOWN = 0,
    // 1 to 255 are printable keys with ASCII values (uppercase letters)
    ALICE_KEY_ESCAPE = 256,
    ALICE_KEY_ENTER,
    ALICE_KEY_TAB,
    ALICE_KEY_BACKSPACE,
    ALICE_KEY_INSERT,
    ALICE_KEY_DELETE,
    ALICE_KEY_RIGHT,
    ALICE_KEY_LEFT,
    ALICE_KEY_DOWN,
    ALICE_KEY_UP,
    ALICE_KEY_PAGE_UP,
    ALICE_KEY_PAGE_DOWN,
    ALICE_KEY_HOME,
    ALICE_KEY_END,
    ALICE_KEY_CAPS_LOCK,
    ALICE_KEY_SCROLL_LOCK,
    ALICE_KEY_NUM_LOCK,
    ALICE_KEY_PRINT_SCREEN,
    ALICE_KEY_PAUSE,
    ALICE_KEY_F1,
    ALICE_KEY_F2,
    ALICE_KEY_F3,
    ALICE_KEY_F4,
    ALICE_KEY_F5,
    ALICE_KEY_F6,
    ALICE_KEY_F7,
    ALICE_KEY_F8,
    ALICE_KEY_F9,
    ALICE_KEY_F10,
    ALICE_KEY_F11,
    ALICE_KEY_F12,
    ALICE_KEY_LEFT_SHIFT,
    ALICE_KEY_RIGHT_SHIFT,
    ALICE_KEY_LEFT_CTRL,
    ALICE_KEY_RIGHT_CTRL,
    ALICE_KEY_LEFT_ALT,
    ALICE_KEY_RIGHT_ALT,
    ALICE_KEY_LEFT_SUPER,
    ALICE_KEY_RIGHT_SUPER,
    ALICE_KEY_COUNT = 512
};

enum {
    ALICE_MOUSE_LEFT = 1,
    ALICE_MOUSE_MIDDLE,
    ALICE_MOUSE_RIGHT,
    ALICE_MOUSE_X1,
    ALICE_MOUSE_X2,
    ALICE_MOUSE_BUTTON_COUNT = 8
};

typedef enum alice_event_type {
    ALICE_EVENT_NONE = 0,
    ALICE_EVENT_QUIT,
    ALICE_EVENT_RESIZE,
    ALICE_EVENT_KEY_DOWN,
    ALICE_EVENT_KEY_UP,
    ALICE_EVENT_MOUSE_MOVE,
    ALICE_EVENT_MOUSE_DOWN,
    ALICE_EVENT_MOUSE_UP,
    ALICE_EVENT_MOUSE_WHEEL,
    ALICE_EVENT_DROP_FILE
} alice_event_type;

typedef struct alice_event {
    alice_event_type type;
    int key;
    int x;
    int y;
    int button;
    int wheel;
    bool repeat;
    int width;
    int height;
    const char *path; // UTF-8 path string, dnd dropped file
} alice_event;


//// window

// create and show a window. returns NULL upon failure
ALICE_API alice_window *alice_create(const alice_config *config);
ALICE_API void alice_destroy(alice_window *window);

//// event

ALICE_API bool alice_poll_event(alice_window *window, alice_event *event);
ALICE_API void alice_push_event(alice_window *window, const alice_event *event);

ALICE_API bool alice_key_down(const alice_window *window, int key);
ALICE_API bool alice_mouse_down(const alice_window *window, int button);
ALICE_API void alice_get_mouse(const alice_window *window, int *x, int *y);

//// rendering

ALICE_API alice_framebuffer *alice_get_framebuffer(alice_window *window);
ALICE_API void alice_present(alice_window *window);

ALICE_API void alice_clear(alice_window *window, uint32_t color);
ALICE_API void alice_draw_point(alice_window *window, int x, int y, uint32_t color);
ALICE_API void alice_draw_line(alice_window *window, int start_x, int start_y, int end_x, int end_y, uint32_t color);
ALICE_API void alice_draw_rect(alice_window *window, int x, int y, int width, int height, uint32_t color);

ALICE_API void alice_draw_rect_outline(alice_window *window, int x, int y, int width, int height, uint32_t color);
ALICE_API void alice_draw_circle(alice_window *window, int center_x, int center_y, int radius, uint32_t color);
ALICE_API void alice_draw_circle_outline(alice_window *window, int center_x, int center_y, int radius, uint32_t color);

// gets the argb value of the pixel at (x, y). this will return 0x00000000 outside of the framebuffer
ALICE_API uint32_t alice_get_pixel(const alice_window *window, int x, int y);

// copy an area of pixels onto the framebuffer
ALICE_API void alice_blit(alice_window *window, int x, int y, const uint32_t *pixels, int width, int height, int pitch);
ALICE_API void alice_blit_keyed(alice_window *window, int x, int y, const uint32_t *pixels, int width, int height, int pitch, uint32_t key);

// save the framebuffer as BMP a file, returns false upon failure
ALICE_API bool alice_save_bmp(const alice_window *window, const char *path);

//// colour

// for constants (like in static tables, etc..)
#define ALICE_RGB(r, g, b) ((((uint32_t)(r) & 0xFFu) << 16) | (((uint32_t)(g) & 0xFFu) << 8) | ((uint32_t)(b) & 0xFFu))

ALICE_API uint32_t alice_rgb(int r, int g, int b);
ALICE_API int alice_color_r(uint32_t color);
ALICE_API int alice_color_g(uint32_t color);
ALICE_API int alice_color_b(uint32_t color);

ALICE_API uint32_t alice_hue(float hue);
ALICE_API uint32_t alice_hsv(float hue, float saturation, float value);
ALICE_API uint32_t alice_hsl(float hue, float saturation, float lightness);

// range 0 to 360, may be NULL
ALICE_API void alice_to_hsv(uint32_t color, float *hue, float *saturation, float *value);
ALICE_API void alice_to_hsl(uint32_t color, float *hue, float *saturation, float *lightness);

// t = 0 gives a, t = 1 gives b
ALICE_API uint32_t alice_mix(uint32_t a, uint32_t b, float t);

ALICE_API uint32_t alice_lighten(uint32_t color, float amount);
ALICE_API uint32_t alice_darken(uint32_t color, float amount);
ALICE_API uint32_t alice_saturate(uint32_t color, float amount);
ALICE_API uint32_t alice_desaturate(uint32_t color, float amount);

ALICE_API uint32_t alice_rotate_hue(uint32_t color, float degrees);
ALICE_API uint32_t alice_invert(uint32_t color);

ALICE_API int alice_luma(uint32_t color);
ALICE_API uint32_t alice_grayscale(uint32_t color);

//// time

// monotonic milliseconds from an arbitrary starting point
ALICE_API uint64_t alice_time(void);

// monotonic nanoseconds from an arbitrary starting point
ALICE_API uint64_t alice_time_ns(void);

ALICE_API void alice_sleep(uint32_t milliseconds);

ALICE_API void alice_wait_until(uint64_t time_ms);
ALICE_API void alice_wait_until_ns(uint64_t time_ns);

ALICE_API bool alice_begin_timer_period(uint32_t milliseconds);
ALICE_API void alice_end_timer_period(void);

typedef struct alice_frame_timer {
    uint64_t next_ns; // when the next frame is due
    uint64_t last_ns; // when alice_frame_wait last returned
    bool started;
} alice_frame_timer;

ALICE_API double alice_frame_wait(alice_frame_timer *timer, double fps);

#ifdef __cplusplus
}
#endif

#endif /* ALICE_H */

#if defined(ALICE_IMPLEMENTATION) && !defined(ALICE_IMPLEMENTATION_ONCE)
#define ALICE_IMPLEMENTATION_ONCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if !defined(_WIN32) && !defined(CLOCK_MONOTONIC)
    #error "alice.h needs POSIX timers: \
        compile with -D_POSIX_C_SOURCE=200809L (or drop -std=c99 strict mode), \
        or include alice.h before other headers."
#endif

#ifndef ALICE_MALLOC
    #define ALICE_MALLOC(size)          malloc(size)
    #define ALICE_CALLOC(count, size)   calloc((count), (size))
    #define ALICE_FREE(ptr)             free(ptr)
#endif

#ifndef ALICE_WAIT_SPIN_NS
    #define ALICE_WAIT_SPIN_NS 2000000u
#endif

#if defined(__GNUC__)
    #define ALICE_UNUSED __attribute__((unused))
#else
    #define ALICE_UNUSED
#endif

#if defined(_MSC_VER)
    #define ALICE_THREAD_LOCAL __declspec(thread)
#else
    #define ALICE_THREAD_LOCAL __thread
#endif

// backend selector
#if defined(ALICE_NO_BACKEND)
    #define ALICE_BACKEND_NONE
#elif defined(ALICE_USE_SDL2)
    #define ALICE_BACKEND_SDL2
#elif defined(_WIN32)
    #define ALICE_BACKEND_WIN32
#elif defined(__APPLE__)
    #error "alice.h has no native macOS backend. #define ALICE_USE_SDL2 (and link SDL2) or ALICE_NO_BACKEND."
#elif defined(__unix__) || defined(__linux__)
    #define ALICE_BACKEND_X11
#else
    #error "alice.h: unsupported platform. #define ALICE_USE_SDL2 or ALICE_NO_BACKEND."
#endif

// clock and sleep functions need this on windows with any backend
#if defined(_WIN32) && !defined(ALICE_BACKEND_WIN32)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#endif

#if defined(ALICE_BACKEND_WIN32)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #include <shellapi.h>
#elif defined(ALICE_BACKEND_X11)
    #include <X11/Xlib.h>
    #include <X11/XKBlib.h>
    #include <X11/Xutil.h>
    #include <X11/Xatom.h>
    #include <X11/keysym.h>
    #if defined(ALICE_X11_SHM)
        #include <X11/extensions/XShm.h>
        #include <sys/ipc.h>
        #include <sys/shm.h>
    #endif
#elif defined(ALICE_BACKEND_SDL2)
    #if defined(__has_include)
        #if __has_include(<SDL2/SDL.h>)
            #include <SDL2/SDL.h>
        #else
            #include <SDL.h>
        #endif
    #else
        #include <SDL.h>
    #endif
#endif

// generic window state

#define ALICE_EVENT_QUEUE_SIZE 512u
#define ALICE_MAX_WINDOW_SIDE  32767

struct alice_window {
    int             width, height;
    int             scale;
    bool            resizable;
    bool            stretch;

    uint32_t       *pixels;
    alice_framebuffer
                    framebuffer;

    int             client_width;
    int             client_height;

    int             view_x, view_y;
    int             view_w, view_h;

    bool            keys[ALICE_KEY_COUNT];
    bool            mouse_buttons[ALICE_MOUSE_BUTTON_COUNT];
    int             mouse_x;
    int             mouse_y;

    alice_event     events[ALICE_EVENT_QUEUE_SIZE];
    unsigned        event_head, event_count;
    bool            quit_queued;
    char           *drop_path;

    // backend specific
#if defined(ALICE_BACKEND_WIN32)
    HWND            hwnd;
    HDC             dc;
    HDC             mem_dc; // staging bitmap
    HBITMAP         dib;
    HGDIOBJ         old_bitmap;
    uint32_t       *dib_pixels;
    int             dib_width, dib_height;
    int            *column_map;
    bool            borders_dirty;
    int             wheel_remainder;
#elif defined(ALICE_BACKEND_X11)
    Display        *display;
    int             screen;
    Window          window;
    GC              gc;
    Atom            wm_protocols, wm_delete;
    bool            detectable_repeat;

    Atom            xdnd_aware, xdnd_enter, xdnd_position, xdnd_status, xdnd_leave;
    Atom            xdnd_drop, xdnd_finished, xdnd_selection, xdnd_type_list;
    Atom            xdnd_action_copy, uri_list, drop_property;
    Window          xdnd_source;
    int             xdnd_version;
    bool            xdnd_accept;
    int             xdnd_x, xdnd_y;

    XImage         *image;
    char           *image_data; // allocated pixels (NULL for shared memory)
    int             image_width, image_height;
    int            *column_map;
    bool            fast_format;
    unsigned long   channel_mask[3];
    int             channel_shift[3];
#if defined(ALICE_X11_SHM)
    XShmSegmentInfo shm;
    bool            use_shm;
    bool            shm_pending;
#endif
#elif defined(ALICE_BACKEND_SDL2)
    SDL_Window     *window;
    SDL_Renderer   *renderer;
    SDL_Texture    *texture;
    uint32_t        window_id;
    bool            video_initialised;
#endif
};

static bool alice_backend_create(alice_window *window, const alice_config *config);
static void alice_backend_destroy(alice_window *window);
static void alice_backend_pump(alice_window *window);
static void alice_backend_present(alice_window *window);

static void alice_update_viewport(alice_window *window);

static char *alice_copy_string(const char *text) {
    size_t length = strlen(text);
    char *copy = (char *)ALICE_MALLOC(length + 1u);
    if (copy)
        memcpy(copy, text, length + 1u);
    return copy;
}

static void alice_event_free_path(alice_event *event) {
    if (event->type == ALICE_EVENT_DROP_FILE && event->path) {
        ALICE_FREE((void *)event->path);
        event->path = NULL;
    }
}

static bool alice_queue_push(alice_window *window, const alice_event *event) {
    unsigned last
    =   (window->event_head + window->event_count + ALICE_EVENT_QUEUE_SIZE - 1u)
    %   ALICE_EVENT_QUEUE_SIZE;

    if (event->type == ALICE_EVENT_MOUSE_MOVE && window->event_count > 0
    &&  window->events[last].type == ALICE_EVENT_MOUSE_MOVE
    ) {
        window->events[last] = *event;
        return true;
    }

    if (window->event_count >= ALICE_EVENT_QUEUE_SIZE) {
        if (event->type == ALICE_EVENT_QUIT) {
            alice_event_free_path(&window->events[last]);
            window->events[last] = *event;
        }
        return false;
    }
    window->events[(window->event_head + window->event_count) % ALICE_EVENT_QUEUE_SIZE] = *event;
    window->event_count++;
    return true;
}

void alice_push_event(alice_window *window, const alice_event *event) {
    if (!window || !event || event->type == ALICE_EVENT_NONE)
        return;

    switch (event->type) {
    case ALICE_EVENT_KEY_DOWN:
    case ALICE_EVENT_KEY_UP:
        if (event->key > 0 && event->key < ALICE_KEY_COUNT)
            window->keys[event->key] = (event->type == ALICE_EVENT_KEY_DOWN);
        break;

    case ALICE_EVENT_MOUSE_MOVE:
        window->mouse_x = event->x;
        window->mouse_y = event->y;
        break;

    case ALICE_EVENT_MOUSE_DOWN:
    case ALICE_EVENT_MOUSE_UP:
        if (event->button > 0 && event->button < ALICE_MOUSE_BUTTON_COUNT) {
            window->mouse_buttons[event->button] = (event->type == ALICE_EVENT_MOUSE_DOWN);
        }
        window->mouse_x = event->x;
        window->mouse_y = event->y;
        break;

    case ALICE_EVENT_RESIZE:
        if (event->width <= 0 || event->height <= 0)
            return;
        if (event->width == window->client_width && event->height == window->client_height)
            return;
        
        window->client_width  = event->width;
        window->client_height = event->height;
        alice_update_viewport(window);
        break;

    case ALICE_EVENT_QUIT:
        if (window->quit_queued)
            return;
        window->quit_queued = true;
        break;

    case ALICE_EVENT_DROP_FILE: {
        // the queue keeps its own copy of the path
        alice_event owned;
        if (!event->path)
            return;
        owned = *event;
        owned.path = alice_copy_string(event->path);
        if (!owned.path)
            return;
        if (!alice_queue_push(window, &owned))
            ALICE_FREE((void *)owned.path);
        return;
    }

    default:
        break;
    }
    alice_queue_push(window, event);
}

bool alice_poll_event(alice_window *window, alice_event *event) {
    if (!window || !event)
        return false;

    ALICE_FREE(window->drop_path);
    window->drop_path = NULL;

    if (window->event_count == 0)
        alice_backend_pump(window);
    if (window->event_count == 0)
        return false;

    *event = window->events[window->event_head];
    window->event_head = (window->event_head + 1u) % ALICE_EVENT_QUEUE_SIZE;
    window->event_count--;
    
    if (event->type == ALICE_EVENT_QUIT)
        window->quit_queued = false;
    else if (event->type == ALICE_EVENT_DROP_FILE)
        window->drop_path = (char *)event->path;

    return true;
}

static void alice_update_viewport(alice_window *window) {
    int kx = window->client_width  / window->width;
    int ky = window->client_height / window->height;
    int scale_factor  = kx < ky ? kx : ky;

    if (window->stretch || scale_factor < 1) {
        window->view_x = 0;
        window->view_y = 0;
        window->view_w = window->client_width;
        window->view_h = window->client_height;
    } else {
        window->view_w = window->width  * scale_factor;
        window->view_h = window->height * scale_factor;
        window->view_x = (window->client_width  - window->view_w) / 2;
        window->view_y = (window->client_height - window->view_h) / 2;
    }
}

// convert a window pixel coordinate to framebuffer coordinate
static ALICE_UNUSED int alice_to_framebuffer(int value, int framebuffer_size, int client_size) {
    int64_t value_count, quotient;

    if (client_size <= 0)
        return 0;

    value_count = (int64_t)value * framebuffer_size;
    quotient = value_count / client_size;
    if (value_count % client_size < 0)
        --quotient;
    return (int)quotient;
}

static ALICE_UNUSED int alice_map_x(const alice_window *window, int pixel_x) {
    return alice_to_framebuffer(pixel_x - window->view_x, window->width, window->view_w);
}

static ALICE_UNUSED int alice_map_y(const alice_window *window, int pixel_y) {
    return alice_to_framebuffer(pixel_y - window->view_y, window->height, window->view_h);
}

static ALICE_UNUSED void alice_fill_scaled(
    const alice_window *window, void *image, size_t pitch,
    int image_width, int image_height, const int *column_map
) {
    int fwidth  = window->width;
    int fheight = window->height;

    int scale_factor = (
        image_width % fwidth == 0
    &&  image_height % fheight == 0
    &&  image_width / fwidth == image_height / fheight
    ) ? image_width / fwidth : 0;

    int y, previous_row = -1;
    const uint32_t *previous_dst = NULL;

    for (y = 0; y < image_height; ++y) {
        int source_y = alice_to_framebuffer(y, fheight, image_height);
        const uint32_t *source_pixels = window->pixels + (size_t)source_y * (size_t)fwidth;
        uint32_t *destination_pixels = (uint32_t *)((char *)image + (size_t)y * pitch);
        int x;

        if (source_y == previous_row) {
            memcpy(destination_pixels, previous_dst, (size_t)image_width * sizeof(uint32_t));
        } else if (scale_factor == 1) {
            memcpy(destination_pixels, source_pixels, (size_t)image_width * sizeof(uint32_t));
        } else if (scale_factor == 2) {
            for (x = 0; x < fwidth; ++x) {
                destination_pixels[2 * x]     = source_pixels[x];
                destination_pixels[2 * x + 1] = source_pixels[x];
            }
        } else if (scale_factor > 2) {
            uint32_t *out = destination_pixels;
            for (x = 0; x < fwidth; ++x) {
                uint32_t pixel_pointer = source_pixels[x];
                int inner_index;
                for (inner_index = 0; inner_index < scale_factor; ++inner_index) {
                    *out++ = pixel_pointer;
                }
            }
        } else {
            for (x = 0; x < image_width; ++x) {
                destination_pixels[x] = source_pixels[column_map[x]];
            }
        }
        previous_dst = destination_pixels;
        previous_row = source_y;
    }
}

static ALICE_UNUSED alice_event alice_make_event(alice_event_type type) {
    alice_event event;
    memset(&event, 0, sizeof(event));
    event.type = type;
    return event;
}

static ALICE_UNUSED void alice_emit_key(alice_window *window, int key, bool down, bool repeat) {
    alice_event event = alice_make_event(down ? ALICE_EVENT_KEY_DOWN : ALICE_EVENT_KEY_UP);
    event.key = key;
    event.repeat = down && repeat;
    alice_push_event(window, &event);
}

static ALICE_UNUSED void alice_emit_mouse_move(alice_window *window, int pixel_x, int pixel_y) {
    alice_event event = alice_make_event(ALICE_EVENT_MOUSE_MOVE);
    event.x = alice_map_x(window, pixel_x);
    event.y = alice_map_y(window, pixel_y);
    alice_push_event(window, &event);
}

static ALICE_UNUSED void alice_emit_button(alice_window *window, int button, bool down, int pixel_x, int pixel_y) {
    alice_event event = alice_make_event(down ? ALICE_EVENT_MOUSE_DOWN : ALICE_EVENT_MOUSE_UP);
    event.button = button;
    event.x = alice_map_x(window, pixel_x);
    event.y = alice_map_y(window, pixel_y);
    alice_push_event(window, &event);
}

static ALICE_UNUSED void alice_emit_wheel(alice_window *window, int delta, int pixel_x, int pixel_y) {
    alice_event event = alice_make_event(ALICE_EVENT_MOUSE_WHEEL);
    event.wheel = delta;
    event.x = alice_map_x(window, pixel_x);
    event.y = alice_map_y(window, pixel_y);
    alice_push_event(window, &event);
}

static ALICE_UNUSED void alice_emit_drop(alice_window *window, const char *path, int pixel_x, int pixel_y) {
    alice_event event = alice_make_event(ALICE_EVENT_DROP_FILE);
    event.path = path;
    event.x = alice_map_x(window, pixel_x);
    event.y = alice_map_y(window, pixel_y);
    alice_push_event(window, &event);
}

static ALICE_UNUSED void alice_emit_resize(alice_window *window, int client_width, int client_height) {
    alice_event event = alice_make_event(ALICE_EVENT_RESIZE);
    event.width = client_width;
    event.height = client_height;
    alice_push_event(window, &event);
}

static ALICE_UNUSED void alice_emit_quit(alice_window *window) {
    alice_event event = alice_make_event(ALICE_EVENT_QUIT);
    alice_push_event(window, &event);
}

static ALICE_UNUSED void alice_release_all(alice_window *window) {
    int index;
    for (index = 1; index < ALICE_KEY_COUNT; ++index) {
        if (window->keys[index]) alice_emit_key(window, index, false, false);
    }
    for (index = 1; index < ALICE_MOUSE_BUTTON_COUNT; ++index) {
        if (window->mouse_buttons[index]) {
            alice_event event = alice_make_event(ALICE_EVENT_MOUSE_UP);
            event.button = index;
            event.x = window->mouse_x;
            event.y = window->mouse_y;
            alice_push_event(window, &event);
        }
    }
}

#if defined(ALICE_BACKEND_NONE)

static bool alice_backend_create(alice_window *window, const alice_config *config) {
    (void)window;
    (void)config;
    return true;
}
static void alice_backend_destroy(alice_window *window)  { (void)window; }
static void alice_backend_pump(alice_window *window)     { (void)window; }
static void alice_backend_present(alice_window *window)  { (void)window; }

#elif defined(ALICE_BACKEND_WIN32)

static const wchar_t alice_win_class_name[] = L"AliceDefaultWindow";
static bool alice_win_class_registered;

static const struct { WORD virtual_key; int key; } alice_win_key_table[] = {
    { VK_ESCAPE,    ALICE_KEY_ESCAPE },     { VK_RETURN,   ALICE_KEY_ENTER },
    { VK_TAB,       ALICE_KEY_TAB },        { VK_BACK,     ALICE_KEY_BACKSPACE },
    { VK_INSERT,    ALICE_KEY_INSERT },     { VK_DELETE,   ALICE_KEY_DELETE },
    { VK_RIGHT,     ALICE_KEY_RIGHT },      { VK_LEFT,     ALICE_KEY_LEFT },
    { VK_DOWN,      ALICE_KEY_DOWN },       { VK_UP,       ALICE_KEY_UP },
    { VK_PRIOR,     ALICE_KEY_PAGE_UP },    { VK_NEXT,     ALICE_KEY_PAGE_DOWN },
    { VK_HOME,      ALICE_KEY_HOME },       { VK_END,      ALICE_KEY_END },
    { VK_CAPITAL,   ALICE_KEY_CAPS_LOCK },  { VK_SCROLL,   ALICE_KEY_SCROLL_LOCK },
    { VK_NUMLOCK,   ALICE_KEY_NUM_LOCK },   { VK_SNAPSHOT, ALICE_KEY_PRINT_SCREEN },
    { VK_PAUSE,     ALICE_KEY_PAUSE },
    { VK_LSHIFT,    ALICE_KEY_LEFT_SHIFT }, { VK_RSHIFT,   ALICE_KEY_RIGHT_SHIFT },
    { VK_LCONTROL,  ALICE_KEY_LEFT_CTRL },  { VK_RCONTROL, ALICE_KEY_RIGHT_CTRL },
    { VK_LMENU,     ALICE_KEY_LEFT_ALT },   { VK_RMENU,    ALICE_KEY_RIGHT_ALT },
    { VK_LWIN,      ALICE_KEY_LEFT_SUPER }, { VK_RWIN,     ALICE_KEY_RIGHT_SUPER },
    { VK_SPACE,     ' ' },
    { VK_OEM_1,     ';' },  { VK_OEM_PLUS,      '=' },  { VK_OEM_COMMA, ',' },
    { VK_OEM_MINUS, '-' },  { VK_OEM_PERIOD,    '.' },  { VK_OEM_2,     '/' },
    { VK_OEM_3,     '`' },  { VK_OEM_4,         '[' },  { VK_OEM_5,     '\\' },
    { VK_OEM_6,     ']' },  { VK_OEM_7,         '\'' }, { VK_OEM_102,   '\\' },
    { VK_MULTIPLY,  '*' },  { VK_ADD,           '+' },  { VK_SUBTRACT,  '-' },
    { VK_DECIMAL,   '.' },  { VK_DIVIDE,        '/' }
};

// translate WM_KEYDOWN/UP virtual key
static int alice_win_key(WPARAM virtual_key, LPARAM message_parameter) {
    unsigned scancode = (unsigned)((message_parameter >> 16) & 0xFF);
    bool extended = (message_parameter & (1L << 24)) != 0;
    size_t index;

    if (virtual_key == VK_SHIFT)
        virtual_key = MapVirtualKeyW(scancode, MAPVK_VSC_TO_VK_EX);
    else if (virtual_key == VK_CONTROL)
        virtual_key = extended ? VK_RCONTROL : VK_LCONTROL;
    else if (virtual_key == VK_MENU)
        virtual_key = extended ? VK_RMENU : VK_LMENU;

    if ((virtual_key >= 'A' && virtual_key <= 'Z')
    ||  (virtual_key >= '0' && virtual_key <= '9')
    )   return (int)virtual_key;

    if (virtual_key >= VK_NUMPAD0 && virtual_key <= VK_NUMPAD9)
        return '0' + (int)(virtual_key - VK_NUMPAD0);
    if (virtual_key >= VK_F1 && virtual_key <= VK_F12)
        return ALICE_KEY_F1 + (int)(virtual_key - VK_F1);

    for (index = 0; index < sizeof(alice_win_key_table) / sizeof(alice_win_key_table[0]); ++index) {
        if (alice_win_key_table[index].virtual_key == virtual_key)
            return alice_win_key_table[index].key;
    }
    
    return ALICE_KEY_UNKNOWN;
}

static int alice_win_button(UINT msg, WPARAM message_word) {
    switch (msg) {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP: 
        return ALICE_MOUSE_LEFT;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        return ALICE_MOUSE_MIDDLE;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
        return ALICE_MOUSE_RIGHT;
    default:
        return GET_XBUTTON_WPARAM(message_word) == XBUTTON1 ? ALICE_MOUSE_X1 : ALICE_MOUSE_X2;
    }
}

static bool alice_win_any_button_down(const alice_window *window) {
    int index;
    for (index = 1; index < ALICE_MOUSE_BUTTON_COUNT; ++index) {
        if (window->mouse_buttons[index])
            return true;
    }
    return false;
}

static void alice_win_free_dib(alice_window *window) {
    if (window->dib) {
        SelectObject(window->mem_dc, window->old_bitmap);
        DeleteObject(window->dib);
        window->dib = NULL;
        window->old_bitmap = NULL;
        window->dib_pixels = NULL;
    }
    ALICE_FREE(window->column_map);
    window->column_map = NULL;
}

static bool alice_win_sync_dib(alice_window *window) {
    BITMAPINFO bmi;
    void *bits = NULL;
    HBITMAP dib;
    int *columns;
    int x;

    if (!window->mem_dc)
        return true;
    if (window->dib
    &&  window->dib_width == window->view_w
    &&  window->dib_height == window->view_h
    )   return true;
    if (window->view_w <= 0 || window->view_h <= 0)
        return false;

    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = window->view_w;
    bmi.bmiHeader.biHeight      = -window->view_h;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    dib = CreateDIBSection(window->mem_dc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
    columns = (int *)ALICE_MALLOC((size_t)window->view_w * sizeof(int));
    if (!dib || !bits || !columns) {
        if (dib)
            DeleteObject(dib);
        ALICE_FREE(columns);
        return false;
    }

    alice_win_free_dib(window);
    window->dib = dib;
    window->old_bitmap = SelectObject(window->mem_dc, dib);
    window->dib_pixels = (uint32_t *)bits;
    window->dib_width = window->view_w;
    window->dib_height = window->view_h;
    window->column_map = columns;
    for (x = 0; x < window->view_w; ++x) {
        columns[x] = alice_to_framebuffer(x, window->width, window->view_w);
    }
    
    window->borders_dirty = true;
    return true;
}

static void alice_win_paint(alice_window *window) {
    int client_width = window->client_width, client_height = window->client_height;

    if (!window->hwnd
    ||  !window->dc
    ||  !window->dib
    ||  client_width <= 0
    ||  client_height <= 0
    ) return;

    // black borders
    if (window->borders_dirty) {
        if (window->view_x > 0) {
            PatBlt(window->dc, 0, 0, window->view_x, client_height, BLACKNESS);
            PatBlt(
                window->dc,
                window->view_x + window->view_w,0,
                client_width - (window->view_x + window->view_w), client_height,
                BLACKNESS
            );
        }
        if (window->view_y > 0) {
            PatBlt(window->dc, 0, 0, client_width, window->view_y, BLACKNESS);
            PatBlt(
                window->dc,
                0, window->view_y + window->view_h, 
                client_width, client_height - (window->view_y + window->view_h),
                BLACKNESS
            );
        }
        window->borders_dirty = false;
    }

    alice_fill_scaled(
        window, window->dib_pixels, (size_t)window->dib_width * sizeof(uint32_t),
        window->dib_width, window->dib_height, window->column_map
    );
    BitBlt(
        window->dc,
        window->view_x, window->view_y,
        window->view_w, window->view_h,
        window->mem_dc, 0, 0, SRCCOPY
    );
    GdiFlush();
}

static LRESULT CALLBACK alice_win_proc(HWND hwnd, UINT msg, WPARAM message_word, LPARAM message_parameter) {
    alice_window *window = (alice_window *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    if (msg == WM_NCCREATE) {
        window = (alice_window *)((CREATESTRUCTW *)message_parameter)->lpCreateParams;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)window);
        if (window)
            window->hwnd = hwnd;
    }
    if (!window)
        return DefWindowProcW(hwnd, msg, message_word, message_parameter);

    switch (msg) {
    case WM_CLOSE:
        alice_emit_quit(window); 
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        window->borders_dirty = true;
        alice_win_paint(window);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        alice_emit_key(
            window,
            alice_win_key(message_word, message_parameter),
            true,
            (message_parameter & (1L << 30)) != 0
        );
        break;

    case WM_KEYUP:
    case WM_SYSKEYUP:
        alice_emit_key(
            window,
            alice_win_key(message_word, message_parameter),
            false,
            false
        );
        break;

    case WM_KILLFOCUS:
        alice_release_all(window);
        break;

    case WM_SIZE:
        if (message_word != SIZE_MINIMIZED) {
            alice_emit_resize(
                window,
                (int)LOWORD(message_parameter),
                (int)HIWORD(message_parameter)
            );
            alice_win_sync_dib(window);
            window->borders_dirty = true;
        }
        break;

    case WM_MOUSEMOVE:
        alice_emit_mouse_move(
            window,
            (int)(short)LOWORD(message_parameter),
            (int)(short)HIWORD(message_parameter)
        );
        break;

    case WM_LBUTTONDOWN: case WM_MBUTTONDOWN: case WM_RBUTTONDOWN: case WM_XBUTTONDOWN:
        SetCapture(hwnd);
        alice_emit_button(
            window,
            alice_win_button(msg, message_word),
            true,
            (int)(short)LOWORD(message_parameter),
            (int)(short)HIWORD(message_parameter)
        );
        return msg == WM_XBUTTONDOWN ? TRUE : 0;

    case WM_LBUTTONUP: case WM_MBUTTONUP: case WM_RBUTTONUP: case WM_XBUTTONUP:
        alice_emit_button(
            window, alice_win_button(msg, message_word),
            false,
            (int)(short)LOWORD(message_parameter),
            (int)(short)HIWORD(message_parameter)
        );
        if (!alice_win_any_button_down(window))
            ReleaseCapture();
        return msg == WM_XBUTTONUP ? TRUE : 0;

    case WM_MOUSEWHEEL: {
        POINT pixel_pointer;
        int notches;
        window->wheel_remainder += GET_WHEEL_DELTA_WPARAM(message_word);
        notches = window->wheel_remainder / WHEEL_DELTA;
        window->wheel_remainder -= notches * WHEEL_DELTA;

        if (notches != 0) {
            pixel_pointer.x = (short)LOWORD(message_parameter);
            pixel_pointer.y = (short)HIWORD(message_parameter);
            ScreenToClient(hwnd, &pixel_pointer);
            alice_emit_wheel(window, notches, pixel_pointer.x, pixel_pointer.y);
        }

        return 0;
    }

    case WM_DROPFILES: {
        HDROP drop = (HDROP)message_word;
        POINT point;
        UINT count, index;

        point.x = point.y = 0;
        DragQueryPoint(drop, &point);
        count = DragQueryFileW(drop, 0xFFFFFFFFu, NULL, 0);
        for (index = 0; index < count; ++index) {
            UINT length = DragQueryFileW(drop, index, NULL, 0);
            wchar_t *wide = (wchar_t *)ALICE_MALLOC(((size_t)length + 1u) * sizeof(wchar_t));
            if (!wide)
                continue;
            if (DragQueryFileW(drop, index, wide, length + 1u)) {
                int size = WideCharToMultiByte(CP_UTF8, 0, wide, -1, NULL, 0, NULL, NULL);
                char *utf8 = size > 0 ? (char *)ALICE_MALLOC((size_t)size) : NULL;
                if (utf8 && WideCharToMultiByte(CP_UTF8, 0, wide, -1, utf8, size, NULL, NULL) > 0)
                    alice_emit_drop(window, utf8, point.x, point.y);
                ALICE_FREE(utf8);
            }
            ALICE_FREE(wide);
        }
        DragFinish(drop);
        return 0;
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_DESTROY:
        window->hwnd = NULL;
        return 0;

    default:
        break;
    }
    return DefWindowProcW(hwnd, msg, message_word, message_parameter);
}

static bool alice_backend_create(alice_window *window, const alice_config *config) {
    HINSTANCE instance = GetModuleHandleW(NULL);
    wchar_t title[256];
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT rect;

    if (!alice_win_class_registered) {
        WNDCLASSEXW wc;
        memset(&wc, 0, sizeof(wc));
        wc.cbSize           = sizeof(wc);
        wc.style            = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc      = alice_win_proc;
        wc.hInstance        = instance;
        wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName    = alice_win_class_name;

        if (!RegisterClassExW(&wc)
        &&  GetLastError() != ERROR_CLASS_ALREADY_EXISTS
        ) return false;

        alice_win_class_registered = true;
    }

    if (!MultiByteToWideChar(
        CP_UTF8, 0, config->title ? config->title : "Alice",
        -1, title, 256)
    ) lstrcpyW(title, L"Alice");

    if (window->resizable) style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
    rect.left = 0;
    rect.top = 0;
    rect.right = window->client_width;
    rect.bottom = window->client_height;
    AdjustWindowRect(&rect, style, FALSE);

    window->hwnd = CreateWindowExW(
        0, alice_win_class_name, title, style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        NULL, NULL, instance, window
    );
    if (!window->hwnd) return false;
    DragAcceptFiles(window->hwnd, TRUE);

    window->dc = GetDC(window->hwnd);
    if (!window->dc) return false;
    window->mem_dc = CreateCompatibleDC(window->dc);
    if (!window->mem_dc) return false;
    window->borders_dirty = true;
    if (!alice_win_sync_dib(window)) return false;

    ShowWindow(window->hwnd, SW_SHOW);
    UpdateWindow(window->hwnd);
    return true;
}

static void alice_backend_destroy(alice_window *window) {
    alice_win_free_dib(window);
    if (window->mem_dc) {
        DeleteDC(window->mem_dc);
        window->mem_dc = NULL;
    }
    if (window->hwnd) {
        if (window->dc)
            ReleaseDC(window->hwnd, window->dc);
        window->dc = NULL;
        SetWindowLongPtrW(window->hwnd, GWLP_USERDATA, 0);
        DestroyWindow(window->hwnd);
        window->hwnd = NULL;
    }
}

static void alice_backend_pump(alice_window *window) {
    MSG msg;
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            alice_emit_quit(window);
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

static void alice_backend_present(alice_window *window) {
    alice_win_paint(window);
}

#elif defined(ALICE_BACKEND_X11)

static const struct { KeySym key_symbol; int key; } alice_x_key_table[] = {
    { XK_Escape,    ALICE_KEY_ESCAPE },     { XK_Return,    ALICE_KEY_ENTER },
    { XK_KP_Enter,  ALICE_KEY_ENTER },      { XK_Tab,       ALICE_KEY_TAB },
    { XK_ISO_Left_Tab, ALICE_KEY_TAB },     { XK_BackSpace, ALICE_KEY_BACKSPACE },
    { XK_Insert,    ALICE_KEY_INSERT },     { XK_Delete,    ALICE_KEY_DELETE },
    { XK_Right,     ALICE_KEY_RIGHT },      { XK_Left,      ALICE_KEY_LEFT },
    { XK_Down,      ALICE_KEY_DOWN },       { XK_Up,        ALICE_KEY_UP },
    { XK_Page_Up,   ALICE_KEY_PAGE_UP },    { XK_Page_Down, ALICE_KEY_PAGE_DOWN },
    { XK_Home,      ALICE_KEY_HOME },       { XK_End,       ALICE_KEY_END },
    { XK_Caps_Lock, ALICE_KEY_CAPS_LOCK },  { XK_Scroll_Lock, ALICE_KEY_SCROLL_LOCK },
    { XK_Num_Lock,  ALICE_KEY_NUM_LOCK },   { XK_Print,     ALICE_KEY_PRINT_SCREEN },
    { XK_Pause,     ALICE_KEY_PAUSE },
    { XK_Shift_L,   ALICE_KEY_LEFT_SHIFT }, { XK_Shift_R,   ALICE_KEY_RIGHT_SHIFT },
    { XK_Control_L, ALICE_KEY_LEFT_CTRL },  { XK_Control_R, ALICE_KEY_RIGHT_CTRL },
    { XK_Alt_L,     ALICE_KEY_LEFT_ALT },   { XK_Meta_L,    ALICE_KEY_LEFT_ALT },
    { XK_Alt_R,     ALICE_KEY_RIGHT_ALT },  { XK_Meta_R,    ALICE_KEY_RIGHT_ALT },
    { XK_ISO_Level3_Shift, ALICE_KEY_RIGHT_ALT },
    { XK_Super_L,   ALICE_KEY_LEFT_SUPER }, { XK_Super_R,   ALICE_KEY_RIGHT_SUPER },
    { XK_KP_Home,   ALICE_KEY_HOME },       { XK_KP_End,    ALICE_KEY_END },
    { XK_KP_Left,   ALICE_KEY_LEFT },       { XK_KP_Right,  ALICE_KEY_RIGHT },
    { XK_KP_Up,     ALICE_KEY_UP },         { XK_KP_Down,   ALICE_KEY_DOWN },
    { XK_KP_Prior,  ALICE_KEY_PAGE_UP },    { XK_KP_Next,   ALICE_KEY_PAGE_DOWN },
    { XK_KP_Insert, ALICE_KEY_INSERT },     { XK_KP_Delete, ALICE_KEY_DELETE },
    { XK_KP_Decimal,    '.' },  { XK_KP_Add,    '+' },  { XK_KP_Subtract,   '-' },
    { XK_KP_Multiply,   '*' },  { XK_KP_Divide, '/' }
};

static int alice_x_key(KeySym key_symbol) {
    size_t index;

    if (key_symbol >= XK_a && key_symbol <= XK_z)
        return (int)(key_symbol - XK_a + 'A');
    if (key_symbol >= XK_space && key_symbol <= XK_asciitilde)
        return (int)key_symbol;
    if (key_symbol >= XK_KP_0 && key_symbol <= XK_KP_9)
        return '0' + (int)(key_symbol - XK_KP_0);
    if (key_symbol >= XK_F1 && key_symbol <= XK_F12)
        return ALICE_KEY_F1 + (int)(key_symbol - XK_F1);

    for (index = 0; index < sizeof(alice_x_key_table) / sizeof(alice_x_key_table[0]); ++index)
        if (alice_x_key_table[index].key_symbol == key_symbol)
            return alice_x_key_table[index].key;
    return ALICE_KEY_UNKNOWN;
}

static KeySym alice_x_keysym(XKeyEvent *key) {
    KeySym key_symbol = XLookupKeysym(key, 0);
    bool keypad_nav = (key_symbol >= XK_KP_Home && key_symbol <= XK_KP_Delete) || key_symbol == XK_KP_Insert;
    if (keypad_nav && (key->state & Mod2Mask))
        key_symbol = XLookupKeysym(key, 1);
    return key_symbol;
}

static int alice_x_button(unsigned button) {
    switch (button) {
    case Button1:   return ALICE_MOUSE_LEFT;
    case Button2:   return ALICE_MOUSE_MIDDLE;
    case Button3:   return ALICE_MOUSE_RIGHT;
    case 8:         return ALICE_MOUSE_X1;
    case 9:         return ALICE_MOUSE_X2;
    default:        return 0;
    }
}

static void alice_x_dnd_reset(alice_window *window) {
    window->xdnd_source = 0;
    window->xdnd_version = 0;
    window->xdnd_accept = false;
}

static void alice_x_dnd_send(alice_window *window, Atom message, long data1, long data2, long data4) {
    XEvent reply;
    memset(&reply, 0, sizeof(reply));
    reply.xclient.type          = ClientMessage;
    reply.xclient.display       = window->display;
    reply.xclient.window        = window->xdnd_source;
    reply.xclient.message_type  = message;
    reply.xclient.format        = 32;
    reply.xclient.data.l[0]     = (long)window->window;
    reply.xclient.data.l[1]     = data1;
    reply.xclient.data.l[2]     = data2;
    reply.xclient.data.l[4]     = data4;
    XSendEvent(window->display, window->xdnd_source, False, NoEventMask, &reply);
    XFlush(window->display);
}

static bool alice_x_dnd_offers_files(alice_window *window, const XClientMessageEvent *message) {
    unsigned long index;

    if (!(message->data.l[1] & 1)) {
        for (index = 2; index < 5; ++index) {
            if ((Atom)message->data.l[index] == window->uri_list)
                return true;
        }
        return false;
    }

    {
        Atom actual_type;
        int actual_format;
        unsigned long count = 0, remaining;
        unsigned char *data = NULL;
        bool found = false;

        if (XGetWindowProperty(
                window->display,(Window)message->data.l[0], window->xdnd_type_list,
                0, 1024, False, XA_ATOM, &actual_type, &actual_format, &count, &remaining, &data
            ) == Success && data && actual_type == XA_ATOM && actual_format == 32
        ) {
            const Atom *types = (const Atom *)data;
            for (index = 0; index < count; ++index) {
                if (types[index] == window->uri_list)
                    found = true;
            }
        }
        if (data)
            XFree(data);
        return found;
    }
}

static int alice_hex_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static void alice_x_dnd_emit_uri(alice_window *window, const char *uri, size_t length) {
    static const char prefix[] = "file://";
    size_t in = 0, out = 0;
    char *path;

    if (length < sizeof(prefix) - 1 || memcmp(uri, prefix, sizeof(prefix) - 1) != 0)
        return;
    uri += sizeof(prefix) - 1;
    length -= sizeof(prefix) - 1;

    // skip the host, which is empty for most local files
    while (in < length && uri[in] != '/')
        ++in;
    if (in >= length)
        return;

    path = (char *)ALICE_MALLOC(length - in + 1u);
    if (!path)
        return;
    while (in < length) {
        int high = -1, low = -1;
        if (uri[in] == '%' && in + 2 < length) {
            high = alice_hex_value(uri[in + 1]);
            low  = alice_hex_value(uri[in + 2]);
        }
        if (high >= 0 && low >= 0) {
            path[out++] = (char)(high * 16 + low);
            in += 3;
        } else {
            path[out++] = uri[in++];
        }
    }
    path[out] = '\0';

    alice_emit_drop(window, path, window->xdnd_x, window->xdnd_y);
    ALICE_FREE(path);
}

static void alice_x_dnd_parse(alice_window *window, const char *text, size_t length) {
    size_t start = 0;
    while (start < length) {
        size_t end = start;
        while (end < length && text[end] != '\r' && text[end] != '\n' && text[end] != '\0')
            ++end;
        if (end > start && text[start] != '#')
            alice_x_dnd_emit_uri(window, text + start, end - start);
        start = end + 1;
    }
}

static void alice_x_dnd_enter(alice_window *window, const XClientMessageEvent *message) {
    int version = (int)(((unsigned long)message->data.l[1] >> 24) & 0xFFu);

    alice_x_dnd_reset(window);
    window->xdnd_source  = (Window)message->data.l[0];
    window->xdnd_version = version > 5 ? 5 : version;
    window->xdnd_accept  = alice_x_dnd_offers_files(window, message);
}

static void alice_x_dnd_position(alice_window *window, const XClientMessageEvent *message) {
    Window child;
    int root_x, root_y, x = 0, y = 0;

    if ((Window)message->data.l[0] != window->xdnd_source)
        return;

    root_x = (int)(((unsigned long)message->data.l[2] >> 16) & 0xFFFFu);
    root_y = (int)((unsigned long)message->data.l[2] & 0xFFFFu);
    XTranslateCoordinates(
        window->display, RootWindow(window->display, window->screen), window->window,
        root_x, root_y, &x, &y, &child
    );
    window->xdnd_x = x;
    window->xdnd_y = y;

    alice_x_dnd_send(
        window, window->xdnd_status,
        window->xdnd_accept ? 1 : 0, 0,
        window->xdnd_accept ? (long)window->xdnd_action_copy : 0
    );
}

static void alice_x_dnd_drop(alice_window *window, const XClientMessageEvent *message) {
    if ((Window)message->data.l[0] != window->xdnd_source)
        return;

    if (window->xdnd_accept) {
        Time stamp = window->xdnd_version >= 1 ? (Time)message->data.l[2] : CurrentTime;
        XConvertSelection(
            window->display, window->xdnd_selection, window->uri_list,
            window->drop_property, window->window, stamp
        );
    } else {
        if (window->xdnd_version >= 2)
            alice_x_dnd_send(window, window->xdnd_finished, 0, 0, 0);
        alice_x_dnd_reset(window);
    }
}

static void alice_x_dnd_selection(alice_window *window, const XSelectionEvent *event) {
    Atom actual_type;
    int actual_format;
    unsigned long count = 0, remaining;
    unsigned char *data = NULL;
    bool success = false;

    if (event->selection != window->xdnd_selection || window->xdnd_source == 0)
        return;

    if (event->property == window->drop_property
    &&  XGetWindowProperty(
            window->display, window->window, window->drop_property,
            0, 65536, True, AnyPropertyType, &actual_type, &actual_format, &count, &remaining, &data
        ) == Success && data && actual_format == 8
    ) {
        alice_x_dnd_parse(window, (const char *)data, (size_t)count);
        success = true;
    }
    if (data)
        XFree(data);

    if (window->xdnd_version >= 2)
        alice_x_dnd_send(
            window, window->xdnd_finished,
            success ? 1 : 0, success ? (long)window->xdnd_action_copy : 0, 0
        );
    alice_x_dnd_reset(window);
}

static int alice_x_lowest_bit(unsigned long mask) {
    int shift = 0;
    while (mask && !(mask & 1ul)) {
        mask >>= 1;
        ++shift;
    }
    return shift;
}

#if defined(ALICE_X11_SHM)

static int alice_x_shm_failed;

static int alice_x_shm_error_handler(Display *display, XErrorEvent *error) {
    (void)display;
    (void)error;
    alice_x_shm_failed = 1;
    return 0;
}

static void alice_x_release_shm(Display *display, XImage *image, XShmSegmentInfo *info) {
    XShmDetach(display, info);
    XSync(display, False);
    shmdt(info->shmaddr);
    image->data = NULL;
    XDestroyImage(image);
}

static XImage *alice_x_create_shm_image(alice_window *window, int width, int height, XShmSegmentInfo *info) {
    XImage *image;
    int (*old_handler)(Display *, XErrorEvent *);

    if (!XShmQueryExtension(window->display))
        return NULL;

    image = XShmCreateImage(
        window->display, DefaultVisual(window->display, window->screen),
        (unsigned)DefaultDepth(window->display, window->screen), ZPixmap, NULL, info,
        (unsigned)width, (unsigned)height
    );
    if (!image)
        return NULL;

    info->shmid = shmget(IPC_PRIVATE, (size_t)image->bytes_per_line * (size_t)height, IPC_CREAT | 0600);
    if (info->shmid < 0) {
        XDestroyImage(image);
        return NULL;
    }
    info->shmaddr = image->data = (char *)shmat(info->shmid, NULL, 0);
    info->readOnly = False;
    if (info->shmaddr == (char *)-1) {
        shmctl(info->shmid, IPC_RMID, NULL);
        image->data = NULL;
        XDestroyImage(image);
        return NULL;
    }

    alice_x_shm_failed = 0;
    old_handler = XSetErrorHandler(alice_x_shm_error_handler);
    XShmAttach(window->display, info);
    XSync(window->display, False);
    XSetErrorHandler(old_handler);

    shmctl(info->shmid, IPC_RMID, NULL);

    if (alice_x_shm_failed) {
        shmdt(info->shmaddr);
        image->data = NULL;
        XDestroyImage(image);
        return NULL;
    }
    return image;
}

#endif

static void alice_x_free_image(alice_window *window) {
    if (window->image) {
#if defined(ALICE_X11_SHM)
        if (window->use_shm) {
            alice_x_release_shm(window->display, window->image, &window->shm);
            window->use_shm = false;
            window->shm_pending = false;
        } else
#endif
        {
            ALICE_FREE(window->image_data);
            window->image->data = NULL;
            XDestroyImage(window->image);
        }
        window->image_data = NULL;
        window->image = NULL;
    }
    ALICE_FREE(window->column_map);
    window->column_map = NULL;
}

static bool alice_x_resize_image(alice_window *window, int width, int height) {
    XImage *image = NULL;
    char *data = NULL;
    int *columns;
    int x;
    bool shared = false;
#if defined(ALICE_X11_SHM)
    XShmSegmentInfo shm;
    memset(&shm, 0, sizeof(shm));
#endif

    if (width <= 0 || height <= 0)
        return false;

#if defined(ALICE_X11_SHM)
    image = alice_x_create_shm_image(window, width, height, &shm);
    shared = image != NULL;
#endif
    if (!image) {
        image = XCreateImage(
            window->display, DefaultVisual(window->display, window->screen),
            (unsigned)DefaultDepth(window->display, window->screen), ZPixmap, 0, NULL,
            (unsigned)width, (unsigned)height, 32, 0
        );
        if (!image)
            return false;
        data = (char *)ALICE_CALLOC((size_t)image->bytes_per_line, (size_t)height);
    }

    columns = (int *)ALICE_MALLOC((size_t)width * sizeof(int));
    if (!columns || (!shared && !data)) {
        ALICE_FREE(columns);
        ALICE_FREE(data);
#if defined(ALICE_X11_SHM)
        if (shared) {
            alice_x_release_shm(window->display, image, &shm);
            return false;
        }
#endif
        XDestroyImage(image);
        return false;
    }
    if (!shared)
        image->data = data;

    alice_x_free_image(window);
    window->image = image;
    window->image_data = data;
    window->column_map = columns;
    window->image_width = width;
    window->image_height = height;
#if defined(ALICE_X11_SHM)
    window->shm = shm;
    window->use_shm = shared;
    if (shared) image->obdata = (char *)&window->shm;
#endif
    for (x = 0; x < width; ++x) {
        columns[x] = alice_to_framebuffer(x, window->width, width);
    }

    return true;
}

static bool alice_x_setup_format(alice_window *window) {
    unsigned long one = 1;
    bool host_little = *(const unsigned char *)&one == 1;
    int index;

    window->channel_mask[0] = window->image->red_mask;
    window->channel_mask[1] = window->image->green_mask;
    window->channel_mask[2] = window->image->blue_mask;
    for (index = 0; index < 3; ++index) {
        if (window->channel_mask[index] == 0) {
            return false;
        }
        window->channel_shift[index] = alice_x_lowest_bit(window->channel_mask[index]);
    }

    window->fast_format
    =   window->image->bits_per_pixel == 32
    &&  window->channel_mask[0] == 0xFF0000ul
    &&  window->channel_mask[1] == 0x00FF00ul
    &&  window->channel_mask[2] == 0x0000FFul
    &&  (window->image->byte_order == LSBFirst) == host_little;
    return true;
}

static unsigned long alice_x_pack(const alice_window *window, uint32_t color) {
    unsigned long value = 0;
    int index;
    for (index = 0; index < 3; ++index) {
        unsigned long c8 = (color >> (16 - 8 * index)) & 0xFFu;
        unsigned long max = window->channel_mask[index] >> window->channel_shift[index];
        value |= (c8 * max / 255ul) << window->channel_shift[index];
    }

    return value;
}

static void alice_x_fill_fast(alice_window *window) {
    alice_fill_scaled(
        window, window->image->data, (size_t)window->image->bytes_per_line,
        window->image_width, window->image_height, window->column_map
    );
}

static void alice_x_fill_slow(alice_window *window) {
    XImage *img = window->image;

    int y;
    for (y = 0; y < window->image_height; ++y) {
        int source_y = alice_to_framebuffer(y, window->height, window->image_height);
        const uint32_t *source_pixels = window->pixels + (size_t)source_y * (size_t)window->width;

        int x;
        for (x = 0; x < window->image_width; ++x) {
            XPutPixel(img, x, y, alice_x_pack(window, source_pixels[window->column_map[x]]));
        }
    }
}

static bool alice_x_uses_shm(const alice_window *window) {
#if defined(ALICE_X11_SHM)
    return window->use_shm;
#else
    (void)window;
    return false;
#endif
}

static void alice_x_present(alice_window *window) {
    XImage *img = window->image;
    bool direct = false;

    if (!window->display || !window->window || !img)
        return;

#if defined(ALICE_X11_SHM)
    if (window->shm_pending) {
        XSync(window->display, False);
        window->shm_pending = false;
    }
#endif

    if (!window->fast_format) {
        alice_x_fill_slow(window);
    } else if (!alice_x_uses_shm(window)
           &&  window->image_width == window->width && window->image_height == window->height
           &&  img->bytes_per_line == window->framebuffer.pitch
    ) {
        img->data = (char *)window->pixels;
        direct = true;
    } else {
        alice_x_fill_fast(window);
    }

#if defined(ALICE_X11_SHM)
    if (window->use_shm) {
        XShmPutImage(
            window->display, window->window, window->gc, img, 0, 0, window->view_x, window->view_y,
            (unsigned)window->image_width, (unsigned)window->image_height, False
        );
        window->shm_pending = true;
    } else
#endif
    {
        XPutImage(
            window->display, window->window, window->gc, img, 0, 0, window->view_x, window->view_y,
            (unsigned)window->image_width, (unsigned)window->image_height
        );
    }
    if (direct)
        img->data = window->image_data;
    XFlush(window->display);
}

static void alice_backend_destroy(alice_window *window) {
    alice_x_free_image(window);
    if (window->display) {
        if (window->gc)
            XFreeGC(window->display, window->gc);
        if (window->window)
            XDestroyWindow(window->display, window->window);
        XCloseDisplay(window->display);
    }
    window->gc = NULL;
    window->window = 0;
    window->display = NULL;
}

static bool alice_backend_create(alice_window *window, const alice_config *config) {
    unsigned width = (unsigned)window->client_width, height = (unsigned)window->client_height;
    const char *title = config->title ? config->title : "Alice";
    Bool supported = False;
    long events
    =   ExposureMask
    |   KeyPressMask
    |   KeyReleaseMask
    |   ButtonPressMask
    |   ButtonReleaseMask
    |   PointerMotionMask
    |   StructureNotifyMask
    |   FocusChangeMask;

    window->display = XOpenDisplay(NULL);
    if (!window->display)
        return false;
    window->screen = DefaultScreen(window->display);

    window->window = XCreateSimpleWindow(
        window->display, RootWindow(window->display, window->screen),
        0, 0, width, height, 0,
        BlackPixel(window->display, window->screen),
        BlackPixel(window->display, window->screen)
    );
    if (!window->window) return false;

    Xutf8SetWMProperties(window->display, window->window, title, title, NULL, 0, NULL, NULL, NULL);
    XSelectInput(window->display, window->window, events);

    window->wm_protocols = XInternAtom(window->display, "WM_PROTOCOLS", False);
    window->wm_delete = XInternAtom(window->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(window->display, window->window, &window->wm_delete, 1);

    window->xdnd_aware       = XInternAtom(window->display, "XdndAware", False);
    window->xdnd_enter       = XInternAtom(window->display, "XdndEnter", False);
    window->xdnd_position    = XInternAtom(window->display, "XdndPosition", False);
    window->xdnd_status      = XInternAtom(window->display, "XdndStatus", False);
    window->xdnd_leave       = XInternAtom(window->display, "XdndLeave", False);
    window->xdnd_drop        = XInternAtom(window->display, "XdndDrop", False);
    window->xdnd_finished    = XInternAtom(window->display, "XdndFinished", False);
    window->xdnd_selection   = XInternAtom(window->display, "XdndSelection", False);
    window->xdnd_type_list   = XInternAtom(window->display, "XdndTypeList", False);
    window->xdnd_action_copy = XInternAtom(window->display, "XdndActionCopy", False);
    window->uri_list         = XInternAtom(window->display, "text/uri-list", False);
    window->drop_property    = XInternAtom(window->display, "ALICE_DROP", False);
    {
        Atom xdnd_version = 5;
        XChangeProperty(
            window->display, window->window, window->xdnd_aware, XA_ATOM, 32,
            PropModeReplace, (unsigned char *)&xdnd_version, 1
        );
    }

    if (!window->resizable) {
        XSizeHints hints;
        memset(&hints, 0, sizeof(hints));
        hints.flags = PMinSize | PMaxSize;
        hints.min_width = hints.max_width = (int)width;
        hints.min_height = hints.max_height = (int)height;
        XSetWMNormalHints(window->display, window->window, &hints);
    }

    window->detectable_repeat = XkbSetDetectableAutoRepeat(window->display, True, &supported) && supported;

    window->gc = XCreateGC(window->display, window->window, 0, NULL);
    if (!window->gc || !alice_x_resize_image(window, window->view_w, window->view_h) || !alice_x_setup_format(window))
        return false;

    XMapWindow(window->display, window->window);
    XFlush(window->display);
    return true;
}

static void alice_backend_pump(alice_window *window) {
    while (window->display && XPending(window->display) > 0) {
        XEvent event_data;
        XNextEvent(window->display, &event_data);

        switch (event_data.type) {
        case ClientMessage: {
            Atom message = event_data.xclient.message_type;

            if (message == window->wm_protocols) {
                if ((Atom)event_data.xclient.data.l[0] == window->wm_delete)
                    alice_emit_quit(window);
            } else if (message == window->xdnd_enter) {
                alice_x_dnd_enter(window, &event_data.xclient);
            } else if (message == window->xdnd_position) {
                alice_x_dnd_position(window, &event_data.xclient);
            } else if (message == window->xdnd_leave) {
                alice_x_dnd_reset(window);
            } else if (message == window->xdnd_drop) {
                alice_x_dnd_drop(window, &event_data.xclient);
            }
            break;
        }

        case SelectionNotify:
            alice_x_dnd_selection(window, &event_data.xselection);
            break;

        case DestroyNotify:
            window->window = 0;
            alice_emit_quit(window);
            break;

        case Expose:
            if (event_data.xexpose.count == 0)
                alice_x_present(window);
            break;

        case ConfigureNotify: {
            bool size_changed
            =   event_data.xconfigure.width  != window->client_width
            ||  event_data.xconfigure.height != window->client_height;

            alice_emit_resize(window, event_data.xconfigure.width, event_data.xconfigure.height);
            if (window->view_w != window->image_width || window->view_h != window->image_height)
                alice_x_resize_image(window, window->view_w, window->view_h);
            if (size_changed)
                XClearWindow(window->display, window->window); // repaint the borders black
            break;
        }

        case KeyPress: {
            int key = alice_x_key(alice_x_keysym(&event_data.xkey));
            bool repeat = key > 0 && key < ALICE_KEY_COUNT && window->keys[key];
            alice_emit_key(window, key, true, repeat);
            break;
        }

        case KeyRelease: {
            int key = alice_x_key(alice_x_keysym(&event_data.xkey));

            if (!window->detectable_repeat && XPending(window->display) > 0) {
                XEvent next;
                XPeekEvent(window->display, &next);
                if (next.type == KeyPress
                &&  next.xkey.time == event_data.xkey.time
                &&  next.xkey.keycode == event_data.xkey.keycode
                ) {
                    XNextEvent(window->display, &next);
                    alice_emit_key(window, key, true, true);
                    break;
                }
            }
            alice_emit_key(window, key, false, false);
            break;
        }

        case MotionNotify:
            alice_emit_mouse_move(window, event_data.xmotion.x, event_data.xmotion.y);
            break;

        case ButtonPress:
            if (event_data.xbutton.button == Button4 || event_data.xbutton.button == Button5)
                alice_emit_wheel(
                    window, event_data.xbutton.button == Button4 ? 1 : -1,
                    event_data.xbutton.x, event_data.xbutton.y
                );
            else if (alice_x_button(event_data.xbutton.button))
                alice_emit_button(
                    window, alice_x_button(event_data.xbutton.button), true,
                    event_data.xbutton.x, event_data.xbutton.y
                );
            break;

        case ButtonRelease:
            if (alice_x_button(event_data.xbutton.button))
                alice_emit_button(
                    window, alice_x_button(event_data.xbutton.button), false,
                    event_data.xbutton.x, event_data.xbutton.y
                );
            break;

        case FocusOut:
            alice_release_all(window);
            break;

        default:
            break;
        }
    }
}

static void alice_backend_present(alice_window *window) {
    alice_x_present(window);
}

#elif defined(ALICE_BACKEND_SDL2)

static const struct { SDL_Keycode key_symbol; int key; } alice_sdl_key_table[] = {
    { SDLK_ESCAPE,      ALICE_KEY_ESCAPE },         { SDLK_RETURN,      ALICE_KEY_ENTER },
    { SDLK_KP_ENTER,    ALICE_KEY_ENTER },          { SDLK_TAB,         ALICE_KEY_TAB },
    { SDLK_BACKSPACE,   ALICE_KEY_BACKSPACE },      { SDLK_INSERT,      ALICE_KEY_INSERT },
    { SDLK_DELETE,      ALICE_KEY_DELETE },         { SDLK_RIGHT,       ALICE_KEY_RIGHT },
    { SDLK_LEFT,        ALICE_KEY_LEFT },           { SDLK_DOWN,        ALICE_KEY_DOWN },
    { SDLK_UP,          ALICE_KEY_UP },             { SDLK_PAGEUP,      ALICE_KEY_PAGE_UP },
    { SDLK_PAGEDOWN,    ALICE_KEY_PAGE_DOWN },      { SDLK_HOME,        ALICE_KEY_HOME },
    { SDLK_END,         ALICE_KEY_END },            { SDLK_CAPSLOCK,    ALICE_KEY_CAPS_LOCK },
    { SDLK_SCROLLLOCK,  ALICE_KEY_SCROLL_LOCK },    { SDLK_NUMLOCKCLEAR,ALICE_KEY_NUM_LOCK },
    { SDLK_PRINTSCREEN, ALICE_KEY_PRINT_SCREEN },   { SDLK_PAUSE,       ALICE_KEY_PAUSE },
    { SDLK_LSHIFT,      ALICE_KEY_LEFT_SHIFT },     { SDLK_RSHIFT,      ALICE_KEY_RIGHT_SHIFT },
    { SDLK_LCTRL,       ALICE_KEY_LEFT_CTRL },      { SDLK_RCTRL,       ALICE_KEY_RIGHT_CTRL },
    { SDLK_LALT,        ALICE_KEY_LEFT_ALT },       { SDLK_RALT,        ALICE_KEY_RIGHT_ALT },
    { SDLK_LGUI,        ALICE_KEY_LEFT_SUPER },     { SDLK_RGUI,        ALICE_KEY_RIGHT_SUPER },
    { SDLK_KP_0, '0' },     { SDLK_KP_PERIOD,   '.' },  { SDLK_KP_PLUS,     '+' },
    { SDLK_KP_MINUS, '-' }, { SDLK_KP_MULTIPLY, '*' },  { SDLK_KP_DIVIDE,   '/' }
};

static int alice_sdl_key(SDL_Keycode key_symbol) {
    size_t index;

    if (key_symbol >= 'a' && key_symbol <= 'z')
        return (int)(key_symbol - 'a' + 'A');
    if (key_symbol >= ' ' && key_symbol <= '~')
        return (int)key_symbol;
    if (key_symbol >= SDLK_F1 && key_symbol <= SDLK_F12)
        return ALICE_KEY_F1 + (int)(key_symbol - SDLK_F1);
    if (key_symbol >= SDLK_KP_1 && key_symbol <= SDLK_KP_9)
        return '1' + (int)(key_symbol - SDLK_KP_1);

    for (index = 0; index < sizeof(alice_sdl_key_table) / sizeof(alice_sdl_key_table[0]); ++index)
        if (alice_sdl_key_table[index].key_symbol == key_symbol)
            return alice_sdl_key_table[index].key;
    return ALICE_KEY_UNKNOWN;
}

static void alice_backend_destroy(alice_window *window) {
    if (window->texture)
        SDL_DestroyTexture(window->texture);
    if (window->renderer)
        SDL_DestroyRenderer(window->renderer);
    if (window->window)
        SDL_DestroyWindow(window->window);
    if (window->video_initialised)
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    window->texture = NULL;
    window->renderer = NULL;
    window->window = NULL;
    window->video_initialised = false;
}

static bool alice_backend_create(alice_window *window, const alice_config *config) {
    Uint32 flags = SDL_WINDOW_SHOWN | (window->resizable ? SDL_WINDOW_RESIZABLE : 0);
    int client_width, client_height;

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
        return false;
    window->video_initialised = true;

    window->window = SDL_CreateWindow(
        config->title ? config->title : "Alice",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        window->client_width, window->client_height, flags
    );
    if (!window->window) return false;

    window->window_id = SDL_GetWindowID(window->window);
    window->renderer = SDL_CreateRenderer(window->window, -1, 0);
    if (!window->renderer) return false;

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    window->texture = SDL_CreateTexture(
        window->renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, window->width, window->height
    );
    if (!window->texture) return false;

    SDL_GetWindowSize(window->window, &client_width, &client_height);
    window->client_width = client_width;
    window->client_height = client_height;
    alice_update_viewport(window);
    return true;
}

static void alice_backend_present(alice_window *window) {
    if (!window->texture)
        return;
    SDL_UpdateTexture(window->texture, NULL, window->pixels, window->framebuffer.pitch);
    SDL_RenderClear(window->renderer);
    {
        SDL_Rect destination_pixels;
        destination_pixels.x = window->view_x;
        destination_pixels.y = window->view_y;
        destination_pixels.w = window->view_w;
        destination_pixels.h = window->view_h;
        SDL_RenderCopy(window->renderer, window->texture, NULL, &destination_pixels);
    }
    SDL_RenderPresent(window->renderer);
}

static void alice_backend_pump(alice_window *window) {
    SDL_Event event_data;
    while (SDL_PollEvent(&event_data)) {
        switch (event_data.type) {
        case SDL_QUIT:
            alice_emit_quit(window);
            break;

        case SDL_WINDOWEVENT:
            if (event_data.window.windowID != window->window_id)
                break;
            if (event_data.window.event == SDL_WINDOWEVENT_CLOSE)
                alice_emit_quit(window);
            else if (event_data.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                alice_emit_resize(window, event_data.window.data1, event_data.window.data2);
            else if (event_data.window.event == SDL_WINDOWEVENT_EXPOSED)
                alice_backend_present(window);
            else if (event_data.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                alice_release_all(window);
            break;

        case SDL_KEYDOWN:
            alice_emit_key(window, alice_sdl_key(event_data.key.keysym.sym), true, event_data.key.repeat != 0);
            break;

        case SDL_KEYUP:
            alice_emit_key(window, alice_sdl_key(event_data.key.keysym.sym), false, false);
            break;

        case SDL_MOUSEMOTION:
            alice_emit_mouse_move(window, event_data.motion.x, event_data.motion.y);
            break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            if (event_data.button.button >= 1 && event_data.button.button <= ALICE_MOUSE_X2)
                alice_emit_button(
                    window, (int)event_data.button.button, event_data.type == SDL_MOUSEBUTTONDOWN,
                    event_data.button.x, event_data.button.y
                );
            break;

        case SDL_DROPFILE:
            if (event_data.drop.file) {
                if (event_data.drop.windowID == window->window_id) {
                    int mx, my;
                    SDL_GetMouseState(&mx, &my);
                    alice_emit_drop(window, event_data.drop.file, mx, my);
                }
                SDL_free(event_data.drop.file);
            }
            break;

        case SDL_MOUSEWHEEL: {
            int mx, my;
            SDL_GetMouseState(&mx, &my);
            if (event_data.wheel.y != 0)
                alice_emit_wheel(window, event_data.wheel.y, mx, my);
            break;
        }

        default:
            break;
        }
    }
}

#endif /* backends */

////////////////////////////////////////////////////////////
//   Window   //////////////////////////////////////
////////////////////////////////////////////

alice_window *alice_create(const alice_config *config) {
    alice_window *window;
    int scale;

    if (!config
    ||  config->width <= 0
    ||  config->height <= 0
    ) return NULL;

    scale = config->scale > 0 ? config->scale : 1;
    if ((int64_t)config->width * scale > ALICE_MAX_WINDOW_SIDE
    ||  (int64_t)config->height * scale > ALICE_MAX_WINDOW_SIDE
    ) return NULL;

    window = (alice_window *)ALICE_CALLOC(1, sizeof(*window));
    if (!window) return NULL;
    window->width         = config->width;
    window->height        = config->height;
    window->scale         = scale;
    window->resizable     = config->resizable;
    window->client_width  = config->width * scale;
    window->client_height = config->height * scale;
    window->stretch       = config->stretch;
    alice_update_viewport(window);

    window->pixels = (uint32_t *)ALICE_CALLOC((size_t)window->width * (size_t)window->height, sizeof(uint32_t));
    if (!window->pixels) {
        ALICE_FREE(window);
        return NULL;
    }
    window->framebuffer.pixels = window->pixels;
    window->framebuffer.width  = window->width;
    window->framebuffer.height = window->height;
    window->framebuffer.pitch  = window->width * (int)sizeof(uint32_t);

    if (!alice_backend_create(window, config)) {
        alice_backend_destroy(window);
        ALICE_FREE(window->pixels);
        ALICE_FREE(window);
        return NULL;
    }
    return window;
}

void alice_destroy(alice_window *window) {
    unsigned index;
    if (!window) return;
    for (index = 0; index < window->event_count; ++index)
        alice_event_free_path(&window->events[(window->event_head + index) % ALICE_EVENT_QUEUE_SIZE]);
    ALICE_FREE(window->drop_path);
    alice_backend_destroy(window);
    ALICE_FREE(window->pixels);
    ALICE_FREE(window);
}

void alice_present(alice_window *window) {
    if (window) alice_backend_present(window);
}

////////////////////////////////////////////////////////////
//   Input   ///////////////////////////////////////
////////////////////////////////////////////

bool alice_key_down(const alice_window *window, int key) {
    if (!window) return false;

    if (key >= 'a' && key <= 'z')
        key -= 'a' - 'A';
    return key > 0
        && key < ALICE_KEY_COUNT
        && window->keys[key];
}

bool alice_mouse_down(const alice_window *window, int button) {
    return window
        && button > 0
        && button < ALICE_MOUSE_BUTTON_COUNT
        && window->mouse_buttons[button];
}

void alice_get_mouse(const alice_window *window, int *x, int *y) {
    if (!window) return;
    if (x) *x = window->mouse_x;
    if (y) *y = window->mouse_y;
}

alice_framebuffer *alice_get_framebuffer(alice_window *window) {
    return window ? &window->framebuffer : NULL;
}

////////////////////////////////////////////////////////////
//   Drawing   /////////////////////////////////////
////////////////////////////////////////////

// clear the screen
void alice_clear(alice_window *window, uint32_t color) {
    if (!window || !window->pixels) return;
    
    size_t index, count;
    count = (size_t)window->width * (size_t)window->height;
    for (index = 0; index < count; ++index) {
        window->pixels[index] = color;
    }
}

// draw a point (a pixel) to the screen
void alice_draw_point(alice_window *window, int x, int y, uint32_t color) {
    if (!window
    ||  !window->pixels
    ||  x < 0
    ||  y < 0
    ||  x >= window->width
    ||  y >= window->height
    ) return;
    
    window->pixels[(size_t)y * (size_t)window->width + (size_t)x] = color;
}

// Cohen-Sutherland outcode
// returns which side(s) of the framebuffer the point is on
static int alice_region(const alice_window *window, int x, int y) {
    return  (x < 0 ? 1 : 0) | (x >= window->width  ? 2 : 0)
        |   (y < 0 ? 4 : 0) | (y >= window->height ? 8 : 0);
}

static void alice_line_steps(uint64_t scale_factor, uint64_t array, uint64_t button, uint64_t *xs, uint64_t *ys) {
    uint64_t major = array >= button ? array : button;
    uint64_t minor = array >= button ? button : array;
    uint64_t time = scale_factor * minor;
    uint64_t m = time / major + ((time % major) * 2 >= major ? 1u : 0u);

    if (array >= button) {
        *xs = scale_factor;
        *ys = m;
    } else {
        *xs = m;
        *ys = scale_factor;
    }
}

static bool alice_line_before(
    const alice_window *window, int start_x, int start_y, int source_x, int source_y,
    uint64_t scale_factor, uint64_t array, uint64_t button
) {
    uint64_t xs, ys;
    int64_t x, y;

    alice_line_steps(scale_factor, array, button, &xs, &ys);
    x = (int64_t)start_x + source_x * (int64_t)xs;
    y = (int64_t)start_y + source_y * (int64_t)ys;
    
    return (source_x > 0 ? x < 0 : x >= window->width)
        || (source_y > 0 ? y < 0 : y >= window->height);
}

void alice_draw_line(alice_window *window, int start_x, int start_y, int end_x, int end_y, uint32_t color) {
    int64_t delta_x, delta_y, error_code;
    uint64_t array, button, value_count, low_value, high_value, scale_factor, xs, ys, event;
    int64_t x, y;
    int source_x, source_y;

    if (!window || !window->pixels)
        return;

    if (start_x == end_x
    &&  start_y == end_y
    ) {
        alice_draw_point(window, start_x, start_y, color);
        return;
    }
    
    if (alice_region(window, start_x, start_y) & alice_region(window, end_x, end_y))
        return;

    delta_x = end_x > start_x ?  (int64_t)end_x - start_x   :   (int64_t)start_x - end_x;
    delta_y = end_y > start_y ? -((int64_t)end_y - start_y) : -((int64_t)start_y - end_y);
    source_x = start_x < end_x ? 1 : -1;
    source_y = start_y < end_y ? 1 : -1;

    array = (uint64_t)delta_x;
    button = (uint64_t)-delta_y;
    value_count = array >= button ? array : button;

    low_value = 0;
    high_value = value_count + 1;
    while (low_value < high_value) {
        uint64_t mid = low_value + (high_value - low_value) / 2;
        if (alice_line_before(window, start_x, start_y, source_x, source_y, mid, array, button))
            low_value = mid + 1;
        else
            high_value = mid;
    }

    if (low_value > value_count)
        return;

    scale_factor = low_value;

    alice_line_steps(scale_factor, array, button, &xs, &ys);
    x = (int64_t)start_x + source_x * (int64_t)xs;
    y = (int64_t)start_y + source_y * (int64_t)ys;
    event = array - button - xs * button + ys * array;
    error_code = event <= (uint64_t)INT64_MAX ? (int64_t)event : -(int64_t)(~event) - 1;

    for (;; ++scale_factor) {
        int64_t e2;

        if (x < 0 || y < 0 || x >= window->width || y >= window->height)
            break;
        window->pixels[(size_t)y * (size_t)window->width + (size_t)x] = color;
        if (scale_factor == value_count)
            break;

        e2 = 2 * error_code;
        if (e2 >= delta_y) {
            error_code += delta_y;
            x += source_x;
        }
        
        if (e2 <= delta_x) {
            error_code += delta_x;
            y += source_y;
        }
    }
}

void alice_draw_rect(alice_window *window, int x, int y, int width, int height, uint32_t color) {
    int64_t start_x, start_y, end_x, end_y, pixel_x, pixel_y;

    if (!window
    ||  !window->pixels
    ||  width <= 0
    ||  height <= 0
    ) return;

    start_x = x < 0 ? 0 : x;
    start_y = y < 0 ? 0 : y;
    end_x = (int64_t)x + width;
    end_y = (int64_t)y + height;

    if (end_x > window->width)
        end_x = window->width;
    if (end_y > window->height)
        end_y = window->height;

    for (pixel_y = start_y; pixel_y < end_y; ++pixel_y) {
        for (pixel_x = start_x; pixel_x < end_x; ++pixel_x) {
            window->pixels[
                (size_t)pixel_y 
                * (size_t)window->width
                + (size_t)pixel_x
            ] = color;
        }
    }
}

uint32_t alice_get_pixel(const alice_window *window, int x, int y) {
    if (!window
    ||  !window->pixels
    ||  x < 0
    ||  y < 0
    ||  x >= window->width
    ||  y >= window->height
    ) return 0;

    return window->pixels[(size_t)y * (size_t)window->width + (size_t)x];
}

// a horizontal run of pixels
static void alice_span(alice_window *window, int64_t start_x, int64_t end_x, int64_t y, uint32_t color) {
    uint32_t *row;

    if (y < 0 || y >= window->height)
        return;
    if (start_x < 0)
        start_x = 0;
    if (end_x >= window->width)
        end_x = (int64_t)window->width - 1;
    if (start_x > end_x)
        return;

    row = window->pixels + (size_t)y * (size_t)window->width;
    for (; start_x <= end_x; ++start_x)
        row[start_x] = color;
}

void alice_draw_rect_outline(alice_window *window, int x, int y, int width, int height, uint32_t color) {
    int64_t left, right, top, bottom, row, first, last;

    if (!window || !window->pixels || width <= 0 || height <= 0)
        return;

    left   = x;
    right  = (int64_t)x + width - 1;
    top    = y;
    bottom = (int64_t)y + height - 1;

    alice_span(window, left, right, top, color);
    alice_span(window, left, right, bottom, color);

    first = top + 1 < 0 ? 0 : top + 1;
    last  = bottom - 1 >= window->height ? (int64_t)window->height - 1 : bottom - 1;
    for (row = first; row <= last; ++row) {
        alice_span(window, left, left, row, color);
        alice_span(window, right, right, row, color);
    }
}

static uint64_t alice_isqrt(uint64_t value) {
    uint64_t root = 0, bit = (uint64_t)1 << 62;

    while (bit > value)
        bit >>= 2;
    while (bit) {
        if (value >= root + bit) {
            value -= root + bit;
            root = (root >> 1) + bit;
        } else {
            root >>= 1;
        }
        bit >>= 2;
    }
    return root;
}

static int64_t alice_circle_reach(int64_t radius, int64_t dy) {
    if (dy < 0)
        dy = -dy;
    if (dy > radius)
        return -1;
    return (int64_t)alice_isqrt((uint64_t)(radius * radius - dy * dy));
}

void alice_draw_circle(alice_window *window, int center_x, int center_y, int radius, uint32_t color) {
    int64_t first, last, y;

    if (!window || !window->pixels || radius < 0)
        return;

    first = (int64_t)center_y - radius;
    last  = (int64_t)center_y + radius;
    if (first < 0)
        first = 0;
    if (last >= window->height)
        last = (int64_t)window->height - 1;

    for (y = first; y <= last; ++y) {
        int64_t reach = alice_circle_reach(radius, y - center_y);
        alice_span(window, (int64_t)center_x - reach, (int64_t)center_x + reach, y, color);
    }
}

void alice_draw_circle_outline(alice_window *window, int center_x, int center_y, int radius, uint32_t color) {
    int64_t first, last, y;

    if (!window || !window->pixels || radius < 0)
        return;

    first = (int64_t)center_y - radius;
    last  = (int64_t)center_y + radius;
    if (first < 0)
        first = 0;
    if (last >= window->height)
        last = (int64_t)window->height - 1;

    for (y = first; y <= last; ++y) {
        int64_t dy    = y - center_y;
        int64_t reach = alice_circle_reach(radius, dy);
        int64_t above = alice_circle_reach(radius, dy - 1);
        int64_t below = alice_circle_reach(radius, dy + 1);
        int64_t inner = (above < below ? above : below) + 1;
        if (inner > reach) inner = reach;
        
        alice_span(window, (int64_t)center_x + inner, (int64_t)center_x + reach, y, color);
        alice_span(window, (int64_t)center_x - reach, (int64_t)center_x - inner, y, color);
    }
}

static bool alice_blit_clip(
    const alice_window *window, int x, int y, int width, int height,
    int64_t *first_col, int64_t *last_col, int64_t *first_row, int64_t *last_row
) {
    *first_col = x < 0 ? -(int64_t)x : 0;
    *first_row = y < 0 ? -(int64_t)y : 0;
    *last_col  = (int64_t)width  - 1;
    *last_row  = (int64_t)height - 1;

    if (*last_col > (int64_t)window->width  - 1 - x)
        *last_col = (int64_t)window->width  - 1 - x;

    if (*last_row > (int64_t)window->height - 1 - y)
        *last_row = (int64_t)window->height - 1 - y;

    return *first_col <= *last_col
        && *first_row <= *last_row;
}

static bool alice_blit_backwards(
    const alice_window *window, int x, int y, const uint32_t *pixels, size_t stride,
    int64_t first_col, int64_t first_row
) {
    const char *source
    =   (const char *)pixels
    +   (size_t)first_row * stride
    +   (size_t)first_col * sizeof(uint32_t);

    const char *target
    =   (const char *) (
            window->pixels
            +   (size_t)(y + first_row)
            *   (size_t)window->width
            +   (size_t)(x + first_col)
        );
    return (uintptr_t)target > (uintptr_t)source;
}

void alice_blit(alice_window *window, int x, int y, const uint32_t *pixels, int width, int height, int pitch) {
    int64_t first_col, last_col, first_row, last_row, count;
    size_t stride;
    bool backwards;

    if (!window
    ||  !window->pixels
    ||  !pixels
    ||  width <= 0 || height <= 0
    ||  pitch < 0
    )   return;
    if (!alice_blit_clip(window, x, y, width, height, &first_col, &last_col, &first_row, &last_row))
        return;

    stride = pitch ? (size_t)pitch : (size_t)width * sizeof(uint32_t);
    backwards = alice_blit_backwards(window, x, y, pixels, stride, first_col, first_row);

    for (count = first_row; count <= last_row; ++count) {
        int64_t row = backwards ? last_row - (count - first_row) : count;
        const char *source
        =   (const char *)pixels
        +   (size_t)row * stride
        +   (size_t)first_col * sizeof(uint32_t);
        
        uint32_t *target
        =   window->pixels
        +   (size_t)(y + row)
        *   (size_t)window->width
        +   (size_t)(x + first_col);

        memmove(target, source, (size_t)(last_col - first_col + 1) * sizeof(uint32_t));
    }
}

void alice_blit_keyed(
    alice_window *window,
    int x, int y,
    const uint32_t *pixels,
    int width, int height,
    int pitch,
    uint32_t key
) {
    int64_t first_col, last_col, first_row, last_row, count, col;
    size_t stride;
    bool backwards;

    if (!window || !window->pixels || !pixels || width <= 0 || height <= 0 || pitch < 0)
        return;
    if (!alice_blit_clip(window, x, y, width, height, &first_col, &last_col, &first_row, &last_row))
        return;

    key &= 0xFFFFFFu;
    stride = pitch ? (size_t)pitch : (size_t)width * sizeof(uint32_t);
    backwards = alice_blit_backwards(window, x, y, pixels, stride, first_col, first_row);

    for (count = first_row; count <= last_row; ++count) {
        int64_t row = backwards ? last_row - (count - first_row) : count;
        const uint32_t *source
        =   (const uint32_t *)(
                (const char *)pixels + (size_t)row * stride
            );
        uint32_t *target
        =   window->pixels
        +   (size_t)(y + row)
        *   (size_t)window->width
        +   (size_t)x;

        for (col = first_col; col <= last_col; ++col) {
            int64_t c = backwards ? last_col - (col - first_col) : col;
            if ((source[c] & 0xFFFFFFu) != key) {
                target[c] = source[c];
            }
        }
    }
}

static FILE *alice_fopen_write(const char *path) {
#if defined(_WIN32)
    FILE *file;
    wchar_t *wide;
    int count = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);

    if (count <= 0)
        return fopen(path, "wb");

    wide = (wchar_t *)ALICE_MALLOC((size_t)count * sizeof(wchar_t));
    if (!wide) return NULL;

    MultiByteToWideChar(CP_UTF8, 0, path, -1, wide, count);
    file = _wfopen(wide, L"wb");
    ALICE_FREE(wide);

    return file;
#else
    return fopen(path, "wb");
#endif
}

static void alice_put_u32(unsigned char *out, uint32_t value) {
    out[0] = (unsigned char)(value & 0xFFu);
    out[1] = (unsigned char)((value >> 8) & 0xFFu);
    out[2] = (unsigned char)((value >> 16) & 0xFFu);
    out[3] = (unsigned char)((value >> 24) & 0xFFu);
}

bool alice_save_bmp(const alice_window *window, const char *path) {
    unsigned char header[54];
    unsigned char *row;
    size_t stride;
    uint32_t image_size;
    FILE *file;
    bool ok = true;
    int x, y;

    if (!window || !window->pixels || !path)
        return false;

    stride = ((size_t)window->width * 3u + 3u) & ~(size_t)3u;
    image_size = (uint32_t)(stride * (size_t)window->height);

    memset(header, 0, sizeof(header));
    header[0] = 'B';
    header[1] = 'M';
    alice_put_u32(header + 2, 54u + image_size);        // file size
    alice_put_u32(header + 10, 54u);                    // start of pixels
    alice_put_u32(header + 14, 40u);                    // size of the info header
    alice_put_u32(header + 18, (uint32_t)window->width);
    alice_put_u32(header + 22, (uint32_t)window->height);
    header[26] = 1;                                     // planes
    header[28] = 24;                                    // bits per pixel
    alice_put_u32(header + 34, image_size);
    alice_put_u32(header + 38, 2835u);                  // 72 dpi
    alice_put_u32(header + 42, 2835u);

    row = (unsigned char *)ALICE_MALLOC(stride);
    if (!row)
        return false;
    memset(row, 0, stride);

    file = alice_fopen_write(path);
    if (!file) {
        ALICE_FREE(row);
        return false;
    }

    ok = fwrite(header, sizeof(header), 1, file) == 1;
    for (y = window->height - 1; ok && y >= 0; --y) {
        const uint32_t *source = window->pixels + (size_t)y * (size_t)window->width;
        for (x = 0; x < window->width; ++x) {
            row[x * 3 + 0] = (unsigned char)(source[x] & 0xFFu);
            row[x * 3 + 1] = (unsigned char)((source[x] >> 8) & 0xFFu);
            row[x * 3 + 2] = (unsigned char)((source[x] >> 16) & 0xFFu);
        }
        ok = fwrite(row, stride, 1, file) == 1;
    }

    ALICE_FREE(row);
    if (fclose(file) != 0)
        ok = false;
    return ok;
}

////////////////////////////////////////////////////////////
//   Colour   //////////////////////////////////////
////////////////////////////////////////////

static float alice_clamp01(float value) {
    if (!(value > 0.0f))    // also catches NaN
        return 0.0f;
    if (value > 1.0f)
        return 1.0f;
    return value;
}

static uint32_t alice_to_byte(float value) {
    return (uint32_t)(alice_clamp01(value) * 255.0f + 0.5f);
}

static uint32_t alice_pack_rgb(float r, float g, float b) {
    return (alice_to_byte(r) << 16) | (alice_to_byte(g) << 8) | alice_to_byte(b);
}

static float alice_wrap_hue(float hue) {
    if (!(hue > -1.0e7f && hue < 1.0e7f))
        return 0.0f;
    hue -= 360.0f * (float)(int)(hue / 360.0f);
    if (hue < 0.0f)
        hue += 360.0f;
    if (hue >= 360.0f) // can happen from rounding
        hue = 0.0f;
    return hue;
}

uint32_t alice_rgb(int r, int g, int b) {
    r = r < 0 ? 0 : r > 255 ? 255 : r;
    g = g < 0 ? 0 : g > 255 ? 255 : g;
    b = b < 0 ? 0 : b > 255 ? 255 : b;
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

int alice_color_r(uint32_t color) { return (int)((color >> 16) & 0xFFu); }
int alice_color_g(uint32_t color) { return (int)((color >> 8) & 0xFFu); }
int alice_color_b(uint32_t color) { return (int)(color & 0xFFu); }

uint32_t alice_hsv(float hue, float saturation, float value) {
    float s = alice_clamp01(saturation);
    float v = alice_clamp01(value);
    float h = alice_wrap_hue(hue) / 60.0f;
    int sector = (int)h;
    float f;
    float p, q, t;

    if (sector > 5) sector = 0, h = 0.0f;
    f = h - (float)sector;
    p = v * (1.0f - s);
    q = v * (1.0f - s * f);
    t = v * (1.0f - s * (1.0f - f));

    switch (sector) {
    case 0:  return alice_pack_rgb(v, t, p);
    case 1:  return alice_pack_rgb(q, v, p);
    case 2:  return alice_pack_rgb(p, v, t);
    case 3:  return alice_pack_rgb(p, q, v);
    case 4:  return alice_pack_rgb(t, p, v);
    default: return alice_pack_rgb(v, p, q);
    }
}

uint32_t alice_hue(float hue) {
    return alice_hsv(hue, 1.0f, 1.0f);
}

uint32_t alice_hsl(float hue, float saturation, float lightness) {
    float s = alice_clamp01(saturation);
    float l = alice_clamp01(lightness);
    float v = l + s * (l < 0.5f ? l : 1.0f - l);
    float sv = v > 0.0f ? 2.0f * (1.0f - l / v) : 0.0f;

    return alice_hsv(hue, sv, v);
}

static void alice_analyse(uint32_t color, float *max_out, float *min_out, float *hue_out) {
    float r = (float)((color >> 16) & 0xFFu) / 255.0f;
    float g = (float)((color >> 8) & 0xFFu) / 255.0f;
    float b = (float)(color & 0xFFu) / 255.0f;
    float max = r > g ? (r > b ? r : b) : (g > b ? g : b);
    float min = r < g ? (r < b ? r : b) : (g < b ? g : b);
    float delta = max - min;
    float hue = 0.0f;

    if (delta > 0.0f) {
        if (max == r)
            hue = (g - b) / delta;
        else if (max == g)
            hue = 2.0f + (b - r) / delta;
        else
            hue = 4.0f + (r - g) / delta;
        hue *= 60.0f;
        if (hue < 0.0f)
            hue += 360.0f;
    }
    *max_out = max;
    *min_out = min;
    *hue_out = hue;
}

void alice_to_hsv(uint32_t color, float *hue, float *saturation, float *value) {
    float max, min, h;
    alice_analyse(color, &max, &min, &h);
    if (hue)        *hue = h;
    if (saturation) *saturation = max > 0.0f ? (max - min) / max : 0.0f;
    if (value)      *value = max;
}

void alice_to_hsl(uint32_t color, float *hue, float *saturation, float *lightness) {
    float max, min, h, l, d, s = 0.0f;
    alice_analyse(color, &max, &min, &h);
    l = (max + min) * 0.5f;
    d = 2.0f * l - 1.0f;
    if (d < 0.0f)
        d = -d;
    if (max > min)
        s = (max - min) / (1.0f - d);
    if (hue)        *hue = h;
    if (saturation) *saturation = s;
    if (lightness)  *lightness = l;
}

uint32_t alice_mix(uint32_t a, uint32_t b, float t) {
    uint32_t out = 0;
    int shift;

    t = alice_clamp01(t);
    for (shift = 16; shift >= 0; shift -= 8) {
        float from = (float)((a >> shift) & 0xFFu);
        float to   = (float)((b >> shift) & 0xFFu);
        out |= (uint32_t)(from + (to - from) * t + 0.5f) << shift;
    }
    return out;
}

uint32_t alice_lighten(uint32_t color, float amount) {
    return alice_mix(color, 0xFFFFFFu, amount);
}

uint32_t alice_darken(uint32_t color, float amount) {
    return alice_mix(color, 0x000000u, amount);
}

uint32_t alice_saturate(uint32_t color, float amount) {
    float h, s, l;
    alice_to_hsl(color, &h, &s, &l);
    if (s <= 0.0f)
        return color & 0xFFFFFFu;
    return alice_hsl(h, s + (1.0f - s)* alice_clamp01(amount), l);
}

uint32_t alice_desaturate(uint32_t color, float amount) {
    float h, s, l;
    alice_to_hsl(color, &h, &s, &l);
    return alice_hsl(h, s * (1.0f - alice_clamp01(amount)), l);
}

uint32_t alice_rotate_hue(uint32_t color, float degrees) {
    float h, s, v;
    alice_to_hsv(color, &h, &s, &v);
    return alice_hsv(h + alice_wrap_hue(degrees), s, v);
}

uint32_t alice_invert(uint32_t color) {
    return ~color & 0xFFFFFFu;
}

int alice_luma(uint32_t color) {
    // 54 + 183 + 19 = 256, so white is exactly 255
    return (int)((54u * ((color >> 16) & 0xFFu) + 183u * ((color >> 8) & 0xFFu) + 19u * (color & 0xFFu) + 128u) >> 8);
}

uint32_t alice_grayscale(uint32_t color) {
    uint32_t l = (uint32_t)alice_luma(color);
    return (l << 16) | (l << 8) | l;
}

////////////////////////////////////////////////////////////
//   Time   ////////////////////////////////////////
////////////////////////////////////////////

uint64_t alice_time_ns(void) {
#if defined(_WIN32)
    static LARGE_INTEGER frequency;
    LARGE_INTEGER count;
    uint64_t ticks, per_second;

    if (frequency.QuadPart == 0)
        QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&count);
    ticks = (uint64_t)count.QuadPart;
    per_second = (uint64_t)frequency.QuadPart;
    return (ticks / per_second) * 1000000000u + (ticks % per_second) * 1000000000u / per_second;
#else
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint64_t)now.tv_sec * 1000000000u + (uint64_t)now.tv_nsec;
#endif
}

uint64_t alice_time(void) {
    return alice_time_ns() / 1000000u;
}

void alice_sleep(uint32_t milliseconds) {
#if defined(_WIN32)
    static ALICE_THREAD_LOCAL HANDLE timer;
    static ALICE_THREAD_LOCAL bool timer_unavailable;

    if (!timer && !timer_unavailable) {
        timer = CreateWaitableTimerExW(
            NULL, NULL, 0x00000002 /* HIGH_RESOLUTION */,
            TIMER_ALL_ACCESS
        );
        if (!timer)
            timer_unavailable = true;
    }

    if (timer) {
        LARGE_INTEGER due;
        due.QuadPart = -(LONGLONG)milliseconds * 10000;
        if (SetWaitableTimer(timer, &due, 0, NULL, NULL, FALSE)) {
            WaitForSingleObject(timer, INFINITE);
            return;
        }
    }
    Sleep(milliseconds);
#else
    struct timespec req;
    req.tv_sec  = (time_t)(milliseconds / 1000u);
    req.tv_nsec = (long)(milliseconds % 1000u) * 1000000L;
    while (nanosleep(&req, &req) != 0 && errno == EINTR) ;
#endif
}

static void alice_cpu_relax(void) {
#if defined(_WIN32)
    YieldProcessor();
#elif defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
    __asm__ __volatile__("pause");
#elif defined(__GNUC__) && defined(__aarch64__)
    __asm__ __volatile__("yield");
#endif
}

void alice_wait_until_ns(uint64_t time_ns) {
    for (;;) {
        uint64_t now = alice_time_ns();
        uint64_t left;

        if (now >= time_ns)
            return;
        left = time_ns - now;

        if (left > ALICE_WAIT_SPIN_NS 
        &&  left - ALICE_WAIT_SPIN_NS >= 1000000u
        ) {
            uint64_t milliseconds = (left - ALICE_WAIT_SPIN_NS) / 1000000u;
            alice_sleep(milliseconds > 0xFFFFFFFFu ? 0xFFFFFFFFu : (uint32_t)milliseconds);
        } else {
            alice_cpu_relax();
        }
    }
}

void alice_wait_until(uint64_t time_ms) {
    alice_wait_until_ns(time_ms > UINT64_MAX / 1000000u ? UINT64_MAX : time_ms * 1000000u);
}

static uint32_t alice_timer_period;

#if defined(_WIN32)
typedef UINT (WINAPI *alice_time_period_fn)(UINT);
static alice_time_period_fn alice_win_begin_period;
static alice_time_period_fn alice_win_end_period;

static bool alice_win_load_winmm(void) {
    if (!alice_win_begin_period || !alice_win_end_period) {
        HMODULE module = LoadLibraryW(L"winmm.dll");
        if (!module) return false;

        alice_win_begin_period = (alice_time_period_fn)(void (*)(void))GetProcAddress(module, "timeBeginPeriod");
        alice_win_end_period   = (alice_time_period_fn)(void (*)(void))GetProcAddress(module, "timeEndPeriod");
        
        if (!alice_win_begin_period || !alice_win_end_period) {
            alice_win_begin_period = alice_win_end_period = NULL;
            return false;
        }
    }
    return true;
}
#endif

bool alice_begin_timer_period(uint32_t milliseconds) {
    if (milliseconds == 0)
        return false;

    alice_end_timer_period();

#if defined(_WIN32)
    if (!alice_win_load_winmm() || alice_win_begin_period((UINT)milliseconds) != 0)
        return false;
#endif
    alice_timer_period = milliseconds;
    return true;
}

void alice_end_timer_period(void) {
#if defined(_WIN32)
    if (alice_timer_period && alice_win_end_period)
        alice_win_end_period((UINT)alice_timer_period);
#endif
    alice_timer_period = 0;
}

double alice_frame_wait(alice_frame_timer *timer, double fps) {
    uint64_t now, after;
    double dt;

    if (!timer)
        return 0.0;

    now = alice_time_ns();
    if (!timer->started) {
        timer->started = true;
        timer->next_ns = now;
        timer->last_ns = now;
    }

    if (fps > 0.0) {
        double period = 1000000000.0 / fps;
        if (period > 1.0e18)
            period = 1.0e18;
        timer->next_ns += (uint64_t)period;
        if (now >= timer->next_ns)
            timer->next_ns = now;
        else
            alice_wait_until_ns(timer->next_ns);
    }

    after = alice_time_ns();
    dt = (double)(after - timer->last_ns) * 1.0e-9;
    timer->last_ns = after;
    return dt;
}

#endif /* ALICE_IMPLEMENTATION */