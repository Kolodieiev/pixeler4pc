#pragma once

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef ESP32
#include <Arduino.h>
#else
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>

#include "String/WString.h"

//--------------------------------------------------------------------------

#define PI 3.1415926535897932384626433832795

//--------------------------------------------------------------------------

#if defined(_WIN32)
    #include <stdlib.h>
    #define __bswap_16(x) _byteswap_ushort(x)

#elif defined(__ANDROID__)
    static inline uint16_t __bswap_16(uint16_t x) {
        return (uint16_t)((x >> 8) | (x << 8));
    }

#else
    #include <byteswap.h>
#endif // #if defined(_WIN32)

//--------------------------------------------------------------------------

#ifdef _WIN32
#ifndef DT_UNKNOWN
#define DT_UNKNOWN 0
#define DT_REG 1
#define DT_DIR 2
#endif
#endif

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

using TaskFunction_t = void (*)(void*);
using BaseType_t = int;

constexpr BaseType_t pdPASS = 1;
constexpr BaseType_t pdFAIL = 0;

inline BaseType_t xTaskCreatePinnedToCore(
    TaskFunction_t pvTaskCode,
    const char* pcName,
    uint32_t usStackDepth,
    void* pvParameters,
    uint32_t uxPriority,
    void* pxCreatedTask,
    int xCoreID)
{
  try
  {
    std::thread t(pvTaskCode, pvParameters);
    t.detach();
    return pdPASS;
  }
  catch (...)
  {
    return pdFAIL;
  }
}

using SemaphoreHandle_t = std::mutex*;
constexpr unsigned long portMAX_DELAY = (unsigned long)-1;

inline SemaphoreHandle_t xSemaphoreCreateMutex()
{
  return new std::mutex();
}

inline void vSemaphoreDelete(SemaphoreHandle_t handle)
{
  delete handle;
}

inline bool xSemaphoreTake(SemaphoreHandle_t handle, unsigned long timeout_ms)
{
  using namespace std::chrono;

  if (timeout_ms == portMAX_DELAY)
  {
    handle->lock();
    return true;
  }

  auto start = steady_clock::now();
  while (!handle->try_lock())
  {
    if (duration_cast<milliseconds>(steady_clock::now() - start).count() >= timeout_ms)
      return false;
    std::this_thread::sleep_for(milliseconds(1));
  }
  return true;
}

inline void xSemaphoreGive(SemaphoreHandle_t handle)
{
  handle->unlock();
}

//--------------------------------------------------------------------------

#define log_e(fmt, ...) printf("E: " fmt "\n", ##__VA_ARGS__)
#define log_i(fmt, ...) printf("I: " fmt "\n", ##__VA_ARGS__)
#define debug(fmt, ...) printf("I: " fmt, ##__VA_ARGS__)

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

#endif  // #ifdef ESP32
