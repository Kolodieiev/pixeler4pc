#pragma GCC optimize("O3")
#include "DisplayWrapper.h"

#include <cmath>

namespace pixeler
{
#ifdef GRAPHICS_ENABLED
  DisplayWrapper::DisplayWrapper()
  {
  }

  DisplayWrapper::~DisplayWrapper()
  {
  }

  void DisplayWrapper::fillScreen(uint16_t color)
  {
    _canvas->fillScreen(color);
    _is_buff_changed = true;
  }

  void DisplayWrapper::setCursor(int16_t x, int16_t y)
  {
    _canvas->setCursor(x, y);
  }

  void DisplayWrapper::setTextWrap(bool state)
  {
    _canvas->setTextWrap(state);
  }

  size_t DisplayWrapper::print(const char* str)
  {
    return _canvas->print(str);
    _is_buff_changed = true;
  }

  void DisplayWrapper::setFont(const uint8_t* font)
  {
    _canvas->setFont(font);
  }

  void DisplayWrapper::setTextSize(uint8_t size)
  {
    _canvas->setTextSize(size, size);
  }

  void DisplayWrapper::setTextColor(uint16_t color)
  {
    _canvas->setTextColor(color);
  }

  void DisplayWrapper::calcTextBounds(const char* str, int16_t x, int16_t y, int16_t& x_out, int16_t& y_out, uint16_t& w_out, uint16_t& h_out)
  {
    _canvas->getTextBounds(str, x, y, x_out, y_out, w_out, h_out);
  }

  void DisplayWrapper::drawPixel(int16_t x, int16_t y, uint16_t color)
  {
    _canvas->drawPixel(x, y, color);
    _is_buff_changed = true;
  }

  void DisplayWrapper::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
  {
    _canvas->drawLine(x0, y0, x1, y1, color);
    _is_buff_changed = true;
  }

  void DisplayWrapper::drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color)
  {
    _canvas->drawCircle(x, y, r, color);
    _is_buff_changed = true;
  }

  void DisplayWrapper::fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color)
  {
    _canvas->fillCircle(x, y, r, color);
    _is_buff_changed = true;
  }

  void DisplayWrapper::drawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color)
  {
    _canvas->drawRect(x, y, w, h, color);
    _is_buff_changed = true;
  }

  void DisplayWrapper::fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color)
  {
    _canvas->fillRect(x, y, w, h, color);
    _is_buff_changed = true;
  }

  void DisplayWrapper::drawRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius, uint16_t color)
  {
    _canvas->drawRoundRect(x, y, w, h, radius, color);
    _is_buff_changed = true;
  }

  void DisplayWrapper::fillRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius, uint16_t color)
  {
    _canvas->fillRoundRect(x, y, w, h, radius, color);
    _is_buff_changed = true;
  }

  void DisplayWrapper::drawBitmap(int16_t x, int16_t y, const uint16_t* bitmap, int16_t w, int16_t h)
  {
    _canvas->draw16bitRGBBitmap(x, y, bitmap, w, h);
    _is_buff_changed = true;
  }

  void DisplayWrapper::drawBitmapTransp(int16_t x, int16_t y, const uint16_t* bitmap, int16_t w, int16_t h)
  {
    _canvas->draw16bitRGBBitmapWithTranColor(x, y, bitmap, COLOR_TRANSPARENT, w, h);
    _is_buff_changed = true;
  }

  void DisplayWrapper::drawBitmapRotated(int16_t x, int16_t y, const uint16_t* bitmap, int16_t w, int16_t h, int16_t piv_x, int16_t piv_y, float angle)
  {
    float rad = angle * M_PI / 180.0f;
    float cos_a = cosf(rad);
    float sin_a = sinf(rad);

    uint16_t rotated[w * h];

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
  }

  uint16_t DisplayWrapper::getWidth()
  {
    return _canvas->width();
  }

  uint16_t DisplayWrapper::getHeight()
  {
    return _canvas->height();
  }

  uint16_t DisplayWrapper::getFontHeight(const uint8_t* font, uint8_t size)
  {
    int16_t x1, y1;
    uint16_t w, h;
    _display.setTextSize(size);
    _display.setFont(font);
    _display.calcTextBounds("Ґg", 0, 0, x1, y1, w, h);
    return h;
  }

  void DisplayWrapper::init(sf::RenderWindow* window)
  {
    _canvas = new Arduino_Canvas{TFT_WIDTH, TFT_HEIGHT}; // Перевірку ініціалізації не додаємо. Очікується, що користувач завжди викликає цей метод ДО будь-яких малювань.

    if (!_canvas->begin(window))
      return;

    _canvas->setTextWrap(false);
  }

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
        _frame_counter = _temp_frame_counter + 2;
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
      _canvas->print(fps_str.c_str());
#endif  // SHOW_FPS

      _canvas->flushMainBuff();
    }
  }

  DisplayWrapper _display;
#endif  // GRAPHICS_ENABLED
}  // namespace pixeler