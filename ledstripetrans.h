#ifndef _LEDSTRIPETRANS_H
#define _LEDSTRIPETRANS_H

#include <Arduino.h>
#include <inttypes.h>
#include "config.h"
#include "ledstripestate.h"

using namespace std;


////////////////////////////////////////////////////////////////////////////////////
// This class represents transition form one colot to another within certain amount of time
// 0 timed transition will just set colors and stop updating, it will also never expire thus
// never execute next one if present in control list.
class LedStripeTrans 
{
    friend class LedStripeCtl;

public:
    LedStripeTrans() : LedStripeTrans(PWM_DUTY_CYCLE) {}
    LedStripeTrans(uint16_t a_duty_cycle) : LedStripeTrans(0, 0, 0, 0, a_duty_cycle) {}
    LedStripeTrans(uint16_t a_tr, uint16_t a_tg, uint16_t a_tb, uint32_t a_time_ms, uint16_t a_duty_cycle) : 
        LedStripeTrans(0, 0, 0, a_tr, a_tg, a_tb, a_time_ms, a_duty_cycle) {}
    LedStripeTrans(uint16_t a_r, uint16_t a_g, uint16_t a_b, uint16_t a_tr, uint16_t a_tg, uint16_t a_tb, uint32_t a_time_ms, uint16_t a_duty_cycle) :
        LedStripeTrans(a_r, a_g, a_b, a_tr, a_tg, a_tb, a_time_ms, a_duty_cycle, FUN_LINEAR) {}
    LedStripeTrans(uint16_t a_r, uint16_t a_g, uint16_t a_b, uint16_t a_tr, uint16_t a_tg, uint16_t a_tb, uint32_t a_time_ms, uint16_t a_duty_cycle, uint8_t a_fun) : 
        duty_cycle(a_duty_cycle), 
        trans_time(a_time_ms), 
        trans_step_r(0), 
        trans_step_g(0), 
        trans_step_b(0), 
        elapsed_time(0), 
        in_trans(false),
        next(nullptr),
        on_list(false),
        funct(a_fun)
    {
        Setup(a_r, a_g, a_b, a_tr, a_tg, a_tb, a_time_ms);
    }

    void Setup(uint16_t a_r, uint16_t a_g, uint16_t a_b, uint16_t a_tr, uint16_t a_tg, uint16_t a_tb, uint32_t a_time_ms)
    {
        trans_time = a_time_ms;
        initial = LedStripeState(a_r, a_g, a_b, MAX_LED_TRANS_TIME);
        current = initial;
        target = LedStripeState(a_tr, a_tg, a_tb, MAX_LED_TRANS_TIME);

        // recalculate duty step / ms
        if(trans_time > 0)
        {
            trans_step_r = (int)(target.r - current.r) / (int)trans_time;
            trans_step_g = (int)(target.g - current.g) / (int)trans_time;
            trans_step_b = (int)(target.b - current.b) /(int) trans_time;
        }
        
        in_trans = true;    // unlock updating in case it was locked

        //Serial.printf("Setup: r:%u, g:%u, b:%u -> r:%u, g:%u, b:%u  a_time_ms = %u\r\n", current.r, current.g, current.b, target.r, target.g, target.b, trans_time);
        //Serial.printf("Setup: steps r:%d g:%d, b:%d\r\n", trans_step_r, trans_step_g, trans_step_b);
    }

    void Reset(void) 
    {
        current = initial;
        elapsed_time = 0;
        //Serial.printf("Reset: r:%u, g:%u, b:%u -> r:%u, g:%u, b:%u  a_time_ms = %u\r\n", current.r, current.g, current.b, target.r, target.g, target.b, trans_time);
    }

    uint16_t GetCurrent_R(void)
    {
        return current.GetDuty_R();
    }
    
    uint16_t GetCurrent_G(void)
    {
        return current.GetDuty_G();
    }
    
    uint16_t GetCurrent_B(void)
    {
        return current.GetDuty_B();
    }

    bool Update(uint32_t delta) 
    {
        if(!in_trans)
            return false;

        if(trans_time == 0)
        {
            // simply change color
            current = target;
            in_trans = false; // prevent rewriteing without color changes
            return false; // this one is endless never switch to next
        }
        else
        {
            elapsed_time += delta;
            current.r += delta * trans_step_r;
            if(current.r & 0x10000000) current.r = 0;
            current.g += delta * trans_step_g;
            if(current.g & 0x10000000) current.g = 0;
            current.b += delta * trans_step_b;
            if(current.b & 0x10000000) current.b = 0;
            

            //Serial.printf("UPDATE: r:%u, g:%u, b:%u -> r:%u, g:%u, b:%u  elapsed_time = %u\r\n", current.r, current.g, current.b, target.r, target.g, target.b, elapsed_time);
            if(elapsed_time > trans_time)   // when transsition is done switch to next in looped list
            {
                //Serial.printf("elapsed_time switching: elapsed_time = %u > trans_time = %u\r\n", elapsed_time, trans_time);
                return true;
            }

            return false;
        }
    }

    static constexpr uint8_t FUN_LINEAR = 0;
    static constexpr uint8_t FUN_SIN = 1;
    static constexpr uint8_t FUN_EXP = 2;
    static constexpr uint8_t FUN_LOG = 3;

protected:
    LedStripeState initial;         // initial durty cycles
    LedStripeState current;         // current durty cycles
    LedStripeState target;
    

    uint16_t duty_cycle;
    int32_t trans_step_r;
    int32_t trans_step_g;
    int32_t trans_step_b;
    uint32_t elapsed_time;
    uint32_t trans_time;
    bool in_trans;

    LedStripeTrans * next;
    bool on_list;
    uint8_t funct;
};


#endif
