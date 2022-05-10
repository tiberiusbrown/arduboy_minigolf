#include "game.hpp"

// TODO: more efficient left-right tri clipping
//       (interpolate segments)

static void set_pixel(uint8_t x, uint8_t y)
{
    if(x < FBW && y < FBH)
        buf[y / 8 * FBW + x] |= (1 << (y % 8));
}

static constexpr uint8_t YMASK0[8] PROGMEM =
{
    0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80
};
static constexpr uint8_t YMASK1[8] PROGMEM =
{
    0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff
};

static int16_t div16s(int16_t x)
{
    uint16_t r = (uint16_t)x >> 4;
    if(x < 0) r |= 0xf000;
    return (int16_t)r;
}

static constexpr uint16_t PATTERNS[5] PROGMEM =
{
    0x0000,
    0xaa00,
    0xaa55,
    0xff55,
    0xffff,
};

static void draw_tri_vline(uint8_t x, int16_t y0, int16_t y1, uint16_t pat)
{
    if(x >= FBW) return;
    if(y0 > y1) return;
    if(y1 < 0) return;
    if(y0 >= FBH) return;

    uint8_t ty0 = (uint8_t)tclamp<int16_t>(y0, 0, FBH - 1);
    uint8_t ty1 = (uint8_t)tclamp<int16_t>(y1, 0, FBH - 1);

    uint8_t t0 = ty0 & 0xf8;
    uint8_t t1 = ty1 & 0xf8;
    ty0 &= 7;
    ty1 &= 7;

    uint8_t pattern = (x & 1) ? uint8_t(pat) : uint8_t(pat >> 8);

    uint8_t* p = &buf[t0 * (FBW / 8) + x];

    if(t0 == t1)
    {
        uint8_t m =
            pgm_read_byte(&YMASK0[ty0]) &
            pgm_read_byte(&YMASK1[ty1]);
        uint8_t tp = *p;
        tp |= (pattern & m);
        tp &= (pattern | ~m);
        *p = tp;
        return;
    }

    {
        uint8_t m = pgm_read_byte(&YMASK0[ty0]);
        uint8_t tp = *p;
        tp |= (pattern & m);
        tp &= (pattern | ~m);
        *p = tp;
        p += FBW;
    }

    for(int8_t t = t1 - t0 - 8; t > 0; t -= 8)
    {
        *p = pattern;
        p += FBW;
    }

    {
        uint8_t m = pgm_read_byte(&YMASK1[ty1]);
        uint8_t tp = *p;
        tp |= (pattern & m);
        tp &= (pattern | ~m);
        *p = tp;
    }

}

static void draw_tri_segment(
    int16_t ax,
    int16_t ay0,
    int16_t ay1,
    int16_t bx,
    int16_t by0,
    int16_t by1,
    uint16_t pat)
{
    if(ay0 > ay1 || by0 > by1)
    {
        swap(ay0, ay1);
        swap(by0, by1);
    }

    int16_t dx = bx - ax;
    int16_t dy0 = tabs(by0 - ay0);
    int16_t dy1 = tabs(by1 - ay1);

    int16_t e0, e1;
    {
        uint8_t ay0mask = uint8_t(ay0 & 15);
        uint8_t ay1mask = uint8_t(ay1 & 15);
        if(ay0 > by0) ay0mask = (16 - ay0mask) & 15;
        if(ay1 > by1) ay1mask = (16 - ay1mask) & 15;
        int8_t tax = ((ax & 15) - 8);
        e0 = div16s(dx * (ay0mask - 16) - dy0 * tax);
        e1 = div16s(dx * (ay1mask - 16) - dy1 * tax);
    }

    int8_t sy0 = (ay0 < by0 ? 1 : -1);
    int8_t sy1 = (ay1 < by1 ? 1 : -1);
    if(ay0 > by0) ay0 -= 1;
    if(ay1 > by1) ay1 -= 1;

    int16_t pxa = div16s(ax);
    int16_t pxb = div16s(bx + 7);
    int16_t py0 = div16s(ay0);
    int16_t py1 = div16s(ay1);

    while(pxa != pxb)
    {
        while(e0 > 0)
        {
            py0 += sy0;
            e0 -= dx;
        }
        while(e1 > 0)
        {
            py1 += sy1;
            e1 -= dx;
        }
        draw_tri_vline((uint8_t)pxa, py0, py1, pat);
        //set_pixel((uint8_t)pxa, (uint8_t)py0);
        //set_pixel((uint8_t)pxa, (uint8_t)py1);
        pxa += 1;
        e0 += dy0;
        e1 += dy1;
    }
}

static int16_t interp(int16_t a, int16_t b, int16_t c, int16_t x, int16_t z)
{
    // x + (z-x) * (b-a)/(c-a)

#if 0

    s24 t = s24(z - x) * uint16_t(b - a);

    uint16_t ac = uint16_t(c - a);
    uint8_t n = 0;
    while(ac >= 256)
        n += 1, ac >>= 1;
    int16_t r = divlut(t, uint8_t(ac));
    while(n > 0)
        n -= 1, r /= 2;
    return x + r;

#elif 1
    uint16_t xz = (x < z ? uint16_t(z - x) : uint16_t(x - z));
    u24 p = u24(xz) * uint16_t(b - a);
    u24 ac = u24(c - a) << 7;
    int16_t t = 0;
    uint8_t j = 0x80;
    do
    {
        while(p >= ac) t += j, p -= ac;
        ac >>= 1;
        j >>= 1;
    } while(j != 0);
    if(x > z) t = -t;
    return x + t;
#else
    int32_t t = uint16_t(b - a);
    t *= (z - x);
    t /= uint16_t(c - a);
    return x + (int16_t)t;
#endif
}

void draw_tri(dvec2 v0, dvec2 v1, dvec2 v2, uint8_t pati)
{
    // backface culling

    int16_t t = 0;
    t += int8_t(v0.x >> 4) * int8_t(v1.y >> 4);
    t += int8_t(v1.x >> 4) * int8_t(v2.y >> 4);
    t += int8_t(v2.x >> 4) * int8_t(v0.y >> 4);
    t -= int8_t(v1.x >> 4) * int8_t(v0.y >> 4);
    t -= int8_t(v2.x >> 4) * int8_t(v1.y >> 4);
    t -= int8_t(v0.x >> 4) * int8_t(v2.y >> 4);
    if(t > 0) return;

    // sort by x coord
    if(v0.x > v1.x) swap(v0, v1);
    if(v1.x > v2.x) swap(v1, v2);
    if(v0.x > v1.x) swap(v0, v1);

    if(v2.x < 0 || v0.x >= FBW * 16 || v0.x == v2.x) return;

    // interpolate vt.y: between v0, v2, with vt.x = v1.x
    int16_t ty = interp(v0.x, v1.x, v2.x, v0.y, v2.y);

    uint16_t pat = pgm_read_word(&PATTERNS[pati]);

    draw_tri_segment(v0.x, v0.y, v0.y, v1.x, v1.y, ty, pat);
    draw_tri_segment(v1.x, v1.y, ty, v2.x, v2.y, v2.y, pat);
}
