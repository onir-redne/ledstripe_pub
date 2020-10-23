#ifndef _SAVEDCOLORS_H
#define _SAVEDCOLORS_H

#include <Arduino.h>
#include <inttypes.h>
#include <cstring>
#include "LittleFS.h"
#include "config.h"

/////////////////////////////////
// saved colors
#define MAX_SAVED_COLORS        64
#define COLOR_NAME_LEN          17
#define COLOR_SAVE_BYTES        7
#define COLOR_SAVE_R            0
#define COLOR_SAVE_G            1
#define COLOR_SAVE_B            2
#define COLOR_SAVE_H            3
#define COLOR_SAVE_S            4
#define COLOR_SAVE_V            5
#define COLOR_SAVE_SET          6
#define COLOR_SAVE_NAME         7

#define COLOR_RECORD_LEN        COLOR_NAME_LEN + COLOR_SAVE_BYTES

using namespace std;

class SavedColors
{
public:
    SavedColors(const String& file_name) :
    free_colors(0)
    {
        fs::File file = LittleFS.open(file_name, "r");
        if(file.isFile() && !(file.size() % COLOR_NAME_LEN + COLOR_SAVE_BYTES))
        {
            int osset = 0;
            while(file.available() >= COLOR_NAME_LEN + COLOR_SAVE_BYTES)
                osset += file.readBytes((char*)colors + osset, COLOR_NAME_LEN + COLOR_SAVE_BYTES);
        }

        // update free indexes
        GetFreeColors();
    }

    // sync to file
    void Sync(void) {}

    uint8_t GetColorsCount(void)
    {
        return MAX_SAVED_COLORS;
    }

    uint8_t GetFreeColors(void)
    {
        free_colors = 0;
        for(int i = 0; i < MAX_SAVED_COLORS; i++)
        {
            if(colors[(i * COLOR_RECORD_LEN) + COLOR_SAVE_SET] & (uint8_t)0x01)
                fcolors[free_colors++] = i;
        }

        return free_colors;
    }

    void SetColot(uint8_t index, uint8_t r, uint8_t g, uint8_t b, uint8_t h, uint8_t s, uint8_t v, const char * name)
    {
        if(index <= MAX_SAVED_COLORS)
        {
            colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_R]  = r;
            colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_G]  = g;
            colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_B]  = b;
            colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_H]  = h;
            colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_S]  = s;
            colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_V]  = v;

            if(!(colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_SET] & (uint8_t)0x01))
                free_colors--;

            colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_SET] |= (uint8_t)0x01;
            
            if(name)
                strncpy((char*)&colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_NAME], name, COLOR_NAME_LEN);
            
        }
    }

    bool AddColor(uint8_t r, uint8_t g, uint8_t b, uint8_t h, uint8_t s, uint8_t v, const char * name)
    {
        if(free_colors > 0)
        {
            SetColot(fcolors[free_colors - 1], r, g, b, h, s, v, name);
            return true;
        }
        return false;
    }

    uint8_t GetFreeColors(void)
    {
        return free_colors;
    }


protected:
    // color name up to 16 chars
    // r, g, b, + hue
    uint8_t colors[MAX_SAVED_COLORS * (COLOR_SAVE_BYTES + COLOR_NAME_LEN)];
    uint8_t fcolors[MAX_SAVED_COLORS];
    uint8_t free_colors;
};


#endif
