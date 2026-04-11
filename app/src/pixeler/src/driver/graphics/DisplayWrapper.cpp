#pragma GCC optimize("O3")
#include "DisplayWrapper.h"

#include <cmath>

#include "pixeler/src/util/img/BmpUtil.h"

namespace pixeler
{
  DisplayWrapper::DisplayWrapper()
  {
  }

  DisplayWrapper::~DisplayWrapper()
  {
  }

  void DisplayWrapper::fillScreen(uint16_t color)
  {
#ifdef GRAPHICS_ENABLED
    _canvas->fillScreen(color);
    _is_buff_changed = true;
#endif  // #ifdef GRAPHICS_ENABLED
  }

  void DisplayWrapper::setCursor(int16_t x, int16_t y)
  {
#ifdef GRAPHICS_ENABLED
    _canvas->setCursor(x, y);
#endif  // #ifdef GRAPHICS_ENABLED
  }

  void DisplayWrapper::setTextWrap(bool state)
  {
#ifdef GRAPHICS_ENABLED
    _canvas->setTextWrap(state);
#endif  // #ifdef GRAPHICS_ENABLED
  }

  void DisplayWrapper::setTextBound(int16_t x, int16_t y, int16_t w, int16_t h)
  {
#ifdef GRAPHICS_ENABLED
    _is_buff_changed = true;
    return _canvas->setTextBound(x, y, w, h);
#endif  // #ifdef GRAPHICS_ENABLED
  }

  void DisplayWrapper::resetTextBound()
  {
#ifdef GRAPHICS_ENABLED
    _is_buff_changed = true;
    return _canvas->resetTextBound();
#endif  // #ifdef GRAPHICS_ENABLED
  }

  size_t DisplayWrapper::print(const char* str)
  {
#ifdef GRAPHICS_ENABLED
    _is_buff_changed = true;
    return _canvas->print(str);
#else   // not def GRAPHICS_ENABLED
    return 0;
#endif  // #ifdef GRAPHICS_ENABLED
  }

  void DisplayWrapper::setPPAState(bool state)
  {
  }

  bool DisplayWrapper::isPPAEnabled() const
  {
    return false;
  }

  void DisplayWrapper::setFont(const uint8_t* font)
  {
#ifdef GRAPHICS_ENABLED
    _canvas->setFont(font);
#endif  // #ifdef GRAPHICS_ENABLED
  }

  void DisplayWrapper::setTextSize(uint8_t size)
  {
#ifdef GRAPHICS_ENABLED
    _canvas->setTextSize(size, size, 0);
#endif  // #ifdef GRAPHICS_ENABLED
  }

  void DisplayWrapper::setTextColor(uint16_t color)
  {
#ifdef GRAPHICS_ENABLED
    _canvas->setTextColor(color);
#endif  // #ifdef GRAPHICS_ENABLED
  }

  void DisplayWrapper::calcTextBounds(const char* str, int16_t x, int16_t y, int16_t& x_out, int16_t& y_out, uint16_t& w_out, uint16_t& h_out)
  {
#ifdef GRAPHICS_ENABLED
    _canvas->getTextBounds(str, x, y, x_out, y_out, w_out, h_out);
#endif  // #ifdef GRAPHICS_ENABLED
  }

  void DisplayWrapper::drawPixel(int16_t x, int16_t y, uint16_t color)
  {
#ifdef GRAPHICS_ENABLED
    _canvas->drawPixel(x, y, color);
    _is_buff_changed = true;
#endif  // #ifdef GRAPHICS_ENABLED
  }

  void DisplayWrapper::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
  {
#ifdef GRAPHICS_ENABLED
    _canvas->drawLine(x0, y0, x1, y1, color);
    _is_buff_changed = true;
#endif  // #ifdef GRAPHICS_ENABLED
  }

  void DisplayWrapper::drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color)
  {
#ifdef GRAPHICS_ENABLED
    _canvas->drawCircle(x, y, r, color);
    _is_buff_changed = true;
#endif  // #ifdef GRAPHICS_ENABLED
  }

  void DisplayWrapper::fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color)
  {
#ifdef GRAPHICS_ENABLED
    _canvas->fillCircle(x, y, r, color);
    _is_buff_changed = true;
#endif  // #ifdef GRAPHICS_ENABLED
  }

  void DisplayWrapper::drawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color)
  {
#ifdef GRAPHICS_ENABLED
    _canvas->drawRect(x, y, w, h, color);
    _is_buff_changed = true;
#endif  // #ifdef GRAPHICS_ENABLED
  }

  void DisplayWrapper::fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color)
  {
#ifdef GRAPHICS_ENABLED
    _canvas->fillRect(x, y, w, h, color);
    _is_buff_changed = true;
#endif  // #ifdef GRAPHICS_ENABLED
  }

  void DisplayWrapper::drawRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius, uint16_t color)
  {
#ifdef GRAPHICS_ENABLED
    _canvas->drawRoundRect(x, y, w, h, radius, color);
    _is_buff_changed = true;
#endif  // #ifdef GRAPHICS_ENABLED
  }

  void DisplayWrapper::fillRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius, uint16_t color)
  {
#ifdef GRAPHICS_ENABLED
    _canvas->fillRoundRect(x, y, w, h, radius, color);
    _is_buff_changed = true;
#endif  // #ifdef GRAPHICS_ENABLED
  }

  void DisplayWrapper::drawBitmap(int16_t x, int16_t y, const uint16_t* bitmap, int16_t w, int16_t h)
  {
#ifdef GRAPHICS_ENABLED
    _canvas->draw16bitRGBBitmap(x, y, bitmap, w, h);
    _is_buff_changed = true;
#endif  // #ifdef GRAPHICS_ENABLED
  }

  void DisplayWrapper::drawBitmapTransp(int16_t x, int16_t y, const uint16_t* bitmap, int16_t w, int16_t h)
  {
#ifdef GRAPHICS_ENABLED
    _canvas->draw16bitRGBBitmapWithTranColor(x, y, bitmap, COLOR_TRANSPARENT, w, h);
    _is_buff_changed = true;
#endif  // #ifdef GRAPHICS_ENABLED
  }

  void DisplayWrapper::drawBitmapRotated(int16_t x, int16_t y, const uint16_t* bitmap, int16_t w, int16_t h, int16_t piv_x, int16_t piv_y, float angle)
  {
#ifdef GRAPHICS_ENABLED
    float rad = angle * M_PI / 180.0f;
    float cos_a = cosf(rad);
    float sin_a = sinf(rad);

    uint16_t* rotated{nullptr};

    rotated = new uint16_t[w * h];

    if (!rotated)
    {
      log_e("Помилка виділення пам'яті для повернутого зображення");
      esp_restart();
    }

    std::fill(rotated, rotated + w * h, COLOR_TRANSPARENT);

    for (int16_t dy = 0; dy < h; dy++)
    {
      for (int16_t dx = 0; dx < w; dx++)
      {
        float rel_x = dx - piv_x;
        float rel_y = dy - piv_y;

        float orig_x = rel_x * cos_a + rel_y * sin_a + piv_x;
        float orig_y = -rel_x * sin_a + rel_y * cos_a + piv_y;

        // Отримуємо піксель з оригінального зображення (білінійна інтерполяція)
        int16_t ix = static_cast<int16_t>(orig_x);
        int16_t iy = static_cast<int16_t>(orig_y);

        // Проста вибірка найближчого пікселя
        if (ix >= 0 && ix < w && iy >= 0 && iy < h)
          rotated[dy * w + dx] = bitmap[iy * w + ix];
      }
    }

    _canvas->draw16bitRGBBitmapWithTranColor(x, y, rotated, COLOR_TRANSPARENT, w, h);
    delete[] rotated;
#endif  // #ifdef GRAPHICS_ENABLED
  }

  uint16_t DisplayWrapper::getFontHeight(const uint8_t* font, uint8_t size)
  {
#ifdef GRAPHICS_ENABLED
    int16_t x1, y1;
    uint16_t w, h;
    _display.setTextSize(size);
    _display.setFont(font);
    _display.calcTextBounds("Ґg", 0, 0, x1, y1, w, h);
    return h;
#else   // not def GRAPHICS_ENABLED
    return 0;
#endif  // #ifdef GRAPHICS_ENABLED
  }

  SemaphoreHandle_t DisplayWrapper::getMutex()
  {
#ifdef GRAPHICS_ENABLED
    return _sync_mutex;
#else   // not def GRAPHICS_ENABLED
    return nullptr;
#endif  // #ifdef GRAPHICS_ENABLED
  }

  void DisplayWrapper::__init(sf::RenderWindow* window)
  {
#ifdef GRAPHICS_ENABLED
    _canvas = new Arduino_Canvas{UI_WIDTH, UI_HEIGHT};  // Перевірку ініціалізації не додаємо. Очікується, що користувач завжди викликає цей метод ДО будь-яких малювань.
    _canvas->begin(window);
    _canvas->setTextWrap(false);

#ifdef INVERT_COLORS
    _output->invertDisplay(INVERT_COLORS);
#endif  //  #ifdef INVERT_COLORS

    _sync_mutex = xSemaphoreCreateMutex();

    if (!_sync_mutex)
    {
      log_e("Недостатньо ресурсів для роботи графічного драйвера");
      esp_restart();
    }

#endif  // #ifdef GRAPHICS_ENABLED
  }

#ifdef ENABLE_SCREENSHOTER
  void DisplayWrapper::takeScreenshot()
  {
    _take_screenshot = true;
    _is_buff_changed = true;
  }
#endif  // ENABLE_SCREENSHOTER

#ifdef PIN_DISPLAY_BL
  void DisplayWrapper::enableBackLight()
  {
#ifdef HAS_BL_PWM
    ledcAttach(PIN_DISPLAY_BL, DISPLAY_BL_PWM_FREQ, DISPLAY_BL_PWM_RES);
    ledcWrite(PIN_DISPLAY_BL, _cur_brightness);
#else
    pinMode(PIN_DISPLAY_BL, OUTPUT);
    digitalWrite(PIN_DISPLAY_BL, HIGH);
#endif  // HAS_BL_PWM
  }

  void DisplayWrapper::disableBackLight()
  {
#ifdef HAS_BL_PWM
    ledcDetach(PIN_DISPLAY_BL);
#endif  // HAS_BL_PWM
    digitalWrite(PIN_DISPLAY_BL, LOW);
  }

#ifdef HAS_BL_PWM
  void DisplayWrapper::setBrightness(uint8_t value)
  {
    if (value == 0)
      fillScreen(COLOR_BLACK);

    _cur_brightness = value;
    ledcWrite(PIN_DISPLAY_BL, value);
  }

  uint8_t DisplayWrapper::getBrightness() const
  {
    return _cur_brightness;
  }
#endif  // HAS_BL_PWM
#endif  // PIN_DISPLAY_BL

#ifdef GRAPHICS_ENABLED
  void DisplayWrapper::__flush()
  {
    if (_is_buff_changed)
    {
      _is_buff_changed = false;

#ifdef SHOW_FPS
      if (millis() - _frame_timer < 1000)
      {
        ++_temp_frame_counter;
      }
      else
      {
        _frame_counter = _temp_frame_counter + 1;
        _temp_frame_counter = 0;
        _frame_timer = millis();
      }

      String fps_str = String(_frame_counter);

      _canvas->setTextSize(1);
      _canvas->setFont(font_unifont);
      _canvas->setTextColor(COLOR_RED);

      int16_t x{0};
      int16_t y{0};
      int16_t x_out{0};
      int16_t y_out{0};
      uint16_t w{0};
      uint16_t h{0};

      _canvas->getTextBounds(fps_str.c_str(), x, y, x_out, y_out, w, h);
      //
      uint16_t fps_x_pos = _canvas->width() / 2 - w;
      _canvas->fillRect(fps_x_pos - 3, 0, w + 6, h + 9, COLOR_BLACK);
      _canvas->setCursor(fps_x_pos, h + 3);
      _canvas->print(fps_str);
#endif  // SHOW_FPS

      xSemaphoreTake(_sync_mutex, portMAX_DELAY);
      _canvas->flushMainBuff();
      xSemaphoreGive(_sync_mutex);

#ifdef ENABLE_SCREENSHOTER
      if (_take_screenshot)
        takeScreenshot(this);
#endif  // ENABLE_SCREENSHOTER
    }
  }
#endif  // #ifdef GRAPHICS_ENABLED

#ifdef ENABLE_SCREENSHOTER
  void DisplayWrapper::takeScreenshot(DisplayWrapper* self)
  {
    self->_take_screenshot = false;

    BmpHeader header;
    header.width = self->_canvas->width();
    header.height = self->_canvas->height();

    String path_to_bmp = "/screenshot_";
    path_to_bmp += millis();
    path_to_bmp += ".bmp";

    bool res = BmpUtil::saveBmp(header,
                                self->_canvas->getFramebuffer(),
                                path_to_bmp.c_str(),
                                true);

    if (res)
      log_i("Скріншот %s успішно збережено", path_to_bmp.c_str());
    else
      log_e("Помилка створення скріншоту");
  }
#endif  // ENABLE_SCREENSHOTER

  DisplayWrapper _display;
}  // namespace pixeler