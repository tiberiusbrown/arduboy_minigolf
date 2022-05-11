#include "game.hpp"

#define PERFDOOM 10

#ifndef ARDUINO
array<uint8_t, BUF_BYTES> buf;
#endif

void clear_buf()
{
    uint8_t* pa = &buf[0];
    uint8_t* pb = pa + 1024;
    while(pa < pb) *pa++ = 0;
}

#if 1
#include "levels/level_00.hpp"
static int8_t const* const VERTS = LEVEL_00_VERTS;
static constexpr size_t NUM_VERTS = sizeof(LEVEL_00_VERTS) / 3;
static uint8_t const* const FACES = LEVEL_00_FACES;
//static constexpr size_t NUM_FACES = sizeof(LEVEL_00_FACES) / 4;
static constexpr size_t NUM_FACES = 4;
#else
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
#endif

static constexpr int16_t ZNEAR = 256 * 0.5;  // near

static array<uint8_t, MAX_FACES> face_order;

void game_loop()
{
#if 1


    dvec2 vs[NUM_VERTS];

    // camera look angle (roll not supported)
    static uint8_t yaw = 0;
    static uint8_t pitch = 0;

    // camera position
    static int16_t cx = 0;
    static int16_t cy = 256 * 3;
    static int16_t cz = -256 * 8;

    //cx = -704;
    //cz = -1328;

    {
        uint8_t btns = poll_btns();
        if(btns & BTN_LEFT ) if(btns & BTN_A) yaw += 1; else cx -= 8;
        if(btns & BTN_RIGHT) if(btns & BTN_A) yaw -= 1; else cx += 8;
        if(btns & BTN_UP   ) if(btns & BTN_A) cy += 8; else if(btns & BTN_B) pitch += 1; else cz += 8;
        if(btns & BTN_DOWN ) if(btns & BTN_A) cy -= 8; else if(btns & BTN_B) pitch -= 1; else cz -= 8;
    }

    pitch = (uint8_t)tclamp<int8_t>(int8_t(pitch), -64, 64);

    mat3 m;
    rotation(m, yaw, pitch);

#if PERFDOOM
    for(int i = 0; i < PERFDOOM; ++i) {
#endif

    uint8_t nv = 0;
    uint8_t nf = 0;

    // translate and rotate vertices
    for(uint8_t j = nv = 0; nv < NUM_VERTS; ++nv, j += 3)
    {
        dvec3 dv;
        dv.x = (int8_t)pgm_read_byte(&VERTS[j + 0]) << 8;
        dv.y = (int8_t)pgm_read_byte(&VERTS[j + 1]) << 8;
        dv.z = (int8_t)pgm_read_byte(&VERTS[j + 2]) << 8;

        // 1. translate
        dv.x -= cx;
        dv.y -= cy;
        dv.z -= cz;

        // 3. rotate
        dv = matvec(m, dv);

        // 4. project
#if 1
        if(dv.z >= 0)
        {
#if 0
            int32_t nx = int32_t(dv.x) * 256 / (uint16_t)dv.z;
            int32_t ny = int32_t(dv.y) * 256 / (uint16_t)dv.z;
            nx = tclamp(nx * 4, -8192, 8192);
            ny = tclamp(ny * 4, -8192, 8192);
            dv.x = nx;
            dv.y = ny;
#else
            uint16_t zs = uint16_t(dv.z) >> 4;
            uint16_t f;
            if(zs >= 256)
                f = inv8(uint8_t(uint16_t(dv.z) >> 8)) >> 6;
            else
                f = inv8((uint8_t)zs) >> 2;
            int32_t nx = int32_t(f) * dv.x;
            int32_t ny = int32_t(f) * dv.y;
            nx = tclamp<int32_t>(nx, (int32_t)INT16_MIN * 240, (int32_t)INT16_MAX * 240);
            ny = tclamp<int32_t>(ny, (int32_t)INT16_MIN * 240, (int32_t)INT16_MAX * 240);
            dv.x = int16_t(nx >> 8);
            dv.y = int16_t(ny >> 8);
#endif
        }
        else
        {
            if(dv.x < 0) dv.x = int16_t((int32_t)INT16_MIN * 240 / 256);
            else         dv.x = int16_t((int32_t)INT16_MAX * 240 / 256);
            if(dv.y < 0) dv.y = int16_t((int32_t)INT16_MIN * 240 / 256);
            else         dv.y = int16_t((int32_t)INT16_MAX * 240 / 256);
        }
#endif

        // 4. center in framebuffer
        dv.x += (FBW / 2 * 16);
        dv.y = (FBH / 2 * 16) - dv.y;

        // save vertex
        vs[nv] = { dv.x, dv.y };
        fd.vz[nv] = dv.z;
    }

    // order and clip faces
    for(uint8_t i = 0; i < NUM_FACES; ++i)
    {
        uint8_t j = i << 2;
        uint8_t v0 = pgm_read_byte(&FACES[j + 0]);
        uint8_t v1 = pgm_read_byte(&FACES[j + 1]);
        uint8_t v2 = pgm_read_byte(&FACES[j + 2]);
        int16_t z0 = fd.vz[v0];
        int16_t z1 = fd.vz[v1];
        int16_t z2 = fd.vz[v2];

        // discard if fully behind near plane
        int16_t mz = tmax(z0, z1, z2);
        if(mz < ZNEAR) continue;

        // clip if partially behind z plane
        mz = tmin(z0, z1, z2);
        if(mz < ZNEAR)
        {
            // TODO
        }

        face_order[nf] = i;
        fd.fz[nf] = z0 + z1 + z2;
        ++nf;
    }

    // order faces
#if 1
    {
        uint8_t i = 1;
        while(i < nf)
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
    for(uint8_t i = 0; i < nf; ++i)
    {
        uint8_t j = face_order[i] << 2;
        uint8_t i0 = pgm_read_byte(&FACES[j + 0]);
        uint8_t i1 = pgm_read_byte(&FACES[j + 1]);
        uint8_t i2 = pgm_read_byte(&FACES[j + 2]);
        uint8_t pt = pgm_read_byte(&FACES[j + 3]);
        draw_tri(vs[i0], vs[i1], vs[i2], pt);
    }

    //++yaw;
#if PERFDOOM
    }
#endif

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
