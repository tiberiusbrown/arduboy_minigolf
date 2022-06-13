#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

static constexpr int FBW = 1 * 128;
static constexpr int FBH = 1 * 64;

#ifndef ARDUGOLF_FX
#define ARDUGOLF_FX 1
#endif

#define USE_AVR_INLINE_ASM 1
#define AVOID_FMULSU 1

// draw ball outline behind walls
#define BALL_XRAY 1

// don't include button "B" icon behind audio graphic in main menu
#define SHORTENED_AUDIO_GRAPHIC 1

// save to flash chip instead of EEPROM
#define SAVE_TO_FLASH_CHIP ARDUGOLF_FX

// select optimizations that favor speed over size
#define OPT_SPEED_DIV_FRAC_S ARDUGOLF_FX

// rasterizer subpixel bits
#define FB_FRAC_BITS 3
static constexpr uint8_t FB_FRAC_COEF = 1 << FB_FRAC_BITS;
static constexpr uint8_t FB_FRAC_MASK = FB_FRAC_COEF - 1;

#define DEBUG_SERIAL 1

// platform functionality
uint16_t time_ms();
uint8_t poll_btns();
void save_audio_on_off();
void toggle_audio();
bool audio_enabled();

// game logic
void game_setup();
void game_loop();

static constexpr int FBR = FBH / 8;

#if !(__cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1900))
#error "Building minigolf requires support for at least C++11"
#endif

#if defined(__GNUC__)
#define FORCEINLINE __attribute__((always_inline)) inline
#define NOINLINE __attribute__((noinline))
#define CONST_FUNC __attribute__((const))
#elif defined(_MSC_VER)
#define FORCEINLINE __forceinline
#define NOINLINE __declspec(noinline)
#define CONST_FUNC
#else
#define FORCEINLINE inline
#define NOINLINE
#define CONST_FUNC
#endif

#define SERIAL_PRINT(...)
#ifdef ARDUINO

#include <Arduino.h>

#include <ArduboyTones.h>
extern ArduboyTones sound;
#define play_tone sound.tone

#if ARDUGOLF_FX
#include "ArduboyFX.h"
#endif

#define myassert(...)

#else

#define PSTR(str_) str_
#define PROGMEM
inline uint8_t pgm_read_byte(void const* p) { return *(uint8_t*)p; }
inline uint16_t pgm_read_word(void const* p) { return *(uint16_t*)p; }
inline void const* pgm_read_ptr(void const* p) { return *(void const**)p; }

#include <assert.h>
#define myassert assert

#include <string.h>
#define memcpy_P memcpy

#define play_tone(...)

#endif

#if !defined(__AVR__)
#undef USE_AVR_INLINE_ASM
#define USE_AVR_INLINE_ASM 0
#endif

// useful when T is a pointer type, like function pointer or char const*
template<class T>
FORCEINLINE T pgmptr(T const* p) { return (T)pgm_read_ptr(p); }

template<class T, size_t N>
struct array
{
    T d_[N];
    FORCEINLINE T* data() { return d_; }
    FORCEINLINE constexpr T const* data() const { return d_; }
    FORCEINLINE T& operator[](size_t i) { verify(i); return d_[i]; }
    FORCEINLINE constexpr T const& operator[](size_t i) const
    {
#if defined(_MSC_VER) && !defined(NDEBUG)
        verify(i);
#endif
        return d_[i];
    }
    FORCEINLINE constexpr size_t size() const { return N; }
    FORCEINLINE T* begin() { return d_; }
    FORCEINLINE constexpr T const* begin() const { return d_; }
    FORCEINLINE T* end() { return d_ + N; }
    FORCEINLINE constexpr T const* end() const { return d_ + N; }
private:
    void verify(size_t i) const
    {
        (void)i;
#if defined(_MSC_VER) && !defined(NDEBUG)
        if(i >= N) __debugbreak();
#endif
    }
};

template<class T>
FORCEINLINE void tswap(T& a, T& b)
{
    T c = a;
    a = b;
    b = c;
}

template<class T> FORCEINLINE T tmin(T a, T b) { return a < b ? a : b; }
template<class T> FORCEINLINE T tmax(T a, T b) { return a < b ? b : a; }

template<class T> FORCEINLINE T tmin(T a, T b, T c) { return tmin(tmin(a, b), c); }
template<class T> FORCEINLINE T tmax(T a, T b, T c) { return tmax(tmax(a, b), c); }

template<class T> FORCEINLINE T tclamp(T x, T a, T b) { return tmin(tmax(x, a), b); }

template<class T> FORCEINLINE T tabs(T x) { return x < 0 ? -x : x; }

inline uint8_t u8abs(uint8_t x)
{
    return (x & 0x80) ? -x : x;
}
inline uint8_t u8max(uint8_t a, uint8_t b)
{
    return a < b ? b : a;
}

// 1.7 x 1.7 -> 1.15
static FORCEINLINE int16_t fmuls(int8_t x, int8_t y)
{
#ifdef ARDUINO
    return __builtin_avr_fmuls(x, y);
#else
    return (x * y) << 1;
#endif
}

// 1.7 x 1.7 -> 1.7
static FORCEINLINE int8_t fmuls8(int8_t x, int8_t y)
{
#ifdef ARDUINO
    return int8_t((uint16_t)__builtin_avr_fmuls(x, y) >> 8);
#else
    return int8_t((x * y) >> 7);
#endif
}

static constexpr uint8_t BTN_UP    = 0x80;
static constexpr uint8_t BTN_DOWN  = 0x10;
static constexpr uint8_t BTN_LEFT  = 0x20;
static constexpr uint8_t BTN_RIGHT = 0x40;
static constexpr uint8_t BTN_A     = 0x08;
static constexpr uint8_t BTN_B     = 0x04;

static constexpr size_t BUF_BYTES = FBW * FBH / 8;
#ifdef ARDUINO
extern uint8_t* const buf;
#else
extern array<uint8_t, BUF_BYTES> buf;
#endif

static constexpr int16_t BALL_RADIUS = 256 * 0.5;
static constexpr int16_t FLAG_RADIUS = 256 * 1.0;

#ifdef ARDUINO
using int24_t = __int24;
using uint24_t = __uint24;
using s24 = int24_t;
using u24 = uint24_t;
#else
using __int24 = int32_t;
using __uint24 = uint32_t;
using int24_t = int32_t;
using uint24_t = uint32_t;

template<class T>
static int24_t s24(T x)
{
    return int24_t(x);
}
static int24_t s24(uint32_t x)
{
    myassert((x & 0xff800000) == 0);
    return int24_t(x);
}
static int24_t s24(int32_t x)
{
    if(x & 0x800000)
        myassert((x & 0xff000000) == 0xff000000);
    else
        myassert((x & 0xff000000) == 0);
    return int24_t(x);
}

template<class T>
static uint24_t u24(T x)
{
    return uint24_t(x);
}
static uint24_t u24(uint32_t x)
{
    myassert((x & 0xff000000) == 0);
    return uint24_t(x);
}
static uint24_t u24(int32_t x)
{
    if(x & 0x800000)
        myassert((x & 0xff000000) == 0xff000000);
    else
        myassert((x & 0xff000000) == 0);
    return uint24_t(x);
}
static uint24_t u24(int16_t x) { return u24(int32_t(x)); }
static uint24_t u24(int8_t x) { return u24(int32_t(x)); }
#endif

struct vec2  { int8_t  x, y; };
struct dvec2 { int16_t x, y; };
struct vec3  { int8_t  x, y, z; };
struct uvec3 { uint8_t x, y, z; };
struct dvec3 { int16_t x, y, z; };
using mat3 = array<int8_t, 9>;
using dmat3 = array<int16_t, 9>;

static constexpr uint8_t BOX_SIZE_FACTOR = 16;
static constexpr uint8_t BOX_POS_FACTOR = 64;
struct phys_box
{
    uvec3 size; // half extents
    vec3 pos;
    uint8_t yaw;
    int8_t pitch;
};

static constexpr uint8_t MAX_BOXES = 32;
static constexpr uint8_t MAX_VERTS = 100;
static constexpr uint8_t MAX_FACES = 100;

// max verts/faces for a level (ensures room for ball/flag data)
static constexpr uint8_t LEVEL_MAX_VERTS = MAX_VERTS - 8;
static constexpr uint8_t LEVEL_MAX_FACES = MAX_FACES - 4;

struct level_info_ext
{
    uint8_t num_verts;
    uint8_t pat_faces[5];
    uint8_t num_faces;
    uint8_t num_boxes;
    dvec3   ball_pos;
    dvec3   flag_pos;
    uint8_t par;
};

struct level_info
{
    int8_t const*   verts;
    uint8_t const*  faces;
    phys_box const* boxes;
    level_info_ext  ext;
};

struct fx_level_header
{
    uint8_t            num_holes;
    array<uint8_t, 18> pars;
    array<char, 32>    name;
    array<char, 32>    author;
    array<char, 128>   desc;
};
static_assert(sizeof(fx_level_header) <= 256, "");

struct fx_level_info
{
    array<int8_t  , MAX_VERTS * 1> vy;
    array<uint8_t , 8 * 2        > vdist_start;
    array<int8_t  , MAX_VERTS * 2> vxz;
    array<uint8_t , 4 * 2        > fdist_start;
    array<uint8_t , MAX_FACES * 3> faces;
    array<phys_box, MAX_BOXES    > boxes;
    level_info_ext                 ext;
};
static constexpr size_t SIZEOF_FX_LEVEL_INFO = sizeof(fx_level_info);
static_assert(SIZEOF_FX_LEVEL_INFO <= BUF_BYTES, "");
#define fxlevel (*(fx_level_info*)&buf[0])

// reused buffer data
#define buf_vy    ((int8_t  *)(&buf[offsetof(fx_level_info, vy)]))
#define buf_vxz   ((int8_t  *)(&buf[offsetof(fx_level_info, vxz)]))
#define buf_vdist ((uint16_t*)(&buf[offsetof(fx_level_info, vdist_start)]))
#define buf_faces ((uint8_t *)(&buf[offsetof(fx_level_info, faces)]))
#define buf_fdist ((uint16_t*)(&buf[offsetof(fx_level_info, fdist_start)]))
#define buf_boxes ((phys_box*)(&buf[offsetof(fx_level_info, boxes)]))
#define buf_tvz   ((int16_t *)(buf_boxes))

// ensure buf_vz fits in buf
static_assert(offsetof(fx_level_info, boxes) + sizeof(int16_t) * MAX_VERTS <= BUF_BYTES, "");

// game.cpp
enum class st : uint8_t
{
    TITLE,      // title screen
    LEVEL,      // circle around the level for a bit
    AIM,        // aim the ball for a shot
    ROLLING,    // watch the ball after a shot
    HOLE,       // ball went into hole
    SCORE,      // viewing score card
    MENU,       // in-game menu
    OVERVIEW,
    PITCH,      // pitch adjustment
    FX_COURSE,  // select course (FX only)
};
extern st state;
extern uint8_t nframe;
extern uint16_t yaw_aim;
extern uint8_t power_aim;
extern array<uint8_t, 18> shots;
extern uint8_t leveli;
#if ARDUGOLF_FX
extern uint8_t fx_course;
uint24_t get_hole_fx_addr(uint8_t i);
void fx_read_data_bytes(uint24_t addr, uint8_t* dst, size_t n);
#else
extern level_info const* current_level;
#endif
extern level_info_ext levelext;
bool ball_in_hole();
void set_level(uint8_t index);
void move_forward(int16_t amount);
void move_right(int16_t amount);
void move_up(int16_t amount);
void look_up(int16_t amount);
void look_right(int16_t amount);
void load_level_from_prog(); // loads current_level

// physics.cpp
extern dvec3 ball;         // position
extern dvec3 ball_vel;     // velocity
extern dvec3 ball_vel_ang; // angular velocity
bool physics_step(); // returns true if ball has stopped

// camera.cpp
extern dvec3 cam;
extern uint16_t yaw;
extern int16_t  pitch;
uint16_t yaw_to_flag();
void update_camera(
    dvec3 tcam, uint16_t tyaw, int16_t tpitch,
    uint8_t move_speed, uint8_t look_speed);
void update_camera_look_at(
    dvec3 tlookat, uint16_t tyaw, int16_t tpitch, uint16_t dist,
    uint8_t move_speed, uint8_t look_speed);
void update_camera_look_at_fastangle(
    dvec3 tlookat, uint16_t tyaw, int16_t tpitch, uint16_t dist,
    uint8_t move_speed, uint8_t look_speed);
void update_camera_follow_ball(
    uint16_t dist,
    uint8_t move_speed, uint8_t look_speed);
void update_camera_reset_velocities();

// draw.cpp
int16_t interp(int16_t a, int16_t b, int16_t c, int16_t x, int16_t z);
void set_pixel(uint8_t x, uint8_t y);
void inv_pixel(uint8_t x, uint8_t y);
void clear_pixel(uint8_t x, uint8_t y);
void draw_tri(dvec2 v0, dvec2 v1, dvec2 v2, uint8_t pati);
void draw_ball_filled(dvec2 c, uint16_t r, uint16_t pat);
void draw_ball_outline(dvec2 c, uint16_t r);

// render_scene.cpp
struct face { uint8_t i0, i1, i2, pt; };
extern array<face, MAX_FACES> fs;
extern array<dvec2, MAX_VERTS> vs;
#if ARDUGOLF_FX
// used for displaying course info
#define fxcourseinfo (*(fx_level_header*)&vs)
static_assert(sizeof(fx_level_header) <= sizeof(vs), "");
#endif
void clear_buf();
uint8_t render_scene();
#ifndef ARDUINO
uint8_t render_scene_persp();
uint8_t render_scene_ortho(int zoom);
dvec3 transform_point(dvec3 dv, bool ortho, int ortho_zoom);
#endif

// sincos.cpp
int8_t fsin(uint8_t angle) CONST_FUNC; // output is signed 1.7
int8_t fcos(uint8_t angle) CONST_FUNC; // output is signed 1.7
int16_t fsin16(uint16_t angle);
int16_t fcos16(uint16_t angle);
int16_t atan2(int16_t y, int16_t x);

// mat.cpp
void rotation(mat3& m, uint8_t yaw, int8_t pitch);
void rotation16(dmat3& m, uint16_t yaw, int16_t pitch);
void rotation_phys(mat3& m, uint8_t yaw, int8_t pitch);
dvec3 matvec  (mat3 m, vec3  v);
dvec3 matvec_t(mat3 m, vec3  v); // transpose
dvec3 matvec  (mat3 m, dvec3 v);
dvec3 matvec_t(mat3 m, dvec3 v); // transpose
dvec3 matvec (dmat3 m, dvec3 v);
dvec3 normalized(dvec3 v);       // normalize to 8.8
int16_t dot(dvec3 a, dvec3 b);

// div.cpp
uint16_t inv8(uint8_t x)   CONST_FUNC;   // approximates 2^16 / x
uint16_t inv16(uint16_t x) CONST_FUNC; // (x >= 256) approximates 2^24 / x

// mul.cpp
// key: mul_f[shift]_[dst]
//      shift: fraction bits / right shift of product
//      dst:   type of destination
int16_t  mul_f7_s16 (int16_t  a, int8_t   b);
int16_t  mul_f8_s16 (int16_t  a, uint8_t  b);
int16_t  mul_f8_s16 (int16_t  a, uint16_t b);
int16_t  mul_f8_s16 (int16_t  a, int16_t  b);
uint16_t mul_f8_u16 (uint16_t a, uint8_t  b);
uint16_t mul_f8_u16 (uint16_t a, uint16_t b);
int16_t  mul_f15_s16(int16_t  a, int16_t  b);
int16_t  mul_f16_s16(int16_t  a, int16_t  b);

// graphics.cpp
// draw_graphic
//    r: 8-pixel row index (0-7)
//    c: which column (0-127)
//    h: graphic height in rows
//    w: graphic width
enum
{
    GRAPHIC_OVERWRITE,
    GRAPHIC_SET,
    GRAPHIC_CLEAR,
};
void draw_graphic(
    uint8_t const* p,
    uint8_t r, uint8_t c,
    uint8_t h, uint8_t w,
    uint8_t op);
void set_number(uint8_t n, uint8_t r, uint8_t c);
void set_number2(uint8_t n, uint8_t r, uint8_t c);
void set_number3(uint16_t n, uint8_t r, uint8_t c);
extern uint8_t const GFX_INFO_BAR[] PROGMEM;
extern uint8_t const GFX_POWER   [] PROGMEM;
extern uint8_t const GFX_TITLE   [] PROGMEM;
extern uint8_t const GFX_SUBTITLE[] PROGMEM;
extern uint8_t const GFX_NEXT    [] PROGMEM;
extern uint8_t const GFX_QUIT    [] PROGMEM;
extern uint8_t const GFX_MENU    [] PROGMEM;
extern uint8_t const GFX_ARROW   [] PROGMEM;
extern uint8_t const GFX_HIO     [] PROGMEM;
extern uint8_t const GFX_AUDIO   [] PROGMEM;
extern uint8_t const GFX_ARROWS_H[] PROGMEM;
extern uint8_t const GFX_ARROWS_V[] PROGMEM;

// save.cpp
struct course_save_data
{
    array<char, 8>     ident; // "ARDUGOLF"
    array<uint8_t, 18> best_game;
    array<uint8_t, 18> best_holes;
    uint16_t           num_played;

#if SAVE_TO_FLASH_CHIP
    uint8_t padding[64 - 8 - 18 - 18 - 2 - 2];
#endif

    uint16_t           checksum;
};
#if SAVE_TO_FLASH_CHIP
static_assert(sizeof(course_save_data) == 64, "");
#endif
#define savedata (*(course_save_data*)&vs[0])
void load();
void save();
uint16_t checksum();

#if ARDUGOLF_FX
// font.cpp
uint8_t draw_char(uint8_t x, uint8_t y, char c);
void draw_text(uint8_t x, uint8_t y, char const* p);
void draw_text_nonprog(uint8_t x, uint8_t y, char const* p);
uint8_t char_width(char c);
uint8_t text_width(char const* p);
uint8_t text_width_nonprog(char const* p);
#endif

static FORCEINLINE int16_t div_frac_s(int16_t x)
{
#ifdef __AVR__
#if OPT_SPEED_DIV_FRAC_S && FB_FRAC_BITS >= 3
    // avr-gcc generates a loop for asr of 3 and up
    // below costs one extra instruction but saves 9 cycles per invocation
    asm volatile(
        "asr  %B[x]      \n\t"
        "ror  %A[x]      \n\t"
        "asr  %B[x]      \n\t"
        "ror  %A[x]      \n\t"
        "asr  %B[x]      \n\t"
        "ror  %A[x]      \n\t"
        : [x] "+r" (x)
        :
        :
    );
    return x;
#else
    return x >> FB_FRAC_BITS;
#endif
#else
    constexpr uint16_t MASK = uint16_t(0xffff << (16 - FB_FRAC_BITS));
    uint16_t r = (uint16_t)x >> FB_FRAC_BITS;
    if(x < 0) r |= MASK;
    return (int16_t)r;
#endif
}

static inline int8_t hibyte(int16_t x)
{
    return int8_t(uint16_t(x) >> 8);
}
