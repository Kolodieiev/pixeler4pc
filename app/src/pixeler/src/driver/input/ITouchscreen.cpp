#include "ITouchscreen.h"

#include <cstdlib>

#ifdef TOUCHSCREEN_SUPPORT

static constexpr int SWIPE_THRESHOLD = 40;

namespace pixeler
{
  ITouchscreen::ITouchscreen() : _WIDTH{TOUCH_WIDTH},
                                 _HEIGHT{TOUCH_HEIGHT}
  {
  }

  void ITouchscreen::reset()
  {
    _is_pressed = false;
    _is_released = false;
    _swipe = SWIPE_NONE;
  }

  void ITouchscreen::lock(unsigned long lock_duration)
  {
    _is_locked = true;
    _is_holded = false;
    _is_released = false;
    _is_pressed = false;
    _was_pressed = false;
    _has_touch = false;
    _swipe = SWIPE_NONE;
    _lock_duration = lock_duration;
    _lock_time = millis();
  }

  void ITouchscreen::setTouchPos(uint16_t x, uint16_t y)
  {
    if (x > _WIDTH)
      x = _WIDTH;
    if (y > _HEIGHT)
      y = _HEIGHT;

    uint16_t temp;

    switch (_rotation)
    {
      case ROTATION_90:
        temp = x;
        x = _HEIGHT - y;
        y = temp;
        break;
      case ROTATION_180:
        x = _WIDTH - x;
        y = _HEIGHT - y;
        break;
      case ROTATION_270:
        temp = x;
        x = y;
        y = _WIDTH - temp;
        break;
      default:
        break;
    }

    _x_e = x;
    _y_e = y;

    if (!_has_touch)
    {
      _x_s = x;
      _y_s = y;
    }
  }

  void ITouchscreen::processTouch()
  {
    if (!_is_holded)
    {
      if (_has_touch)
      {
        _has_touch = false;
        _was_pressed = false;
        _is_released = true;

        const int x_diff = _x_s - _x_e;
        const int y_diff = _y_s - _y_e;

        if (std::abs(x_diff) > std::abs(y_diff))
        {
          if (std::abs(x_diff) > SWIPE_THRESHOLD)
            _swipe = (x_diff > 0) ? SWIPE_R : SWIPE_L;
        }
        else if (std::abs(y_diff) > SWIPE_THRESHOLD)
        {
          _swipe = (y_diff > 0) ? SWIPE_D : SWIPE_U;
        }
      }

      return;
    }

    if (_was_pressed)
      return;

    if (_has_touch)
    {
      if (millis() - _hold_duration > PRESS_DURATION_TIME_MS)
      {
        _is_pressed = true;
        _was_pressed = true;
      }
    }
    else
    {
      _has_touch = true;
      _hold_duration = millis();
    }
  }
}  // namespace pixeler

#endif  // TOUCHSCREEN_SUPPORT
