#ifndef _IR32_H_
#define _IR32_H_

#include "IRKeymap.h"

#define RMT_CLK_DIV     100
#define RMT_TICK_10_US (80000000/RMT_CLK_DIV/100000)

#include "Arduino.h"

typedef struct {
    const char* tag;
    uint16_t carrier_freq_hz;
    uint8_t duty_cycle;
    uint8_t bit_length;
    bool invert;
    uint16_t header_mark_us;
    uint16_t header_space_us;
    uint16_t one_mark_us;
    uint16_t one_space_us;
    uint16_t zero_mark_us;
    uint16_t zero_space_us;
    uint16_t end_wait_us;
} rmt_timing_t;

    const rmt_timing_t timing_groups[] = {
      {"", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {"NEC",     38000, 33,   32, 0,      9000,        4500,       560,      1690,       560,      560, 560},
      {"MP",      38000, 33,   24, 0,      1000,         500,       500,      1000,       500,      500, 500}
    };
#endif // _IR32_H_
