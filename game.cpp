#include "game.hpp"

#ifndef ARDUINO
array<uint8_t, BUF_BYTES> buf;
#endif

void clear_buf()
{
    uint8_t* pa = &buf[0];
    uint8_t* pb = pa + 1024;
    while(pa < pb) *pa++ = 0;
}

static constexpr int8_t VERTS[] PROGMEM =
{
    -1, -1, +1,
    -1, +1, +1,
    -1, -1, -1,
    -1, +1, -1,
    +1, -1, +1,
    +1, +1, +1,
    +1, -1, -1,
    +1, +1, -1,
};
static constexpr size_t NUM_VERTS = 8;

static constexpr uint8_t FACES[] PROGMEM =
{
    2-1, 3-1, 1-1, 1,
    4-1, 7-1, 3-1, 3,
    8-1, 5-1, 7-1, 1,
    6-1, 1-1, 5-1, 3,
    7-1, 1-1, 3-1, 2,
    4-1, 6-1, 8-1, 2,
    2-1, 4-1, 3-1, 1,
    4-1, 8-1, 7-1, 3,
    8-1, 6-1, 5-1, 1,
    6-1, 2-1, 1-1, 3,
    7-1, 5-1, 1-1, 2,
    4-1, 2-1, 6-1, 2,
};
static constexpr size_t NUM_FACES = 12;

static array<uint8_t, MAX_FACES> face_order;

void game_loop()
{
#if 1

    dvec2 vs[NUM_VERTS];

    static uint8_t yaw   = 0;
    static uint8_t pitch = 0;

    pitch = 16;

    mat3 m;
    rotation(m, yaw, pitch);

    for(uint8_t i = 0, j = 0; i < NUM_VERTS; ++i, j += 3)
    {
        vec3 v;
        v.x = pgm_read_byte(&VERTS[j + 0]);
        v.y = pgm_read_byte(&VERTS[j + 1]);
        v.z = pgm_read_byte(&VERTS[j + 2]);
        dvec3 rv = matvec(m, v);
        rv.x *= 2;
        rv.y *= 2;
        rv.x += FBW / 2 * 16;
        rv.y += FBH / 2 * 16;
        vs[i] = { rv.x, rv.y };
        fd.vz[i] = rv.z;
    }

    for(uint8_t i = 0; i < NUM_FACES; ++i)
    {
        uint8_t j = i << 2;
        uint8_t v0 = pgm_read_byte(&FACES[j + 0]);
        uint8_t v1 = pgm_read_byte(&FACES[j + 1]);
        uint8_t v2 = pgm_read_byte(&FACES[j + 2]);
        face_order[i] = i;
        fd.fz[i] = fd.vz[v0] + fd.vz[v1] + fd.vz[v2];
    }

    // order faces
#if 1
    {
        uint8_t i = 1;
        while(i < NUM_FACES)
        {
            int16_t z = fd.fz[i];
            uint8_t f = face_order[i];
            uint8_t j = i;
            while(j > 0 && fd.fz[j - 1] < z)
            {
                fd.fz[j] = fd.fz[j - 1];
                face_order[j] = face_order[j - 1];
                j -= 1;
            }
            fd.fz[j] = z;
            face_order[j] = f;
            i += 1;
        }
    }
#endif

    clear_buf();
    for(uint8_t i = 0; i < NUM_FACES; ++i)
    {
        uint8_t j = face_order[i] << 2;
        uint8_t i0 = pgm_read_byte(&FACES[j + 0]);
        uint8_t i1 = pgm_read_byte(&FACES[j + 1]);
        uint8_t i2 = pgm_read_byte(&FACES[j + 2]);
        uint8_t pt = pgm_read_byte(&FACES[j + 3]);
        draw_tri(vs[i0], vs[i1], vs[i2], pt);
    }

    ++yaw;

#else
    static int16_t x = 0;
    static uint16_t t = 0;

    vec2 sa = { 0, -16 };
    vec2 sb = { -16, 0 };
    vec2 sc = { +16, 0 };
    vec2 sd = { 0, +16 };

    dvec2 a = frotate(sa, (uint8_t)x);
    dvec2 b = frotate(sb, (uint8_t)x);
    dvec2 c = frotate(sc, (uint8_t)x);
    dvec2 d = frotate(sd, (uint8_t)x);
    a.x += FBW / 2 * 16, a.y += FBH / 2 * 16;
    b.x += FBW / 2 * 16, b.y += FBH / 2 * 16;
    c.x += FBW / 2 * 16, c.y += FBH / 2 * 16;
    d.x += FBW / 2 * 16, d.y += FBH / 2 * 16;
    for(int i = 0; i < 100; ++i)
    {
        draw_tri(a, c, b, 1);
        draw_tri(b, c, d, 3);
    }
    x += 1;
#endif
}
