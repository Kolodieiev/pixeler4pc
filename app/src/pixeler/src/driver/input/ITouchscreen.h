#pragma once
#pragma GCC optimize("O3")
#include "../../defines.h"
#include "pixeler/config/input_config.hpp"

#ifdef TOUCHSCREEN_SUPPORT

namespace pixeler
{
  class ITouchscreen
  {
  public:
    enum Swipe : uint8_t
    {
      SWIPE_NONE = 0,
      SWIPE_L,
      SWIPE_R,
      SWIPE_U,
      SWIPE_D
    };

    enum Rotation : uint8_t
    {
      ROTATION_0 = 0,
      ROTATION_90,
      ROTATION_180,
      ROTATION_270
    };

    ITouchscreen();
    virtual ~ITouchscreen() = default;

    virtual bool __begin() = 0;
    virtual void __update() = 0;

    void reset();
    void lock(unsigned long lock_duration);

    bool isHolded() const
    {
      return _is_holded;
    }

    bool isPressed() const
    {
      return _is_pressed;
    }

    bool isReleased() const
    {
      return _is_released;
    }

    Swipe getSwipe() const
    {
      return _swipe;
    }

    uint16_t getX() const
    {
      return _x_e;
    }

    uint16_t getY() const
    {
      return _y_e;
    }

    void setRotation(Rotation rotation)
    {
      _rotation = rotation;
    }

  protected:
    unsigned long _lock_time{0};
    unsigned long _lock_duration{0};
    unsigned long _hold_duration{0};
    const uint16_t _WIDTH;
    const uint16_t _HEIGHT;
    uint16_t _x_s{0};
    uint16_t _y_s{0};
    uint16_t _x_e{0};
    uint16_t _y_e{0};
    Rotation _rotation{ROTATION_0};
    Swipe _swipe{SWIPE_NONE};
    bool _is_locked{false};
    bool _has_touch{false};
    bool _is_holded{false};
    bool _is_pressed{false};
    bool _was_pressed{false};
    bool _is_released{false};

    void setTouchPos(uint16_t x, uint16_t y);
    void processTouch();
  };
}  // namespace pixeler

#endif  // TOUCHSCREEN_SUPPORT
