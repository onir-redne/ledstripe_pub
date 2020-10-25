#ifndef _LEDSTRIPE_H
#define _LEDSTRIPE_H

#include <Arduino.h>
#include <inttypes.h>
#include <Ticker.h>
#include "ledstripestate.h"
#include "ledstripetrans.h"
#include "config.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////////
// This class represents a RGB led stripe.
// To control add LedStripeTrans to list, it will then execute each state transsition and 
// porceed to next in list, after list is done the cycle will repeat.
class LedStripeCtl
{
public:
    LedStripeCtl(int a_pin_r, int a_pin_g, int a_pin_b, uint16_t a_duty_cycle) :
        call_time(millis()),
        call_time_delta(0),
        pin_r(a_pin_r),
        pin_g(a_pin_g),
        pin_b(a_pin_b),
        transitions(nullptr),
        current_transition(nullptr),
        duty_cycle(a_duty_cycle),
        color_set_flag(false),
        power(false),
        in_color_peek(false)
        {
            pinMode(pin_r, OUTPUT);
            pinMode(pin_g, OUTPUT);
            pinMode(pin_b,OUTPUT);
            
            analogWriteFreq(a_duty_cycle);

            //Serial.printf("LedStripeCtl: %d, %d, %d, %u\r\n", pin_r, pin_g, pin_b, duty_cycle);
            base_color = LedStripeState(0, 0, 0, 1);
            peek_color = LedStripeState(0, 0, 0, 1);
            p_color = & base_color; // set current color to pimary one

            for(LedStripeTrans trb : transitions_buffer)
                trb = LedStripeTrans(duty_cycle);
        }

    // add internal external transition
    void AddTransition(LedStripeTrans * a_tr)
    {
        LedStripeTrans* new_tr = transitions;
        LedStripeTrans* prev_tr = nullptr;
        //Serial.println("AddTransition:");

        // do not allow adding same transition twice this would ruin out list
        if(transitions == a_tr)
        {
           // Serial.println("  - transitions == a_tr SKIP");
            return;
        }

        // if root is null
        if(!transitions)
        {
            //Serial.println("  - transitions empty");
            transitions = a_tr;
            a_tr->on_list = true;
            current_transition = transitions;
            current_transition->next = transitions;
        }
        else
        {
            // find ending
            //Serial.println("  - looking for last el.");
           while(new_tr->next != transitions)
           {
                new_tr = new_tr->next;
                if(new_tr == a_tr)
                    return; // if on list do not allow to add.
           }
            //Serial.printf("  - last el: %p\t\n", new_tr);
            new_tr->next = a_tr; // insert
            a_tr->next = transitions; // set to root
        }

            //Serial.printf(" added %p\r\n", a_tr);
            //Serial.printf("  - transitions = %p\r\n", transitions);
            //Serial.printf("  - current_transition = %p\r\n", current_transition);
    }

    // set static color, and remove any transitions, static color is used when transitions is null
    void SetColor(uint16_t a_r, uint16_t a_g, uint16_t a_b)
    {
        p_color->SetColor_R(a_r);
        p_color->SetColor_G(a_g);
        p_color->SetColor_B(a_b);

        ClearTransitions();
    }

    // set secondary color just 
    void Switch2PeekColor(void)
    {
        p_color = &peek_color;
        in_color_peek = true;
        color_set_flag = false; // refresh color
    }

    // set secondary color just 
    void Switch2BaseColor(void)
    {
        p_color = &base_color;
        in_color_peek = false;
        color_set_flag = false; // refresh color
    }

    bool ActiveTransitions(void) 
    {
        return (transitions) ? true : false;
    }

    void PowerOff(void) 
    {
        analogWrite(pin_r, 0);
        analogWrite(pin_g, 0);
        analogWrite(pin_b, 0);
        power = false;
    }

    void PowerOn(void) 
    {
        power = true;
        color_set_flag = false;
    }

    void SetPower(bool power)
    {
        if(power)
            PowerOn();
        else
            PowerOff();        
    }

    const char * GetStateStr(void)
    {
        if(!power)
            return JSON_STRIPE_OFF;

        if(transitions)
            return JSON_STRIPE_TRNASITION;

        return JSON_STRIPE_STATIC_COLOR;
    }

    // return reference to static color (always return base color)
    LedStripeState & GetColorState(void) { return base_color; }

    // use internal transition (pointer returned for manilulation outside the object) if no memory left returns null
    LedStripeTrans * AddTransition(uint16_t a_r, uint16_t a_g, uint16_t a_b, uint16_t a_tr, uint16_t a_tg, uint16_t a_tb, uint32_t a_time_ms)
    {
        for(int i = 0; i < MAX_STRIPE_TRANSITIONS; i++)
        {   
            //Serial.printf("AddTransition int: checking [%d] %p\r\n", i, &transitions_buffer[i]);
            if(!transitions_buffer[i].on_list)
            {
                //Serial.printf(" - picked up %p\r\n", &transitions_buffer[i]);
                transitions_buffer[i].Setup(a_r, a_g, a_b, a_tr, a_tg, a_tb, a_time_ms);
                AddTransition(&transitions_buffer[i]);
                transitions_buffer[i].on_list = true;
                return &transitions_buffer[i];
            }
        }
        return nullptr;
    }

    void ClearTransitions(void)
    {
        transitions = nullptr;
        current_transition = nullptr;

        for(LedStripeTrans trb : transitions_buffer)
        {
            trb.next = nullptr;
            trb.on_list = false;
        }

        // unlock static color Update
        color_set_flag = false;
    }


    void Update(void)
    {

        if(!power)  // stipe off
            return;

        if(current_transition && !in_color_peek)
        {    
            bool trans_done = false;
            uint32_t current_time = millis();
            call_time_delta = current_time - call_time;
            call_time = current_time;

            trans_done = current_transition->Update(call_time_delta);
            //Serial.printf("[%u|%u|%u] ", current_transition->GetCurrent_R(), current_transition->GetCurrent_G(), current_transition->GetCurrent_B());
            analogWrite(pin_r, current_transition->GetCurrent_R());
            analogWrite(pin_g, current_transition->GetCurrent_G());
            analogWrite(pin_b, current_transition->GetCurrent_B());
            //Serial.printf("current_transition: ");

            if(trans_done)  // when transition has 0 time this should never happen
            {
                
                //Serial.printf(">>>>> switching %p [%u,%u,%u] -> %p [%u,%u,%u]\r\n", current_transition, current_transition->current.GetDuty_R(), current_transition->current.GetDuty_G(), current_transition->current.GetDuty_B(), current_transition->next, current_transition->next->current.GetDuty_R(), current_transition->next->current.GetDuty_G(), current_transition->next->current.GetDuty_B());
                current_transition = current_transition->next;
                current_transition->Reset();
            }
        }
        else if(!color_set_flag)
        {
            analogWrite(pin_r, p_color->GetDuty_R());
            analogWrite(pin_g, p_color->GetDuty_G());
            analogWrite(pin_b, p_color->GetDuty_B());
            color_set_flag = true;
        }
        
    }

protected:
    LedStripeTrans * transitions; // root for one-way list
    LedStripeTrans * current_transition;
    
    uint32_t call_time;
    uint32_t call_time_delta;
    int pin_r;
    int pin_g;
    int pin_b;
    uint16_t duty_cycle;

    LedStripeTrans transitions_buffer[MAX_STRIPE_TRANSITIONS];
    LedStripeState base_color;
    LedStripeState peek_color;
    LedStripeState * p_color;
    bool color_set_flag;
    bool in_color_peek;
    bool power;
};

#endif
