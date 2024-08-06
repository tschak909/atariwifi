#ifndef FNHWTIMER_H
#define FNHWTIMER_H

#include "esp_idf_version.h"

#if defined(CONFIG_IDF_TARGET_ESP32S3)
#include "driver/gptimer.h"
#else
#include "driver/timer.h"
#endif

#include "soc/soc.h"
#include "soc/timer_group_struct.h"
#include "soc/clk_tree_defs.h"

// hardware timer parameters for bit-banging I/O

#if defined(CONFIG_IDF_TARGET_ESP32)
#define TIMER_DIVIDER         (2)  //  Hardware timer clock divider
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
#define TIMER_DIVIDER         (8)  //  Hardware timer clock divider
#endif

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#define TIMER_SCALE           (APB_CLK_FREQ / TIMER_DIVIDER)  // convert counter value to seconds
#else
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds
#endif
#define TIMER_100NS_FACTOR    (TIMER_SCALE / 10000000)
#define TIMER_ADJUST          0 // substract this value to adjust for overhead

class HardwareTimer
{
private:
  struct fn_timer_t
  {
    uint32_t tn;
    uint32_t t0;
  } fn_timer;


#if defined(CONFIG_IDF_TARGET_ESP32S3)
gptimer_handle_t gptimer;
gptimer_config_t fn_config;
//gptimer_alarm_config_t alarm_config;
#else
timer_config_t fn_config;
#endif

public:
  void config();

#if defined(CONFIG_IDF_TARGET_ESP32S3)

  void reset() { gptimer_set_raw_count(gptimer, 0); };
  void latch() {};
  void read() { 
    uint64_t count; 
    gptimer_get_raw_count(gptimer, &count);
    fn_timer.t0 = count & 0xFFFFFFFF;
  };

#else

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
  void reset() { TIMERG1.hw_timer[TIMER_1].loadlo.val = 0; TIMERG1.hw_timer[TIMER_1].load.val = 0; };
  void latch() { TIMERG1.hw_timer[TIMER_1].update.val = 0; };
  void read() { fn_timer.t0 = TIMERG1.hw_timer[TIMER_1].lo.val; };
#else
  void reset() { TIMERG1.hw_timer[TIMER_1].load_low = 0; TIMERG1.hw_timer[TIMER_1].reload = 0; };
  void latch() { TIMERG1.hw_timer[TIMER_1].update = 0; };
  void read() { fn_timer.t0 = TIMERG1.hw_timer[TIMER_1].cnt_low; };
#endif

#endif

  bool timeout() { return (fn_timer.t0 > fn_timer.tn); };
  void wait() { do{latch(); read();} while (!timeout()); };
  void alarm_set(int s) { fn_timer.tn = fn_timer.t0 + s * TIMER_100NS_FACTOR - TIMER_ADJUST; };
  void alarm_snooze(int s) { fn_timer.tn += s * TIMER_100NS_FACTOR - TIMER_ADJUST; };
};

extern HardwareTimer fnTimer;

#endif // FNHWTIMER_H
