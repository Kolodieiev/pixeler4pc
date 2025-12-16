#pragma once
#pragma GCC optimize("O3")
#include <SFML/Graphics.hpp>
#include <vector>

#include "../../defines.h"

#include "Arduino_GFX/Arduino_Canvas.h"
#include "graphics_setup.h"

#define WDT_GUARD_TIME 1000UL

namespace pixeler
{
  class DisplayWrapper
  {
  public:
    DisplayWrapper();
    ~DisplayWrapper();

    void __flush();

    void fillScreen(uint16_t color);

    void setFont(const uint8_t* font);
    void setTextSize(uint8_t size);
    void setTextColor(uint16_t color);
    void setCursor(int16_t x, int16_t y);
    void setTextWrap(bool state);
    size_t print(const char* str);

    void calcTextBounds(const char* str, int16_t x, int16_t y, int16_t& x_out, int16_t& y_out, uint16_t& w_out, uint16_t& h_out);

    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

    void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
    void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color);

    void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color);
    void fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color);

    void drawRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius, uint16_t color);
    void fillRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius, uint16_t color);

    void drawBitmap(int16_t x, int16_t y, const uint16_t* bitmap, int16_t w, int16_t h);
    void drawBitmapTransp(int16_t x, int16_t y, const uint16_t* bitmap, int16_t w, int16_t h);
    void drawBitmapRotated(int16_t x, int16_t y, const uint16_t* bitmap, int16_t w, int16_t h, int16_t piv_x, int16_t piv_y, float angle);

    uint16_t getWidth();
    uint16_t getHeight();

    uint16_t getFontHeight(const uint8_t* font, uint8_t size = 1);

    void init(sf::RenderWindow* window);

  private:
    Arduino_Canvas *_canvas;

#ifdef SHOW_FPS
    uint64_t _frame_timer{0};
    uint16_t _frame_counter{0};
    uint16_t _temp_frame_counter{0};
#endif  // SHOW_FPS

    bool _is_buff_changed{false};
  };

  extern DisplayWrapper _display;
}  // namespace pixeler
