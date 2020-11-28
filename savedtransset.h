#ifndef _SAVEDTRANSSET_H
#define _SAVEDTRANSSET_H

#include <Arduino.h>
#include <inttypes.h>
#include <cstring>
#include "LittleFS.h"
#include "config.h"

/////////////////////////////////
// saved colors
#define TRANS_STRIPE_SAVE_R1             0
#define TRANS_STRIPE_SAVE_G1             1
#define TRANS_STRIPE_SAVE_B1             2
#define TRANS_STRIPE_SAVE_R2             3
#define TRANS_STRIPE_SAVE_G2             4
#define TRANS_STRIPE_SAVE_B2             5
#define TRANS_STRIPE_SAVE_TIME           6


#define TRANS_STRIPE_RECORD_LEN     (MAX_STRIPE_TRANSITIONS * TRANS_STRIPE_SAVE_BYTES)
#define TRANS_SET_RECORD_LEN        TRANS_STRIPE_RECORD_LEN + TRANS_SET_SAVE_BYTES + TRANS_SET_NAME_LEN
#define TRANS_SET_ARRAY_SIZE        MAX_SAVED_TRNAS_SETS * TRANS_SET_RECORD_LEN
#define TRANS_SET_STRIPE_OFFSET     0
#define TRANS_SET_ENABLED_MASK      0x01    // transition set is defined
#define TRANS_SET_SYNCED_MASK       0x02     // stored in flash

#define TRANS_SET_SAVE_SET          ( MAX_STRIPE_TRANSITIONS * TRANS_STRIPE_RECORD_LEN)
#define TRANS_SET_SAVE_NAME         ( MAX_STRIPE_TRANSITIONS * TRANS_STRIPE_RECORD_LEN) + 1     

using namespace std;

class SavedTransSet
{
public:
    SavedTransSet(const String& file_name) :
    free_sets(0),
    sync(false),
    filename(file_name),
    transset_ptr(0)
    {
    }

    void Init(void)
    {
        Serial.printf("SavedTransSet: loading from save file [%s]\r\n", filename.c_str());
        fs::File file = LittleFS.open(filename, "r");
        if(file.isFile()/* && !(file.size() % COLOR_RECORD_LEN)*/)
        {
            int offset = 0, i = 0;
            while(file.available() >= TRANS_SET_RECORD_LEN)
            {
                offset += file.readBytes((char*)transsets + offset, TRANS_SET_RECORD_LEN);
                if(!(transsets[offset - TRANS_SET_ARRAY_SIZE + TRANS_SET_SAVE_SET] & (uint8_t)TRANS_SET_ENABLED_MASK))
                    free_sets++;
            #ifdef DEBUG_LOG
                Serial.printf("    #%u [%s]\r\n", i, &transsets[(i * TRANS_SET_RECORD_LEN) + TRANS_SET_SAVE_NAME]);
                Serial.printf("        r:%u g:%u b:%u : r:%u g:%u b:%u\r\n", transsets[(i * TRANS_SET_RECORD_LEN) + TRANS_STRIPE_SAVE_R1], 
                                                                         transsets[(i * TRANS_SET_RECORD_LEN) + TRANS_STRIPE_SAVE_G1], 
                                                                         transsets[(i * TRANS_SET_RECORD_LEN) + TRANS_STRIPE_SAVE_B1], 
                                                                         transsets[(i * TRANS_SET_RECORD_LEN) + TRANS_STRIPE_SAVE_R2],
                                                                         transsets[(i * TRANS_SET_RECORD_LEN) + TRANS_STRIPE_SAVE_G2],
                                                                         transsets[(i * TRANS_SET_RECORD_LEN) + TRANS_STRIPE_SAVE_B2]);
            #endif
                i++;

            }
            file.close();
        }
        else
        {
            // no file will create one in sync
        #ifdef DEBUG_LOG
            Serial.println("SavedTransSet: save file not fond - init empty table!");
        #endif
            memset(transsets, 0, TRANS_SET_ARRAY_SIZE);
            // update free indexes
            free_sets = MAX_SAVED_TRNAS_SETS;
        }
        
    #ifdef DEBUG_LOG        
        Serial.printf("SavedTransSet: memory: %p colors: %u, not set: %u\r\n", transsets,  MAX_SAVED_TRNAS_SETS, free_sets);
    #endif
    }

    // sync to file
    void Sync(void) 
    {
        if(!sync)
            return;

        fs::File file = LittleFS.open(filename, "w+");
        if(file.isFile())
        {
        #ifdef DEBUG_LOG
            Serial.printf("SavedTransSet: writing to file [%s]\r\n", filename.c_str());
        #endif

            for(int i = 0; i < MAX_SAVED_TRNAS_SETS; i++)
            {
            #ifdef DEBUG_LOG
                Serial.printf("    #%u [%s]\r\n", i, &transsets[(i * TRANS_SET_RECORD_LEN) + TRANS_SET_SAVE_NAME]);
                for(int j = 0; j < MAX_STRIPE_TRANSITIONS; j++) 
                {
                    Serial.printf("        r:%u g:%u b:%u -> r:%u g:%u b:%u time:%u\r\n", 
                            transsets[(i * TRANS_SET_RECORD_LEN + j * TRANS_SET_RECORD_LEN) + TRANS_STRIPE_SAVE_R1], 
                            transsets[(i * TRANS_SET_RECORD_LEN + j * TRANS_SET_RECORD_LEN) + TRANS_STRIPE_SAVE_G1], 
                            transsets[(i * TRANS_SET_RECORD_LEN + j * TRANS_SET_RECORD_LEN) + TRANS_STRIPE_SAVE_B1], 
                            transsets[(i * TRANS_SET_RECORD_LEN + j * TRANS_SET_RECORD_LEN) + TRANS_STRIPE_SAVE_R2],
                            transsets[(i * TRANS_SET_RECORD_LEN + j * TRANS_SET_RECORD_LEN) + TRANS_STRIPE_SAVE_G2],
                            transsets[(i * TRANS_SET_RECORD_LEN + j * TRANS_SET_RECORD_LEN) + TRANS_STRIPE_SAVE_B2],
                            transsets[(i * TRANS_SET_RECORD_LEN + j * TRANS_SET_RECORD_LEN) + TRANS_STRIPE_SAVE_TIME]);
                }
            #endif

                transsets[(i * TRANS_SET_RECORD_LEN) + TRANS_SET_SAVE_SET] |= (uint8_t)TRANS_SET_SYNCED_MASK;  // mark files synced
            }

            file.write(transsets, TRANS_SET_ARRAY_SIZE);
            file.close();
            sync = false;
        }
        else
        {
            // no file will create one in sync
        #ifdef DEBUG_LOG
                Serial.println("SavedTransSet: error while writing to file!");
        #endif
        }
    }

    uint8_t GetTransSetsCount(void)
    {
        return MAX_SAVED_TRNAS_SETS;
    }

    void SetTransSet(uint8_t index, uint8_t * stripes, uint16_t stripes_count, const char * name)
    {
        uint8_t * p = &transsets[index * TRANS_SET_RECORD_LEN];
        if(index <= MAX_SAVED_TRNAS_SETS)
        {
            // loop troudh sets and copy
            int i = 0;
            for(i = 0; i < stripes_count; i++) 
            {
                // copy all stripes
                memcpy(p, stripes + i * TRANS_STRIPE_RECORD_LEN, TRANS_STRIPE_RECORD_LEN);
                p += TRANS_STRIPE_RECORD_LEN;
            }

            // zero all remaining (it time == 0 stripe is concidered empty)
            if(i < MAX_STRIPE_TRANSITIONS)
                memset(p, 0, (MAX_STRIPE_TRANSITIONS - i) * TRANS_STRIPE_RECORD_LEN);

            // set trans_set options
            p = &transsets[index * TRANS_SET_RECORD_LEN];

            p[TRANS_SET_SAVE_SET] |= (uint8_t)TRANS_SET_ENABLED_MASK;
            p[TRANS_SET_SAVE_SET] &= ~(uint8_t)TRANS_SET_SYNCED_MASK; // mark unsynced!
            
            //Serial.printf("set: %02X, ", colors[(index * COLOR_RECORD_LEN) + COLOR_SAVE_SET]);
            strncpy((char*)(&p[TRANS_SET_SAVE_NAME]), name, COLOR_NAME_LEN - 1);
            Serial.printf("name: %s\r\n", &p[TRANS_SET_SAVE_NAME]);
            //Serial.printf("SetColor(): %p [%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X]\r\n", p, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15], p[16], p[17], p[18], p[19], p[20]);
            sync = true;
        }
    }

    bool AddTransSet(uint8_t * stripes, uint16_t stripes_count, const char * name)
    {
        int i = 0;
        while(i < MAX_SAVED_COLORS)
        {
            uint8_t s = transsets[i * TRANS_SET_RECORD_LEN + TRANS_SET_SAVE_SET];
            if(s & TRANS_SET_SYNCED_MASK)
            {
                Serial.printf("AddTransSet(): #%u - skip\r\n", i);
                i++;
            }
            else
            {
                Serial.printf("AddTransSet(): #%u is free ...\r\n", i);
                SetTransSet(i, stripes, stripes_count, name);
                free_sets--;
                sync = true;
                return true;
            }   
        }
        return false;
    }

    bool DelTransSet(uint8_t id)
    {
        if(id >= MAX_SAVED_COLORS)
            return false;

        uint8_t s = transsets[id * TRANS_SET_RECORD_LEN + TRANS_SET_SAVE_SET];
        if(s & TRANS_SET_ENABLED_MASK)
        {
            Serial.printf("DelTransSet(): #%u\r\n", id);
            transsets[id * TRANS_SET_RECORD_LEN + TRANS_SET_SAVE_SET] = 0;
            free_sets++;
            sync = true;
        }

        return true;
    }

    uint8_t GetFreeTransSetCount(void)
    {
        return free_sets;
    }

    void Rewind(void)
    {
        transset_ptr = 0;
    }

    bool GetNextTransSet(uint8_t * stripes, uint8_t *set, uint8_t * id, char * name)
    {
        uint8_t * p = nullptr;
        while(transset_ptr < MAX_SAVED_TRNAS_SETS)
        {
            p = &transsets[transset_ptr * TRANS_SET_RECORD_LEN];
            
            char s = p[TRANS_SET_SAVE_SET];
            if(s & TRANS_SET_ENABLED_MASK)
            {
                *set = s;
                memcpy(stripes, (uint8_t*)(&p[TRANS_SET_STRIPE_OFFSET]), TRANS_STRIPE_RECORD_LEN * MAX_STRIPE_TRANSITIONS);  // copy all transtions
                strncpy(name, (char*)(&(p[TRANS_SET_SAVE_NAME])), TRANS_SET_NAME_LEN - 1);
                
                *id = transset_ptr;

            #ifdef DEBUG_LOG
                Serial.printf("    #%u [%s]\r\n", *id, *name);
                for(int j = 0; j < MAX_STRIPE_TRANSITIONS; j++) 
                {
                    Serial.printf("        r:%u g:%u b:%u -> r:%u g:%u b:%u time:%u\r\n", 
                            p[j * TRANS_SET_RECORD_LEN + TRANS_STRIPE_SAVE_R1], 
                            p[j * TRANS_SET_RECORD_LEN + TRANS_STRIPE_SAVE_G1], 
                            p[j * TRANS_SET_RECORD_LEN + TRANS_STRIPE_SAVE_B1], 
                            p[j * TRANS_SET_RECORD_LEN + TRANS_STRIPE_SAVE_R2],
                            p[j * TRANS_SET_RECORD_LEN + TRANS_STRIPE_SAVE_G2],
                            p[j * TRANS_SET_RECORD_LEN + TRANS_STRIPE_SAVE_B2],
                            p[j * TRANS_SET_RECORD_LEN + TRANS_STRIPE_SAVE_TIME]);
                }
            #endif
                
                transset_ptr++;
                return true;
            }
            else
            {
                Serial.printf("(#%u is not set)\r\n", transset_ptr);
                transset_ptr++;
            }
        }
        transset_ptr = 0;
        return false;
    }

    bool SyncNeeded(void) 
    {
        return sync;
    }

    static bool ParseStripe(uint8_t index, const uint8_t * stripes, uint8_t * r1, uint8_t * g1, uint8_t * b1, uint8_t * r2, uint8_t * g2, uint8_t * b2, uint16_t * time) 
    {
        *r1 = stripes[index * TRANS_STRIPE_RECORD_LEN + TRANS_STRIPE_SAVE_R1];
        *g1 = stripes[index * TRANS_STRIPE_RECORD_LEN + TRANS_STRIPE_SAVE_G1];
        *b1 = stripes[index * TRANS_STRIPE_RECORD_LEN + TRANS_STRIPE_SAVE_B1];
        *r2 = stripes[index * TRANS_STRIPE_RECORD_LEN + TRANS_STRIPE_SAVE_R2];
        *g2 = stripes[index * TRANS_STRIPE_RECORD_LEN + TRANS_STRIPE_SAVE_G2];
        *b2 = stripes[index * TRANS_STRIPE_RECORD_LEN + TRANS_STRIPE_SAVE_B2];
        memcpy(time, &stripes[index * TRANS_STRIPE_RECORD_LEN + TRANS_STRIPE_SAVE_TIME], 2);
        
        if(*time == 0)
            return false;

        return true;
    }

    static void Convert2Stripe(uint8_t * stripe, uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, uint16_t time)
    {
        stripe[TRANS_STRIPE_SAVE_R1] = r1;
        stripe[TRANS_STRIPE_SAVE_G1] = g1;
        stripe[TRANS_STRIPE_SAVE_B1] = b1;
        stripe[TRANS_STRIPE_SAVE_R2] = r2;
        stripe[TRANS_STRIPE_SAVE_G2] = g2;
        stripe[TRANS_STRIPE_SAVE_B2] = b2;
        memcpy(&stripe[TRANS_STRIPE_SAVE_TIME], &time, 2);
    }


protected:

    // color name up to 16 chars + null
    uint8_t transsets[TRANS_SET_ARRAY_SIZE];
    uint8_t free_sets;
    uint8_t transset_ptr;
    String filename;
    bool sync;
};


#endif
