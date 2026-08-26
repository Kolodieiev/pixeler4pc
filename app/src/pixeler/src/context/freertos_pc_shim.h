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
