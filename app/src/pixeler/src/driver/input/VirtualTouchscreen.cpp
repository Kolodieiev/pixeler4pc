#include "VirtualTouchscreen.h"

namespace pixeler
{
  bool VirtualTouchscreen::__begin()
  {
    return true;
  }

  void VirtualTouchscreen::__update()
  {
    if (_is_locked)
    {
      if (millis() - _lock_time < _lock_duration)
        return;

      _is_locked = false;
    }

    if (_raw_holded)
    {
      setTouchPos(_raw_x, _raw_y);
      _is_holded = true;
    }
    else
    {
      _is_holded = false;
    }

    processTouch();
  }

  void VirtualTouchscreen::__setState(bool is_holded, int32_t x, int32_t y)
  {
    _raw_holded = is_holded;

    if (is_holded)
    {
      _raw_x = (x < 0) ? 0 : static_cast<uint16_t>(x);
      _raw_y = (y < 0) ? 0 : static_cast<uint16_t>(y);
    }
  }
}  // namespace pixeler
