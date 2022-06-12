#include "game.hpp"

#if !SAVE_TO_FLASH_CHIP

static constexpr uint16_t EEPROM_START_ADDRESS = 700;

#if defined(ARDUINO)
#include <avr/eeprom.h>
static uint8_t eeprom_read(uint16_t i)
{
    return eeprom_read_byte((uint8_t const*)(uintptr_t)i);
}
static void eeprom_update(uint16_t i, uint8_t d)
{
    eeprom_update_byte((uint8_t*)(uintptr_t)i, d);
}
#else
static array<uint8_t, 1024> eeprom_data;
static uint8_t eeprom_read(uint16_t i) { return eeprom_data[i]; }
static void eeprom_update(uint16_t i, uint8_t d) { eeprom_data[i] = d; }
#endif
#else

#if !defined(ARDUINO)
#include <FX_SAVE.hpp>
#endif

static void fx_read_save_bytes(uint24_t addr, void* p, size_t n)
{
#ifdef ARDUINO
    FX::readSaveBytes(addr, (uint8_t*)p, n);
#else
    memcpy(p, &FX_SAVE[addr], n);
#endif
}

static void fx_erase_save_block(uint16_t page)
{
#ifdef ARDUINO
    FX::eraseSaveBlock(page);
#else
    page &= 0xfff0;
    myassert(page * 256 + 4096 <= sizeof(FX_SAVE));
    memset(&FX_SAVE[page * 256], 0xff, 4096);
#endif
}

static void fx_write_save_page(uint16_t page, void const* p)
{
#ifdef ARDUINO
    FX::writeSavePage(page, (uint8_t const*)p);
#else
    myassert(page * 256 + 256 <= sizeof(FX_SAVE));
    for(int i = 0; i < 256; ++i)
        FX_SAVE[page * 256 + i] &= ((uint8_t const*)p)[i];
#endif
}

#endif

uint16_t checksum()
{
    // CRC16
    uint8_t x;
    uint16_t crc = 0xffff;
    for(uint16_t i = 0; i < sizeof(course_save_data) - 2; ++i)
    {
        x = (crc >> 8) ^ ((uint8_t*)&savedata)[i];
        x ^= x >> 4;
        crc = (crc << 8) ^
            (uint16_t(x) << 12) ^
            (uint16_t(x) << 5) ^
            (uint16_t(x) << 0);
    }
    return crc;
}

void load()
{
#if SAVE_TO_FLASH_CHIP
    fx_read_save_bytes(fx_course * 64, &savedata, 64);
#else
    for(uint8_t i = 0; i < sizeof(course_save_data); ++i)
        ((uint8_t*)&savedata)[i] = eeprom_read(EEPROM_START_ADDRESS + i);
#endif
}

void save()
{
    // refuse to save unless ident matches "ARDUGOLF"
    {
        static char const IDENT[8] PROGMEM =
        {
            'A', 'R', 'D', 'U', 'G', 'O', 'L', 'F'
        };
        for(uint8_t i = 0; i < 8; ++i)
            if(savedata.ident[i] != (char)pgm_read_byte(&IDENT[i]))
                return;
    }

    // refuse to save unless checksum matches
    if(checksum() != savedata.checksum)
        return;

#if SAVE_TO_FLASH_CHIP

    static_assert(sizeof(fs) >= 256, "");

    // save pages on flash chip must be erased in blocks of 4KB
    // before being rewritten. to do this we keep two duplicate blocks,
    // erase one, copy from the other while updating relevant page,
    // then copy back to maintain the duplicate

    uint8_t page = fx_course >> 2;
    uint8_t index = fx_course << 6;

    // 1. erase first block
    fx_erase_save_block(0);

    // 2. copy from second block to first block (modify relevant page)
    for(uint8_t p = 0; p < 16; ++p)
    {
        uint8_t* pd = (uint8_t*)&fs[0];

        // read page from second block
        fx_read_save_bytes(4096 + p * 256, pd, 256);

        // overwrite page data if necessary
        if(p == page)
            memcpy(&pd[index], &savedata, 64);

        // write page to first block
        fx_write_save_page(p, pd);
    }

    // 3. erase second block
    fx_erase_save_block(16);

    // 4. copy from first block to second block
    for(uint8_t p = 0; p < 16; ++p)
    {
        uint8_t* pd = (uint8_t*)&fs[0];

        // read page from first block and write to second block
        fx_read_save_bytes(p * 256, pd, 256);
        fx_write_save_page(p + 16, pd);
    }

#else
    for(uint8_t i = 0; i < sizeof(course_save_data); ++i)
         eeprom_update(EEPROM_START_ADDRESS + i, ((uint8_t*)&savedata)[i]);
#endif
}
