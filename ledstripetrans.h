#ifndef _LEDSTRIPETRANS_H
#define _LEDSTRIPETRANS_H

#include <Arduino.h>
#include <inttypes.h>
#include "config.h"
#include "ledstripestate.h"

using namespace std;

class LedStripeTrans;

class LedStripeTrans
{
    friend class LedStripeCtl;

public:
    LedStripeTrans(uint16_t a_duty_cycle) :
    duty_cycle(a_duty_cycle),
    next(nullptr),
    on_list(false)
    {}
    
    virtual void Reset(void) = 0;
    virtual uint16_t GetCurrent_R(void)
    {
        return current.GetDuty_R();
    }
    
    virtual uint16_t GetCurrent_G(void)
    {
        return current.GetDuty_G();
    }
    
    virtual uint16_t GetCurrent_B(void)
    {
        return current.GetDuty_B();
    }

    virtual bool Update(uint32_t delta) = 0;

protected:
    LedStripeState current;         // current durty cycles
    uint16_t duty_cycle;
    LedStripeTrans * next;
    bool on_list;
};


#endif
