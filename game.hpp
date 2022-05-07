#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

static constexpr uint16_t SAVE_VERSION = 1;

// platform functionality
void wait();        // wait about 50 ms
uint8_t wait_btn(); // wait for button press
void seed();        // sets rand_seed
void paint_offset(uint8_t x, bool clear = true);
uint8_t read_persistent(uint16_t addr);
void update_persistent(uint16_t addr, uint8_t data);
void flush_persistent(); // (does nothing for Arduino)

// game logic
void game_loop();

#if !(__cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1900))
#error "Building minigolf requires support for at least C++11"
#endif

#if defined(__GNUC__)
#define FORCEINLINE __attribute__((always_inline))
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

static constexpr uint8_t BTN_UP    = 0x80;
static constexpr uint8_t BTN_DOWN  = 0x10;
static constexpr uint8_t BTN_LEFT  = 0x20;
static constexpr uint8_t BTN_RIGHT = 0x40;
static constexpr uint8_t BTN_A     = 0x08;
static constexpr uint8_t BTN_B     = 0x04;
