#pragma once
#include <stdint.h>

#define TOUCHSCREEN_SUPPORT  // Увімкнути підтримку сенсорного екрану

#ifdef TOUCHSCREEN_SUPPORT
#define TOUCH_ROTATION ROTATION_0  // Може мати значення ROTATION_0/ROTATION_90/ROTATION_180/ROTATION_270
#define TOUCH_WIDTH 240
#define TOUCH_HEIGHT 320
#endif  // TOUCHSCREEN_SUPPORT

// -------------------------------- Закріплення віртуальних назв кнопок до виводів МК
// Якщо стан кнопок зчитується з іншого МК, тоді ідентифікатором віртуальної кнопки має бути позиція біта(зліва направо) в масиві байт.

namespace pixeler
{
  enum BtnID : uint8_t
  {
    BTN_OK = 0,
    BTN_BACK,
    BTN_LEFT,
    BTN_RIGHT,
    BTN_UP,
    BTN_DOWN,
  };
}  // namespace pixeler

// -------------------------------- Макрос з ініціалізацією пінів. Між фігурними дужками потрібно додати ініціалізацію пінів, як показано нижче
// -------------------------------- Де Button(uint8_t key_id, bool is_touch)
#define BUTTONS                              \
  {                                          \
      {BTN_OK, Button(BTN_OK, false)},       \
      {BTN_BACK, Button(BTN_BACK, false)},   \
      {BTN_LEFT, Button(BTN_LEFT, false)},   \
      {BTN_RIGHT, Button(BTN_RIGHT, false)}, \
      {BTN_UP, Button(BTN_UP, false)},       \
      {BTN_DOWN, Button(BTN_DOWN, false)},   \
  }

#define PRESS_DURATION_TIME_MS 700UL  // Час(мс), після спливання якого кнопка вважається утримуваною.

#define PRESS_LOCK_TIME_MS 500UL  // Час(мс) блокування кнопки після спрацювання тригеру PRESS
#define CLICK_LOCK_TIME_MS 150UL  // Час(мс) блокування кнопки після спрацювання тригеру CLICK
#define HOLD_LOCK_TIME_MS 100UL   // Час(мс) блокування кнопки після спрацювання тригеру HOLD
