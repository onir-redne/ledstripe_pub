#ifndef _SPECTRUM_H
#define _SPECTRUM_H

#include <Arduino.h>
#include <inttypes.h>
#include "config.h"
#include "arduinoFFT.h"
#include "ledstripetrans.h"

using namespace std;

class SpectrumAnalyser {

public:
    SpectrumAnalyser(uint8_t a_adc_pin, uint16_t a_sampling_rate, uint16_t a_samplec_count) :
    adc_pin(a_adc_pin),
    spec_sample_time(round(1000000 * (1.0 / a_sampling_rate))),
    samplec_count(a_samplec_count),
    sigle_freq_range(a_sampling_rate / a_samplec_count),
    samples_done(0),
    time_lapse(0)
    {}

    void Update(void)
    {
        if(!enabled)
            return;

        time_now = micros();
        if(time_last + spec_sample_time < time_now)
        {
            raw_samples[samples_done] = analogRead(adc_pin); // A conversion takes about 1uS on an ESP32
            time_lapse = time_now - time_last;
            SPRNTF("ADC read_time = %lu\r\n", time_now - time_last);
            time_last = time_now;
            fft_samples[samples_done] = 0;
            samples_done++;

            
            
            if(samples_done >= samplec_count)
            {
                samples_done = 0;
                fft.Windowing(raw_samples, samplec_count, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
                fft.Compute(raw_samples, fft_samples, samplec_count, FFT_FORWARD);
                fft.ComplexToMagnitude(raw_samples, fft_samples, samplec_count);
            }
        }
        
        
        

        //SPRNTF("SpectrumAnalyser:Update() -> micros: %lu ", micros());
        // for (int i = 0; i < samplec_count; i++) 
        // {
        //     time_now = micros() - time_start;
        //     time_start = time_now;
        //     raw_samples[i] = analogRead(adc_pin); // A conversion takes about 1uS on an ESP32
        //     fft_samples[i] = 0;
        //     while (micros() < (time_now + spec_sample_time)) { }
        // }

        // SPRNTF(" :  %lu ", time_start);
        // fft.Windowing(raw_samples, samplec_count, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
        // fft.Compute(raw_samples, fft_samples, samplec_count, FFT_FORWARD);
        // fft.ComplexToMagnitude(raw_samples, fft_samples, samplec_count);
        //SPRNTF(" <-  %lu \r\n", micros());
    }

    uint32_t GetValue4FreqRange(uint16_t a_from_hz, uint16_t a_to_hz)
    {
        uint32_t value = 0;
        if(a_to_hz <= a_from_hz || a_to_hz - a_from_hz < sigle_freq_range)
            a_to_hz = a_from_hz + sigle_freq_range;

        //SPRNTF("GetValue4FreqRange(): Hz: %u : %u\r\n", a_from_hz, a_to_hz);

        for(uint16_t i = a_from_hz / sigle_freq_range; i <= a_to_hz / sigle_freq_range; i++)
            value = ((raw_samples[i] / SPECTRUM_MAX_APMPLITUDE) > value) ? (raw_samples[i] / SPECTRUM_MAX_APMPLITUDE) : value;

        //SPRNTF("GetValue4FreqRange(): value %u\r\n", value);
        return value;
    }

    uint16_t GetMinFreqRange(void)
    {
        return sigle_freq_range;
    }

    void Enable(void) { enabled = true; }

    void Disable(void) { enabled = false; }


protected:
    uint16_t samples_done;
    uint8_t adc_pin;
    bool enabled;
    uint32_t spec_sample_time;
    arduinoFFT fft;
    uint32_t time_last;
    uint32_t time_lapse;
    uint32_t time_now;
    uint16_t samplec_count;
    double raw_samples[SPECTRUM_SAMPLES_COUNT];
    double fft_samples[SPECTRUM_SAMPLES_COUNT];
    uint16_t sigle_freq_range;
};

#endif
