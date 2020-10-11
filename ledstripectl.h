#ifndef _LEDSTRIPE_H
#define _LEDSTRIPE_H

#include <Arduino.h>
#include <inttypes.h>
#include <Ticker.h>
#include "config.h"

using namespace std;

class LedStripeTrans;

////////////////////////////////////////////////////////////////////////////////////
// This class represents RDB Stripe color
class LedStripeState
{
    friend class LedStripeTrans;

public:
    LedStripeState() : LedStripeState(MAX_LED_TRANS_TIME) {}
    LedStripeState(uint32_t a_step_multipler) : LedStripeState(0, 0, 0, a_step_multipler) {}
    LedStripeState(uint16_t a_r, uint16_t a_g, uint16_t a_b, uint32_t a_step_multipler) : step_multipler(a_step_multipler)
    {
        r = a_r * step_multipler * COLOR_RANGE;
        g = a_g * step_multipler * COLOR_RANGE;
        b = a_b * step_multipler * COLOR_RANGE;

       Serial.printf("LedStripeState r = %u g = %u, b = %u, step_multipler = %u\r\n", r, g, b, step_multipler);
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

    uint16_t GetDuty_R(void)
    {
        return (uint16_t)((r / step_multipler)  * COLOR_RANGE);
        
    }

    uint16_t GetDuty_G(void)
    {
        return (uint16_t)((g / step_multipler)  * COLOR_RANGE);
    }

    uint16_t GetDuty_B(void)
    {
        return (uint16_t)((b / step_multipler)  * COLOR_RANGE);
    }

protected:
    uint32_t r;         // current red duty cycle
    uint32_t g;         // current green duty cycle
    uint32_t b;         // current blue duty cycle
    uint32_t step_multipler;    // when step is a fraction we use this multiplier to preserve fracrions
};

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
        duty_cycle(a_duty_cycle), 
        trans_time(a_time_ms), 
        trans_step_r(0), 
        trans_step_g(0), 
        trans_step_b(0), 
        elapsed_time(0), 
        in_trans(false),
        next(nullptr),
        on_list(false)
    {
        Setup(a_r, a_g, a_b, a_tr, a_tg, a_tb, a_time_ms);
        /*current = LedStripeState(a_r, a_g, a_b, MAX_LED_TRANS_TIME);
        target = LedStripeState(a_tr, a_tg, a_tb, MAX_LED_TRANS_TIME);

        // recalculate duty step / ms
        if(trans_time > 0)
        {
            trans_step_r = (target.r - current.r) / trans_time;
            trans_step_g = (target.g - current.g) / trans_time;
            trans_step_b = (target.b - current.b) / trans_time;
        }
        
        in_trans = true;    // unlock updating in case it was locked

        Serial.printf("LedStripeTrans: r:%u, g:%u, b:%u -> r:%u, g:%u, b:%u  a_time_ms = %u\r\n", a_r, a_g, a_b, a_tr, a_tg, a_tb, a_time_ms);
        Serial.printf("LedStripeTrans: steps r:%u, g:%u, b:%u\r\n", trans_step_r, trans_step_g, trans_step_b);*/
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
            trans_step_r = (target.r - current.r) / trans_time;
            trans_step_g = (target.g - current.g) / trans_time;
            trans_step_b = (target.b - current.b) / trans_time;
        }
        
        in_trans = true;    // unlock updating in case it was locked

        Serial.printf("Setup: r:%u, g:%u, b:%u -> r:%u, g:%u, b:%u  a_time_ms = %u\r\n", current.r, current.g, current.b, target.r, target.g, target.b, a_time_ms);
        Serial.printf("Setup: steps r:%u, g:%u, b:%u\r\n", trans_step_r, trans_step_g, trans_step_b);
    }

    void Reset(void) 
    {
        current = initial;
        elapsed_time = 0;
    }

    void SetTargetColor(LedStripeState a_target_state, uint32_t a_trans_time)
    {
        target = a_target_state;
        trans_time = a_trans_time;
        // recalculate duty step / ms

        if(trans_time > 0)
        {
            trans_step_r = (target.r - current.r) / trans_time;
            trans_step_g = (target.g - current.g) / trans_time;
            trans_step_b = (target.b - current.b) / trans_time;
        }

        in_trans = true;    // unlock updating in case it was locked
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
            return false; // this one is endless
        }
        else
        {
            elapsed_time += delta;
            current.r += delta * trans_step_r;
            current.g += delta * trans_step_g;
            current.b += delta * trans_step_b;
            Serial.printf("UPDATE: r:%u, g:%u, b:%u -> r:%u, g:%u, b:%u  elapsed_time = %u\r\n", current.r, current.g, current.b, target.r, target.g, target.b, elapsed_time);
            if(elapsed_time > trans_time)
            {
                Reset();
                //Serial.printf("elapsed_time switching: elapsed_time = %u > trans_time = %u\r\n", elapsed_time, trans_time);
                return true;
            }

            return false;
        }
    }

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
};

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
        duty_cycle(a_duty_cycle)
        {
            pinMode(pin_r, OUTPUT);
            pinMode(pin_g, OUTPUT);
            pinMode(pin_b,OUTPUT);
            
            analogWriteFreq(a_duty_cycle);

            //Serial.printf("LedStripeCtl: %d, %d, %d, %u\r\n", pin_r, pin_g, pin_b, duty_cycle);

            for(LedStripeTrans trb : transitions_buffer)
                trb = LedStripeTrans(duty_cycle);
        }

    // add internal external transition
    void AddTransition(LedStripeTrans * a_tr)
    {
        LedStripeTrans* new_tr = transitions;
        LedStripeTrans* prev_tr = nullptr;
        Serial.println("AddTransition:");

        // do not allow adding same transition twice this would ruin out list
        if(transitions == a_tr)
        {
            Serial.println("  - transitions == a_tr SKIP");
            return;
        }

        // if root is null
        if(!transitions)
        {
            Serial.println("  - transitions empty");
            transitions = a_tr;
            a_tr->on_list = true;
            current_transition = transitions;
            current_transition->next = transitions;
        }
        else
        {
            // find ending
            Serial.println("  - looking for last el.");
           while(new_tr->next != transitions)
           {
                new_tr = new_tr->next;
                if(new_tr == a_tr)
                    return; // if on list do not allow to add.
           }
            Serial.printf("  - last el: %p\t\n", new_tr);
            new_tr->next = a_tr; // insert
            a_tr->next = transitions; // set to root
        }

            Serial.printf(" added %p\r\n", a_tr);
            Serial.printf("  - transitions = %p\r\n", transitions);
            Serial.printf("  - current_transition = %p\r\n", current_transition);
    }

    // use internal transition (pointer returned for manilulation outside the object) if no memory left returns null
    LedStripeTrans * AddTransition(uint16_t a_r, uint16_t a_g, uint16_t a_b, uint16_t a_tr, uint16_t a_tg, uint16_t a_tb, uint32_t a_time_ms)
    {
        for(int i = 0; i < MAX_STRIPE_TRANSITIONS; i++)
        {   
            Serial.printf("AddTransition int: checking [%d] %p\r\n", i, &transitions_buffer[i]);
            if(!transitions_buffer[i].on_list)
            {
                Serial.printf(" - picked up %p\r\n", &transitions_buffer[i]);
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
    }


    void Update(void)
    {
        bool trans_done = false;
        uint32_t current_time = millis();
        call_time_delta = current_time - call_time;
        call_time = current_time;

        if(current_transition)
        {          
            trans_done = current_transition->Update(call_time_delta);
            Serial.printf("[%u|%u|%u] ", current_transition->GetCurrent_R(), current_transition->GetCurrent_G(), current_transition->GetCurrent_B());
            analogWrite(pin_r, current_transition->GetCurrent_R());
            analogWrite(pin_g, current_transition->GetCurrent_G());
            analogWrite(pin_b, current_transition->GetCurrent_B());
            //Serial.printf("current_transition: ");

            if(trans_done)  // when transition has 0 time this should never happen
            {
                current_transition = current_transition->next;
            }
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
};

#endif
