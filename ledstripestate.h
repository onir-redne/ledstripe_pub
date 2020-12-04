#ifndef _LEDSTRIPESTATE_H
#define _LEDSTRIPESTATE_H

#include <Arduino.h>
#include <inttypes.h>
#include "config.h"

using namespace std;


////////////////////////////////////////////////////////////////////////////////////
// This class represents RDB Stripe color
class LedStripeState
{
    friend class LedStripeTransColor;
    friend class LedStripeTransSpectrum;

public:
    LedStripeState() : LedStripeState(MAX_LED_TRANS_TIME) {}
    LedStripeState(uint32_t a_step_multipler) : LedStripeState(0, 0, 0, a_step_multipler) {}
    LedStripeState(uint16_t a_r, uint16_t a_g, uint16_t a_b, uint32_t a_step_multipler) : step_multipler(a_step_multipler)
    {
        r = a_r * step_multipler * COLOR_RANGE;
        g = a_g * step_multipler * COLOR_RANGE;
        b = a_b * step_multipler * COLOR_RANGE;

       //SPRNTF("LedStripeState r = %u g = %u, b = %u, step_multipler = %u\r\n", r, g, b, step_multipler);
    }
    LedStripeState(const LedStripeState & other) 
    {
        this->r = other.r;
        this->g = other.g;
        this->b = other.b;
        this->step_multipler = other.step_multipler;
    }

    LedStripeState& operator= (const LedStripeState & other) 
    {
        this->r = other.r;
        this->g = other.g;
        this->b = other.b;
        this->step_multipler = other.step_multipler;
        return *this;
    }

    void SetDuty_R(uint16_t a_r)
    {
        r = a_r * step_multipler;
    }

    void SetDuty_G(uint16_t a_g)
    {
        g = a_g * step_multipler;
    }

    void SetDuty_B(uint16_t a_b)
    {
        b = a_b * step_multipler;
    }

    void SetColor_R(uint16_t a_r)
    {
        r = (a_r * step_multipler * COLOR_RANGE) + PWM_CUTOFF;
    }

    void SetColor_G(uint16_t a_g)
    {
        g = (a_g * step_multipler * COLOR_RANGE) + PWM_CUTOFF;
    }

    void SetColor_B(uint16_t a_b)
    {
        b = (a_b * step_multipler * COLOR_RANGE) + PWM_CUTOFF;
    }

    uint16_t GetDuty_R(void)
    {
        return (uint16_t)(r / step_multipler);
    }

    uint16_t GetDuty_G(void)
    {
        return (uint16_t)(g / step_multipler);
    }

    uint16_t GetDuty_B(void)
    {
        return (uint16_t)(b / step_multipler);
    }

    uint8_t GetColor_R(void)
    {
         return (uint16_t)(r / step_multipler / COLOR_RANGE)  - PWM_CUTOFF;
    }

    uint8_t GetColor_G(void)
    {
         return (uint16_t)(g / step_multipler / COLOR_RANGE) - PWM_CUTOFF;
    }

    uint8_t GetColor_B(void)
    {
         return (uint16_t)(b / step_multipler / COLOR_RANGE) - PWM_CUTOFF;
    }

protected:
    uint32_t r;         // current red duty cycle
    uint32_t g;         // current green duty cycle
    uint32_t b;         // current blue duty cycle
    uint32_t step_multipler;    // when step is a fraction we use this multiplier to preserve fracrions
};


#endif
