#include <math.h>

#define ALICE_IMPLEMENTATION
#include "alice.h"

#define WIDTH  200
#define HEIGHT 200

typedef struct {
    int a;
    int b;
} Edge;

typedef struct {
    float x;
    float y;
    float z;
} Vec3;

static Vec3 rotate_xyz(Vec3 v, float ax, float ay, float az);

int main(int argc, char const *argv[]) {
    alice_config config = {
        .title      = "3D",
        .width      = WIDTH,
        .height     = HEIGHT,
        .scale      = 2,
        .resizable  = true,
        .stretch    = false
    };

    alice_window *window = alice_create(&config);
    if (!window) return 1;

    uint32_t draw_colour = 0xFFFFFFFF;

    Vec3 verts[8] = {
        {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, {-1,  1, -1},
        {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1}
    };

    Edge edges[12] = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };

    float rotate_x = 0.6f;
    float rotate_y = 0.8f;
    float rotate_z = 0.2f;

    float rotation_speed = 0.01f;

    float fov = 90.0f, viewer_dist = 2.0f;
    int sx2d[8], sy2d[8];

    bool running = true;
    while (running) {
        alice_event event;
        while (alice_poll_event(window, &event)) {
            if (event.type == ALICE_EVENT_QUIT) {
                running = false;
            }
        }
        if (!running) break;
        
        rotate_y += rotation_speed;
        rotate_z += rotation_speed / 1.5f;
        
        for (int i = 0; i < 8; i++) {
            Vec3 r = rotate_xyz(verts[i], rotate_x, rotate_y, rotate_z);
            float z = r.z + viewer_dist;

            if (z <= 0.1f) z = 0.1f;

            float px = (r.x * fov) / z;
            float py = (r.y * fov * 0.5f) / z;
            
            sx2d[i] = (int)(WIDTH / 2 + px);
            sy2d[i] = (int)(HEIGHT / 2 - py);
        }

        // drawing
        alice_clear(window, 0xFF000000);

        for (int i = 0; i < 12; i++) {
            alice_draw_line(window, sx2d[edges[i].a], sy2d[edges[i].a], sx2d[edges[i].b], sy2d[edges[i].b], draw_colour);
        }

        alice_present(window);
        alice_sleep(10);
    }

    return 0;
}

static Vec3 rotate_xyz(Vec3 v, float ax, float ay, float az) {
    float sx = sinf(ax), cx = cosf(ax);
    float sy = sinf(ay), cy = cosf(ay);
    float sz = sinf(az), cz = cosf(az);

    float y1 = v.y * cx - v.z * sx;
    float z1 = v.y * sx + v.z * cx;
    float x1 = v.x;

    float x2 = x1 * cy + z1 * sy;
    float z2 = -x1 * sy + z1 * cy;
    float y2 = y1;

    Vec3 out;
    out.x = x2 * cz - y2 * sz;
    out.y = x2 * sz + y2 * cz;
    out.z = z2;

    return out;
}