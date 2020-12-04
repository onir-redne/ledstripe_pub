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
#define TRANS_SET_RECORD_LEN        (TRANS_STRIPE_RECORD_LEN + TRANS_SET_SAVE_BYTES + TRANS_SET_NAME_LEN)
#define TRANS_SET_ARRAY_SIZE        (MAX_SAVED_TRNAS_SETS * TRANS_SET_RECORD_LEN)
#define TRANS_SET_STRIPE_OFFSET     0
#define TRANS_SET_ENABLED_MASK      0x01    // transition set is defined
#define TRANS_SET_SYNCED_MASK       0x02     // stored in flash

#define TRANS_SET_SAVE_SET          TRANS_STRIPE_RECORD_LEN
#define TRANS_SET_SAVE_NAME         (TRANS_STRIPE_RECORD_LEN + 1)

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
        SPRNTF("SavedTransSet: loading from save file [%s]\r\n", filename.c_str());
        fs::File file = LittleFS.open(filename, "r");
        if(file.isFile()/* && !(file.size() % COLOR_RECORD_LEN)*/)
        {
            int offset = 0, i = 0;
            while(file.available() >= TRANS_SET_RECORD_LEN)
            {
                offset += file.readBytes((char*)transsets + offset, TRANS_SET_RECORD_LEN);
                if(!(transsets[offset - TRANS_SET_ARRAY_SIZE + TRANS_SET_SAVE_SET] & (uint8_t)TRANS_SET_ENABLED_MASK))
                    free_sets++;

                SPRNTF("    #%u [%s]\r\n", i, &transsets[(i * TRANS_SET_RECORD_LEN) + TRANS_SET_SAVE_NAME]);
                i++;
            }
            file.close();
        }
        else
        {
            // no file will create one in sync
            SPRNTLN("SavedTransSet: save file not fond - init empty table!");
            memset(transsets, 0, TRANS_SET_ARRAY_SIZE);
            // update free indexes
            free_sets = MAX_SAVED_TRNAS_SETS;
        }
        
     
    SPRNTF("SavedTransSet: memory: %p sets total: %u, not set: %u\r\n", transsets,  MAX_SAVED_TRNAS_SETS, free_sets);
    SPRNTF("    TRANS_SET_RECORD_LEN: %u ,TRANS_STRIPE_RECORD_LEN: %u\r\n", TRANS_SET_RECORD_LEN, TRANS_STRIPE_RECORD_LEN);
    SPRNTF("    TRANS_SET_ARRAY_SIZE: %u\r\n", TRANS_SET_ARRAY_SIZE);

    }

    // sync to file
    void Sync(void) 
    {
        if(!sync)
            return;

        fs::File file = LittleFS.open(filename, "w+");
        if(file.isFile())
        {

            SPRNTF("SavedTransSet: writing to file [%s]\r\n", filename.c_str());
            for(int i = 0; i < MAX_SAVED_TRNAS_SETS; i++)
            {
                SPRNTF("    #%u [%s]\r\n", i, &transsets[(i * TRANS_SET_RECORD_LEN) + TRANS_SET_SAVE_NAME]);
                transsets[(i * TRANS_SET_RECORD_LEN) + TRANS_SET_SAVE_SET] |= (uint8_t)TRANS_SET_SYNCED_MASK;  // mark files synced
            }

            file.write(transsets, TRANS_SET_ARRAY_SIZE);
            file.close();
            sync = false;
        }
        else
        {
            // no file will create one in sync
            SPRNTLN("SavedTransSet: error while writing to file!");
        }
    }

    uint8_t GetTransSetsCount(void)
    {
        return MAX_SAVED_TRNAS_SETS;
    }

    void SetTransSet(uint8_t index, uint8_t * stripes, uint16_t stripes_count, const char * name)
    {
        uint8_t * p = &transsets[index * TRANS_SET_RECORD_LEN];
        SPRNTF("SetTransSet(): p: %p\r\n", p);
        if(index <= MAX_SAVED_TRNAS_SETS)
        {

            memset(p, 0, TRANS_SET_RECORD_LEN);
            memcpy(&p[TRANS_SET_STRIPE_OFFSET], stripes, stripes_count * TRANS_STRIPE_SAVE_BYTES);
                
            p[TRANS_SET_SAVE_SET] |= (uint8_t)TRANS_SET_ENABLED_MASK;
            p[TRANS_SET_SAVE_SET] &= ~(uint8_t)TRANS_SET_SYNCED_MASK; // mark unsynced!
            
            strncpy((char*)(&p[TRANS_SET_SAVE_NAME]), name, TRANS_SET_NAME_LEN - 1);
            SPRNTF("name: %s\r\n", &p[TRANS_SET_SAVE_NAME]);
            sync = true;
        }
    }

    bool AddTransSet(uint8_t * stripes, uint16_t stripes_count, const char * name)
    {
        int i = 0;
        while(i < MAX_SAVED_TRNAS_SETS)
        {
            uint8_t s = transsets[i * TRANS_SET_RECORD_LEN + TRANS_SET_SAVE_SET];
            if(s & TRANS_SET_ENABLED_MASK)
            {
                SPRNTF("AddTransSet(): #%u - skip\r\n", i);
                i++;
            }
            else
            {
                SPRNTF("AddTransSet(): #%u is free ...\r\n", i);
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
            SPRNTF("DelTransSet(): #%u\r\n", id);
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
        SPRNTF("GetNextTransSet() transset_ptr: %u\r\n", transset_ptr);
        while(transset_ptr < MAX_SAVED_TRNAS_SETS)
        {
            p = &transsets[transset_ptr * TRANS_SET_RECORD_LEN];
            SPRNTF("p:%p\r\n", p);
            
            char s = p[TRANS_SET_SAVE_SET];
            SPRNTF("s:%u\r\n", s);
            if(s & TRANS_SET_ENABLED_MASK)
            {
                *set = s;
                memcpy(stripes, (uint8_t*)(&p[TRANS_SET_STRIPE_OFFSET]), TRANS_STRIPE_RECORD_LEN);  // copy all transtions
                strncpy(name, (char *)(&p[TRANS_SET_SAVE_NAME]), TRANS_SET_NAME_LEN - 1);
                *id = transset_ptr;

                SPRNTF("stripes    #%u [%s]\r\n", *id, name);   
                transset_ptr++;
                return true;
            }
            else
            {
                SPRNTF("(#%u is not set)\r\n", transset_ptr);
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
        const uint8_t * p = &stripes[index * TRANS_STRIPE_SAVE_BYTES];
        //SPRNTF("ParseStripe() [%02X %02X %02X %02X %02X %02X %02X %02X]\r\n", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
        *r1 = p[TRANS_STRIPE_SAVE_R1];
        *g1 = p[TRANS_STRIPE_SAVE_G1];
        *b1 = p[TRANS_STRIPE_SAVE_B1];
        *r2 = p[TRANS_STRIPE_SAVE_R2];
        *g2 = p[TRANS_STRIPE_SAVE_G2];
        *b2 = p[TRANS_STRIPE_SAVE_B2];
        memcpy(time, &p[TRANS_STRIPE_SAVE_TIME], 2);
         //SPRNTF("ParseStripe() r1:%u g1:%u b1:%u -> r2:%u g2:%u b2:%u time:%u\r\n", *r1, *g1, *b1, *r2, *g2, *b2, *time);
        
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

        //SPRNTF("Convert2Stripe() r1:%u g1:%u b1:%u -> r2:%u g2:%u b2:%u time:%u\r\n", r1, g1, b1, r2, g2, b2, time);
        //SPRNTF("Convert2Stripe() [%02X %02X %02X %02X %02X %02X %02X %02X]\r\n", stripe[0], stripe[1], stripe[2], stripe[3], stripe[4], stripe[5], stripe[6], stripe[7]);
    }

    #ifdef DEBUG_LOG
    void DumpArray(void)
    {
        uint8_t * p = nullptr;
        for(int j = 0; j < MAX_SAVED_TRNAS_SETS; j++) 
        {
            p = transsets + TRANS_SET_RECORD_LEN * j;
            SPRNTF("set: %u\r\n", j);
            SPRNTF("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n", *p, *(p+1), *(p+2), *(p+3), *(p+4), *(p+5), *(p+6), *(p+7), *(p+8), *(p+9), *(p+10), *(p+11), *(p+12), *(p+13), *(p+14), *(p+15));
            p += 16;
            SPRNTF("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n", *p, *(p+1), *(p+2), *(p+3), *(p+4), *(p+5), *(p+6), *(p+7), *(p+8), *(p+9), *(p+10), *(p+11), *(p+12), *(p+13), *(p+14), *(p+15));
            p += 16;
            SPRNTF("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n", *p, *(p+1), *(p+2), *(p+3), *(p+4), *(p+5), *(p+6), *(p+7), *(p+8), *(p+9), *(p+10), *(p+11), *(p+12), *(p+13), *(p+14), *(p+15));
            p += 16;
            SPRNTF("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n", *p, *(p+1), *(p+2), *(p+3), *(p+4), *(p+5), *(p+6), *(p+7), *(p+8), *(p+9), *(p+10), *(p+11), *(p+12), *(p+13), *(p+14), *(p+15));
            p += 16;
            SPRNTF("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n", *p, *(p+1), *(p+2), *(p+3), *(p+4), *(p+5), *(p+6), *(p+7), *(p+8), *(p+9), *(p+10), *(p+11), *(p+12), *(p+13), *(p+14), *(p+15));
            p += 16;
            SPRNTF("%02X %02X \r\n\r\n", *p, *(p+1));
            p += 2;
        }
    }
    #endif

    bool GetTransStripe(uint8_t tid, uint8_t strp_num, uint8_t * r1, uint8_t * g1, uint8_t * b1, uint8_t * r2, uint8_t * g2, uint8_t * b2, uint16_t * time)
    {
        return ParseStripe(strp_num, (const uint8_t*)&transsets[tid * TRANS_SET_RECORD_LEN], r1, g1, b1, r2, g2, b2, time);
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
