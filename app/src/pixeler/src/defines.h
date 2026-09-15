#pragma once

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <byteswap.h>

#include <chrono>
#include <functional>
#include <mutex>
#include <thread>

#include "String/WString.h"
#include "freertos_pc_shim.h"

//--------------------------------------------------------------------------

#define PI 3.1415926535897932384626433832795

//--------------------------------------------------------------------------

#define MALLOC_CAP_8BIT 0
#define MALLOC_CAP_SPIRAM 0

inline size_t heap_caps_get_free_size(int)
{
  return 10485760;
}

inline bool psramInit()
{
  return true;
}

inline void* ps_malloc(size_t _Size)
{
  return malloc(_Size);
}

inline void* ps_realloc(void* _Memory, size_t _NewSize)
{
  return realloc(_Memory, _NewSize);
}

//--------------------------------------------------------------------------

#define esp_restart() exit(1)

//--------------------------------------------------------------------------

#define delay(ms) std::this_thread::sleep_for(std::chrono::milliseconds(ms))

#define millis() (std::chrono::duration_cast<std::chrono::milliseconds>(   \
                      std::chrono::steady_clock::now().time_since_epoch()) \
                      .count())

inline bool setCpuFrequencyMhz(int ignored)
{
  return true;
}
