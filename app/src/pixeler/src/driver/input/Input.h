#pragma once
#pragma GCC optimize("O3")
#include <unordered_map>

#include "Button.h"
#include "pixeler/config/input_config.hpp"

#ifdef TOUCHSCREEN_SUPPORT
#include "VirtualTouchscreen.h"
#endif  // TOUCHSCREEN_SUPPORT

namespace pixeler
{
  class Input
  {
  public:
    Input();
    ~Input();

    /**
     * @brief Ініціалізує драйвер вводу відповідно до налаштувань. Не потрібно викликати метод самостійно.
     *
     */
    void __init();

    /**
     * @brief Оновлює стан вводу. Не потрібно викликати метод самостійно.
     *
     */
    void __update();

    /**
     * @brief Вмикає фізичний пін та ініціалізує його в тому режимі, який було передано в конструктор під час створення об'єкта віртуального піна з цим номером.
     * Якщо віртуальний пін з таким номером відсутній, буде викликано виключення std::out_of_range.
     *
     * @param btn_id Номер віртуального піна
     */
    void enableBtn(BtnID btn_id);

    /**
     * @brief Вимикає пін, переводить його в режим високоімпедансного входу, та скидає стани віртуального піна з цим номером.
     * Це може бути корисним, якщо пін не використовується взагалі в поточному контексті або не використовуєтсья тривалий час.
     * Вимкнення піна на тривалий період, може трохи скоротити споживання струму мікроконтролером.
     * Якщо віртуальний пін з таким номером відсутній, буде викликано виключення std::out_of_range.
     *
     * @param btn_id Номер віртуального піна
     */
    void disableBtn(BtnID btn_id);

    /**
     * @brief Перевіряє чи фіксується в даний момент утримання кнопки на віртуальному піні з таким номером.
     *
     * @param btn_id Номер віртуального піна
     * @return true - Якщо пін утримується. false - Інакше
     */
    bool isHolded(BtnID btn_id);

    /**
     * @brief Перевіряє чи утримується пін більше n мілісекунд, що задано в налаштуваннях вводу.
     *
     * @param btn_id Номер віртуального піна
     * @return true - Якщо пін утримується більше n мілісекунд. false - Інакше
     */
    bool isPressed(BtnID btn_id);

    /**
     * @brief Перевіряє чи було пін раніше активовано натисканням та відпущено.
     *
     * @param btn_id Номер віртуального піна
     * @return true - Якщо пін раніше було активовано та відпущено. false - Інакше
     */
    bool isReleased(BtnID btn_id);

    /**
     * @brief Встановлює час автоматичного блокування кнопки,
     * яке буде застосовано до неї після позитивного зчитування стану "is_holded".
     *
     * @param lock_duration_ms Час блокування в мілісекнудах
     */
    void setHoldLockTime(unsigned long lock_duration_ms);

    /**
     * @brief Встановлює час автоматичного блокування кнопки,
     * яке буде застосовано до неї після позитивного зчитування стану "is_released".
     *
     * @param lock_duration_ms Час блокування в мілісекнудах
     */
    void setClickLockTime(unsigned long lock_duration_ms);

    /**
     * @brief Встановлює час автоматичного блокування кнопки,
     * яке буде застосовано до неї після позитивного зчитування стану "is_pressed".
     *
     * @param lock_duration_ms Час блокування в мілісекнудах
     */
    void setPressLockTime(unsigned long lock_duration_ms);

    /**
     * @brief Використовується виключно для сумісності на ПК.
     *
     * @param btn_id
     * @param is_holded
     */
    void __setState(BtnID btn_id, bool is_holded);

#ifdef TOUCHSCREEN_SUPPORT
    /**
     * @brief Перевіряє чи фіксується в даний момент утримання дотику на сенсорному екрані.
     *
     * @return true - Якщо екран утримується. false - Інакше
     */
    bool isHolded() const;

    /**
     * @brief Перевіряє чи утримується дотик на сенсорному екрані більше n мілісекунд.
     *
     * @return true - Якщо дотик утримується більше n мілісекунд. false - Інакше
     */
    bool isPressed() const;

    /**
     * @brief Перевіряє чи було дотик на сенсорному екрані завершено (відпущено).
     *
     * @return true - Якщо дотик щойно відпущено. false - Інакше
     */
    bool isReleased() const;

    /**
     * @brief Повертає напрямок зафіксованого свайпу.
     *
     * @return ITouchscreen::Swipe
     */
    ITouchscreen::Swipe getSwipe();

    /**
     * @brief Повертає поточну або останню координату X дотику.
     *
     * @return uint16_t
     */
    uint16_t getTouchX() const;

    /**
     * @brief Повертає поточну або останню координату Y дотику.
     *
     * @return uint16_t
     */
    uint16_t getTouchY() const;

    /**
     * @brief Використовується виключно для сумісності на ПК.
     *
     * @param is_holded
     * @param x
     * @param y
     */
    void __setTouchState(bool is_holded, int32_t x, int32_t y);
#endif  // TOUCHSCREEN_SUPPORT

  private:
    std::unordered_map<BtnID, Button> _buttons BUTTONS;
#ifdef TOUCHSCREEN_SUPPORT
    VirtualTouchscreen* _touchscreen{nullptr};
#endif  // TOUCHSCREEN_SUPPORT
    unsigned long _hold_lock_time;
    unsigned long _click_lock_time;
    unsigned long _press_lock_time;
  };

  extern Input _input;
}  // namespace pixeler
