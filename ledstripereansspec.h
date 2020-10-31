#ifndef _LEDSTRIPETRANSSPEC_H
#define _LEDSTRIPETRANSSPEC_H

#include <Arduino.h>
#include <inttypes.h>
#include "config.h"
#include "ledstripestate.h"
#include "ledstripetrans.h"
#include "spectrum.h"

using namespace std;


class LedStripeTransSpectrum: protected LedStripeTrans 
{

public:
    LedStripeTransSpectrum() :
        LedStripeTransSpectrum(PWM_DUTY_CYCLE, nullptr) {}
    LedStripeTransSpectrum(uint16_t a_duty_cycle, SpectrumAnalyser * a_p_spectrum_analyser) :
        LedStripeTransSpectrum(a_duty_cycle, 125, 8000, a_p_spectrum_analyser) {}
    LedStripeTransSpectrum(uint16_t a_duty_cycle, uint16_t a_hz_from, uint16_t a_hz_to, SpectrumAnalyser * a_p_spectrum_analyser) : 
        LedStripeTransSpectrum(a_duty_cycle, a_hz_from, a_hz_to, a_hz_from, a_hz_to, a_hz_from, a_hz_to, a_p_spectrum_analyser) {}
    LedStripeTransSpectrum(uint16_t a_duty_cycle, uint16_t a_r_hz_from, uint16_t a_r_hz_to, uint16_t a_g_hz_from, uint16_t a_g_hz_to, uint16_t a_b_hz_from, uint16_t a_b_hz_to, SpectrumAnalyser * a_p_spectrum_analyser) :
        LedStripeTrans(a_duty_cycle),
        p_spectrum_analyser(a_p_spectrum_analyser)
    {

    }
    
    virtual void Reset(void) {}

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

    void SetSpecAnalyser(SpectrumAnalyser * a_p_spectrum_analyser)
    {
        p_spectrum_analyser = a_p_spectrum_analyser;
    }

    void SetRangeR(uint16_t a_hz_from, uint16_t a_hz_to) 
    {
        r_req_range[0] = a_hz_from;
        r_req_range[1] = a_hz_to;
    }
    
    void SetRangeG(uint16_t a_hz_from, uint16_t a_hz_to) 
    {
        g_req_range[0] = a_hz_from;
        g_req_range[1] = a_hz_to;
    }
    
    void SetRangeB(uint16_t a_hz_from, uint16_t a_hz_to) 
    {
        b_req_range[0] = a_hz_from;
        b_req_range[1] = a_hz_to;
    }

    virtual bool Update(uint32_t delta)
    {
        if(!p_spectrum_analyser)
            return false;

        current.r = p_spectrum_analyser->GetValue4FreqRange(r_req_range[0], r_req_range[1]);
        current.g = p_spectrum_analyser->GetValue4FreqRange(g_req_range[0], g_req_range[1]);
        current.b = p_spectrum_analyser->GetValue4FreqRange(b_req_range[0], b_req_range[1]);

        return false;
    }

protected:
    SpectrumAnalyser * p_spectrum_analyser;
    uint16_t r_req_range[2];
    uint16_t g_req_range[2];
    uint16_t b_req_range[2];
};


#endif
