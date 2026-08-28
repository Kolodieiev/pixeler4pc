#pragma once
#pragma GCC optimize("O3")

#include <unordered_map>

#include "Button.h"
#include "pixeler/config/input_config.hpp"

namespace pixeler
{
  class Input
  {
  public:
    Input();

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
     * @brief Скидає стан вводу. Метод викликається контекстом перед його першим оновленням, щоб уникнути захоплення стану вводу з попереднього контексту.
     * Ручний виклик цього майже ніколи не потрібний.
     *
     */
    void reset();

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

  private:
    std::unordered_map<BtnID, Button> _buttons BUTTONS;

    unsigned long _hold_lock_time;
    unsigned long _click_lock_time;
    unsigned long _press_lock_time;
  };

  extern Input _input;
}  // namespace pixeler
