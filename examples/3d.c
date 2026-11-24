#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#define ALICE_IMPLEMENTATION
#include "alice.h"

#define WIDTH  300
#define HEIGHT 300

#define PI 3.14159265358979323846f
#define NEAR_PLANE 0.1f

typedef struct {
    int a;
    int b;
} Edge;

typedef struct {
    float x;
    float y;
    float z;
} Vec3;

typedef struct {
    float x;
    float y;
} Vec2;

typedef struct {
    float sin_x;
    float cos_x;
    float sin_y;
    float cos_y;
    float sin_z;
    float cos_z;
} Rotation;

static Vec3 rotate_xyz(Vec3 v, const Rotation *rotation);
static bool clip_line(Vec3 *a, Vec3 *b, float near_plane);
static Vec2 project_point(Vec3 v, float focal_length, float center_x, float center_y);
static uint32_t get_depth_colour(uint32_t colour, float depth);

int main(int argc, char const *argv[]) {
    alice_config config = {
        .title		= "3D",
        .width		= WIDTH,
        .height		= HEIGHT,
        .scale		= 2,
        .resizable	= true,
        .stretch	= false
    };

    alice_window *window = alice_create(&config);
    if (!window) return 1;

    alice_surface *screen = alice_get_surface(window);

    uint32_t cube_colour = 0xFF00FF00;

    Vec3 verts[8] = {
        {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, {-1,  1, -1},
        {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1}
    };

    Edge edges[12] = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };

    Vec3 rotated_verts[8];
    Vec2 projected_verts[8];

    float rotate_x = 0.6f;
    float rotate_y = 0.8f;
    float rotate_z = 0.2f;

    float rotation_speed = 0.02f;

    float fov = 75.0f;
    float viewer_dist = 4.0f;

    float focal_length = (HEIGHT * 0.5f) / tanf((fov * PI / 180.0f) * 0.5f);
    float center_x = WIDTH * 0.5f;
    float center_y = HEIGHT * 0.5f;

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
        rotate_x -= rotation_speed / 2.5f;

        Rotation rotation = {
            .sin_x = sinf(rotate_x),
            .cos_x = cosf(rotate_x),
            .sin_y = sinf(rotate_y),
            .cos_y = cosf(rotate_y),
            .sin_z = sinf(rotate_z),
            .cos_z = cosf(rotate_z)
        };

        // camera transform
        for (int i = 0; i < 8; i++) {
            rotated_verts[i] = rotate_xyz(verts[i], &rotation);
            rotated_verts[i].z += viewer_dist;
        }

        // near plane project vertices
        for (int i = 0; i < 8; i++) {
            if (rotated_verts[i].z > NEAR_PLANE) {
                projected_verts[i] = project_point(
                    rotated_verts[i],
                    focal_length,
                    center_x,
                    center_y
                );
            }
        }

        alice_clear(screen, 0xFF000000);

        // wireframe edges with nearplane clipping
        for (int i = 0; i < 12; i++) {
            Vec3 a = rotated_verts[edges[i].a];
            Vec3 b = rotated_verts[edges[i].b];

            if (!clip_line(&a, &b, NEAR_PLANE)) {
                continue;
            }

            Vec2 pa = project_point(a, focal_length, center_x, center_y);
            Vec2 pb = project_point(b, focal_length, center_x, center_y);

            float average_depth = (a.z + b.z);
            uint32_t colour = get_depth_colour(cube_colour, average_depth);

            alice_draw_line(
                screen,
                (int)pa.x, (int)pa.y,
                (int)pb.x, (int)pb.y,
                colour
            );
        }

        // crt scanline filter
        for (int y = 1; y < HEIGHT; y += 2) {
            alice_draw_line(screen, 0, y, WIDTH, y, 0x88111111);
        }

        alice_present(window);
        alice_sleep(10);
    }

    return 0;
}

static Vec3 rotate_xyz(Vec3 v, const Rotation *rotation) {
    float y1 = v.y * rotation->cos_x - v.z * rotation->sin_x;
    float z1 = v.y * rotation->sin_x + v.z * rotation->cos_x;
    float x1 = v.x;

    float x2 = x1 * rotation->cos_y + z1 * rotation->sin_y;
    float z2 = -x1 * rotation->sin_y + z1 * rotation->cos_y;
    float y2 = y1;

    Vec3 out;
    out.x = x2 * rotation->cos_z - y2 * rotation->sin_z;
    out.y = x2 * rotation->sin_z + y2 * rotation->cos_z;
    out.z = z2;

    return out;
}

static bool clip_line(Vec3 *a, Vec3 *b, float near_plane) {
    if (a->z <= near_plane && b->z <= near_plane) {
        return false;
    }

    if (a->z <= near_plane || b->z <= near_plane) {
        float t = (near_plane - a->z) / (b->z - a->z);

        Vec3 intersection = {
            .x = a->x + (b->x - a->x) * t,
            .y = a->y + (b->y - a->y) * t,
            .z = near_plane
        };

        if (a->z <= near_plane) {
            *a = intersection;
        } else {
            *b = intersection;
        }
    }

    return true;
}

static Vec2 project_point(Vec3 v, float focal_length, float center_x, float center_y) {
    float inverse_z = 1.0f / v.z;

    Vec2 out;
    out.x = center_x + v.x * focal_length * inverse_z;
    out.y = center_y - v.y * focal_length * inverse_z;

    return out;
}

static uint32_t get_depth_colour(uint32_t colour, float depth) {
    float brightness = 1.0f / depth;
    brightness *= 3.0f;
    if (brightness > 1.0f) brightness = 1.0f;
    if (brightness < 0.15f) brightness = 0.15f;
    
    uint32_t r = (uint32_t)(alice_color_r(colour) * brightness);
    uint32_t g = (uint32_t)(alice_color_g(colour) * brightness);
    uint32_t b = (uint32_t)(alice_color_b(colour) * brightness);

    return 0xFF000000 | (r << 16) | (g << 8) | b;
}