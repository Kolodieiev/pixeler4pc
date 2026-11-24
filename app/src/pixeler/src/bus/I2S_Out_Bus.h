/**
 * @file I2S_Out_Bus.h
 * @brief ПК-врапер над SFML Audio з інтерфейсом I2S_Out_Bus для порту прошивки
 * @details Доступ до класу повинен виконуватися тільки через глобальний об'єкт "_i2s_out".
 */
#pragma once

#include <cstddef>
#include <cstdint>

#include "defines.h"

//TODO

namespace pixeler
{
  class I2S_Out_Bus
  {
  public:
    I2S_Out_Bus();
    ~I2S_Out_Bus();

    I2S_Out_Bus(const I2S_Out_Bus&) = delete;
    I2S_Out_Bus& operator=(const I2S_Out_Bus&) = delete;
    I2S_Out_Bus(I2S_Out_Bus&&) = delete;
    I2S_Out_Bus& operator=(I2S_Out_Bus&&) = delete;

    /**
     * @brief Ініціалізує аудіовихід із заданою частотою дискретизації.
     *
     * @param sample_rate Кількість вибірок за секунду.
     * @return true - Якщо ініціалізацію виконано успішно.
     * @return false - Інакше.
     */
    bool init(uint32_t sample_rate = 16000ul);

    /**
     * @brief Змінює частоту дискретизації.
     * Автоматично зупиняє потік перед зміною, очищує буфер та запускає відтворення знову.
     *
     * @param sample_rate Кількість вибірок за секунду.
     */
    void reconfigSampleRate(uint32_t sample_rate);

    /**
     * @brief Заглушка для сумісності інтерфейсу I2S (не впливає на вивід через SFML).
     *
     * @param comm_fmt Прапор формату I2S LSB.
     */
    void setI2SCommFMT_LSB(bool comm_fmt);

    /**
     * @brief Деініціалізує аудіовихід та звільняє ресурси потоку відтворення.
     */
    void deinit();

    /**
     * @brief Активує відтворення аудіопотоку.
     *
     * @return esp_err_t ESP_OK у разі успіху, ESP_FAIL якщо аудіовихід не ініціалізовано.
     */
    esp_err_t enable();

    /**
     * @brief Зупиняє відтворення аудіопотоку.
     */
    void disable();

    /**
     * @brief Очищує внутрішній кільцевий буфер аудіовиходу.
     */
    void clearBuffer();

    /**
     * @brief Копіює дані із зовнішнього буфера в буфер аудіопотоку з блокуванням при заповненні.
     *
     * @param buffer Зовнішній буфер з 16-бітними PCM-семплами.
     * @param buff_len Кількість int16_t семплів у буфері.
     * @param only_left_chan Прапор, який вказує на тип аудіоданих у буфері.
     * Якщо true - дані перед відтворенням будуть розширені по схемі left_chann -> stereo.
     * Якщо false - дані будуть відтворені в стерео режимі як є.
     * @return size_t - Кількість успішно скопійованих байтів.
     */
    size_t write(const int16_t* buffer, size_t buff_len, bool only_left_chan = false);

    /**
     * @brief Повертає стан ініціалізації аудіовиходу.
     *
     * @return true - Якщо аудіовихід успішно ініціалізовано.
     * @return false - Інакше.
     */
    bool isInited() const;

  private:
    class StreamImpl;
    StreamImpl* _impl{nullptr};
  };

  /**
   * @brief Глобальний об'єкт для виводу аудіо.
   */
  extern I2S_Out_Bus _i2s_out;
}  // namespace pixeler
