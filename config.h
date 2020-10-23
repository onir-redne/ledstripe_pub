#ifndef _CONFIG_H
#define _CONFIG_H

//////////////////////////////////
// leds
#define PWM_DUTY_CYCLE          1023
#define COLOR_RANGE             (PWM_DUTY_CYCLE / 255)
#define MAX_LED_TRANS_TIME      60000
#define MAX_STRIPE_TRANSITIONS  8

//////////////////////////////////
// Webserver and JSON strings
#define JSON_STRIPE_STATE_MAX       64
#define JSON_STRIPE_STATIC_COLOR    "color"
#define JSON_STRIPE_TRNASITION      "trans"
#define JSON_STRIPE_OFF             "off"
#define POWER_TIMER_MAX             7201


#endif