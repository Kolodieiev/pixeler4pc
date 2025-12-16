#pragma GCC optimize("O3")

#include "Input.h"

#include <stdexcept>

const char STR_UNKNOWN_PIN[] = "Незареєстрована віртуальна кнопка";

namespace pixeler
{
  Input::Input() {}

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

  bool Input::isHolded(BtnID btn_id) const
  {
    try
    {
      return _buttons.at(btn_id).isHolded();
    }
    catch (const std::out_of_range& ignored)
    {
      log_e("%s", STR_UNKNOWN_PIN);
      return false;
    }
  }

  bool Input::isPressed(BtnID btn_id) const
  {
    try
    {
      return _buttons.at(btn_id).isPressed();
    }
    catch (const std::out_of_range& ignored)
    {
      log_e("%s", STR_UNKNOWN_PIN);
      return false;
    }
  }

  bool Input::isReleased(BtnID btn_id) const
  {
    try
    {
      return _buttons.at(btn_id).isReleased();
    }
    catch (const std::out_of_range& ignored)
    {
      log_e("%s", STR_UNKNOWN_PIN);
      return false;
    }
  }

  void Input::lock(BtnID btn_id, unsigned long lock_duration)
  {
    try
    {
      _buttons.at(btn_id).lock(lock_duration);
    }
    catch (const std::out_of_range& ignored)
    {
      log_e("%s", STR_UNKNOWN_PIN);
    }
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