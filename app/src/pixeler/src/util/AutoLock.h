#pragma once
#pragma GCC optimize("O3")

#include "pixeler/src/defines.h"

namespace pixeler
{
  class AutoLock
  {
  public:
    explicit AutoLock(SemaphoreHandle_t mutex)
        : _mutex(mutex)
    {
      xSemaphoreTake(_mutex, portMAX_DELAY);
    }

    ~AutoLock()
    {
      xSemaphoreGive(_mutex);
    }

    AutoLock(const AutoLock&) = delete;
    AutoLock& operator=(const AutoLock&) = delete;

  private:
    SemaphoreHandle_t _mutex;
  };
}  // namespace pixeler