#include "game.hpp"

#ifndef ARDUINO
array<uint8_t, BUF_BYTES> buf;
#endif

static constexpr uint8_t SET_MASK[8] PROGMEM =
{
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
};

void set_pixel(uint8_t x, uint8_t y)
{
    //if(x < FBW && y < FBH)
        buf[y / 8 * FBW + x] |= pgm_read_byte(&SET_MASK[y & 7]);
}

static constexpr uint8_t CLEAR_MASK[8] PROGMEM =
{
    0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f,
};

void clear_pixel(uint8_t x, uint8_t y)
{
    //if(x < FBW && y < FBH)
        buf[y / 8 * FBW + x] &= pgm_read_byte(&CLEAR_MASK[y & 7]);
}

static constexpr uint8_t YMASK0[8] PROGMEM =
{
    0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80
};
static constexpr uint8_t YMASK1[8] PROGMEM =
{
    0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff
};

static constexpr uint16_t PATTERNS[5] PROGMEM =
{
    0x0000,
    0xaa00,
    0xaa55,
    0xff55,
    0xffff,
};

static void draw_vline(uint8_t x, int16_t y0, int16_t y1, uint16_t pat)
{
    if(y0 > y1) return;
    if(y1 < 0) return;
    if(y0 >= FBH) return;
    if(x >= FBW) return;

    uint8_t ty0 = (uint8_t)tmax<int16_t>(y0, 0);
    uint8_t ty1 = (uint8_t)tmin<int16_t>(y1, FBH - 1);

    uint8_t t0 = ty0 & 0xf8;
    uint8_t t1 = ty1 & 0xf8;
    ty0 &= 7;
    ty1 &= 7;

    uint8_t pattern = (x & 1) ? uint8_t(pat) : uint8_t(pat >> 8);

    uint8_t* p = &buf[t0 * (FBW / 8) + x];
    uint8_t m0 = pgm_read_byte(&YMASK0[ty0]);
    uint8_t m1 = pgm_read_byte(&YMASK1[ty1]);

    if(t0 == t1)
    {
        uint8_t m = m0 & m1;
        uint8_t tp = *p;
        tp |= (pattern & m);
        tp &= (pattern | ~m);
        *p = tp;
        return;
    }

    {
        uint8_t m = m0;
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
        uint8_t m = m1;
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
    if(ax >= bx) return;

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
        draw_vline((uint8_t)pxa, py0, py1, pat);
        //set_pixel((uint8_t)pxa, (uint8_t)py0);
        //set_pixel((uint8_t)pxa, (uint8_t)py1);
        pxa += 1;
        e0 += dy0;
        e1 += dy1;
    }
}

static void draw_circle_bresenham_segment_filled(
    int16_t cx, int16_t cy, int16_t x, int16_t y, uint16_t pat)
{
    // all inputs in pixels
    draw_vline(uint8_t(cx + x), cy - y, cy + y, pat);
    draw_vline(uint8_t(cx - x), cy - y, cy + y, pat);
    draw_vline(uint8_t(cx + y), cy - x, cy + x, pat);
    draw_vline(uint8_t(cx - y), cy - x, cy + x, pat);
}

static void draw_circle_set_pixel(uint8_t x, int16_t y)
{
    if(x >= FBW) return;
    if(y < 0 || y >= FBH) return;
    uint8_t* p = &buf[(uint16_t(y & 0x38) << 4) + x];
    *p &= ~(1 << (y & 0x7));
}

static void draw_circle_bresenham_segment_outline(
    int16_t cx, int16_t cy, int16_t x, int16_t y)
{
    // all inputs in pixels
    draw_circle_set_pixel(uint8_t(cx + x), cy - y);
    draw_circle_set_pixel(uint8_t(cx + x), cy + y);
    draw_circle_set_pixel(uint8_t(cx - x), cy - y);
    draw_circle_set_pixel(uint8_t(cx - x), cy + y);
    draw_circle_set_pixel(uint8_t(cx + y), cy - x);
    draw_circle_set_pixel(uint8_t(cx + y), cy + x);
    draw_circle_set_pixel(uint8_t(cx - y), cy - x);
    draw_circle_set_pixel(uint8_t(cx - y), cy + x);
}

#if 0
static void draw_ball_tri(dvec2 c, uint8_t r, uint8_t pati)
{
    // approximate ball as N-gon with triangles
    constexpr uint8_t N = 16;
    constexpr uint8_t INVN = 256 / N;
    uint8_t angle = 0;
    dvec2 c0;
    c0.x = c.x;
    c0.y = c.y + int8_t(uint16_t(fmuls(127, r)) >> 8);
    for(uint8_t n = 0, angle = 0; n < N; ++n)
    {
        angle += INVN;
        int8_t sin_angle = int8_t(uint16_t(fmuls(fsin(angle), r)) >> 8);
        int8_t cos_angle = int8_t(uint16_t(fmuls(fcos(angle), r)) >> 8);
        dvec2 c1;
        c1.x = c.x + cos_angle;
        c1.y = c.y + sin_angle;
        draw_tri(c, c0, c1, pati);
        c0 = c1;
    }
}
#endif

void draw_ball_outline(dvec2 c, uint16_t r)
{
    if(c.x < -(int16_t)r || c.y < -(int16_t)r) return;
    if(c.x > FBW * 16 + r || c.y > FBW * 16 + r) return;

    c.x = div16s(c.x);
    c.y = div16s(c.y);
    r >>= 4;

    int16_t dx = 12;
    int16_t dy = 8 - r * 8;
    int16_t e = 5 - r * 4;
    int16_t y = (int16_t)r;

    for(int16_t x = 0; x <= y; ++x)
    {
        draw_circle_bresenham_segment_outline(c.x, c.y, x, y);
        if(e >= 0)
        {
            e += dy;
            dy += 8;
            y -= 1;
        }
        e += dx;
        dx += 8;
    }
}

void draw_ball_filled(dvec2 c, uint16_t r)
{
    if(c.x < -(int16_t)r || c.y < -(int16_t)r) return;
    if(c.x > FBW * 16 + r || c.y > FBW * 16 + r) return;

    c.x = div16s(c.x);
    c.y = div16s(c.y);
    r >>= 4;

    int16_t dx = 12;
    int16_t dy = 8 - r * 8;
    int16_t e = 5 - r * 4;
    int16_t y = (int16_t)r;

    for(int16_t x = 0; x <= y; ++x)
    {
        draw_circle_bresenham_segment_filled(c.x, c.y, x, y, 0xffff);
        if(e >= 0)
        {
            e += dy;
            dy += 8;
            y -= 1;
        }
        e += dx;
        dx += 8;
    }
}

int16_t interp(int16_t a, int16_t b, int16_t c, int16_t x, int16_t z)
{
    // x + (z-x) * (b-a)/(c-a)

    if(a == c) return x;

#if 0

    int32_t t = uint16_t(b - a);
    t *= (z - x);
    t /= uint16_t(c - a);
    return x + (int16_t)t;

#elif 0

    uint32_t t = uint32_t(z - x) * uint16_t(b - a);

    uint16_t ac = uint16_t(c - a);
    uint8_t n = 0;
    while(ac >= 256)
        n += 1, ac >>= 1;
    int16_t r = divlut(t, uint8_t(ac));
    while(n > 0)
        n -= 1, r /= 2;
    return x + r;

#else
    uint16_t xz = (x < z ? uint16_t(z - x) : uint16_t(x - z));
    uint32_t p = uint32_t(xz) * uint16_t(b - a);
    uint16_t ac = uint16_t(c - a);
    int16_t t;
    t = (int16_t)(p / ac);

    if(x > z) t = -t;
    return x + t;
#endif
}

static inline int8_t to8(int16_t x)
{
    return int8_t(uint16_t(x) >> 8);
}

void draw_tri(dvec2 v0, dvec2 v1, dvec2 v2, uint8_t pati)
{
#if 0
    // backface culling
#if 0
    int16_t t = 0;
    t += int8_t(to8(v1.x) - to8(v0.x)) * int8_t(to8(v2.y) - to8(v0.y));
    t -= int8_t(to8(v2.x) - to8(v0.x)) * int8_t(to8(v1.y) - to8(v0.y));
#elif 1
    int32_t t = 0;
    t += int32_t(v1.x - v0.x) * (v2.y - v0.y);
    t -= int32_t(v2.x - v0.x) * (v1.y - v0.y);
#elif 0
    int32_t t = 0;
    t += int32_t(v0.x) * v1.y;
    t += int32_t(v1.x) * v2.y;
    t += int32_t(v2.x) * v0.y;
    t -= int32_t(v1.x) * v0.y;
    t -= int32_t(v2.x) * v1.y;
    t -= int32_t(v0.x) * v2.y;
#else
    int16_t t = 0;
    t += int8_t(v0.x >> 8) * int8_t(v1.y >> 8);
    t += int8_t(v1.x >> 8) * int8_t(v2.y >> 8);
    t += int8_t(v2.x >> 8) * int8_t(v0.y >> 8);
    t -= int8_t(v1.x >> 8) * int8_t(v0.y >> 8);
    t -= int8_t(v2.x >> 8) * int8_t(v1.y >> 8);
    t -= int8_t(v0.x >> 8) * int8_t(v2.y >> 8);
#endif
    if(t < 0) return;
#endif

    if(tmax(v0.y, v1.y, v2.y) < 0) return;
    if(tmin(v0.y, v1.y, v2.y) > FBH * 16) return;

    // sort by x coord
    if(v0.x > v1.x) swap(v0, v1);
    if(v1.x > v2.x) swap(v1, v2);
    if(v0.x > v1.x) swap(v0, v1);

    if(v2.x < 0 || v0.x >= FBW * 16 || v0.x == v2.x) return;

    // interpolate vt.y: between v0, v2, with vt.x = v1.x
    int16_t ty = interp(v0.x, v1.x, v2.x, v0.y, v2.y);

    uint16_t pat = pgm_read_word(&PATTERNS[pati]);

    // left segment
    {
        int16_t ax  = v0.x;
        int16_t bx  = v1.x;
        int16_t ay0 = v0.y;
        int16_t ay1 = v0.y;
        int16_t by0 = v1.y;
        int16_t by1 = ty;
        if(ax < 0)
        {
            ay0 = interp(ax, 0, bx, ay0, by0);
            ay1 = interp(ax, 0, bx, ay1, by1);
            ax = 0;
        }
        if(bx > FBW * 16)
        {
            by0 = interp(ax, FBW * 16, bx, ay0, by0);
            by1 = interp(ax, FBW * 16, bx, ay1, by1);
            bx = FBW * 16;
        }
        draw_tri_segment(ax, ay0, ay1, bx, by0, by1, pat);
    }

    // right segment
    {
        int16_t ax  = v1.x;
        int16_t bx  = v2.x;
        int16_t ay0 = v1.y;
        int16_t ay1 = ty;
        int16_t by0 = v2.y;
        int16_t by1 = v2.y;
        if(ax < 0)
        {
            ay0 = interp(ax, 0, bx, ay0, by0);
            ay1 = interp(ax, 0, bx, ay1, by1);
            ax = 0;
        }
        if(bx > FBW * 16)
        {
            by0 = interp(ax, FBW * 16, bx, ay0, by0);
            by1 = interp(ax, FBW * 16, bx, ay1, by1);
            bx = FBW * 16;
        }
        draw_tri_segment(ax, ay0, ay1, bx, by0, by1, pat);
    }
}
