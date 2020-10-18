#ifndef _SAVEDCOLORS_H
#define _SAVEDCOLORS_H

#include <Arduino.h>
#include <inttypes.h>
#include "LittleFS.h"
#include "config.h"

using namespace std;

class SavedColors
{
public:
    SavedColors() {
    }

    void LoadColors(uint8_t * config_buffer, uint16_t buffer_len) {
        
    }

protected:
    uint8_t colors[MAX_SAVED_COLORS][3];
};


#endif
