#pragma once
#pragma GCC optimize("O3")
#include "../IWidget.h"
#include "../text/Label.h"

namespace pixeler
{
  class Notification final : public IWidget
  {
  public:
    explicit Notification(uint16_t widget_ID);
    virtual ~Notification();
    Notification(const Notification& other) = delete;
    Notification& operator=(const Notification& other) = delete;
    Notification(Notification&& other) = delete;
    Notification& operator=(Notification&& other) = delete;

    /**
     * @brief Викликає процедуру малювання віджета на дисплей.
     * Якщо віджет не було змінено, він автоматично пропустить перемальовування.
     *
     */
    virtual void onDraw() override;

    /**
     * @brief STUB! Не викликай!
     */
    Notification* clone(uint16_t id) const override
    {
      log_e("Клонування цього віджета неможливе");
      esp_restart();
      return nullptr;
    }

    /**
     * @brief Повертає ідентифікатор типу.
     * Використовується в системі приведення типу.
     *
     * @return constexpr TypeID
     */
    static constexpr TypeID getTypeID()
    {
      return TypeID::TYPE_ID_NOTIFICATION;
    }

    /**
     * @brief Встановлює шрифт заголовка та кнопок.
     *
     * @param font_ptr
     */
    void setInfoFont(const uint8_t* font_ptr);

    /**
     * @brief Встановлює розмір шрифту заголовка та кнопок.
     *
     * @param size
     */
    void setInfoTextSize(uint16_t size);

    /**
     * @brief Встановлює текст заголовка повідомлення.
     *
     * @param str
     */
    void setTitleText(const String& str);

    /**
     * @brief Встановлює колір фону заголовка.
     *
     * @param color
     */
    void setTitleBackColor(uint16_t color);

    /**
     * @brief Встановлює колір тексту заголовка.
     *
     * @param color
     */
    void setTitleTextColor(uint16_t color);

    /**
     * @brief Встановлює текст лівої кнопки повідомлення.
     *
     * @param str
     */
    void setLeftText(const String& str);

    /**
     * @brief Встановлює колір фону лівої кнопки повідомлення.
     *
     * @param color
     */
    void setLeftBackColor(uint16_t color);

    /**
     * @brief Встановлює колір тексту лівої кнопки повідомлення.
     *
     * @param color
     */
    void setLeftTextColor(uint16_t color);

    /**
     * @brief Встановлює текст правої кнопки повідомлення.
     *
     * @param str
     */
    void setRightText(const String& str);

    /**
     * @brief Встановлює колір фону правої кнопки повідомлення.
     *
     * @param color
     */
    void setRightBackColor(uint16_t color);

    /**
     * @brief Встановлює колір тексту правої кнопки повідомлення.
     *
     * @param color
     */
    void setRightTextColor(uint16_t color);

    /**
     * @brief Встановлює текст тіла повідомлення.
     *
     * @param str
     */
    void setMsgText(const String& str);

    /**
     * @brief Встановлює колір фону тіла повідомлення.
     *
     * @param color
     */
    void setMsgBackColor(uint16_t color);

    /**
     * @brief Встановлює колір тексту тіла повідомлення.
     *
     * @param color
     */
    void setMsgTextColor(uint16_t color);

    /**
     * @brief Встановлює шрифт тіла повідомлення.
     *
     * @param font_ptr
     */
    void setMsgFont(const uint8_t* font_ptr);

    /**
     * @brief Встановлює розмір шрифту тіла повідомлення.
     *
     * @param size
     */
    void setMsgTextSize(uint16_t size);

    /**
     * @brief Встановлює відступ для віджета від країв дисплею по вертикалі.
     * Значення не коригується автоматично, якщо відступ перевищує висоту дисплею.
     *
     * @param margin
     */
    void setHMargin(uint16_t margin)
    {
      _h_margin = margin;
    }

  private:
    using IWidget::getFocusBackColor;
    using IWidget::removeFocus;
    using IWidget::setBorder;
    using IWidget::setCornerRadius;
    using IWidget::setFocus;
    using IWidget::setFocusBackColor;
    using IWidget::setHeight;
    using IWidget::setVisibility;

    Label* _title_lbl{nullptr};
    Label* _msg_lbl{nullptr};
    Label* _left_lbl{nullptr};
    Label* _right_lbl{nullptr};

    uint16_t _h_margin{30};
  };
}  // namespace pixeler