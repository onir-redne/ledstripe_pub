#ifndef _SAVEDCOLORS_H
#define _SAVEDCOLORS_H

#include <Arduino.h>
#include <inttypes.h>
#include <cstring>
#include "LittleFS.h"
#include "config.h"

/////////////////////////////////
// saved colors
#define COLOR_SAVE_R            0
#define COLOR_SAVE_G            1
#define COLOR_SAVE_B            2
#define COLOR_SAVE_SET          3
#define COLOR_SAVE_NAME         7

#define COLOR_RECORD_LEN        COLOR_NAME_LEN + COLOR_SAVE_BYTES
#define COLOR_SET_ENABLED_MASK  0x01

using namespace std;

class SavedColors
{
public:
    SavedColors(const String& file_name) :
    free_colors(0),
    color_ptr(0),
    filename(file_name)
    {
    }

    void Init(void)
    {
         fs::File file = LittleFS.open(filename, "r");
        if(file.isFile() && !(file.size() % COLOR_NAME_LEN + COLOR_SAVE_BYTES))
        {
            Serial.print("SavedColors: loading from save file...[");
            int osset = 0;
            while(file.available() >= COLOR_NAME_LEN + COLOR_SAVE_BYTES)
            {
                osset += file.readBytes((char*)colors + osset, COLOR_NAME_LEN + COLOR_SAVE_BYTES);
                Serial.print("#");
            }
            Serial.println("]");
        }
        else
        {
            // no file will create one in sync
            Serial.println("SavedColors: save file not fond - init empty table!");
            memset(colors, 0, (COLOR_NAME_LEN + COLOR_SAVE_BYTES) * MAX_SAVED_COLORS);
        }
        

        // update free indexes
        uint8_t free_colors = IndexFreeColors();
        Serial.printf("SavedColors: colors: %u, not set: %u\r\n", MAX_SAVED_COLORS, free_colors);
    }

    // sync to file
    void Sync(void) {}

    uint8_t GetColorsCount(void)
    {
        return MAX_SAVED_COLORS;
    }


    void SetColor(uint8_t index, uint8_t r, uint8_t g, uint8_t b, const char * name)
    {
        if(index <= MAX_SAVED_COLORS)
        {
            colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_R]  = r;
            colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_G]  = g;
            colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_B]  = b;

            if(!(colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_SET] & (uint8_t)0x01))
                free_colors--;

            colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_SET] |= (uint8_t)0x01;
            
            if(name)
                strncpy((char*)&colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_NAME], name, COLOR_NAME_LEN);
            
        }
    }

    bool AddColor(uint8_t r, uint8_t g, uint8_t b, const char * name)
    {
        if(free_colors > 0)
        {
            SetColor(fcolors[free_colors - 1], r, g, b, name);
            return true;
        }
        return false;
    }

    uint8_t GetFreeColorsCount(void)
    {
        return free_colors;
    }

    void Rewind(void)
    {
        color_ptr = 0;
    }

    bool GetNextColor(uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *set, uint8_t * id, const char * name)
    {
        while(color_ptr < MAX_SAVED_COLORS)
        {
            char s = colors[color_ptr * (COLOR_SAVE_BYTES + COLOR_SAVE_SET)];
            if(s & COLOR_SET_ENABLED_MASK)
            {
                *set = s;
                *r = colors[color_ptr * (COLOR_SAVE_BYTES + COLOR_SAVE_R)];
                *g = colors[color_ptr * (COLOR_SAVE_BYTES + COLOR_SAVE_G)];
                *b = colors[color_ptr * (COLOR_SAVE_BYTES + COLOR_SAVE_B)];
                strncpy((char*)&colors[(color_ptr * COLOR_RECORD_LEN) + COLOR_SAVE_NAME], name, COLOR_NAME_LEN);
                *id = color_ptr;
                color_ptr++;
                return true;
            }
            else
            {
                color_ptr++;
            }   
        }
        color_ptr = 0;
        return false;
    }



protected:
    uint8_t IndexFreeColors(void)
    {
        free_colors = 0;
        for(int i = 0; i < MAX_SAVED_COLORS; i++)
        {
            if(!(colors[(i * COLOR_RECORD_LEN) + COLOR_SAVE_SET] & (uint8_t)0x01))
                fcolors[free_colors++] = i;
        }

        return free_colors;
    }

    // color name up to 16 chars
    // r, g, b, + hue
    uint8_t colors[MAX_SAVED_COLORS * (COLOR_SAVE_BYTES + COLOR_NAME_LEN)];
    uint8_t fcolors[MAX_SAVED_COLORS];
    uint8_t free_colors;
    uint8_t color_ptr;
    String filename;
};


#endif
