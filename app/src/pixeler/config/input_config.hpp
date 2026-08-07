#pragma once
#include <stdint.h>

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
}

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

#define PRESS_DURATION (unsigned long)1000  // Час, після спливання якого кнопка вважається утримуваною.

#define PRESS_LOCK (unsigned long)700
#define CLICK_LOCK (unsigned long)250
#define HOLD_LOCK (unsigned long)150
