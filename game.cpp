#include "game.hpp"

#ifndef ARDUINO
array<uint8_t, BUF_BYTES> buf;
#endif

void game_loop()
{
    //static int16_t y = 0;
    //draw_line(0, 0, FBW * 16, y++);

    static int16_t x = 0;
    static uint16_t t = 0;

    vec2 sa = { 0, -32 };
    vec2 sb = { -16, 0 };
    vec2 sc = { +16, 0 };
    vec2 sd = { 0, +32 };
    //dvec2 a = { 0, -32 * 16 };
    //dvec2 b = { -32 * 16, 0 };
    //dvec2 c = { +32 * 16, 0 };
    //dvec2 d = { 0, +32 * 16 };
    dvec2 a = frotate(sa, (uint8_t)x);
    dvec2 b = frotate(sb, (uint8_t)x);
    dvec2 c = frotate(sc, (uint8_t)x);
    dvec2 d = frotate(sd, (uint8_t)x);
    a.x += 64 * 16, a.y += 32 * 16;
    b.x += 64 * 16, b.y += 32 * 16;
    c.x += 64 * 16, c.y += 32 * 16;
    d.x += 64 * 16, d.y += 32 * 16;
#if 1
    for(uint8_t i = 0; i < 50; ++i)
    {
        draw_tri(a, c, b);
        draw_tri(b, c, d);
    }
    //draw_line(a.x, a.y, b.x, b.y);
    //draw_line(b.x, b.y, c.x, c.y);
    //draw_line(a.x, a.y, c.x, c.y);
#else
    draw_line(a.x, a.y, b.x, b.y);
    draw_line(b.x, b.y, d.x, d.y);
    draw_line(d.x, d.y, c.x, c.y);
    draw_line(c.x, c.y, a.x, a.y);
#endif

    x += 1;

    //draw_tri(
    //    { 0, 0 },
    //    { 64, 0 },
    //    { 64, 64 });
}
