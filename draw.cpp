#include "game.hpp"

// TODO: more efficient left-right tri clipping
//       (interpolate segments)

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

static void draw_tri_vline(uint8_t x, int16_t y0, int16_t y1)
{
    if(x >= FBW) return;
    if(y0 > y1) return;

    uint8_t ty0 = (uint8_t)tclamp<int16_t>(y0, 0, FBH - 1);
    uint8_t ty1 = (uint8_t)tclamp<int16_t>(y1, 0, FBH - 1);

    uint8_t t0 = ty0 & 0xf8;
    uint8_t t1 = ty1 & 0xf8;

    uint8_t pattern = 0xff;

    uint16_t i = t0 * 16 + x;

    if(t0 == t1)
    {
        uint8_t m =
            pgm_read_byte(&YMASK0[ty0 & 7]) &
            pgm_read_byte(&YMASK1[ty1 & 7]);
        uint8_t tp = buf[i];
        tp |= (pattern & m);
        tp &= (pattern | ~m);
        buf[i] = tp;
        return;
    }

    {
        uint8_t m = pgm_read_byte(&YMASK0[ty0 & 7]);
        uint8_t tp = buf[i];
        tp |= (pattern & m);
        tp &= (pattern | ~m);
        buf[i] = tp;
        i += 128;
    }

    t1 -= 8;
    for(uint8_t t = t0 + 1; t < t1; t += 8)
    {
        buf[i] = pattern;
        i += 128;
    }

    {
        uint8_t m = pgm_read_byte(&YMASK1[ty1 & 7]);
        uint8_t tp = buf[i];
        tp |= (pattern & m);
        tp &= (pattern | ~m);
        buf[i] = tp;
    }

}

static void draw_tri_segment(
    int16_t ax,
    int16_t ay0,
    int16_t ay1,
    int16_t bx,
    int16_t by0,
    int16_t by1)
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
        draw_tri_vline((uint8_t)pxa, py0, py1);
        pxa += 1;
        e0 += dy0;
        e1 += dy1;
    }
}

void draw_tri(dvec2 v0, dvec2 v1, dvec2 v2)
{
    // sort by x coord
    if(v0.x > v1.x) swap(v0, v1);
    if(v1.x > v2.x) swap(v1, v2);
    if(v0.x > v1.x) swap(v0, v1);

    // interpolate vt.y: between v0, v2, with vt.x = v1.x
    int16_t ty;
    {
        uint16_t f = int32_t(v1.x - v0.x) * 4096 / int16_t(v2.x - v0.x);
        ty = v0.y + int32_t(v2.y - v0.y) * f / 4096;
    }

    draw_tri_segment(v0.x, v0.y, v0.y, v1.x, v1.y, ty);
    draw_tri_segment(v1.x, v1.y, ty, v2.x, v2.y, v2.y);
}
