#include "game.hpp"

/*
Rotation matrix:

A = yaw, B = pitch

 cosA   sinAsinB   sinAcosB
    0       cosB      -sinB
-sinA   cosAsinB   cosAcosB

 cosAcosB  -sinA   cosAsinB
 sinAcosB   cosA   sinAsinB
    -sinB      0       cosB

*/

void rotation(mat3& m, uint8_t yaw, uint8_t pitch)
{
    int8_t sinA = fsin(yaw);
    int8_t cosA = fcos(yaw);
    int8_t sinB = fsin(pitch);
    int8_t cosB = fcos(pitch);

    m[0] = cosA;
    m[1] = fmuls8(sinA, sinB);
    m[2] = fmuls8(sinA, cosB);

    m[3] = 0;
    m[4] = cosB;
    m[5] = -sinB;

    m[6] = -sinA;
    m[7] = fmuls8(cosA, sinB);
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
