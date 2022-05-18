#include "game.hpp"

dvec3 ball;

// velocity, scaled 256x unit: range is (-0.5, +0.5) units per step
// range is OK since the ball should never be moving 0.5 units per step anyway
dvec3 ball_vel;

dvec3 ball_vel_ang;

// 0.25 units per step
static constexpr int16_t MAX_VEL = 256 * 64;

// gravity acceleration: y vel units per step
static constexpr int16_t GRAVITY = 96;

// ball restitution as a fraction of 256. min 0.5
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

    vp = v1 - 1/2cross(w1, n)
    Jv = -3/2*dot(vp, n)*n -1/3*dot(vp, t)*t    [note: factors are adjustable]
    v2 = v1 + Jv
    w2 = w1 - 4*cross(n, Jv)

*/

static dvec3 cross(dvec3 a, dvec3 b)
{
    // function designed to be used with at least one near unit-length arg
    dvec3 r;
    r.x = int16_t(u24(s24(a.y) * b.z - s24(a.z) * b.y) >> 8);
    r.y = int16_t(u24(s24(a.z) * b.x - s24(a.x) * b.z) >> 8);
    r.z = int16_t(u24(s24(a.x) * b.y - s24(a.y) * b.x) >> 8);
    return r;
}

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

    // find contact point on box
    dvec3 cpt;
    cpt.x = tclamp<int16_t>(pt.x, -b.size.x, b.size.x);
    cpt.y = tclamp<int16_t>(pt.y, -b.size.y, b.size.y);
    cpt.z = tclamp<int16_t>(pt.z, -b.size.z, b.size.z);

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

    // tangent vector is rejection of velocity from normal
    dvec3 tangent;
    tangent.x = ball_vel.x - int16_t(u24(s24(normdot) * normal.x) >> 8);
    tangent.y = ball_vel.y - int16_t(u24(s24(normdot) * normal.y) >> 8);
    tangent.z = ball_vel.z - int16_t(u24(s24(normdot) * normal.z) >> 8);
    tangent = normalized(tangent);

    // contact point velocity
    dvec3 vp;
    {
        dvec3 t = cross(ball_vel_ang, normal);
        vp.x = ball_vel.x - t.x / 2;
        vp.y = ball_vel.y - t.y / 2;
        vp.z = ball_vel.z - t.z / 2;
    }

    // impulse factor
    dvec3 Jv;
    {
        int16_t t0 = -dot(vp, normal) * 3 / 2;
        int16_t t1 = -dot(vp, tangent);
        t1 = int16_t(u24(s24(t1) * (256 / 3)) >> 8);
        Jv.x  = int16_t(u24(s24(t0) *  normal.x) >> 8);
        Jv.y  = int16_t(u24(s24(t0) *  normal.y) >> 8);
        Jv.z  = int16_t(u24(s24(t0) *  normal.z) >> 8);
        Jv.x += int16_t(u24(s24(t1) * tangent.x) >> 8);
        Jv.y += int16_t(u24(s24(t1) * tangent.y) >> 8);
        Jv.z += int16_t(u24(s24(t1) * tangent.z) >> 8);
    }

    // velocity update
    ball_vel.x += Jv.x;
    ball_vel.y += Jv.y;
    ball_vel.z += Jv.z;

    // angular velocity update
    {
        dvec3 t = cross(Jv, normal);
        ball_vel_ang.x += t.x * 4;
        ball_vel_ang.y += t.y * 4;
        ball_vel_ang.z += t.z * 4;
    }
}

static void main_step()
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

void physics_step()
{
    for(uint8_t i = 0; i < 2; ++i)
    {
        main_step();
        main_step();

        // angular damping
        ball_vel_ang.x = int16_t(u24(s24(ball_vel_ang.x) * 255) >> 8);
        ball_vel_ang.y = int16_t(u24(s24(ball_vel_ang.y) * 255) >> 8);
        ball_vel_ang.z = int16_t(u24(s24(ball_vel_ang.z) * 255) >> 8);
    }
}
