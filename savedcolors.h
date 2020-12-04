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

#define COLOR_RECORD_LEN        (COLOR_NAME_LEN + COLOR_SAVE_BYTES)
#define COLOR_ARRAY_SIZE        (MAX_SAVED_COLORS * COLOR_RECORD_LEN)
#define COLOR_SET_ENABLED_MASK  0x01    // color is defined
#define COLOR_SET_SYNCED_MASK  0x02     // stored in flash


using namespace std;

class SavedColors
{
public:
    SavedColors(const String& file_name) :
    free_colors(0),
    sync(false),
    filename(file_name)
    {
    }

    void Init(void)
    {
        SPRNTF("SavedColors: loading from save file [%s]\r\n", filename.c_str());
        fs::File file = LittleFS.open(filename, "r");
        if(file.isFile()/* && !(file.size() % COLOR_RECORD_LEN)*/)
        {
            int offset = 0, i = 0;
            while(file.available() >= COLOR_RECORD_LEN)
            {
                offset += file.readBytes((char*)colors + offset, COLOR_RECORD_LEN);
                if(!(colors[offset - COLOR_ARRAY_SIZE] & (uint8_t)COLOR_SET_ENABLED_MASK))
                    free_colors++;

                SPRNTF("   #%u r:%u g:%u b:%u [%s]\r\n", i, colors[(i * COLOR_RECORD_LEN) + COLOR_SAVE_R], colors[(i * COLOR_RECORD_LEN) + COLOR_SAVE_G], colors[(i * COLOR_RECORD_LEN) + COLOR_SAVE_B], &colors[(i * COLOR_RECORD_LEN) + COLOR_SAVE_NAME]);
                i++;

            }
            file.close();
        }
        else
        {
            // no file will create one in sync
            SPRNTLN("SavedColors: save file not fond - init empty table!");
            memset(colors, 0, COLOR_ARRAY_SIZE);
            // update free indexes
            free_colors = MAX_SAVED_COLORS;
        }
        
        
        SPRNTF("SavedColors: memory: %p colors: %u, not set: %u\r\n", colors,  MAX_SAVED_COLORS, free_colors);
    }

    // sync to file
    void Sync(void) 
    {
        if(!sync)
            return;


        fs::File file = LittleFS.open(filename, "w+");
        if(file.isFile())
        {
            SPRNTF("SavedColors: writing to file [%s]\r\n", filename.c_str());

            for(int i = 0; i < MAX_SAVED_COLORS; i++)
            {
                SPRNTF("   #%u r:%u g:%u b:%u [%s]\r\n", i, colors[(i * COLOR_RECORD_LEN) + COLOR_SAVE_R], colors[(i * COLOR_RECORD_LEN) + COLOR_SAVE_G], colors[(i * COLOR_RECORD_LEN) + COLOR_SAVE_B], &colors[(i * COLOR_RECORD_LEN) + COLOR_SAVE_NAME]);
                colors[(i * COLOR_RECORD_LEN) + COLOR_SAVE_SET] |= (uint8_t)COLOR_SET_SYNCED_MASK;  // mark files synced
            }

            file.write(colors, COLOR_ARRAY_SIZE);
            file.close();
            sync = false;
        }
        else
        {
            // no file will create one in sync
            SPRNTLN("SavedColors: error while writing to file!");
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
            uint8_t * p = &colors[index * COLOR_RECORD_LEN];
            SPRNTF("SetColor(): #%u r:%u g:%u b:%u [%s] - ",index, r, g, b, name);
            p[COLOR_SAVE_R]  = r;
            p[COLOR_SAVE_G]  = g;
            p[COLOR_SAVE_B]  = b;

            //if(!(colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_SET] & (uint8_t)COLOR_SET_ENABLED_MASK))
            //    free_colors--;

            p[COLOR_SAVE_SET] |= (uint8_t)COLOR_SET_ENABLED_MASK;
            p[COLOR_SAVE_SET] &= ~(uint8_t)COLOR_SET_SYNCED_MASK; // mark unsynced!
            
            SPRNTF("set: %02X, ", colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_SET]);
            strncpy((char*)(&p[COLOR_SAVE_NAME]), name, COLOR_NAME_LEN - 1);
            SPRNTF("name: %s\r\n", &p[COLOR_SAVE_NAME]);
            //SPRNTF("SetColor(): %p [%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X]\r\n", p, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15], p[16], p[17], p[18], p[19], p[20]);
            sync = true;
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
                SPRNTF("AddColor(): #%u - skip\r\n", i);
                i++;
            }
            else
            {
                SPRNTF("AddColor(): #%u is free ...\r\n", i);
                SetColor(i, r, g, b, name);
                free_colors--;
                sync = true;
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
            SPRNTF("DelColor(): #%u\r\n", id);
            colors[id * COLOR_RECORD_LEN + COLOR_SAVE_SET] = 0;
            free_colors++;
            sync = true;
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
        uint8_t * p = nullptr;
        while(color_ptr < MAX_SAVED_COLORS)
        {
            p = &colors[color_ptr * COLOR_RECORD_LEN];
            SPRNTF("GetNextColor(): %p [%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X] ", p, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15], p[16], p[17], p[18], p[19], p[20]);
            
            char s = p[COLOR_SAVE_SET];
            if(s & COLOR_SET_ENABLED_MASK)
            {
                *set = s;
                *r = p[COLOR_SAVE_R];
                *g = p[COLOR_SAVE_G];
                *b = p[COLOR_SAVE_B];
                strncpy(name, (char*)(&(p[COLOR_SAVE_NAME])), COLOR_NAME_LEN - 1);
                //strcpy(name, (char*)&colors[color_ptr * COLOR_RECORD_LEN + COLOR_SAVE_NAME]);
                *id = color_ptr;

                SPRNTF("(#%u r:%u g:%u b:%u [%s])\r\n", color_ptr, *r, *g, *b, name);

                color_ptr++;
                return true;
            }
            else
            {
                SPRNTF("(#%u is not set)\r\n", color_ptr);
                color_ptr++;
            }
        }
        color_ptr = 0;
        return false;
    }

    bool SyncNeeded(void) 
    {
        return sync;
    }

protected:

    // color name up to 16 chars + null
    uint8_t colors[COLOR_ARRAY_SIZE];
    uint8_t free_colors;
    uint8_t color_ptr;
    String filename;
    bool sync;
};


#endif
