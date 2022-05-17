#include "game.hpp"

int16_t fmuls16(int16_t x, int16_t y)
{
    int32_t p = int32_t(x) * y;
    return int16_t(uint32_t(p) >> 15);
}

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

void rotation16(dmat3& m, uint16_t yaw, int16_t pitch)
{
    int16_t sinA = fsin16((uint16_t)pitch);
    int16_t cosA = fcos16((uint16_t)pitch);
    int16_t sinB = fsin16(yaw);
    int16_t cosB = fcos16(yaw);

    m[0] = cosB;
    m[1] = 0;
    m[2] = sinB;

    m[3] = fmuls16(sinA, sinB);
    m[4] = cosA;
    m[5] = -fmuls16(sinA, cosB);

    m[6] = -fmuls16(cosA, sinB);
    m[7] = sinA;
    m[8] = fmuls16(cosA, cosB);
}


/*
Rotation matrix:

A = pitch, B = yaw

 cosB   sinAsinB   cosAsinB
 0      cosA      -sinA
-sinB   sinAcosB   cosAcosB

*/

void rotation_phys(mat3& m, uint8_t yaw, int8_t pitch)
{
    int8_t sinA = fsin((uint8_t)pitch);
    int8_t cosA = fcos((uint8_t)pitch);
    int8_t sinB = fsin(yaw);
    int8_t cosB = fcos(yaw);

    m[0] = cosB;
    m[1] = fmuls8(sinA, sinB);
    m[2] = fmuls8(cosA, sinB);

    m[3] = 0;
    m[4] = cosA;
    m[5] = -sinA;

    m[6] = -sinB;
    m[7] = fmuls8(sinA, cosB);
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
    r.x = (u24(s24(v.x) * m[0] + s24(v.y) * m[1] + s24(v.z) * m[2]) << 1) >> 8;
    r.y = (u24(s24(v.x) * m[3] + s24(v.y) * m[4] + s24(v.z) * m[5]) << 1) >> 8;
    r.z = (u24(s24(v.x) * m[6] + s24(v.y) * m[7] + s24(v.z) * m[8]) << 1) >> 8;
    return r;
}

dvec3 matvec_t(mat3 m, dvec3 v)
{
    dvec3 r;
    r.x = (u24(s24(v.x) * m[0] + s24(v.y) * m[3] + s24(v.z) * m[6]) << 1) >> 8;
    r.y = (u24(s24(v.x) * m[1] + s24(v.y) * m[4] + s24(v.z) * m[7]) << 1) >> 8;
    r.z = (u24(s24(v.x) * m[2] + s24(v.y) * m[5] + s24(v.z) * m[8]) << 1) >> 8;
    return r;
}

dvec3 matvec(dmat3 m, dvec3 v)
{
    dvec3 r;
    r.x = (uint32_t(int32_t(v.x) * m[0] + int32_t(v.y) * m[1] + int32_t(v.z) * m[2]) << 1) >> 16;
    r.y = (uint32_t(int32_t(v.x) * m[3] + int32_t(v.y) * m[4] + int32_t(v.z) * m[5]) << 1) >> 16;
    r.z = (uint32_t(int32_t(v.x) * m[6] + int32_t(v.y) * m[7] + int32_t(v.z) * m[8]) << 1) >> 16;
    return r;
}


// find x such that (a*x*x) == (1<<24)
static uint16_t inv_sqrt(uint16_t a)
{
    uint16_t x = 0x100; // 1

    for(uint8_t i = 0; i < 8; ++i)
    {
        uint16_t t;
        t = uint16_t((u24(x) * x) >> 8); // x*x
        t = uint16_t((u24(a) * t) >> 8); // a*x*x
        t >>= 1;                         // 0.5 * a*x*x

        x = uint16_t(u24(s24(int16_t(0x180) - int16_t(t)) * x) >> 8);
    }

    return x;
}

dvec3 normalized(dvec3 v)
{
    // TODO: fit v.x into int8_t and propagate precision?
    //       would save a lot of ops
    while(tmax(tabs(v.x), tabs(v.y), tabs(v.z)) >= (1 << 8))
    {
        v.x /= 2;
        v.y /= 2;
        v.z /= 2;
    }
    uint24_t d2 = 0;
    d2 += u24(s24(v.x) * v.x);
    d2 += u24(s24(v.y) * v.y);
    d2 += u24(s24(v.z) * v.z);
    uint16_t d = inv_sqrt(uint16_t(d2 >> 8));
    v.x = int16_t(u24(s24(v.x) * d) >> 8);
    v.y = int16_t(u24(s24(v.y) * d) >> 8);
    v.z = int16_t(u24(s24(v.z) * d) >> 8);
    return v;
}

int16_t dot(dvec3 a, dvec3 b)
{
    int24_t r = 0;
    r += s24(a.x) * b.x;
    r += s24(a.y) * b.y;
    r += s24(a.z) * b.z;
    return int16_t(u24(r) >> 8);
}
