#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

static constexpr uint8_t FBW = 128;
static constexpr uint8_t FBH = 64;

//8-bit grayscale buffer
#define BUFFER_8BIT 0

static constexpr uint16_t SAVE_VERSION = 1;

// platform functionality
uint16_t time_ms();
uint8_t poll_btn();
uint8_t read_persistent(uint16_t addr);
void update_persistent(uint16_t addr, uint8_t data);
void flush_persistent(); // (does nothing for Arduino)

// game logic
void game_loop();

#if !(__cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1900))
#error "Building minigolf requires support for at least C++11"
#endif

#if defined(__GNUC__)
#define FORCEINLINE __attribute__((always_inline)) inline
#define NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define FORCEINLINE __forceinline
#define NOINLINE __declspec(noinline)
#else
#define FORCEINLINE inline
#define NOINLINE
#endif

#ifdef ARDUINO
#include <Arduino.h>

#if 1
// http://michael-buschbeck.github.io/arduino/2013/10/20/string-merging-pstr/
// limitation: cannot use string literal concatenation: PSTR("hi " "there")
#undef PSTR
#define PSTR(str_) \
    (__extension__({ \
        char const* ptr;  \
        asm volatile \
        ( \
            ".pushsection .progmem.pstrs, \"SM\", @progbits, 1" "\n\t" \
            "PSTR%=: .string " #str_                            "\n\t" \
            ".type PSTR%= STT_OBJECT"                           "\n\t" \
            ".popsection"                                       "\n\t" \
            "ldi %A0, lo8(PSTR%=)"                              "\n\t" \
            "ldi %B0, hi8(PSTR%=)"                              "\n\t" \
            : "=d" (ptr) \
        ); \
        ptr; \
    }))
#endif

#define PSTR2(str_) \
    (__extension__({ static char const PROGSTR_[] PROGMEM = str_; PROGSTR_; }))

#else
#define PSTR(str_) str_
#define PSTR2(str_) str_
#define PROGMEM
inline uint8_t pgm_read_byte(void const* p) { return *(uint8_t*)p; }
inline uint16_t pgm_read_word(void const* p) { return *(uint16_t*)p; }
inline void const* pgm_read_ptr(void const* p) { return *(void const**)p; }
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
FORCEINLINE void swap(T& a, T& b)
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

static constexpr uint8_t BTN_UP    = 0x80;
static constexpr uint8_t BTN_DOWN  = 0x10;
static constexpr uint8_t BTN_LEFT  = 0x20;
static constexpr uint8_t BTN_RIGHT = 0x40;
static constexpr uint8_t BTN_A     = 0x08;
static constexpr uint8_t BTN_B     = 0x04;

static constexpr size_t BUF_BYTES = FBW * FBH / (BUFFER_8BIT ? 1 : 8);
#ifdef ARDUINO
extern uint8_t* const buf;
#else
extern array<uint8_t, BUF_BYTES> buf;
#endif

struct vec2  { int8_t  x, y; };
struct dvec2 { int16_t x, y; };
struct dvec3 { int16_t x, y, z; };
struct vec3  { int8_t  x, y, z; };

// draw.cpp
void draw_tri(dvec2 v0, dvec2 v1, dvec2 v2);
void draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1);

// sincos.cpp
int8_t fsin(uint8_t angle); // output is signed 1.7
int8_t fcos(uint8_t angle); // output is signed 1.7

int16_t fsin16(uint16_t angle); // output is signed 1.15
int16_t fcos16(uint16_t angle); // output is signed 1.15

static inline dvec2 frotate(dvec2 v, uint8_t angle)
{
    int8_t fs = fsin(angle);
    int8_t fc = fcos(angle);
    dvec2 r;
    r.x = int16_t(uint32_t(((int32_t)v.x * fc - (int32_t)v.y * fs)) >> 7);
    r.y = int16_t(uint32_t(((int32_t)v.y * fc + (int32_t)v.x * fs)) >> 7);
    return r;
}

static inline dvec2 frotate(vec2 v, uint8_t angle)
{
    int8_t fs = fsin(angle);
    int8_t fc = fcos(angle);
    dvec2 r;
    r.x = (v.x * fc - v.y * fs) / 8;
    r.y = (v.y * fc + v.x * fs) / 8;
    return r;
}

static inline dvec2 frotate16(dvec2 v, uint16_t angle)
{
    int16_t fs = fsin16(angle);
    int16_t fc = fcos16(angle);
    dvec2 r;
    r.x = ((int32_t)v.x * fc - (int32_t)v.y * fs) / 32767;
    r.y = ((int32_t)v.y * fc + (int32_t)v.x * fs) / 32767;
    return r;
}
