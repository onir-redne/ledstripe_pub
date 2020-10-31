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
#define COLOR_SAVE_NAME         4

#define COLOR_RECORD_LEN        COLOR_NAME_LEN + COLOR_SAVE_BYTES
#define COLOR_SET_ENABLED_MASK  0x01    // color is defined
#define COLOR_SET_SYNCED_MASK  0x02     // stored in flash


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
        if(file.isFile() && !(file.size() % COLOR_RECORD_LEN))
        {
            Serial.print("SavedColors: loading from save file...[");
            int osset = 0;
            while(file.available() >= COLOR_RECORD_LEN)
            {
                osset += file.readBytes((char*)colors + osset, COLOR_RECORD_LEN);
                if(!(colors[osset - COLOR_RECORD_LEN + COLOR_SAVE_SET] & (uint8_t)COLOR_SET_ENABLED_MASK))
                    free_colors++;
            }
            Serial.println("]");
            file.close();
        }
        else
        {
            // no file will create one in sync
            Serial.println("SavedColors: save file not fond - init empty table!");
            memset(colors, 0, COLOR_RECORD_LEN * MAX_SAVED_COLORS);
            // update free indexes
            free_colors = MAX_SAVED_COLORS;
        }
        
        
        Serial.printf("SavedColors: colors: %u, not set: %u\r\n", MAX_SAVED_COLORS, free_colors);
    }

    // sync to file
    void Sync(void) 
    {
        fs::File file = LittleFS.open(filename, "w+");
        if(file.isFile())
        {
            Serial.println("SavedColors: writing to file...");

            for(int i = 0; i < MAX_SAVED_COLORS; i++)
            {
                Serial.printf("   #%u r:%u g:%u b:%u [%s]\r\n", i, colors[(i * COLOR_RECORD_LEN) + COLOR_SAVE_R], colors[(i * COLOR_RECORD_LEN) + COLOR_SAVE_G], colors[(i * COLOR_RECORD_LEN) + COLOR_SAVE_B], &colors[(i * COLOR_RECORD_LEN) + COLOR_SAVE_NAME]);
                colors[(i * COLOR_RECORD_LEN) + COLOR_SAVE_SET] |= (uint8_t)COLOR_SET_SYNCED_MASK;  // mark files synced
            }

            file.write(colors, MAX_SAVED_COLORS * COLOR_RECORD_LEN);
            file.close();
        }
        else
        {
            // no file will create one in sync
            Serial.println("SavedColors: error while writing to file!");
        }
    }

    uint8_t GetColorsCount(void)
    {
        return MAX_SAVED_COLORS;
    }

    void SetColor(uint8_t index, uint8_t r, uint8_t g, uint8_t b, const char * name)
    {
        if(index <= MAX_SAVED_COLORS)
        {
            Serial.printf("SetColor(): #%u r:%u g:%u b:%u [%s] - ",index, r, g, b, name);
            colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_R]  = r;
            colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_G]  = g;
            colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_B]  = b;

            if(!(colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_SET] & (uint8_t)COLOR_SET_ENABLED_MASK))
                free_colors--;

            colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_SET] |= (uint8_t)COLOR_SET_ENABLED_MASK;
            colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_SET] &= ~(uint8_t)COLOR_SET_SYNCED_MASK; // mark unsynced!
            
            Serial.printf("set: %02X, ", colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_SET]);
            strncpy((char*)&colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_NAME], name, COLOR_NAME_LEN);
            Serial.printf("name: %s\r\n", &colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_NAME]);
        }
    }

    bool AddColor(uint8_t r, uint8_t g, uint8_t b, const char * name)
    {
        int i = 0;
        while(i < MAX_SAVED_COLORS)
        {
            uint8_t s = colors[i * COLOR_RECORD_LEN + COLOR_SAVE_SET];
            if(s & COLOR_SET_ENABLED_MASK)
            {
                Serial.printf("AddColor(): #%u - skip\r\n", i);
                i++;
            }
            else
            {
                Serial.printf("AddColor(): #%u is free ...\r\n", i);
                SetColor(i, r, g, b, name);
                return true;
            }   
        }
        return false;
    }

    bool DelColor(uint8_t id)
    {
        if(id >= MAX_SAVED_COLORS)
            return false;

        uint8_t s = colors[id * COLOR_RECORD_LEN + COLOR_SAVE_SET];
        if(s & COLOR_SET_ENABLED_MASK)
        {
            Serial.printf("DelColor(): #%u\r\n", id);
            colors[id * COLOR_RECORD_LEN + COLOR_SAVE_SET] = 0;
            free_colors++;
        }

        return true;
    }

    uint8_t GetFreeColorsCount(void)
    {
        return free_colors;
    }

    void Rewind(void)
    {
        color_ptr = 0;
    }

    bool GetNextColor(uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *set, uint8_t * id, char * name)
    {
        while(color_ptr < MAX_SAVED_COLORS)
        {
            char s = colors[color_ptr * COLOR_RECORD_LEN + COLOR_SAVE_SET];
            if(s & COLOR_SET_ENABLED_MASK)
            {
                *set = s;
                *r = colors[color_ptr * COLOR_RECORD_LEN + COLOR_SAVE_R];
                *g = colors[color_ptr * COLOR_RECORD_LEN + COLOR_SAVE_G];
                *b = colors[color_ptr * COLOR_RECORD_LEN + COLOR_SAVE_B];
                strncpy(name, (char*)&colors[color_ptr * COLOR_RECORD_LEN + COLOR_SAVE_NAME], COLOR_NAME_LEN);
                *id = color_ptr;

                Serial.printf("GetNextColor(): #%u r:%u g:%u b:%u [%s]\r\n", color_ptr, *r, *g, *b, name);

                color_ptr++;
                return true;
            }
            else
            {
                Serial.printf("GetNextColor(): #%u is not set...\r\n", color_ptr);
                color_ptr++;
            }   
        }
        color_ptr = 0;
        return false;
    }

protected:

    // color name up to 16 chars
    // r, g, b, + hue
    uint8_t colors[MAX_SAVED_COLORS * (COLOR_SAVE_BYTES + COLOR_NAME_LEN)];
    uint8_t free_colors;
    uint8_t color_ptr;
    String filename;
};


#endif
