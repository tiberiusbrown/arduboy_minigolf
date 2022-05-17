#include "game.hpp"

#define PERFDOOM 0

static constexpr int16_t ZNEAR = 256 * 0.5;  // near

array<uint8_t, MAX_FACES> face_order;
array<uint8_t, MAX_CLIP_FACES * 4> clip_faces;
array<dvec2, MAX_VERTS> vs;

// camera look angle (roll not supported)
uint16_t yaw;
int16_t pitch;

// camera position
dvec3 cam;

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

uint8_t render_scene()
{
    return render_scene(
        pgmptr(&current_level->verts),
        pgmptr(&current_level->faces),
        pgm_read_byte(&current_level->num_verts),
        pgm_read_byte(&current_level->num_faces));
}

uint8_t render_scene(
    int8_t const* verts,
    uint8_t const* faces,
    uint8_t num_verts,
    uint8_t num_faces)
{
    dmat3 m;
    rotation16(m, yaw, pitch);

    inv16(256+128);

#if PERFDOOM
    for(int i = 0; i < PERFDOOM; ++i) {
#endif

    uint8_t nv = 0;
    uint8_t nf = 0;
    uint8_t nclipf = 0;

    // translate and rotate vertices
    for(uint8_t j = nv = 0; nv < num_verts; ++nv, j += 3)
    {
        dvec3 dv;
        dv.x = (int8_t)pgm_read_byte(&verts[j + 0]) << 8;
        dv.y = (int8_t)pgm_read_byte(&verts[j + 1]) << 8;
        dv.z = (int8_t)pgm_read_byte(&verts[j + 2]) << 8;

        // translate
        dv.x -= cam.x;
        dv.y -= cam.y;
        dv.z -= cam.z;

        // vertex distance
        //fd.vdist[nv] = uint16_t(tabs(dv.x) + tabs(dv.y) + tabs(dv.z));
        fd.vdist[nv] = uint16_t(uint32_t(
            int32_t(dv.x) * dv.x +
            int32_t(dv.y) * dv.y +
            int32_t(dv.z) * dv.z) >> 16);
        //fd.vdist[nv] = tabs(dv.y) + (uint16_t(tabs(dv.x) + tabs(dv.z)) >> 4);

        // rotate
        dv = matvec(m, dv);
        dv.z = -dv.z;

        // save vertex
        vs[nv] = { dv.x, dv.y };
        fd.vz[nv] = dv.z;
    }

    // ball vertices (center and right side) and face
    uint8_t balli0, balli1; // ball face indices
    bool ball_valid = false;
    {
        dvec3 dv = ball;
        dv.x -= cam.x;
        dv.y -= cam.y;
        dv.z -= cam.z;
        //fdist -= (uint32_t(32) << 16); // small bias
        //fdist += (uint32_t(32) << 16); // small bias
        dv = matvec(m, dv);
        dv.z = -dv.z;

        dv.y += 32;
        dv.z -= 96;
        int32_t fdist = 0;
        fdist += int32_t(dv.x) * dv.x;
        fdist += int32_t(dv.y) * dv.y;
        fdist += int32_t(dv.z) * dv.z;
        fdist *= 3;

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
            face_order[nf] = 255;
            fd.fdist[nf] = uint16_t(uint32_t(fdist) >> 16);
            //fd.fdist[nf] = tabs(dv.y) + (uint16_t(tabs(dv.x) + tabs(dv.z)) >> 4);;
            nv += 2;
            nf += 1;
        }
    }

    // assemble faces and clip them to near plane
    for(uint8_t i = 0; i < num_faces; ++i)
    {
        if(nf + 1 > MAX_FACES)
            break;

        uint8_t const* fptr = &faces[i * 4];
        uint8_t i0 = pgm_read_byte(fptr + 0);
        uint8_t i1 = pgm_read_byte(fptr + 1);
        uint8_t i2 = pgm_read_byte(fptr + 2);
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
        uint16_t fdist = fd.vdist[i0] + fd.vdist[i1] + fd.vdist[i2];

        // clip: exactly two vertices behind near plane
        if((behind & (behind - 1)) != 0)
        {
            if(nv + 2 > MAX_VERTS)
                continue;
            if(nf + 1 > MAX_FACES)
                continue;
            if(nclipf + 4 > MAX_CLIP_FACES * 4)
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
            face_order[nf] = MAX_FACES + (nclipf >> 2);
            fd.fdist[nf] = fdist;
            ++nf;
            clip_faces[nclipf + 0] = ibase;
            clip_faces[nclipf + 1] = nv - 1;
            clip_faces[nclipf + 2] = nv - 2;
            clip_faces[nclipf + 3] = pgm_read_byte(fptr + 3);
            nclipf += 4;

            continue;
        }

        // clip: exactly one vertex behind near plane
        else if(behind != 0)
        {
            if(nv + 2 > MAX_VERTS)
                continue;
            if(nclipf + 8 > MAX_CLIP_FACES * 4)
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

            face_order[nf] = MAX_FACES + (nclipf >> 2);
            fd.fdist[nf] = fdist;
            ++nf;
            face_order[nf] = MAX_FACES + (nclipf >> 2) + 1;
            fd.fdist[nf] = fdist;
            ++nf;

            uint8_t pat = pgm_read_byte(fptr + 3);

            // TODO: backface culling issue here!

            clip_faces[nclipf + 0] = nv - 2;
            clip_faces[nclipf + 1] = ib;
            clip_faces[nclipf + 2] = ic;
            clip_faces[nclipf + 3] = pat;
            nclipf += 4;

            clip_faces[nclipf + 0] = nv - 2;
            clip_faces[nclipf + 1] = ic;
            clip_faces[nclipf + 2] = nv - 1;
            clip_faces[nclipf + 3] = pat;
            nclipf += 4;

            continue;
        }

        face_order[nf] = i;
        fd.fdist[nf] = fdist;
        ++nf;
    }

    // project vertices
    for(uint8_t i = 0; i < nv; ++i)
    {
        dvec3 dv = { vs[i].x, vs[i].y, fd.vz[i] };

        // multiply x and y by 4/z, i.e., multiply by 1/(64*dv.z)
        if(dv.z >= ZNEAR)
        {
#if 0
            // TODO: figure out how to make this divide better
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
#else
            if(dv.z >= 256)
            {
                uint16_t invz = inv16(dv.z);
                int32_t nx = int32_t(invz) * dv.x;
                int32_t ny = int32_t(invz) * dv.y;
                nx = tclamp<int32_t>(nx, (int32_t)INT16_MIN * 240 * 64, (int32_t)INT16_MAX * 240 * 64);
                ny = tclamp<int32_t>(ny, (int32_t)INT16_MIN * 240 * 64, (int32_t)INT16_MAX * 240 * 64);
                dv.x = int16_t(uint32_t(nx) >> 14);
                dv.y = int16_t(uint32_t(ny) >> 14);
            }
            else
            {
                uint16_t invz = inv8(dv.z);
                int32_t nx = int32_t(invz) * dv.x;
                int32_t ny = int32_t(invz) * dv.y;
                nx = tclamp<int32_t>(nx, (int32_t)INT16_MIN * 60, (int32_t)INT16_MAX * 60);
                ny = tclamp<int32_t>(ny, (int32_t)INT16_MIN * 60, (int32_t)INT16_MAX * 60);
                dv.x = int16_t(uint24_t(nx) >> 6);
                dv.y = int16_t(uint24_t(ny) >> 6);
            }
#endif
        }

        // center in framebuffer
        dv.x += (FBW / 2 * 16);
        dv.y = (FBH / 2 * 16) - dv.y;

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
            uint8_t f = face_order[i];
            uint8_t j = i;
            while(j > 0 && fd.fdist[j - 1] < d)
            {
                fd.fdist[j] = fd.fdist[j - 1];
                face_order[j] = face_order[j - 1];
                j -= 1;
            }
            fd.fdist[j] = d;
            face_order[j] = f;
            i += 1;
        }
    }
#endif

    // finally, render
    if(ball_valid)
        ball_valid = fd.vz[balli0] >= ZNEAR;
    clear_buf(); // buf mem was used during setup
    uint16_t ballr = 0;
    for(uint8_t i = 0; i < nf; ++i)
    {
        uint8_t t = face_order[i];
        uint8_t i0, i1, i2, pt;
        uint16_t j = uint16_t(t) * 4;
        if(t < MAX_FACES)
        {
            // normal face
            i0 = pgm_read_byte(&faces[j + 0]);
            i1 = pgm_read_byte(&faces[j + 1]);
            i2 = pgm_read_byte(&faces[j + 2]);
            pt = pgm_read_byte(&faces[j + 3]);
        }
        else if(t == 255)
        {
            // ball
            if(ball_valid)
            {
                dvec2 c = vs[balli0];
                ballr = uint16_t(vs[balli1].x - c.x);
                draw_ball_filled(vs[balli0], ballr);
            }
            continue;
        }
        else
        {
            // clip face
            i0 = clip_faces[j - MAX_FACES * 4 + 0];
            i1 = clip_faces[j - MAX_FACES * 4 + 1];
            i2 = clip_faces[j - MAX_FACES * 4 + 2];
            pt = clip_faces[j - MAX_FACES * 4 + 3];
        }
        draw_tri(vs[i0], vs[i1], vs[i2], pt);
    }
    if(ball_valid)
        draw_ball_outline(vs[balli0], ballr + 8);

#if PERFDOOM
    }
#endif

    return nf;
}

#ifndef ARDUINO
dvec3 transform_point(dvec3 dv)
{
    dmat3 m;
    rotation16(m, yaw, pitch);

    dv.x -= cam.x;
    dv.y -= cam.y;
    dv.z -= cam.z;

    dv = matvec(m, dv);
    dv.z = -dv.z;

    if(dv.z >= ZNEAR)
    {
        if(dv.z >= 256)
        {
            uint16_t invz = inv16(dv.z);
            int32_t nx = int32_t(invz) * dv.x;
            int32_t ny = int32_t(invz) * dv.y;
            nx = tclamp<int32_t>(nx, (int32_t)INT16_MIN * 240 * 64, (int32_t)INT16_MAX * 240 * 64);
            ny = tclamp<int32_t>(ny, (int32_t)INT16_MIN * 240 * 64, (int32_t)INT16_MAX * 240 * 64);
            dv.x = int16_t(uint32_t(nx) >> 14);
            dv.y = int16_t(uint32_t(ny) >> 14);
        }
        else
        {
            uint16_t invz = inv8(dv.z);
            int32_t nx = int32_t(invz) * dv.x;
            int32_t ny = int32_t(invz) * dv.y;
            nx = tclamp<int32_t>(nx, (int32_t)INT16_MIN * 60, (int32_t)INT16_MAX * 60);
            ny = tclamp<int32_t>(ny, (int32_t)INT16_MIN * 60, (int32_t)INT16_MAX * 60);
            dv.x = int16_t(uint24_t(nx) >> 6);
            dv.y = int16_t(uint24_t(ny) >> 6);
        }
    }

    dv.x += (FBW / 2 * 16);
    dv.y = (FBH / 2 * 16) - dv.y;

    return dv;
}
#endif
