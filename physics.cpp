#include "game.hpp"

dvec3 ball;

static constexpr int16_t BALL_RADIUS = 256 * 0.5;

// velocity, scaled 256x unit: range is (-0.5, +0.5) units per step
// range is OK since the ball should never be moving 0.5 units per step anyway
dvec3 ball_vel;

static constexpr int16_t MAX_VEL = 256 * 64;

// gravity acceleration: y vel units per step
static constexpr int16_t GRAVITY = 96;

// ball restitution as a fraction of 256
static constexpr uint8_t RESTITUTION = 256 * 0.5;

static constexpr int8_t REST_MINUS_ONE = int8_t(RESTITUTION - 256);

// square unsigned (x >= 0)
static constexpr uint32_t FORCEINLINE squ(int16_t x)
{
    return uint32_t(x) * uint16_t(x);
}

static constexpr uint32_t BALL_RADIUS_SQ = squ(BALL_RADIUS);

uint32_t sqdist_point_aabb(dvec3 size, dvec3 pt)
{
    uint32_t d = 0;
    {
        int16_t pv = pt.x, bv = size.x;
        if(pv < -bv) d += squ(-bv - pv);
        if(pv >  bv) d += squ(pv - bv);
    }
    {
        int16_t pv = pt.y, bv = size.y;
        if(pv < -bv) d += squ(-bv - pv);
        if(pv >  bv) d += squ(pv - bv);
    }
    {
        int16_t pv = pt.z, bv = size.z;
        if(pv < -bv) d += squ(-bv - pv);
        if(pv >  bv) d += squ(pv - bv);
    }
    return d;
}

/*

Symbols:

en = normal elasticity = 1/2
et = frictional elasticity = 1
m  = ball mass = 1
I  = ball moment of inertia, I = about m/8 = 1/8

w1 = ball pre collision angular velocity
w2 = ball post collision angular velocity
v1 = ball pre collision velocity
v2 = ball post collision velocity
vp = ball contact point pre collision velocity
n  = collision normal
t  = frictional direction (unit)
rp = vector from ball center to collision point
Jn = normal impulse
Jt = frictional impulse
Jv = total impulse vector

Equations:

    vp = v1 + cross(w1, rp)
    Jn = -((1 + en) * dot(vp, n)) / (1/m + norm2(cross(rp, n))/I)
    Jt = -((1 + et) * dot(vp, t)) / (1/m + norm2(cross(rp, t))/I)
    Jv = Jn*n+Jt*t
    v2 = v1 + Jv/m
    w2 = w1 + cross(rp, Jv)/I

If m = 1, I = 1/8, en = 1/2, et = 1, rp = -(1/2)n, and t is orthogonal to rp:

    vp = v1 + 1/2cross(w1, n)
    Jv = -(3/2*dot(vp, n) + 1/3*dot(vp, t))    [note: factors are adjustable]
    v2 = v1 + Jv
    w2 = w1 + 8*cross(rp, Jv)

*/

static void physics_collision(phys_box b)
{
    mat3 m;

    dvec3 pt;
    pt.x = ball.x - b.pos.x;
    pt.y = ball.y - b.pos.y;
    pt.z = ball.z - b.pos.z;

    if(b.yaw != 0 || b.pitch != 0)
    {
        rotation_phys(m, b.yaw, b.pitch);
        // rotate pt
        pt = matvec_t(m, pt);
    }
    uint32_t d = sqdist_point_aabb(b.size, pt);

    // early exit if no collision
    if(d > BALL_RADIUS_SQ)
        return;

    //if(b.pitch != 0) __debugbreak();

#if 1

    // find contact point on box
    dvec3 cpt;
    cpt.x = tclamp<int16_t>(pt.x, -b.size.x, b.size.x);
    cpt.y = tclamp<int16_t>(pt.y, -b.size.y, b.size.y);
    cpt.z = tclamp<int16_t>(pt.z, -b.size.z, b.size.z);

    //if(b.pitch != 0) __debugbreak();

    // collision normal is difference between contact point and ball center
    dvec3 normal;
    normal.x = pt.x - cpt.x;
    normal.y = pt.y - cpt.y;
    normal.z = pt.z - cpt.z;
    normal = normalized(normal);

    if(b.yaw != 0 || b.pitch != 0)
    {
        // rotate normal
        normal = matvec(m, normal);
    }

    // ignore collision if normal aligned with current velocity
    // this happens if collision is processed but ball still penetrates
    int16_t normdot = dot(ball_vel, normal);
    if(normdot > 0)
        return;

    dvec3 absnormal;
    absnormal.x = tabs(normal.x);
    absnormal.y = tabs(normal.y);
    absnormal.z = tabs(normal.z);

    // hack: resolve penetration by moving ball back by velocity
    //       (assumption is that ball did not penetrate at previous step)
    ball.x -= int8_t(u24(s24(ball_vel.x + 128) * absnormal.x) >> 16);
    ball.y -= int8_t(u24(s24(ball_vel.y + 128) * absnormal.y) >> 16);
    ball.z -= int8_t(u24(s24(ball_vel.z + 128) * absnormal.z) >> 16);

    // reflect velocity across normal and apply restitution
    // v = v - (2*dot(v, n)) * n
    //if(0)
    {
        s24 d = s24(normdot) * 2;
#if 0
        // hack: to enable rolling up hills without angular velocity,
        //       retain full horizontal velocity
        ball_vel.x -= int16_t(u24(d * normal.x) >> 8);
        ball_vel.y -= int16_t(u24(d * normal.y) >> 8);
        ball_vel.z -= int16_t(u24(d * normal.z) >> 8);
        uint8_t restx = 255;
        uint8_t resty = int8_t(uint16_t(absnormal.y * REST_MINUS_ONE) >> 8) + 255;
        uint8_t restz = 255;
#else
        ball_vel.x -= int16_t(u24(d * normal.x) >> 8);
        ball_vel.y -= int16_t(u24(d * normal.y) >> 8);
        ball_vel.z -= int16_t(u24(d * normal.z) >> 8);
        uint8_t restx = int8_t(uint16_t(absnormal.x * REST_MINUS_ONE) >> 8) + 255;
        uint8_t resty = int8_t(uint16_t(absnormal.y * REST_MINUS_ONE) >> 8) + 255;
        uint8_t restz = int8_t(uint16_t(absnormal.z * REST_MINUS_ONE) >> 8) + 255;
#endif

        // apply restitution via absnormal: an*(R-1) + 1
        ball_vel.x = int16_t(u24(s24(ball_vel.x) * restx) >> 8);
        ball_vel.y = int16_t(u24(s24(ball_vel.y) * resty) >> 8);
        ball_vel.z = int16_t(u24(s24(ball_vel.z) * restz) >> 8);
    }

#else
    ball_vel.x = int16_t(u24(s24(-ball_vel.x) * RESTITUTION) >> 8);
    ball_vel.y = int16_t(u24(s24(-ball_vel.y) * RESTITUTION) >> 8);
    ball_vel.z = int16_t(u24(s24(-ball_vel.z) * RESTITUTION) >> 8);
#endif
}

void physics_step()
{
    // TODO: check collision and perform restitution here
    {
        phys_box const* boxes = pgmptr(&current_level->boxes);
        uint8_t num_boxes = pgm_read_byte(&current_level->num_boxes);
        for(uint8_t i = 0; i < num_boxes; ++i)
        {
            phys_box b;
            memcpy_P(&b, &boxes[i], sizeof(b));
            physics_collision(b);
        }
    }

    ball_vel.y -= GRAVITY;

    ball_vel.x = tclamp<int16_t>(ball_vel.x, -MAX_VEL, MAX_VEL);
    ball_vel.y = tclamp<int16_t>(ball_vel.y, -MAX_VEL, MAX_VEL);
    ball_vel.z = tclamp<int16_t>(ball_vel.z, -MAX_VEL, MAX_VEL);

    ball.x += int8_t(uint16_t(ball_vel.x + 128) >> 8);
    ball.y += int8_t(uint16_t(ball_vel.y + 128) >> 8);
    ball.z += int8_t(uint16_t(ball_vel.z + 128) >> 8);
}
