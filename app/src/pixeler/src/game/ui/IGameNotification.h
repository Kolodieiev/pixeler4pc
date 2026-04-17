#pragma once
#include "pixeler/src/defines.h"
#include "pixeler/src/driver/graphics/DisplayWrapper.h"
#include "pixeler/src/widget/notification/Notification.h"

namespace pixeler
{
  class IGameNotification
  {
  public:
    IGameNotification()
    {
      try
      {
        _notification = new Notification(1);
      }
      catch (const std::bad_alloc& e)
      {
        log_e("%s", e.what());
        esp_restart();
      }
    }

    virtual ~IGameNotification()
    {
      delete _notification;
    }

    virtual void onDraw() = 0;
    IGameNotification(const IGameNotification& rhs) = delete;
    IGameNotification& operator=(const IGameNotification& rhs) = delete;

  protected:
    Notification* _notification{nullptr};
  };
}  // namespace pixeler
