#include "game.hpp"

#define PERFDOOM 0

#define FDIST_CAMY_MOD 1

static constexpr int16_t ZNEAR = 256;  // near

static constexpr uint8_t FLAG_HEIGHT = 4;
static constexpr uint8_t FLAG_SIZE = 2;
static constexpr uint8_t FLAG_POLE_PAT = 4; // 3? more visible against flat
static constexpr uint8_t FLAG_PAT = 3;

static uint8_t flag_anim = 0;

array<face, MAX_FACES> fs;
array<dvec2, MAX_VERTS> vs;

static dvec2 interpz(dvec2 a, int16_t az, dvec2 b, int16_t bz)
{
    dvec2 r;
    r.x = interp(az, ZNEAR, bz, a.x, b.x);
    r.y = interp(az, ZNEAR, bz, a.y, b.y);
    return r;
}

void clear_buf()
{
    uint8_t* pa = &buf[0];
    uint8_t* pb = pa + 1024;
    while(pa < pb) *pa++ = 0;
}

static uint8_t add_vertex(dmat3 const& m, dvec3 dv, uint8_t nv)
{
    // translate
    dv.x -= cam.x;
    dv.y -= cam.y;
    dv.z -= cam.z;

    // vertex distance
    fd.vdist[nv] = uint16_t(uint32_t(
        int32_t(dv.x) * dv.x +
        int32_t(dv.y) * dv.y +
        int32_t(dv.z) * dv.z) >> 16);

    // rotate
    dv = matvec(m, dv);
    dv.z = -dv.z;

    // save vertex
    vs[nv] = { dv.x, dv.y };
    fd.vz[nv] = dv.z;

    return nv + 1;
}

static uint16_t calc_fdist(
    uint8_t i0, uint8_t i1, uint8_t i2,
    int16_t y0, int16_t y1, int16_t y2)
{
    uint16_t fdist = fd.vdist[i0] + fd.vdist[i1] + fd.vdist[i2];
#if FDIST_CAMY_MOD
    int16_t a0 = tabs(cam.y - y0);
    int16_t a1 = tabs(cam.y - y1);
    int16_t a2 = tabs(cam.y - y2);
    uint16_t amin = (uint16_t)tmin(a0, a1, a2);
    uint16_t amin2 = uint16_t((uint32_t(amin) * amin) >> 16);
    fdist += amin2 * 16;
#endif
    return fdist;
}

#ifndef ARDUINO
static int ortho_zoom;
#endif

template<bool ortho>
static uint8_t render_scene(
    int8_t const* verts,
    uint8_t const* faces,
    uint8_t num_verts,
    uint8_t const* num_faces)
{
    dmat3 m;
    rotation16(m, yaw, pitch);

    inv16(256+128);

#if PERFDOOM
    for(int i = 0; i < PERFDOOM; ++i) {
#endif

    uint8_t nv = 0;
    uint8_t nf = 0;

    // translate and rotate vertices
    for(uint8_t j = nv = 0; nv < num_verts; j += 3)
    {
        dvec3 dv;
        int8_t tvx, tvy, tvz;

        tvx = (int8_t)pgm_read_byte(&verts[j + 0]);
        tvy = (int8_t)pgm_read_byte(&verts[j + 1]);
        tvz = (int8_t)pgm_read_byte(&verts[j + 2]);

        dv.x = tvx * 64;
        dv.y = tvy * 64;
        dv.z = tvz * 64;

        fd.cached_vy[nv] = tvy;

        nv = add_vertex(m, dv, nv);
    }

    // ball vertices (center and right side) and face
    // ball "face" is index 0
    uint8_t balli0, balli1; // ball face indices
    bool ball_valid = false;
    if(!ball_in_hole())
    {
        dvec3 dv = ball;
        dv.x -= cam.x;
        dv.y -= cam.y;
        dv.z -= cam.z;
        dv = matvec(m, dv);
        dv.z = -dv.z;

        if(dv.z >= ZNEAR)
        {
            myassert(nv + 1 <= MAX_VERTS);
            vs[nv] = { dv.x, dv.y };
            vs[nv + 1] = { int16_t(dv.x + 128), dv.y };
            fd.vz[nv] = dv.z;
            fd.vz[nv + 1] = dv.z;
            balli0 = nv;
            balli1 = nv + 1;
            ball_valid = true;
            int16_t by = ball.y;
            uint16_t fdist = calc_fdist(nv, nv, nv, by, by, by);
            fdist += 400;
            myassert(nf == 0);
            fd.fdist[0] = fdist;
            fs[0].pt = 255;
            nv += 2;
            nf += 1;
        }
    }

    // flag vertices and faces
    {
        bool flag_valid = true;
        uint8_t fi = nv;
        dvec3 dv;
        memcpy_P(&dv, &current_level->flag_pos, sizeof(dv));
        dv.y += 256 * 1;
        int16_t dvy0 = dv.y;
        nv = add_vertex(m, dv, nv);
        flag_valid &= fd.vz[nv - 1] >= ZNEAR;
        {
            dvec2 tv = vs[nv - 1];
            tv.x += 48;
            vs[nv] = tv;
            fd.vz[nv] = fd.vz[nv - 1];
            ++nv;
        }
        dv.y += 256 * FLAG_HEIGHT;
        int16_t dvy1 = dv.y;
        nv = add_vertex(m, dv, nv);
        flag_valid &= fd.vz[nv - 1] >= ZNEAR;
        {
            dvec2 tv = vs[nv - 1];
            tv.x += 48;
            vs[nv] = tv;
            fd.vz[nv] = fd.vz[nv - 1];
            ++nv;
        }
        dv.y += 256 * FLAG_SIZE;
        nv = add_vertex(m, dv, nv);
        flag_valid &= fd.vz[nv - 1] >= ZNEAR;
        dv.y -= 256 * FLAG_SIZE / 2;
        dv.x += fsin(nframe << 3);
        dv.z -= 256 * FLAG_SIZE;
        nv = add_vertex(m, dv, nv);
        flag_valid &= fd.vz[nv - 1] >= ZNEAR;

        if(flag_valid)
        {
            fd.fdist[nf] = calc_fdist(
                fi + 0, fi + 1, fi + 2,
                dvy0, dvy0, dvy1);
            fs[nf] = { uint8_t(fi + 0), uint8_t(fi + 1), uint8_t(fi + 2), FLAG_POLE_PAT };
            ++nf;

            fd.fdist[nf] = calc_fdist(
                fi + 1, fi + 2, fi + 3,
                dvy0, dvy1, dvy1);
            fs[nf] = { uint8_t(fi + 1), uint8_t(fi + 2), uint8_t(fi + 3), FLAG_POLE_PAT };
            ++nf;

            fd.fdist[nf] = calc_fdist(
                fi + 3, fi + 4, fi + 5,
                dvy1, dvy1, dvy1);
            fs[nf] = { uint8_t(fi + 2), uint8_t(fi + 4), uint8_t(fi + 5), FLAG_PAT };
            ++nf;
        }
    }

    // assemble faces and clip them to near plane
    int8_t camy8 = hibyte(cam.y * 2);
    for(uint8_t pat = 0, tnf = 0, i = 0; pat < 5; ++pat)
    {
        tnf += pgm_read_byte(&num_faces[pat]);
        for(; i < tnf; ++i)
        {
            if(nf + 2 > MAX_FACES)
                break;

            uint8_t const* fptr = &faces[i * 3];
            uint8_t i0, i1, i2;

            i0 = pgm_read_byte(fptr + 0);
            i1 = pgm_read_byte(fptr + 1);
            i2 = pgm_read_byte(fptr + 2);
            int16_t z0 = fd.vz[i0];
            int16_t z1 = fd.vz[i1];
            int16_t z2 = fd.vz[i2];

            uint8_t behind = 0;
            if(z0 < ZNEAR) behind |= 1;
            if(z1 < ZNEAR) behind |= 2;
            if(z2 < ZNEAR) behind |= 4;

            // discard if fully behind near plane
            if(behind == 7) continue;

            // face distance
            uint16_t fdist;
            {
                int16_t y0 = fd.cached_vy[i0] * 64;
                int16_t y1 = fd.cached_vy[i1] * 64;
                int16_t y2 = fd.cached_vy[i2] * 64;
                fdist = calc_fdist(i0, i1, i2, y0, y1, y2);
            }

            // clip: exactly two vertices behind near plane
            if((behind & (behind - 1)) != 0)
            {
                if(nv + 2 > MAX_VERTS)
                    continue;
                if(nf + 2 > MAX_FACES)
                    continue;

                dvec2 newv0, newv1;
                int16_t splitz;
                uint8_t ibase;

                // adjust the two near vertices.
                if(!(behind & 1))
                {
                    ibase = i0;
                    splitz = z0;
                    newv0 = interpz(vs[i1], z1, vs[i0], z0);
                    newv1 = interpz(vs[i2], z2, vs[i0], z0);
                }
                else if(!(behind & 2))
                {
                    ibase = i1;
                    splitz = z1;
                    newv0 = interpz(vs[i0], z0, vs[i1], z1);
                    newv1 = interpz(vs[i2], z2, vs[i1], z1);
                }
                else
                {
                    ibase = i2;
                    splitz = z2;
                    newv0 = interpz(vs[i0], z0, vs[i2], z2);
                    newv1 = interpz(vs[i1], z1, vs[i2], z2);
                }

                // add vertices
                vs[nv] = newv0;
                fd.vz[nv] = ZNEAR;
                ++nv;
                vs[nv] = newv1;
                fd.vz[nv] = ZNEAR;
                ++nv;

                // add clip face
                fd.fdist[nf] = fdist;
                fs[nf] = { ibase, uint8_t(nv - 1), uint8_t(nv - 2), pat };
                ++nf;

                continue;
            }

            // clip: exactly one vertex behind near plane
            else if(behind != 0)
            {
                if(nv + 2 > MAX_VERTS)
                    continue;
                if(nf + 2 > MAX_FACES)
                    continue;

                uint8_t ia, ib, ic; // winding order indices
                dvec2 newv0, newv1;
                int16_t splitz0;
                int16_t splitz1;
                if(behind & 1)
                {
                    ia = i0, ib = i1, ic = i2;
                    newv0 = interpz(vs[i0], z0, vs[i1], z1);
                    newv1 = interpz(vs[i0], z0, vs[i2], z2);
                    splitz0 = z1;
                    splitz1 = z2;
                }
                else if(behind & 2)
                {
                    ia = i1, ib = i2, ic = i0;
                    newv0 = interpz(vs[i1], z1, vs[i2], z2);
                    newv1 = interpz(vs[i1], z1, vs[i0], z0);
                    splitz0 = z2;
                    splitz1 = z0;
                }
                else
                {
                    ia = i2, ib = i0, ic = i1;
                    newv0 = interpz(vs[i2], z2, vs[i0], z0);
                    newv1 = interpz(vs[i2], z2, vs[i1], z1);
                    splitz0 = z0;
                    splitz1 = z1;
                }

                // add new vertices
                vs[nv] = newv0;
                fd.vz[nv] = ZNEAR;
                ++nv;
                vs[nv] = newv1;
                fd.vz[nv] = ZNEAR;
                ++nv;

                fd.fdist[nf] = fdist;
                fs[nf] = { uint8_t(nv - 2), ib, ic, pat };
                ++nf;

                fd.fdist[nf] = fdist;
                fs[nf] = { uint8_t(nv - 2), ic, uint8_t(nv - 1), pat };
                ++nf;

                continue;
            }

            fd.fdist[nf] = fdist;
            fs[nf] = { i0, i1, i2, pat };
            ++nf;
        }
    }

    // project vertices
    for(uint8_t i = 0; i < nv; ++i)
    {
        dvec3 dv = { vs[i].x, vs[i].y, fd.vz[i] };

#ifndef ARDUINO
        if(ortho)
        {
            // divide x and y by ortho_zoom
            dv.x = int32_t(dv.x) * ortho_zoom / 256 / FB_FRAC_COEF;
            dv.y = int32_t(dv.y) * ortho_zoom / 256 / FB_FRAC_COEF;
        }
        else
#endif
        // multiply x and y by FB_FRAC_COEF/4 / z
        if(dv.z >= ZNEAR)
        {
            uint16_t invz = inv16(dv.z);
            int32_t nx = int32_t(invz) * dv.x;
            int32_t ny = int32_t(invz) * dv.y;
#if FB_FRAC_BITS > 2
            static constexpr int32_t CLAMP_VAL =
                (int32_t)INT16_MAX * 240 / FB_FRAC_COEF * 1024;
            nx = tclamp<int32_t>(nx, -CLAMP_VAL, CLAMP_VAL);
            ny = tclamp<int32_t>(ny, -CLAMP_VAL, CLAMP_VAL);
#endif
            dv.x = int16_t(uint32_t(nx) >> (18 - FB_FRAC_BITS));
            dv.y = int16_t(uint32_t(ny) >> (18 - FB_FRAC_BITS));
        }

        // center in framebuffer
        dv.x += (FBW / 2 * FB_FRAC_COEF);
        dv.y = (FBH / 2 * FB_FRAC_COEF) - dv.y;

        // save vertex
        vs[i] = { dv.x, dv.y };
    }

    // order faces
#if 1
    {
        uint8_t i = 1;
        while(i < nf)
        {
            int16_t d = fd.fdist[i];
            face f = fs[i];
            uint8_t j = i;
            while(j > 0 && fd.fdist[j - 1] < d)
            {
                fd.fdist[j] = fd.fdist[j - 1];
                fs[j] = fs[j - 1];
                j -= 1;
            }
            fd.fdist[j] = d;
            fs[j] = f;
            i += 1;
        }
    }
#endif

    // finally, render
    if(ball_valid)
        ball_valid = fd.vz[balli0] >= ZNEAR;
    clear_buf(); // buf mem was used during setup
    uint16_t ballr = 0;
    uint8_t nfpat[4];
    nfpat[0] =            pgm_read_byte(&num_faces[0]);
    nfpat[1] = nfpat[0] + pgm_read_byte(&num_faces[1]);
    nfpat[2] = nfpat[1] + pgm_read_byte(&num_faces[2]);
    nfpat[3] = nfpat[2] + pgm_read_byte(&num_faces[3]);
    for(uint8_t i = 0; i < nf; ++i)
    {
        face f = fs[i];
        if(f.pt == 255)
        {
            // ball
            if(ball_valid)
            {
                dvec2 c = vs[balli0];
                ballr = uint16_t(vs[balli1].x - c.x);
                draw_ball_filled(vs[balli0], ballr, 0xffff);
#if !BALL_XRAY
                draw_ball_outline(vs[balli0], ballr + (FB_FRAC_COEF / 2));
#endif
            }
            continue;
        }
        else
        {
            draw_tri(vs[f.i0], vs[f.i1], vs[f.i2], f.pt);
        }
    }
#if BALL_XRAY
    if(ball_valid)
        draw_ball_outline(vs[balli0], ballr + (FB_FRAC_COEF / 2));
#endif

#if PERFDOOM
    }
#endif

    return nf;
}

uint8_t render_scene()
{
    return render_scene<false>(
        pgmptr(&current_level->verts),
        pgmptr(&current_level->faces),
        pgm_read_byte(&current_level->num_verts),
        current_level->num_faces);
}

#ifndef ARDUINO
uint8_t render_scene_persp(
    int8_t const* verts,
    uint8_t const* faces,
    uint8_t num_verts,
    uint8_t const* num_faces)
{
    return render_scene<false>(verts, faces, num_verts, num_faces);
}
uint8_t render_scene_ortho(
    int zoom,
    int8_t const* verts,
    uint8_t const* faces,
    uint8_t num_verts,
    uint8_t const* num_faces)
{
    ortho_zoom = zoom;
    return render_scene<true>(verts, faces, num_verts, num_faces);
}
dvec3 transform_point(dvec3 dv, bool ortho, int ortho_zoom)
{
    dmat3 m;
    rotation16(m, yaw, pitch);

    dv.x -= cam.x;
    dv.y -= cam.y;
    dv.z -= cam.z;

    dv = matvec(m, dv);
    dv.z = -dv.z;

    if(ortho)
    {
        dv.x = int32_t(dv.x) * ortho_zoom / 256 / FB_FRAC_COEF;
        dv.y = int32_t(dv.y) * ortho_zoom / 256 / FB_FRAC_COEF;
    }
    else if(dv.z >= ZNEAR)
    {
        uint16_t invz = inv16(dv.z);
        int32_t nx = int32_t(invz) * dv.x;
        int32_t ny = int32_t(invz) * dv.y;
#if FB_FRAC_BITS > 2
        static constexpr int32_t CLAMP_VAL =
            (int32_t)INT16_MAX * 240 / FB_FRAC_COEF * 1024;
        nx = tclamp<int32_t>(nx, -CLAMP_VAL, CLAMP_VAL);
        ny = tclamp<int32_t>(ny, -CLAMP_VAL, CLAMP_VAL);
#endif
        dv.x = int16_t(uint32_t(nx) >> (18 - FB_FRAC_BITS));
        dv.y = int16_t(uint32_t(ny) >> (18 - FB_FRAC_BITS));

    }

    dv.x += (FBW / 2 * FB_FRAC_COEF);
    dv.y = (FBH / 2 * FB_FRAC_COEF) - dv.y;

    return dv;
}
#endif
