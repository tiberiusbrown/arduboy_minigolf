#include "game.hpp"

/*
Rotation matrix:

A = pitch, B = yaw

     cosB      0       sinB
 sinAsinB   cosA  -sinAcosB
-cosAsinB  -sinA   cosAcosB

*/

void rotation(mat3& m, uint8_t yaw, uint8_t pitch)
{
    int8_t sinA = fsin(pitch);
    int8_t cosA = fcos(pitch);
    int8_t sinB = fsin(yaw);
    int8_t cosB = fcos(yaw);

    m[0] = cosB;
    m[1] = 0;
    m[2] = sinB;

    m[3] = fmuls8(sinA, sinB);
    m[4] = cosA;
    m[5] = -fmuls8(sinA, cosB);

    m[6] = -fmuls8(cosA, sinB);
    m[7] = -sinA;
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

dvec3 matvec(mat3 m, dvec3 v)
{
    dvec3 r;
    r.x = ((s24(v.x) * m[0] + s24(v.y) * m[1] + s24(v.z) * m[2]) << 1) >> 8;
    r.y = ((s24(v.x) * m[3] + s24(v.y) * m[4] + s24(v.z) * m[5]) << 1) >> 8;
    r.z = ((s24(v.x) * m[6] + s24(v.y) * m[7] + s24(v.z) * m[8]) << 1) >> 8;
    return r;
}
