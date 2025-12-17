#pragma GCC optimize("O3")

#include "Button.h"

#include "../../setup/input_setup.hpp"

namespace pixeler
{
  Button::Button(uint8_t btn_id, bool is_touch) : _btn_id{btn_id}
  {
    enable();
  }

  void Button::lock(unsigned long lock_duration)
  {
    _is_locked = true;
    _is_holded = false;
    _is_released = false;
    _is_pressed = false;
    _is_touched = false;
    _lock_duration = lock_duration;
    _lock_time = millis();
  }

  void Button::__update()
  {
    if (!_is_enabled)
      return;

    if (_is_locked)
    {
      if (millis() - _lock_time < _lock_duration)
        return;
      _is_locked = false;
    }

    updateState();
  }

  void Button::__extUpdate(bool is_holded)
  {
    if (!_is_enabled)
      return;

    if (_is_locked)
    {
      if (millis() - _lock_time < _lock_duration)
        return;
      _is_locked = false;
    }

    _is_holded = is_holded;

    updateState();
  }

  void Button::updateState()
  {
    if (!_is_holded)
    {
      if (_is_touched)
      {
        _is_touched = false;
        _is_released = true;
      }

      return;
    }

    if (_is_touched)
    {
      if (!_is_pressed)
        if (millis() - _hold_duration > PRESS_DURATION)
        {
          _is_touched = false;
          _is_pressed = true;
        }
    }
    else
    {
      _is_touched = true;
      _is_released = false;
      _hold_duration = millis();
    }
  }

  void Button::reset()
  {
    _is_holded = false;
    _is_pressed = false;
    _is_released = false;
  }

  void Button::enable()
  {
    _is_enabled = true;
  }

  void Button::disable()
  {
    _is_enabled = false;
    reset();
  }
}  // namespace pixeler