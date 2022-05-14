#include "game.hpp"

/*
Rotation matrix:

A = pitch, B = yaw

     cosB      0       sinB
 sinAsinB   cosA  -sinAcosB
-cosAsinB   sinA   cosAcosB

*/

void rotation(mat3& m, uint8_t yaw, int8_t pitch)
{
    int8_t sinA = fsin((uint8_t)pitch);
    int8_t cosA = fcos((uint8_t)pitch);
    int8_t sinB = fsin(yaw);
    int8_t cosB = fcos(yaw);

    m[0] = cosB;
    m[1] = 0;
    m[2] = sinB;

    m[3] = fmuls8(sinA, sinB);
    m[4] = cosA;
    m[5] = -fmuls8(sinA, cosB);

    m[6] = -fmuls8(cosA, sinB);
    m[7] = sinA;
    m[8] = fmuls8(cosA, cosB);
}

dvec3 matvec(mat3 m, vec3 v)
{
    dvec3 r;
    r.x = v.x * m[0] + v.y * m[1] + v.z * m[2];
    r.y = v.x * m[3] + v.y * m[4] + v.z * m[5];
    r.z = v.x * m[6] + v.y * m[7] + v.z * m[8];
    return r;
}

dvec3 matvec_t(mat3 m, vec3 v)
{
    dvec3 r;
    r.x = v.x * m[0] + v.y * m[3] + v.z * m[6];
    r.y = v.x * m[1] + v.y * m[4] + v.z * m[7];
    r.z = v.x * m[2] + v.y * m[5] + v.z * m[8];
    return r;
}

dvec3 matvec(mat3 m, dvec3 v)
{
    dvec3 r;
    r.x = ((s24(v.x) * m[0] + s24(v.y) * m[1] + s24(v.z) * m[2]) << 1) >> 8;
    r.y = ((s24(v.x) * m[3] + s24(v.y) * m[4] + s24(v.z) * m[5]) << 1) >> 8;
    r.z = ((s24(v.x) * m[6] + s24(v.y) * m[7] + s24(v.z) * m[8]) << 1) >> 8;
    return r;
}

dvec3 matvec_t(mat3 m, dvec3 v)
{
    dvec3 r;
    r.x = ((s24(v.x) * m[0] + s24(v.y) * m[3] + s24(v.z) * m[6]) << 1) >> 8;
    r.y = ((s24(v.x) * m[1] + s24(v.y) * m[4] + s24(v.z) * m[7]) << 1) >> 8;
    r.z = ((s24(v.x) * m[2] + s24(v.y) * m[5] + s24(v.z) * m[8]) << 1) >> 8;
    return r;
}

// find x such that (a*x*x) == (1<<24)
static uint16_t inv_sqrt(uint16_t a)
{
    uint16_t x = 0x100; // 1

    for(int i = 0; i < 8; ++i)
    {
        uint16_t t;
        t = uint16_t((u24(x) * x) >> 8); // x*x
        t = uint16_t((u24(a) * t) >> 8); // a*x*x
        t >>= 1;                         // 0.5 * a*x*x

        x = uint16_t(u24(u24(x) * int16_t(0x180 - t)) >> 8);
    }

    return x;
}

dvec3 normalized(dvec3 v)
{
    u24 d2 = 0;
    d2 += u24(v.x) * v.x;
    d2 += u24(v.y) * v.y;
    d2 += u24(v.z) * v.z;
    uint16_t d = inv_sqrt(uint16_t(d2 >> 8));
    v.x = int16_t(u24(s24(v.x) * d) >> 8);
    v.y = int16_t(u24(s24(v.y) * d) >> 8);
    v.z = int16_t(u24(s24(v.z) * d) >> 8);
    return v;
}

int16_t dot(dvec3 a, dvec3 b)
{
    s24 r = 0;
    r += s24(a.x) * b.x;
    r += s24(a.y) * b.y;
    r += s24(a.z) * b.z;
    return int16_t(u24(r) >> 8);
}
