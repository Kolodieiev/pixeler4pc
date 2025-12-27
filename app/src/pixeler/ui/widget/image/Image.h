#pragma once
#pragma GCC optimize("O3")
#include "../IWidget.h"

namespace pixeler
{
  class Image final : public IWidget
  {
    // TODO додати анімацію
  public:
    explicit Image(uint16_t widget_ID);
    virtual ~Image();

    /**
     * @brief Викликає процедуру малювання віджета на дисплей.
     * Якщо віджет не було змінено, він автоматично пропустить перемальовування.
     *
     */
    virtual void onDraw() override;

    /**
     * @brief Повертає вказівник на глибоку копію віджета.
     *
     * @param id Ідентифікатор, який буде присвоєно новому віджету.
     * @return Image*
     */
    virtual Image* clone(uint16_t id) const override;

    /**
     * @brief Повертає ідентифікатор типу.
     * Використовується в системі приведення типу.
     *
     * @return constexpr TypeID
     */
    static constexpr TypeID getTypeID()
    {
      return TypeID::TYPE_ID_IMAGE;
    }

    /**
     * @brief Встановлює прапор, який змінює поведінку виводу зображення на дисплей.
     *
     * @param state Якщо true зображення буде виводитися з ігноруванням кольору COLOR_TRANSPARENT. 
     * Інакше буде виводитись, як є.
     */
    void setTransparency(bool state);

    /**
     * @brief Встановлює вказівник на буффер із зображенням.
     * Буфер не буде видалено разом із віджетом.
     *
     * @param image_ptr Вказівник на буфер із зображенням.
     */
    void setSrc(const uint16_t* image_ptr);

  private:
    const uint16_t* _img_ptr{nullptr};
    bool _has_transparency{false};
  };

}  // namespace pixeler