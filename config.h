#ifndef _CONFIG_H
#define _CONFIG_H

//////////////////////////////////
// pins
#define PIN_D0                  16
#define PIN_D1                  5
#define PIN_D2                  4
#define PIN_D3                  0
#define PIN_D4                  2
#define PIN_D5                  14
#define PIN_D6                  12
#define PIN_D7                  13
#define PIN_D8                  15
#define PIN_RX                  3
#define PIN_TX                  1
#define PIN_ADC0                17


//////////////////////////////////
// leds
#define PWM_DUTY_CYCLE          1023
#define COLOR_RANGE             (PWM_DUTY_CYCLE / 255)

//////////////////////////////////
// saved colors limits
#define MAX_SAVED_COLORS        32
#define COLOR_NAME_LEN          17
#define COLOR_SAVE_BYTES        4
#define DEFAULT_PEEK_COLOR_TOUT 5
#define SAVED_COLORS_FILE       "/config/colors.bin"

//////////////////////////////////
// saved transitions
#define MAX_SAVED_TRNAS_SETS    16
#define MAX_LED_TRANS_TIME      60000
#define MAX_STRIPE_TRANSITIONS  8
#define SAVED_TRANS_FILE       "/config/transitions.bin"

//////////////////////////////////
// spectrum analyser
#define SPECTRUM_MAX_APMPLITUDE     50
#define SPECTRUM_SAMPLING_FREQ      10000
#define SPECTRUM_SAMPLES_COUNT      256
#define SPECTRUM_NOISE_FILTER       200

//////////////////////////////////
// Webserver and JSON strings
#define JSON_STRIPE_STATE_MAX       64
#define JSON_STRIPE_STATIC_COLOR    "color"
#define JSON_STRIPE_TRNASITION      "trans"
#define JSON_STRIPE_OFF             "off"
#define POWER_TIMER_MAX             7201
#define JSON_SAVED_COLOR_ENTRY      COLOR_NAME_LEN + 12 + COLOR_SAVE_BYTES * 10


#endif