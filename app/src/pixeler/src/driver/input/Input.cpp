#include "Input.h"

#include <stdexcept>

static constexpr char STR_UNKNOWN_PIN[] = "Незареєстрована віртуальна кнопка";

namespace pixeler
{
  Input::Input() : _hold_lock_time{HOLD_LOCK_TIME_MS},
                   _click_lock_time{CLICK_LOCK_TIME_MS},
                   _press_lock_time{PRESS_LOCK_TIME_MS}
  {
  }

  Input::~Input()
  {
#ifdef TOUCHSCREEN_SUPPORT
    delete _touchscreen;
    _touchscreen = nullptr;
#endif  // TOUCHSCREEN_SUPPORT
  }

  void Input::__init()
  {
#ifdef TOUCHSCREEN_SUPPORT
    if (!_touchscreen)
    {
      _touchscreen = new VirtualTouchscreen();
      _touchscreen->__begin();
    }
#endif  // TOUCHSCREEN_SUPPORT
  }

  void Input::__update()
  {
#ifdef TOUCHSCREEN_SUPPORT
    if (_touchscreen)
    {
      _touchscreen->reset();
      _touchscreen->__update();
    }
#endif  // TOUCHSCREEN_SUPPORT
  }

  void Input::enableBtn(BtnID btn_id)
  {
    try
    {
      _buttons.at(btn_id).enable();
    }
    catch (const std::out_of_range& ignored)
    {
      log_e("%s : id[%u]", STR_UNKNOWN_PIN, static_cast<unsigned>(btn_id));
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
      log_e("%s : id[%u]", STR_UNKNOWN_PIN, static_cast<unsigned>(btn_id));
    }
  }

  bool Input::isHolded(BtnID btn_id)
  {
    try
    {
      Button& btn = _buttons.at(btn_id);
      const bool result = btn.isHolded();

      if (result)
        btn.lock(_hold_lock_time);

      return result;
    }
    catch (const std::out_of_range& ignored)
    {
      log_e("%s : id[%u]", STR_UNKNOWN_PIN, static_cast<unsigned>(btn_id));
      return false;
    }
  }

  bool Input::isPressed(BtnID btn_id)
  {
    try
    {
      Button& btn = _buttons.at(btn_id);
      const bool result = btn.isPressed();

      if (result)
        btn.lock(_press_lock_time);

      return result;
    }
    catch (const std::out_of_range& ignored)
    {
      log_e("%s : id[%u]", STR_UNKNOWN_PIN, static_cast<unsigned>(btn_id));
      return false;
    }
  }

  bool Input::isReleased(BtnID btn_id)
  {
    try
    {
      Button& btn = _buttons.at(btn_id);
      const bool result = btn.isReleased();

      if (result)
        btn.lock(_click_lock_time);

      return result;
    }
    catch (const std::out_of_range& ignored)
    {
      log_e("%s : id[%u]", STR_UNKNOWN_PIN, static_cast<unsigned>(btn_id));
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
      log_e("%s : id[%u]", STR_UNKNOWN_PIN, static_cast<unsigned>(btn_id));
    }
  }

#ifdef TOUCHSCREEN_SUPPORT
  bool Input::isHolded() const
  {
    const bool result = _touchscreen->isHolded();
    if (result)
      _touchscreen->lock(_hold_lock_time);

    return result;
  }

  bool Input::isPressed() const
  {
    const bool result = _touchscreen->isPressed();
    if (result)
      _touchscreen->lock(_press_lock_time);

    return result;
  }

  bool Input::isReleased() const
  {
    const bool result = _touchscreen->isReleased();
    if (result)
      _touchscreen->lock(_click_lock_time);

    return result;
  }

  ITouchscreen::Swipe Input::getSwipe()
  {
    const ITouchscreen::Swipe swipe = _touchscreen->getSwipe();
    if (swipe != ITouchscreen::SWIPE_NONE)
      _touchscreen->lock(_click_lock_time);

    return swipe;
  }

  uint16_t Input::getTouchX() const
  {
    return _touchscreen->getX();
  }

  uint16_t Input::getTouchY() const
  {
    return _touchscreen->getY();
  }

  void Input::__setTouchState(bool is_holded, int32_t x, int32_t y)
  {
    if (_touchscreen)
      _touchscreen->__setState(is_holded, x, y);
  }
#endif  // TOUCHSCREEN_SUPPORT

  Input _input;
}  // namespace pixeler
