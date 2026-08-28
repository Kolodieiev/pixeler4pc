#pragma GCC optimize("O3")

#include "Input.h"

#include <stdexcept>

const char STR_UNKNOWN_PIN[] = "Незареєстрована віртуальна кнопка";

namespace pixeler
{
  Input::Input() : _hold_lock_time{HOLD_LOCK_TIME_MS}, _click_lock_time{CLICK_LOCK_TIME_MS}, _press_lock_time{PRESS_LOCK_TIME_MS}
  {
  }

  void Input::__init()
  {
  }

  void Input::__update()
  {
  }

  void Input::reset()
  {
    for (auto&& btn : _buttons)
      btn.second.reset();
  }

  void Input::enableBtn(BtnID btn_id)
  {
    try
    {
      _buttons.at(btn_id).enable();
    }
    catch (const std::out_of_range& ignored)
    {
      log_e("%s", STR_UNKNOWN_PIN);
    }
  }

  void Input::disableBtn(BtnID btn_id)
  {
    try
    {
      _buttons.at(btn_id).disable();
    }
    catch (const std::out_of_range& ignored)
    {
      log_e("%s", STR_UNKNOWN_PIN);
    }
  }

  bool Input::isHolded(BtnID btn_id)
  {
    try
    {
      bool result = _buttons.at(btn_id).isHolded();

      if (result)
        _buttons.at(btn_id).lock(_hold_lock_time);

      return result;
    }
    catch (const std::out_of_range& ignored)
    {
      log_e("%s : id[%u]", STR_UNKNOWN_PIN);
      return false;
    }
  }

  bool Input::isPressed(BtnID btn_id)
  {
    try
    {
      bool result = _buttons.at(btn_id).isPressed();

      if (result)
        _buttons.at(btn_id).lock(_press_lock_time);

      return result;
    }
    catch (const std::out_of_range& ignored)
    {
      log_e("%s : id[%u]", STR_UNKNOWN_PIN);
      return false;
    }
  }

  bool Input::isReleased(BtnID btn_id)
  {
    try
    {
      bool result = _buttons.at(btn_id).isReleased();

      if (result)
        _buttons.at(btn_id).lock(_click_lock_time);

      return result;
    }
    catch (const std::out_of_range& ignored)
    {
      log_e("%s : id[%u]", STR_UNKNOWN_PIN);
      return false;
    }
  }

  void Input::setHoldLockTime(unsigned long lock_duration_ms)
  {
    _hold_lock_time = lock_duration_ms;
  }

  void Input::setClickLockTime(unsigned long lock_duration_ms)
  {
    _click_lock_time = lock_duration_ms;
  }

  void Input::setPressLockTime(unsigned long lock_duration_ms)
  {
    _press_lock_time = lock_duration_ms;
  }

  void Input::__setState(BtnID btn_id, bool is_holded)
  {
    try
    {
      _buttons.at(btn_id).__extUpdate(is_holded);
    }
    catch (const std::out_of_range& ignored)
    {
      log_e("%s", STR_UNKNOWN_PIN);
    }
  }

  Input _input;
}  // namespace pixeler
