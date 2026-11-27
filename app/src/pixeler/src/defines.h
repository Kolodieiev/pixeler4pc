#pragma once

#include <byteswap.h>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <mutex>
#include <thread>

#include "String/WString.h"
#include "freertos_pc_shim.h"

//--------------------------------------------------------------------------
using esp_err_t = int;
#define ESP_OK 0
#define ESP_FAIL -1

//--------------------------------------------------------------------------

#define PI 3.1415926535897932384626433832795

//--------------------------------------------------------------------------

#define MALLOC_CAP_8BIT 0
#define MALLOC_CAP_INTERNAL 0
#define MALLOC_CAP_SPIRAM 0

#define esp_ptr_external_ram(p) false

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

inline void* heap_caps_realloc(void* ptr, size_t size, uint32_t caps)
{
  (void)caps;
  return realloc(ptr, size);
}

//--------------------------------------------------------------------------

typedef struct esp_timer* esp_timer_handle_t;
typedef void (*esp_timer_cb_t)(void* arg);

typedef enum
{
  ESP_TIMER_TASK = 0,
} esp_timer_dispatch_t;

typedef struct
{
  esp_timer_cb_t callback;
  void* arg;
  esp_timer_dispatch_t dispatch_method;
  const char* name;
  bool skip_unhandled_events;
} esp_timer_create_args_t;

inline esp_err_t esp_timer_create(const esp_timer_create_args_t* create_args, esp_timer_handle_t* out_handle)
{
  (void)create_args;
  if (out_handle)
    *out_handle = nullptr;
  return ESP_OK;
}

inline esp_err_t esp_timer_start_once(esp_timer_handle_t timer, uint64_t timeout_us)
{
  (void)timer;
  (void)timeout_us;
  return ESP_OK;
}

inline esp_err_t esp_timer_stop(esp_timer_handle_t timer)
{
  (void)timer;
  return ESP_OK;
}

inline esp_err_t esp_timer_delete(esp_timer_handle_t timer)
{
  (void)timer;
  return ESP_OK;
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
