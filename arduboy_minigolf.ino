#include "game.hpp"

#include <SPI.h>
#include <EEPROM.h>
#include <Arduboy2.h>
#include <Arduino.h>

Arduboy2 a;
uint8_t* const buf = Arduboy2Base::sBuffer;

uint16_t time_ms()
{
    return (uint16_t)millis();
}

uint8_t read_persistent(uint16_t addr)
{
    return EEPROM.read((int)addr);
}

void update_persistent(uint16_t addr, uint8_t data)
{
    EEPROM.update((int)addr, data);
}

void flush_persistent() {}

uint8_t poll_btns()
{
    return Arduboy2::buttonsState();
}

#define REMOVE_USB 0
#if REMOVE_USB
int main()
#else
void setup()
#endif
{
    Arduboy2Base::boot();
#if REMOVE_USB

    if(Arduboy2Core::buttonsState() & DOWN_BUTTON)
    {
        Arduboy2Core::exitToBootloader();
    }
#endif
    if(Arduboy2Core::buttonsState() & UP_BUTTON)
    {
        Arduboy2Core::sendLCDCommand(OLED_ALL_PIXELS_ON);
        Arduboy2Core::digitalWriteRGB(RGB_ON, RGB_ON, RGB_ON);
        Arduboy2Core::digitalWriteRGB(RED_LED, RGB_ON);
        power_timer0_disable();
        for(;;) Arduboy2Core::idle();
    }

    a.setFrameRate(30);
    uint16_t pt = time_ms();
    uint16_t dt = 0;
    uint16_t fps = 0;
    uint16_t nf = 0;
    for(;;)
    {
        if (WDTCSR & _BV(WDE))
        {
            // disable ints and set magic key
            cli();
            *(volatile uint8_t*)0x800 = 0x77;
            *(volatile uint8_t*)0x801 = 0x77;
            for(;;);
        }
      
        //while(!a.nextFrame())
            ;
        game_loop();
        uint16_t t = time_ms();
        uint16_t tdt = t - pt;
        pt = t;
        dt += tdt;
        ++nf;
        if(dt >= 250)
        {
            fps = uint32_t(1000) * nf / dt;
            nf = 0;
            dt = 0;
        }

        a.setCursor(0, 0);
        uint16_t f = fps;
        a.write('0' + f / 100);
        f %= 100;
        a.write('0' + f /  10);
        f %=  10;
        a.write('0' + f);
        Arduboy2Base::display(true);
    }
  
#if REMOVE_USB
    return 0;
#endif
}
