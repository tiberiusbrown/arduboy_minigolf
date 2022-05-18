#include "game.hpp"

struct ctrl
{
    int16_t dydt;

    // x: target
    // y: current state
    // return: next state
    int16_t step(int16_t y, int16_t x)
    {
        int16_t diff = x - y;
        int16_t d2ydt = diff / 4 - dydt;
        y += dydt / 4;
        dydt += d2ydt / 4;
        return y;
    }
};

static ctrl ctrl_cam[5];

void update_camera(dvec3 tcam, uint16_t tyaw, int16_t tpitch)
{
    cam.x = ctrl_cam[0].step(cam.x, tcam.x);
    cam.y = ctrl_cam[1].step(cam.y, tcam.y);
    cam.z = ctrl_cam[2].step(cam.z, tcam.z);
    yaw = ctrl_cam[3].step(yaw, tyaw);
}

void camera_follow_ball()
{
    uint16_t tyaw = atan2(ball_vel.x, ball_vel.z) + 8192;
    dvec3 tcam = ball;
    tcam.y += 256 * 4;
    {
        int16_t fs = fsin16(tyaw);
        int16_t fc = fcos16(tyaw);
        tcam.x -= int8_t(uint16_t(fc) >> 8) * 12;
        tcam.z -= int8_t(uint16_t(fs) >> 8) * 12;
    }
    tyaw += 16384;
    int16_t tpitch = 1000; // TODO

    update_camera(tcam, tyaw, tpitch);
}
