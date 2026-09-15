// freertos_pc_shim.h — PC-реалізація підмножини FreeRTOS API, яку використовує IContext
#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#define log_e(fmt, ...) printf("E: " fmt "\n", ##__VA_ARGS__)
#define log_i(fmt, ...) printf("I: " fmt "\n", ##__VA_ARGS__)
#define debug(fmt, ...) printf("I: " fmt, ##__VA_ARGS__)

using TaskFunction_t = void (*)(void*);
using BaseType_t = int;
using TickType_t = uint32_t;

constexpr int pdTRUE = 1;
constexpr int pdFALSE = 0;

constexpr BaseType_t pdPASS = 1;
constexpr BaseType_t pdFAIL = 0;

// Кидається з vTaskDelete(nullptr) для штатного завершення поточного таска.
// Перехоплюється у обгортці, створеній xTaskCreatePinnedToCore.
struct TaskDeletedException
{
};

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
    std::thread t([pvTaskCode, pvParameters]()
                  {
      try
      {
        pvTaskCode(pvParameters);
      }
      catch (const TaskDeletedException&)
      {
        // vTaskDelete(nullptr) — штатне самовидалення, не помилка.
      } });
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

inline TickType_t pdMS_TO_TICKS(unsigned long ms)
{
  return static_cast<TickType_t>(ms);
}
// ---- TaskHandle_t: обгортка над std::thread::id, що підтримує ініціалізацію nullptr'ом ----
class TaskHandleImpl
{
public:
  TaskHandleImpl() = default;
  TaskHandleImpl(std::nullptr_t) {}
  TaskHandleImpl(std::thread::id id) : _id(id), _valid(true) {}

  bool operator==(const TaskHandleImpl& other) const
  {
    return _valid == other._valid && (!_valid || _id == other._id);
  }
  bool operator!=(const TaskHandleImpl& other) const
  {
    return !(*this == other);
  }

private:
  std::thread::id _id{};
  bool _valid{false};
};

using TaskHandle_t = TaskHandleImpl;

inline TaskHandle_t xTaskGetCurrentTaskHandle()
{
  return TaskHandle_t(std::this_thread::get_id());
}

inline void vTaskDelete(TaskHandle_t task_to_delete)
{
  if (task_to_delete != nullptr && task_to_delete != xTaskGetCurrentTaskHandle())
  {
    log_e("Підтримується лише самовидалення (nullptr або handle поточного таска). \nвидалення \"чужого\" таска не має ефекту");
    return;
  }

  throw TaskDeletedException();
}

// ---- черга ----
class QueueImpl
{
public:
  QueueImpl(size_t depth, size_t item_size)
      : _depth(depth), _item_size(item_size) {}

  bool send(const void* item, TickType_t ticks_to_wait)
  {
    std::unique_lock<std::mutex> lock(_mutex);

    bool ok;
    if (ticks_to_wait == portMAX_DELAY)
    {
      _not_full.wait(lock, [this]
                     { return _queue.size() < _depth; });
      ok = true;
    }
    else
    {
      ok = _not_full.wait_for(lock, std::chrono::milliseconds(ticks_to_wait),
                              [this]
                              { return _queue.size() < _depth; });
    }

    if (!ok)
      return false;

    std::vector<uint8_t> buf(_item_size);
    std::memcpy(buf.data(), item, _item_size);
    _queue.push(std::move(buf));

    lock.unlock();
    _not_empty.notify_one();
    return true;
  }

  bool receive(void* out_item, TickType_t ticks_to_wait)
  {
    std::unique_lock<std::mutex> lock(_mutex);

    if (ticks_to_wait == 0)
    {
      if (_queue.empty())
        return false;
    }
    else if (ticks_to_wait == portMAX_DELAY)
    {
      _not_empty.wait(lock, [this]
                      { return !_queue.empty(); });
    }
    else
    {
      bool ok = _not_empty.wait_for(lock, std::chrono::milliseconds(ticks_to_wait),
                                    [this]
                                    { return !_queue.empty(); });
      if (!ok)
        return false;
    }

    std::memcpy(out_item, _queue.front().data(), _item_size);
    _queue.pop();

    lock.unlock();
    _not_full.notify_one();
    return true;
  }

private:
  std::mutex _mutex;
  std::condition_variable _not_empty;
  std::condition_variable _not_full;
  std::queue<std::vector<uint8_t>> _queue;
  size_t _depth;
  size_t _item_size;
};

using QueueHandle_t = QueueImpl*;

inline QueueHandle_t xQueueCreate(size_t depth, size_t item_size)
{
  return new QueueImpl(depth, item_size);
}

inline int xQueueSend(QueueHandle_t q, const void* item, TickType_t ticks_to_wait)
{
  return q->send(item, ticks_to_wait) ? pdTRUE : pdFALSE;
}

inline int xQueueReceive(QueueHandle_t q, void* out_item, TickType_t ticks_to_wait)
{
  return q->receive(out_item, ticks_to_wait) ? pdTRUE : pdFALSE;
}

inline void vQueueDelete(QueueHandle_t q)
{
  delete q;
}
