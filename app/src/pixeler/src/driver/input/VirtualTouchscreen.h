#pragma once
#pragma GCC optimize("O3")
#include "ITouchscreen.h"

namespace pixeler
{
  class VirtualTouchscreen : public ITouchscreen
  {
  public:
    bool __begin() override;
    void __update() override;

    /**
     * @brief Використовується виключно для сумісності на ПК.
     * Оновлює сирий стан дотику від подій миші SFML.
     *
     * @param is_holded true - якщо кнопка миші затиснута, false - інакше
     * @param x Координата X курсора у вікні
     * @param y Координата Y курсора у вікні
     */
    void __setState(bool is_holded, int32_t x, int32_t y);

  private:
    uint16_t _raw_x{0};
    uint16_t _raw_y{0};
    bool _raw_holded{false};
  };
}  // namespace pixeler
