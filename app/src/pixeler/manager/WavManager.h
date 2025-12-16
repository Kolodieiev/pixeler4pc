#pragma once
#pragma GCC optimize("O3")
#include <list>
#include <unordered_map>

#include "../defines.h"

namespace pixeler
{
  class WavTrack
  {
  public:
    WavTrack(uint8_t* data_buf, uint32_t data_size) {}

    void play();

    void stop()
    {
    }
    int16_t getNextSample();

    void setOnRepeat(bool repeate)
    {
    }

    bool isPlaying() const
    {
      return false;
    };

    void setVolume(uint8_t volume);

    uint8_t getVolume() const
    {
      return 0;
    }

    // Встановити коефіцієнт фільтрації шуму. По замовченню встановлено 1. Без фільтрації.
    void setFiltrationLvl(uint16_t lvl)
    {
    }

    WavTrack* clone();

  private:
    WavTrack() {}

  private:
  };

  class WavManager
  {
  public:
    ~WavManager();

    /*!
     * @brief
     *      Додати звукову доріжку до списку міксування. Ресурси зі списку міксування звільняються автоматично.
     * @param  track
     *      Вказівник на передзавантажену доріжку.
     * @return
     *      Ідентифікатор на доріжку в списку міксування.
     */
    uint16_t addToMix(WavTrack* track);

    /*!
     * @brief
     *     Видалити доріжку із списку міксування.
     * @param  id
     *     Ідентифікатор, який було присвоєно доріжці, під час додавання до міксу.
     */
    void removeFromMix(uint16_t id);

    /*!
     * @brief
     *     Стартувати задачу відтворення міксу.
     */
    void startMix();

    /*!
     * @brief
     *    Поставити на паузу/відновити вітворення міксу.
     */
    void pauseResume();

  private:
    static void mixTask(void* params);

  private:
  };
}  // namespace pixeler