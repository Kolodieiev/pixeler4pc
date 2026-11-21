#include "I2S_Out_Bus.h"

#include <SFML/Audio/SoundChannel.hpp>
#include <SFML/Audio/SoundStream.hpp>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <memory>
#include <mutex>

static constexpr uint32_t CHANNEL_COUNT{2u};
static constexpr size_t STEREO_FRAME_SAMPLES{2u};
static constexpr size_t CHUNK_SAMPLES{512u};
static constexpr size_t RING_CAPACITY{4096u};
static constexpr uint32_t UNDERRUN_WAIT_MS{5u};

namespace pixeler
{
  class I2S_Out_Bus::StreamImpl
  {
  public:
    bool init(const uint32_t sample_rate)
    {
      std::lock_guard<std::mutex> ctrl_lock(_ctrl_mutex);
      if (_is_inited.load(std::memory_order_acquire))
        return true;

      if (sample_rate == 0u)
      {
        log_e("Помилка ініціалізації I2S_Out_Bus: некоректна частота дискретизації");
        return false;
      }

      {
        std::lock_guard<std::mutex> data_lock(_data_mutex);
        _head = 0u;
        _tail = 0u;
        _count = 0u;
        _sample_rate = sample_rate;
        _is_enabled = false;
        _is_stopping = false;
      }

      _sf_stream = std::make_unique<SfmlStream>(*this, _sample_rate);
      _is_inited.store(true, std::memory_order_release);
      log_i("I2S-вихід (SFML) успішно ініціалізовано");
      return true;
    }

    void reconfigSampleRate(const uint32_t sample_rate)
    {
      std::lock_guard<std::mutex> ctrl_lock(_ctrl_mutex);
      if (!_is_inited.load(std::memory_order_acquire))
      {
        log_e("I2S-вихід ще не було ініціалізовано");
        return;
      }

      if (sample_rate == 0u)
      {
        log_e("Некоректна частота дискретизації: 0");
        return;
      }

      stopStreamLocked();
      _sf_stream.reset();
      _sf_stream = std::make_unique<SfmlStream>(*this, sample_rate);

      {
        std::lock_guard<std::mutex> data_lock(_data_mutex);
        _head = 0u;
        _tail = 0u;
        _count = 0u;
        _sample_rate = sample_rate;
        _is_stopping = false;
        _is_enabled = true;
      }

      _sf_stream->play();
    }

    void setI2SCommFMT_LSB(const bool comm_fmt)
    {
      std::lock_guard<std::mutex> ctrl_lock(_ctrl_mutex);
      _comm_fmt = comm_fmt;
    }

    void deinit()
    {
      std::lock_guard<std::mutex> ctrl_lock(_ctrl_mutex);
      if (!_is_inited.load(std::memory_order_acquire))
        return;

      stopStreamLocked();
      _sf_stream.reset();

      {
        std::lock_guard<std::mutex> data_lock(_data_mutex);
        _head = 0u;
        _tail = 0u;
        _count = 0u;
      }

      _is_inited.store(false, std::memory_order_release);
    }

    esp_err_t enable()
    {
      std::lock_guard<std::mutex> ctrl_lock(_ctrl_mutex);
      if (!_is_inited.load(std::memory_order_acquire) || !_sf_stream)
      {
        log_e("I2S-вихід ще не було ініціалізовано");
        return ESP_FAIL;
      }

      {
        std::lock_guard<std::mutex> data_lock(_data_mutex);
        if (_is_enabled)
          return ESP_OK;

        _is_stopping = false;
        _is_enabled = true;
      }

      _sf_stream->play();
      return ESP_OK;
    }

    void disable()
    {
      std::lock_guard<std::mutex> ctrl_lock(_ctrl_mutex);
      if (!_is_inited.load(std::memory_order_acquire))
        return;

      stopStreamLocked();
    }

    void clearBuffer()
    {
      if (!_is_inited.load(std::memory_order_acquire))
        return;

      {
        std::lock_guard<std::mutex> data_lock(_data_mutex);
        _head = 0u;
        _tail = 0u;
        _count = 0u;
      }

      _cv_space.notify_all();
    }

    size_t write(const int16_t* buffer, const size_t buff_len, const bool only_left_chan)
    {
      if (!_is_inited.load(std::memory_order_acquire))
      {
        log_e("I2S-вихід ще не було ініціалізовано");
        return 0u;
      }

      if (!buffer || buff_len == 0u)
        return 0u;

      size_t total_written_samples{0u};
      size_t src_index{0u};

      while (src_index < buff_len)
      {
        std::unique_lock<std::mutex> lock(_data_mutex);

        const size_t min_space = only_left_chan ? STEREO_FRAME_SAMPLES : 1u;
        _cv_space.wait(lock, [this, min_space]()
                       { return (RING_CAPACITY - _count) >= min_space || !_is_enabled || _is_stopping; });

        if (!_is_enabled || _is_stopping)
          break;

        const size_t free_space = RING_CAPACITY - _count;

        if (!only_left_chan)
        {
          const size_t to_write = std::min(buff_len - src_index, free_space);
          const size_t first_part = std::min(to_write, RING_CAPACITY - _head);

          std::memcpy(_ring + _head, buffer + src_index, first_part * sizeof(int16_t));
          const size_t second_part = to_write - first_part;
          if (second_part > 0u)
            std::memcpy(_ring, buffer + src_index + first_part, second_part * sizeof(int16_t));

          _head = (_head + to_write) % RING_CAPACITY;
          _count += to_write;
          src_index += to_write;
          total_written_samples += to_write;
        }
        else
        {
          const size_t pairs_fit = std::min(buff_len - src_index, free_space / STEREO_FRAME_SAMPLES);
          for (size_t i{0u}; i < pairs_fit; ++i)
          {
            const int16_t sample = buffer[src_index + i];
            _ring[_head] = sample;
            if (++_head == RING_CAPACITY)
              _head = 0u;

            _ring[_head] = sample;
            if (++_head == RING_CAPACITY)
              _head = 0u;
          }

          const size_t samples_added = pairs_fit * STEREO_FRAME_SAMPLES;
          _count += samples_added;
          src_index += pairs_fit;
          total_written_samples += samples_added;
        }

        lock.unlock();
        _cv_data.notify_one();
      }

      return total_written_samples * sizeof(int16_t);
    }

    bool isInited() const
    {
      return _is_inited.load(std::memory_order_acquire);
    }

  private:
    class SfmlStream final : public sf::SoundStream
    {
    public:
      SfmlStream(StreamImpl& owner, const uint32_t sample_rate) : _owner{owner}
      {
        initialize(CHANNEL_COUNT, sample_rate, {sf::SoundChannel::FrontLeft, sf::SoundChannel::FrontRight});
      }

      ~SfmlStream() override
      {
        stop();
      }

    private:
      bool onGetData(Chunk& data) override
      {
        return _owner.pullChunk(data);
      }

      void onSeek(const sf::Time /*time_offset*/) override
      {
      }

      StreamImpl& _owner;
    };

    bool pullChunk(sf::SoundStream::Chunk& data)
    {
      std::unique_lock<std::mutex> lock(_data_mutex);

      _cv_data.wait_for(lock, std::chrono::milliseconds(UNDERRUN_WAIT_MS), [this]()
                        { return _count >= CHUNK_SAMPLES || _is_stopping; });

      if (_is_stopping)
        return false;

      // Парна кількість семплів зберігає вирівнювання стерео-каналів L/R
      const size_t to_read = std::min(_count, CHUNK_SAMPLES) & ~size_t{1u};

      if (to_read == 0u)
      {
        std::memset(_out_chunk, 0, sizeof(_out_chunk));
        data.samples = _out_chunk;
        data.sampleCount = CHUNK_SAMPLES;
        return true;
      }

      const size_t first_part = std::min(to_read, RING_CAPACITY - _tail);
      std::memcpy(_out_chunk, _ring + _tail, first_part * sizeof(int16_t));
      const size_t second_part = to_read - first_part;
      if (second_part > 0u)
        std::memcpy(_out_chunk + first_part, _ring, second_part * sizeof(int16_t));

      _tail = (_tail + to_read) % RING_CAPACITY;
      _count -= to_read;

      lock.unlock();
      _cv_space.notify_one();

      data.samples = _out_chunk;
      data.sampleCount = to_read;
      return true;
    }

    void stopStreamLocked()
    {
      {
        std::lock_guard<std::mutex> data_lock(_data_mutex);
        _is_enabled = false;
        _is_stopping = true;
      }

      _cv_space.notify_all();
      _cv_data.notify_all();

      // stop() викликається поза _data_mutex, щоб уникнути дедлоку з onGetData()
      if (_sf_stream)
        _sf_stream->stop();
    }

    std::mutex _ctrl_mutex;
    std::mutex _data_mutex;
    std::condition_variable _cv_space;
    std::condition_variable _cv_data;
    std::unique_ptr<SfmlStream> _sf_stream;
    size_t _head{0u};
    size_t _tail{0u};
    size_t _count{0u};
    uint32_t _sample_rate{16000u};
    int16_t _ring[RING_CAPACITY]{};
    int16_t _out_chunk[CHUNK_SAMPLES]{};
    std::atomic<bool> _is_inited{false};
    bool _is_enabled{false};
    bool _is_stopping{false};
    bool _comm_fmt{false};
  };

  I2S_Out_Bus::I2S_Out_Bus() : _impl{new StreamImpl()}
  {
  }

  I2S_Out_Bus::~I2S_Out_Bus()
  {
    if (_impl)
    {
      _impl->deinit();
      delete _impl;
      _impl = nullptr;
    }
  }

  bool I2S_Out_Bus::init(const uint32_t sample_rate)
  {
    return _impl->init(sample_rate);
  }

  void I2S_Out_Bus::reconfigSampleRate(const uint32_t sample_rate)
  {
    _impl->reconfigSampleRate(sample_rate);
  }

  void I2S_Out_Bus::setI2SCommFMT_LSB(const bool comm_fmt)
  {
    _impl->setI2SCommFMT_LSB(comm_fmt);
  }

  void I2S_Out_Bus::deinit()
  {
    _impl->deinit();
  }

  esp_err_t I2S_Out_Bus::enable()
  {
    return _impl->enable();
  }

  void I2S_Out_Bus::disable()
  {
    _impl->disable();
  }

  void I2S_Out_Bus::clearBuffer()
  {
    _impl->clearBuffer();
  }

  size_t I2S_Out_Bus::write(const int16_t* buffer, const size_t buff_len, const bool only_left_chan)
  {
    return _impl->write(buffer, buff_len, only_left_chan);
  }

  bool I2S_Out_Bus::isInited() const
  {
    return _impl->isInited();
  }

  I2S_Out_Bus _i2s_out;
}  // namespace pixeler
