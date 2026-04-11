/*
 * start rewrite from:
 * https://github.com/adafruit/Adafruit-GFX-Library.git
 *
 * Arc function come from:
 * https://github.com/lovyan03/LovyanGFX.git
 */

#pragma GCC optimize("O3")
#include "Arduino_Canvas.h"

#include "float.h"
#include "font/glcdfont.h"
#include "pixeler/setup/graphics_setup.hpp"

Arduino_Canvas::Arduino_Canvas(int16_t w, int16_t h)
    : MAX_X{static_cast<uint16_t>(WIDTH - 1)},
      MAX_Y{static_cast<uint16_t>(HEIGHT - 1)},
      WIDTH(w),
      HEIGHT(h),
      FRAMEBUFF_SIZE{static_cast<uint32_t>(w * h * sizeof(uint16_t))},
      _framebuffer{new uint16_t[w * h]},
      _opengl_rgba_buff{new uint8_t[w * h * 4]}
{
  _width = WIDTH;
  _height = HEIGHT;
  _max_x = _width - 1;   ///< x zero base bound
  _max_y = _height - 1;  ///< y zero base bound
  _min_text_x = 0;
  _min_text_y = 0;
  _max_text_x = _max_x;
  _max_text_y = _max_y;
  cursor_x = 0;
  cursor_y = 0;
  textcolor = 0xFFFF;
  textbgcolor = 0xFFFF;
  textsize_x = 1;
  textsize_y = 1;
  text_pixel_margin = 0;
  wrap = true;
  u8g2Font = NULL;
}

Arduino_Canvas::~Arduino_Canvas()
{
  delete[] _framebuffer;
  delete[] _opengl_rgba_buff;
}

void Arduino_Canvas::invertDisplay(bool i)
{
  // stub
}

void Arduino_Canvas::displayOn()
{
  // stub
}

void Arduino_Canvas::displayOff()
{
  // stub
}

bool Arduino_Canvas::begin(sf::RenderWindow* window)
{
  if (!window)
  {
    printf("Помилка ініціалізації Canvas\n");
    return false;
  }

  _window = window;

  if (!_framebuffer)
  {
    log_e("Помилка виділення пам'яті для буферів дисплея");
    esp_restart();
  }

  log_e("Canvas ініціалізовано");
  return true;
}

void Arduino_Canvas::writePixelPreclipped(int16_t x, int16_t y, uint16_t color)
{
  uint16_t* fb = _framebuffer;
  fb += (int32_t)y * _width;
  fb += x;
  *fb = color;
}

void Arduino_Canvas::writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
  writeFastVLineCore(x, y, h, color);
}

void Arduino_Canvas::writeFastVLineCore(int16_t x, int16_t y, int16_t h, uint16_t color)
{
  // log_i("writeFastVLineCore(x: %d, y: %d, h: %d)", x, y, h);
  if (_ordered_in_range(x, 0, MAX_X) && h)
  {  // X on screen, nonzero height
    if (h < 0)
    {              // If negative height...
      y += h + 1;  //   Move Y to top edge
      h = -h;      //   Use positive height
    }
    if (y <= MAX_Y)
    {  // Not off bottom
      int16_t y2 = y + h - 1;
      if (y2 >= 0)
      {  // Not off top
        // Line partly or fully overlaps screen
        if (y < 0)
        {
          y = 0;
          h = y2 + 1;
        }  // Clip top
        if (y2 > MAX_Y)
        {
          h = MAX_Y - y + 1;
        }  // Clip bottom

        uint16_t* fb = _framebuffer + ((int32_t)y * WIDTH) + x;
        while (h--)
        {
          *fb = color;
          fb += WIDTH;
        }
      }
    }
  }
}

void Arduino_Canvas::writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
  writeFastHLineCore(x, y, w, color);
}

void Arduino_Canvas::writeFastHLineCore(int16_t x, int16_t y, int16_t w, uint16_t color)
{
  // log_i("writeFastHLineCore(x: %d, y: %d, w: %d)", x, y, w);
  if (_ordered_in_range(y, 0, MAX_Y) && w)
  {  // Y on screen, nonzero width
    if (w < 0)
    {              // If negative width...
      x += w + 1;  //   Move X to left edge
      w = -w;      //   Use positive width
    }
    if (x <= MAX_X)
    {  // Not off right
      int16_t x2 = x + w - 1;
      if (x2 >= 0)
      {  // Not off left
        // Line partly or fully overlaps screen
        if (x < 0)
        {
          x = 0;
          w = x2 + 1;
        }  // Clip left
        if (x2 > MAX_X)
        {
          w = MAX_X - x + 1;
        }  // Clip right

        uint16_t* fb = _framebuffer + ((int32_t)y * WIDTH) + x;
        while (w--)
        {
          *(fb++) = color;
        }
      }
    }
  }
}

void Arduino_Canvas::writeFillRectPreclipped(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  // log_i("writeFillRectPreclipped(x: %d, y: %d, w: %d, h: %d)", x, y, w, h);
  uint16_t* row = _framebuffer;
  row += y * WIDTH;
  row += x;
  for (int j = 0; j < h; j++)
  {
    for (int i = 0; i < w; i++)
    {
      row[i] = color;
    }
    row += WIDTH;
  }
}

void Arduino_Canvas::draw16bitRGBBitmap(int16_t x, int16_t y, const uint16_t* bitmap, int16_t w, int16_t h)
{
  if (
      ((x + w - 1) < 0) ||  // Outside left
      ((y + h - 1) < 0) ||  // Outside top
      (x > MAX_X) ||        // Outside right
      (y > MAX_Y)           // Outside bottom
  )
    return;

  drawBitmapToFramebuffer(bitmap, w, h, _framebuffer, x, y, _width, _height);
}

void Arduino_Canvas::writeSlashLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
  bool steep = _diff(y1, y0) > _diff(x1, x0);
  if (steep)
  {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if (x0 > x1)
  {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx = x1 - x0;
  int16_t dy = _diff(y1, y0);
  int16_t err = dx >> 1;
  int16_t step = (y0 < y1) ? 1 : -1;

  for (; x0 <= x1; x0++)
  {
    if (steep)
    {
      writePixel(y0, x0, color);
    }
    else
    {
      writePixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0)
    {
      err += dx;
      y0 += step;
    }
  }
}

void Arduino_Canvas::draw16bitRGBBitmapWithTranColor(int16_t x, int16_t y, const uint16_t* bitmap, uint16_t transparent_color, int16_t w, int16_t h)
{
  if (
      ((x + w - 1) < 0) ||  // Outside left
      ((y + h - 1) < 0) ||  // Outside top
      (x > _max_x) ||       // Outside right
      (y > _max_y)          // Outside bottom
  )
  {
    return;
  }
  else
  {
    int16_t x_skip = 0;
    if ((y + h - 1) > _max_y)
    {
      h -= (y + h - 1) - _max_y;
    }
    if (y < 0)
    {
      bitmap -= y * w;
      h += y;
      y = 0;
    }
    if ((x + w - 1) > _max_x)
    {
      x_skip = (x + w - 1) - _max_x;
      w -= x_skip;
    }
    if (x < 0)
    {
      bitmap -= x;
      x_skip -= x;
      w += x;
      x = 0;
    }
    uint16_t* row = _framebuffer;
    row += y * _width;
    row += x;
    int16_t i;
    int16_t wi;
    uint16_t p;
    while (h--)
    {
      i = 0;
      wi = w;
      while (wi >= 4)
      {
        uint32_t b32 = *((uint32_t*)bitmap);
        p = (b32 & 0xffff);
        if (p != transparent_color)
        {
          row[i] = p;
        }
        ++i;
        p = (b32 & 0xffff0000) >> 16;
        if (p != transparent_color)
        {
          row[i] = p;
        }
        ++i;
        wi -= 2;
        bitmap += 2;
      }
      while (i < w)
      {
        p = *bitmap++;
        if (p != transparent_color)
        {
          row[i] = p;
        }
        ++i;
      }
      bitmap += x_skip;
      row += _width;
    }
  }
}

void Arduino_Canvas::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg)
{
  int16_t block_w, block_h, curX, curY, curW, curH;

  if (u8g2Font)
  {
    _u8g2_target_y = y - ((_u8g2_char_height + _u8g2_char_y) * textsize_y);
    if (_u8g2_target_y > _max_text_y)
    {
      return;
    }

    if ((_u8g2_decode_ptr) && (_u8g2_char_width > 0))
    {
      uint8_t a, b;

      _u8g2_target_x = x + (_u8g2_char_x * textsize_x);
      // log_d("_u8g2_target_x: %d, _u8g2_target_y: %d", _u8g2_target_x, _u8g2_target_y);

      /* reset local x/y position */
      _u8g2_dx = 0;
      _u8g2_dy = 0;
      /* decode glyph */
      for (;;)
      {
        a = u8g2_font_decode_get_unsigned_bits(_u8g2_bits_per_0);
        b = u8g2_font_decode_get_unsigned_bits(_u8g2_bits_per_1);
        // log_d("a: %d, b: %d", a, b);
        do
        {
          u8g2_font_decode_len(a, 0, color, bg);
          u8g2_font_decode_len(b, 1, color, bg);
        } while (u8g2_font_decode_get_unsigned_bits(1) != 0);

        if (_u8g2_dy >= _u8g2_char_height)
          break;
      }
    }
  }
  else  // glcdfont
  {
    block_w = 6 * textsize_x;
    block_h = 8 * textsize_y;
    if (
        (x > _max_text_x) ||                  // Clip right
        (y > _max_text_y) ||                  // Clip bottom
        ((x + block_w - 1) < _min_text_x) ||  // Clip left
        ((y + block_h - 1) < _min_text_y)     // Clip top
    )
    {
      return;
    }

    if (textsize_x == 1 && textsize_y == 1)
    {
      curX = x;
      for (int8_t i = 0; i < 5; ++i, ++curX)  // Char bitmap = 5 columns
      {
        uint8_t line = (*(const unsigned char*)(&font[c * 5 + i]));
        if (curX <= _max_text_x)
        {
          curY = y;
          for (int8_t j = 0; j < 8; ++j, ++curY, line >>= 1)
          {
            if (curY <= _max_text_y)
            {
              if (line & 1)
              {
                writePixelPreclipped(curX, curY, color);
              }
              else if (bg != color)
              {
                writePixelPreclipped(curX, curY, bg);
              }
            }
          }
        }
      }
      if (bg != color)  // If opaque, draw vertical line for last column
      {
        curX = x + 5;
        if (curX <= _max_text_x)
        {
          curH = ((y + 8 - 1) <= _max_text_y) ? 8 : (_max_text_y - y + 1);
          writeFastVLine(curX, y, curH, bg);
        }
      }
    }
    else  // (textsize_x > 1 || textsize_y > 1)
    {
      curX = x;
      for (int8_t i = 0; i < 5; ++i, curX += textsize_x)  // Char bitmap = 5 columns
      {
        if ((curX + textsize_x - 1) <= _max_text_x)
        {
          uint8_t line = (*(const unsigned char*)(&font[c * 5 + i]));
          curY = y;
          for (int8_t j = 0; j < 8; j++, line >>= 1, curY += textsize_y)
          {
            if ((curY + textsize_y - 1) <= _max_text_y)
            {
              if (line & 1)
              {
                if (text_pixel_margin > 0)
                {
                  writeFillRect(curX, y + j * textsize_y, textsize_x - text_pixel_margin, textsize_y - text_pixel_margin, color);
                  writeFillRect(curX + textsize_x - text_pixel_margin, y + j * textsize_y, text_pixel_margin, textsize_y, bg);
                  writeFillRect(curX, y + ((j + 1) * textsize_y) - text_pixel_margin, textsize_x - text_pixel_margin, text_pixel_margin, bg);
                }
                else
                {
                  writeFillRect(curX, curY, textsize_x, textsize_y, color);
                }
              }
              else if (bg != color)
              {
                writeFillRect(curX, curY, textsize_x, textsize_y, bg);
              }
            }
          }
        }
      }
      if (bg != color)  // If opaque, draw vertical line for last column
      {
        curX = x + 5 * textsize_x;
        if ((curX + textsize_x - 1) <= _max_text_x)
        {
          curH = 8 * textsize_y;
          while ((y + curH - 1) > _max_text_y)
          {
            curH -= textsize_y;
          }
          writeFillRect(curX, y, textsize_x, curH, bg);
        }
      }
    }
  }
}

void Arduino_Canvas::setTextBound(int16_t x, int16_t y, int16_t w, int16_t h)
{
  _min_text_x = x;
  _min_text_y = y;
  _max_text_x = x + w - 1;
  _max_text_y = y + h - 1;
}

void Arduino_Canvas::resetTextBound()
{
  _min_text_x = 0;
  _min_text_y = 0;
  _max_text_x = _max_x;
  _max_text_y = _max_y;
}

size_t Arduino_Canvas::write(uint8_t ch)
{
  if (u8g2Font)
  {
    _u8g2_decode_ptr = 0;

    if (_enableUTF8Print)
    {
      if (_utf8_state == 0)
      {
        if (ch >= 0xfc) /* 6 byte sequence */
        {
          _utf8_state = 5;
          ch &= 1;
        }
        else if (ch >= 0xf8)
        {
          _utf8_state = 4;
          ch &= 3;
        }
        else if (ch >= 0xf0)
        {
          _utf8_state = 3;
          ch &= 7;
        }
        else if (ch >= 0xe0)
        {
          _utf8_state = 2;
          ch &= 15;
        }
        else if (ch >= 0xc0)
        {
          _utf8_state = 1;
          ch &= 0x01f;
        }
        _encoding = ch;
      }
      else
      {
        _utf8_state--;
        /* The case b < 0x080 (an illegal UTF8 encoding) is not checked here. */
        _encoding <<= 6;
        ch &= 0x03f;
        _encoding |= ch;
      }
    }  // _enableUTF8Print
    else
    {
      _encoding = ch;
    }

    if (_utf8_state == 0)
    {
      if (_encoding == '\n')
      {
        cursor_x = _min_text_x;
        cursor_y += (int16_t)textsize_y * _u8g2_max_char_height;
      }
      else if (_encoding != '\r')
      {  // Ignore carriage returns
        const uint8_t* font = u8g2Font;
        const uint8_t* glyph_data = 0;

        // extract from u8g2_font_get_glyph_data()
        font += 23;  // U8G2_FONT_DATA_STRUCT_SIZE
        if (_encoding <= 255)
        {
          if (_encoding >= 'a')
          {
            font += _u8g2_start_pos_lower_a;
          }
          else if (_encoding >= 'A')
          {
            font += _u8g2_start_pos_upper_A;
          }

          for (;;)
          {
            if ((*(const unsigned char*)(font + 1)) == 0)
              break;
            if ((*(const unsigned char*)(font)) == _encoding)
            {
              glyph_data = font + 2; /* skip encoding and glyph size */
            }
            font += (*(const unsigned char*)(font + 1));
          }
        }
        else
        {
          uint16_t e;
          font += _u8g2_start_pos_unicode;
          const uint8_t* unicode_lookup_table = font;

          /* issue 596: search for the glyph start in the unicode lookup table */
          do
          {
            font += u8g2_font_get_word(unicode_lookup_table, 0);
            e = u8g2_font_get_word(unicode_lookup_table, 2);
            unicode_lookup_table += 4;
          } while (e < _encoding);

          for (;;)
          {
            e = u8g2_font_get_word(font, 0);

            if (e == 0)
              break;

            if (e == _encoding)
            {
              glyph_data = font + 3; /* skip encoding and glyph size */
              break;
            }
            font += (*(const unsigned char*)(font + 2));
          }
        }

        if (glyph_data)
        {
          // u8g2_font_decode_glyph
          _u8g2_decode_ptr = glyph_data;
          _u8g2_decode_bit_pos = 0;

          _u8g2_char_width = u8g2_font_decode_get_unsigned_bits(_u8g2_bits_per_char_width);
          _u8g2_char_height = u8g2_font_decode_get_unsigned_bits(_u8g2_bits_per_char_height);
          _u8g2_char_x = u8g2_font_decode_get_signed_bits(_u8g2_bits_per_char_x);
          _u8g2_char_y = u8g2_font_decode_get_signed_bits(_u8g2_bits_per_char_y);
          _u8g2_delta_x = u8g2_font_decode_get_signed_bits(_u8g2_bits_per_delta_x);
          // log_d("c: %c, _encoding: %d, _u8g2_char_width: %d, _u8g2_char_height: %d, _u8g2_char_x: %d, _u8g2_char_y: %d, _u8g2_delta_x: %d",
          //       c, _encoding, _u8g2_char_width, _u8g2_char_height, _u8g2_char_x, _u8g2_char_y, _u8g2_delta_x);

          if (_u8g2_char_width > 0)
          {
            if (wrap && ((cursor_x + (textsize_x * _u8g2_char_width) - 1) > _max_text_x))
            {
              cursor_x = _min_text_x;
              cursor_y += (int16_t)textsize_y * _u8g2_max_char_height;
            }
          }

          drawChar(cursor_x, cursor_y, ch, textcolor, textbgcolor);
          cursor_x += (int16_t)textsize_x * _u8g2_delta_x;
        }
      }
    }
  }
  else  // glcdfont
  {
    if (ch == '\n')
    {                              // Newline?
      cursor_x = _min_text_x;      // Reset x to zero,
      cursor_y += textsize_y * 8;  // advance y one line
    }
    else if (ch != '\r')  // Normal char; ignore carriage returns
    {
      if (wrap && ((cursor_x + (textsize_x * 6) - 1) > _max_text_x))  // Off right?
      {
        cursor_x = _min_text_x;      // Reset x to zero,
        cursor_y += textsize_y * 8;  // advance y one line
      }
      drawChar(cursor_x, cursor_y, ch, textcolor, textbgcolor);
      cursor_x += textsize_x * 6;  // Advance x one char
    }
  }
  return 1;
}

size_t Arduino_Canvas::write(const uint8_t* buffer, size_t size)
{
  size_t n = 0;
  while (size--)
    n += write(*buffer++);

  return n;
}

size_t Arduino_Canvas::print(const char* str)
{
  if (!str)
    return 0;

  return write((const uint8_t*)str, strlen(str));
}

size_t Arduino_Canvas::print(char ch)
{
  return write(ch);
}

void Arduino_Canvas::writePixel(int16_t x, int16_t y, uint16_t color)
{
  if (_ordered_in_range(y, 0, _max_y) && _ordered_in_range(x, 0, _max_x))
    writePixelPreclipped(x, y, color);
}

void Arduino_Canvas::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  writePixel(x, y, color);
}

void Arduino_Canvas::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  if (w && h)
  {  // Nonzero width and height?
    if (w < 0)
    {              // If negative width...
      x += w + 1;  //   Move X to left edge
      w = -w;      //   Use positive width
    }
    if (x <= _max_x)
    {  // Not off right
      if (h < 0)
      {              // If negative height...
        y += h + 1;  //   Move Y to top edge
        h = -h;      //   Use positive height
      }
      if (y <= _max_y)
      {  // Not off bottom
        int16_t x2 = x + w - 1;
        if (x2 >= 0)
        {  // Not off left
          int16_t y2 = y + h - 1;
          if (y2 >= 0)
          {  // Not off top
            // Rectangle partly or fully overlaps screen
            if (x < 0)
            {
              x = 0;
              w = x2 + 1;
            }  // Clip left
            if (y < 0)
            {
              y = 0;
              h = y2 + 1;
            }  // Clip top
            if (x2 > _max_x)
            {
              w = _max_x - x + 1;
            }  // Clip right
            if (y2 > _max_y)
            {
              h = _max_y - y + 1;
            }  // Clip bottom
            writeFillRectPreclipped(x, y, w, h, color);
          }
        }
      }
    }
  }
}

void Arduino_Canvas::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  writeFillRect(x, y, w, h, color);
}

void Arduino_Canvas::fillScreen(uint16_t color)
{
  fillRect(0, 0, _width, _height, color);
}

void Arduino_Canvas::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
  if (x0 == x1)
  {
    if (y0 > y1)
    {
      _swap_int16_t(y0, y1);
    }
    writeFastVLine(x0, y0, y1 - y0 + 1, color);
  }
  else if (y0 == y1)
  {
    if (x0 > x1)
    {
      _swap_int16_t(x0, x1);
    }
    writeFastHLine(x0, y0, x1 - x0 + 1, color);
  }
  else
  {
    writeSlashLine(x0, y0, x1, y1, color);
  }
}

void Arduino_Canvas::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  writeFastHLine(x, y, w, color);
  writeFastHLine(x, y + h - 1, w, color);
  writeFastVLine(x, y, h, color);
  writeFastVLine(x + w - 1, y, h, color);
}

void Arduino_Canvas::drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color)
{
  writeEllipseHelper(x, y, r, r, 0xf, color);
}

void Arduino_Canvas::fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color)
{
  writeFillEllipseHelper(x, y, r, r, 3, 0, color);
}

void Arduino_Canvas::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);
}

void Arduino_Canvas::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  int16_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1)
  {
    _swap_int16_t(y0, y1);
    _swap_int16_t(x0, x1);
  }
  if (y1 > y2)
  {
    _swap_int16_t(y2, y1);
    _swap_int16_t(x2, x1);
  }
  if (y0 > y1)
  {
    _swap_int16_t(y0, y1);
    _swap_int16_t(x0, x1);
  }

  if (y0 == y2)
  {  // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if (x1 < a)
      a = x1;
    else if (x1 > b)
      b = x1;
    if (x2 < a)
      a = x2;
    else if (x2 > b)
      b = x2;
    writeFastHLine(a, y0, b - a + 1, color);
    return;
  }

  int16_t
      dx01 = x1 - x0,
      dy01 = y1 - y0,
      dx02 = x2 - x0,
      dy02 = y2 - y0,
      dx12 = x2 - x1,
      dy12 = y2 - y1;
  int32_t
      sa = 0,
      sb = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if (y1 == y2)
  {
    last = y1;  // Include y1 scanline
  }
  else
  {
    last = y1 - 1;  // Skip it
  }

  for (y = y0; y <= last; y++)
  {
    a = x0 + sa / dy01;
    b = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if (a > b)
    {
      _swap_int16_t(a, b);
    }
    writeFastHLine(a, y, b - a + 1, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = (int32_t)dx12 * (y - y1);
  sb = (int32_t)dx02 * (y - y0);
  for (; y <= y2; y++)
  {
    a = x1 + sa / dy12;
    b = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if (a > b)
    {
      _swap_int16_t(a, b);
    }
    writeFastHLine(a, y, b - a + 1, color);
  }
}

void Arduino_Canvas::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, uint16_t color)
{
  int16_t max_radius = ((w < h) ? w : h) / 2;  // 1/2 minor axis
  if (radius > max_radius)
    radius = max_radius;
  // smarter version
  writeFastHLine(x + radius, y, w - 2 * radius, color);          // Top
  writeFastHLine(x + radius, y + h - 1, w - 2 * radius, color);  // Bottom
  writeFastVLine(x, y + radius, h - 2 * radius, color);          // Left
  writeFastVLine(x + w - 1, y + radius, h - 2 * radius, color);  // Right
  // draw four corners
  writeEllipseHelper(x + radius, y + radius, radius, radius, 1, color);
  writeEllipseHelper(x + w - radius - 1, y + radius, radius, radius, 2, color);
  writeEllipseHelper(x + w - radius - 1, y + h - radius - 1, radius, radius, 4, color);
  writeEllipseHelper(x + radius, y + h - radius - 1, radius, radius, 8, color);
}

void Arduino_Canvas::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, uint16_t color)
{
  int16_t max_radius = ((w < h) ? w : h) / 2;  // 1/2 minor axis
  if (radius > max_radius)
    radius = max_radius;
  // smarter version
  writeFillRect(x, y + radius, w, h - (radius << 1), color);
  // draw four corners
  writeFillEllipseHelper(x + radius, y + radius, radius, radius, 1, w - 2 * radius - 1, color);
  writeFillEllipseHelper(x + radius, y + h - radius - 1, radius, radius, 2, w - 2 * radius - 1, color);
}

void Arduino_Canvas::getTextBounds(const char* str, int16_t x, int16_t y, int16_t& x1, int16_t& y1, uint16_t& w, uint16_t& h)
{
  uint8_t c;  // Current character

  x1 = x;
  y1 = y;
  w = h = 0;

  int16_t minx = _max_text_x, miny = _max_text_y, maxx = _min_text_x, maxy = _min_text_y;
  int16_t curr_x = x, curr_y = y;

  while ((c = *str++))
  {
    if (u8g2Font)
    {
      _u8g2_decode_ptr = 0;

      if (_enableUTF8Print)
      {
        if (_utf8_state == 0)
        {
          if (c >= 0xfc) /* 6 byte sequence */
          {
            _utf8_state = 5;
            c &= 1;
          }
          else if (c >= 0xf8)
          {
            _utf8_state = 4;
            c &= 3;
          }
          else if (c >= 0xf0)
          {
            _utf8_state = 3;
            c &= 7;
          }
          else if (c >= 0xe0)
          {
            _utf8_state = 2;
            c &= 15;
          }
          else if (c >= 0xc0)
          {
            _utf8_state = 1;
            c &= 0x01f;
          }
          _encoding = c;
        }
        else
        {
          _utf8_state--;
          /* The case b < 0x080 (an illegal UTF8 encoding) is not checked here. */
          _encoding <<= 6;
          c &= 0x03f;
          _encoding |= c;
        }
      }  // _enableUTF8Print
      else
      {
        _encoding = c;
      }

      if (_utf8_state == 0)
      {
        if (_encoding == '\n')
        {
          curr_x = _min_text_x;
          curr_y += (int16_t)textsize_y * _u8g2_max_char_height;
        }
        else if (_encoding != '\r')
        {  // Ignore carriage returns
          const uint8_t* font = u8g2Font;
          const uint8_t* glyph_data = 0;

          // extract from u8g2_font_get_glyph_data()
          font += 23;  // U8G2_FONT_DATA_STRUCT_SIZE
          if (_encoding <= 255)
          {
            if (_encoding >= 'a')
            {
              font += _u8g2_start_pos_lower_a;
            }
            else if (_encoding >= 'A')
            {
              font += _u8g2_start_pos_upper_A;
            }

            for (;;)
            {
              if ((*(const unsigned char*)(font + 1)) == 0)
                break;
              if ((*(const unsigned char*)(font)) == _encoding)
              {
                glyph_data = font + 2; /* skip encoding and glyph size */
              }
              font += (*(const unsigned char*)(font + 1));
            }
          }
          else
          {
            uint16_t e;
            font += _u8g2_start_pos_unicode;
            const uint8_t* unicode_lookup_table = font;

            /* issue 596: search for the glyph start in the unicode lookup table */
            do
            {
              font += u8g2_font_get_word(unicode_lookup_table, 0);
              e = u8g2_font_get_word(unicode_lookup_table, 2);
              unicode_lookup_table += 4;
            } while (e < _encoding);

            for (;;)
            {
              e = u8g2_font_get_word(font, 0);

              if (e == 0)
                break;

              if (e == _encoding)
              {
                glyph_data = font + 3; /* skip encoding and glyph size */
                break;
              }
              font += (*(const unsigned char*)(font + 2));
            }
          }

          if (glyph_data)
          {
            // u8g2_font_decode_glyph
            _u8g2_decode_ptr = glyph_data;
            _u8g2_decode_bit_pos = 0;

            _u8g2_char_width = u8g2_font_decode_get_unsigned_bits(_u8g2_bits_per_char_width);
            _u8g2_char_height = u8g2_font_decode_get_unsigned_bits(_u8g2_bits_per_char_height);
            _u8g2_char_x = u8g2_font_decode_get_signed_bits(_u8g2_bits_per_char_x);
            _u8g2_char_y = u8g2_font_decode_get_signed_bits(_u8g2_bits_per_char_y);
            _u8g2_delta_x = u8g2_font_decode_get_signed_bits(_u8g2_bits_per_delta_x);

            if (_u8g2_char_width > 0)
            {
              if (wrap && ((curr_x + (textsize_x * _u8g2_char_width) - 1) > _max_text_x))
              {
                curr_x = _min_text_x;
                curr_y += (int16_t)textsize_y * _u8g2_max_char_height;
              }
            }

            int16_t x1_tmp = curr_x + ((int16_t)_u8g2_char_x * textsize_x),
                    y1_tmp = curr_y - (((int16_t)_u8g2_char_height + _u8g2_char_y) * textsize_y),
                    x2 = x1_tmp + ((int16_t)_u8g2_char_width * textsize_x) - 1,
                    y2 = y1_tmp + ((int16_t)_u8g2_char_height * textsize_y) - 1;
            if (x1_tmp < minx)
            {
              minx = x1_tmp;
            }
            if (y1_tmp < miny)
            {
              miny = y1_tmp;
            }
            if (x2 > maxx)
            {
              maxx = x2;
            }
            if (y2 > maxy)
            {
              maxy = y2;
            }
            curr_x += (int16_t)textsize_x * _u8g2_delta_x;
          }
        }
      }
    }
    else  // glcdfont
    {
      if (c == '\n')
      {                            // Newline?
        curr_x = _min_text_x;      // Reset x to zero,
        curr_y += textsize_y * 8;  // advance y one line
                                   // min/max x/y unchaged -- that waits for next 'normal' character
      }
      else if (c != '\r')  // Normal char; ignore carriage returns
      {
        if (wrap && ((curr_x + (textsize_x * 6) - 1) > _max_text_x))  // Off right?
        {
          curr_x = _min_text_x;      // Reset x to zero,
          curr_y += textsize_y * 8;  // advance y one line
        }
        int16_t x2 = curr_x + textsize_x * 6 - 1;  // Lower-right pixel of char
        int16_t y2 = curr_y + textsize_y * 8 - 1;
        if (x2 > maxx)
        {
          maxx = x2;  // Track max x, y
        }
        if (y2 > maxy)
        {
          maxy = y2;
        }
        if (curr_x < minx)
        {
          minx = curr_x;  // Track min x, y
        }
        if (curr_y < miny)
        {
          miny = curr_y;
        }
        curr_x += textsize_x * 6;  // Advance x one char
      }
    }
  }

  if (maxx >= minx)
  {
    x1 = minx;
    w = maxx - minx + 1;
  }
  if (maxy >= miny)
  {
    y1 = miny;
    h = maxy - miny + 1;
  }
}

void Arduino_Canvas::setTextSize(uint8_t s)
{
  setTextSize(s, s, 0);
}

void Arduino_Canvas::setTextSize(uint8_t sx, uint8_t sy)
{
  setTextSize(sx, sy, 0);
}

void Arduino_Canvas::setTextSize(uint8_t sx, uint8_t sy, uint8_t pixel_margin)
{
  text_pixel_margin = ((pixel_margin < sx) && (pixel_margin < sy)) ? pixel_margin : 0;
  textsize_x = (sx > 0) ? sx : 1;
  textsize_y = (sy > 0) ? sy : 1;
}

void Arduino_Canvas::setFont(const uint8_t* font)
{
  u8g2Font = font;

  // extract from u8g2_read_font_info()
  /* offset 0 */
  _u8g2_glyph_cnt = (*(const unsigned char*)(font));
  // uint8_t bbx_mode = pgm_read_byte(font + 1);
  _u8g2_bits_per_0 = (*(const unsigned char*)(font + 2));
  _u8g2_bits_per_1 = (*(const unsigned char*)(font + 3));
  // log_d("_u8g2_glyph_cnt: %d, bbx_mode: %d, _u8g2_bits_per_0: %d, _u8g2_bits_per_1: %d",
  //       _u8g2_glyph_cnt,
  //       bbx_mode,
  //       _u8g2_bits_per_0,
  //       _u8g2_bits_per_1);

  /* offset 4 */
  _u8g2_bits_per_char_width = (*(const unsigned char*)(font + 4));
  _u8g2_bits_per_char_height = (*(const unsigned char*)(font + 5));
  _u8g2_bits_per_char_x = (*(const unsigned char*)(font + 6));
  _u8g2_bits_per_char_y = (*(const unsigned char*)(font + 7));
  _u8g2_bits_per_delta_x = (*(const unsigned char*)(font + 8));
  // log_d("_u8g2_bits_per_char_width: %d, _u8g2_bits_per_char_height: %d, _u8g2_bits_per_char_x: %d, _u8g2_bits_per_char_y: %d, _u8g2_bits_per_delta_x: %d",
  //       _u8g2_bits_per_char_width,
  //       _u8g2_bits_per_char_height,
  //       _u8g2_bits_per_char_x,
  //       _u8g2_bits_per_char_y,
  //       _u8g2_bits_per_delta_x);

  /* offset 9 */
  _u8g2_max_char_width = (*(const unsigned char*)(font + 9));
  _u8g2_max_char_height = (*(const unsigned char*)(font + 10));
  // int8_t x_offset = pgm_read_byte(font + 11);
  // int8_t y_offset = pgm_read_byte(font + 12);
  // log_d("_u8g2_max_char_width: %d, _u8g2_max_char_height: %d, x_offset: %d, y_offset: %d",
  //       _u8g2_max_char_width, _u8g2_max_char_height, x_offset, y_offset);

  /* offset 13 */
  // int8_t ascent_A = pgm_read_byte(font + 13);
  // int8_t descent_g = pgm_read_byte(font + 14);
  // int8_t ascent_para = pgm_read_byte(font + 15);
  // int8_t descent_para = pgm_read_byte(font + 16);
  // log_d("ascent_A: %d, descent_g: %d, ascent_para: %d, descent_para: %d",
  //       ascent_A, descent_g, ascent_para, descent_para);

  /* offset 17 */
  _u8g2_start_pos_upper_A = u8g2_font_get_word(font, 17);
  _u8g2_start_pos_lower_a = u8g2_font_get_word(font, 19);
  _u8g2_start_pos_unicode = u8g2_font_get_word(font, 21);
  _u8g2_first_char = (*(const unsigned char*)(font + 23));
  // log_d("_u8g2_start_pos_upper_A: %d, _u8g2_start_pos_lower_a: %d, _u8g2_start_pos_unicode: %d, _u8g2_first_char: %d",
  //       _u8g2_start_pos_upper_A, _u8g2_start_pos_lower_a, _u8g2_start_pos_unicode, _u8g2_first_char);
}

void Arduino_Canvas::fillEllipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint16_t color)
{
  writeFillEllipseHelper(x, y, rx, ry, 3, 0, color);
}

void Arduino_Canvas::setPPAState(bool state)
{
  log_e("Модуль PPA відсутній на даному чіпі");
}

bool Arduino_Canvas::isPPAEnabled() const
{
  return false;
}

void Arduino_Canvas::flushMainBuff()
{
  convertToRGBA(_framebuffer, _opengl_rgba_buff, WIDTH * HEIGHT);
  renderWindow();
}

void Arduino_Canvas::duplicateMainBuff()
{
  log_e("Подвійну буферизацію не було увімкнуто в налаштуваннях прошивки");
  esp_restart();
}

void Arduino_Canvas::flushSecondBuff()
{
  log_e("Подвійну буферизацію не було увімкнуто в налаштуваннях прошивки");
  esp_restart();
}

uint16_t* Arduino_Canvas::getFramebuffer()
{
  return _framebuffer;
}

uint16_t Arduino_Canvas::u8g2_font_get_word(const uint8_t* font, uint8_t offset)
{
  uint16_t pos;
  font += offset;
  pos = (*(const unsigned char*)(font));
  font++;
  pos <<= 8;
  pos += (*(const unsigned char*)(font));

  return pos;
}

uint8_t Arduino_Canvas::u8g2_font_decode_get_unsigned_bits(uint8_t cnt)
{
  uint8_t val;
  uint8_t bit_pos = _u8g2_decode_bit_pos;
  uint8_t bit_pos_plus_cnt;

  val = (*(const unsigned char*)(_u8g2_decode_ptr));

  val >>= bit_pos;
  bit_pos_plus_cnt = bit_pos;
  bit_pos_plus_cnt += cnt;
  if (bit_pos_plus_cnt >= 8)
  {
    uint8_t s = 8;
    s -= bit_pos;
    _u8g2_decode_ptr++;
    val |= (*(const unsigned char*)(_u8g2_decode_ptr)) << (s);
    bit_pos_plus_cnt -= 8;
  }
  val &= (1U << cnt) - 1;

  _u8g2_decode_bit_pos = bit_pos_plus_cnt;

  return val;
}

int8_t Arduino_Canvas::u8g2_font_decode_get_signed_bits(uint8_t cnt)
{
  int8_t v, d;
  v = (int8_t)u8g2_font_decode_get_unsigned_bits(cnt);
  d = 1;
  cnt--;
  d <<= cnt;
  v -= d;

  return v;
}

void Arduino_Canvas::u8g2_font_decode_len(uint8_t len, uint8_t is_foreground, uint16_t color, uint16_t bg)
{
  uint8_t cnt;     /* total number of remaining pixels, which have to be drawn */
  uint8_t rem;     /* remaining pixel to the right edge of the glyph */
  uint8_t current; /* number of pixels, which need to be drawn for the draw procedure */
  /* current is either equal to cnt or equal to rem */

  /* target position on the screen */
  uint16_t x, y;

  cnt = len;

  /* get the local position */
  uint8_t lx = _u8g2_dx;
  uint8_t ly = _u8g2_dy;

  int16_t curW;
  for (;;)
  {
    /* calculate the number of pixel to the right edge of the glyph */
    rem = _u8g2_char_width;
    rem -= lx;

    /* calculate how many pixel to draw. This is either to the right edge */
    /* or lesser, if not enough pixel are left */
    current = rem;
    if (cnt < rem)
      current = cnt;

    /* now draw the line, but apply the rotation around the glyph target position */
    // u8g2_font_decode_draw_pixel(u8g2, lx,ly,current, is_foreground);

    if (textsize_x == 1 && textsize_y == 1)
    {
      /* get target position */
      x = _u8g2_target_x + lx;
      y = _u8g2_target_y + ly;

      /* draw foreground and background (if required) */
      if ((x <= _max_text_x) && (y <= _max_text_y))
      {
        curW = current;
        if ((x + curW - 1) > _max_text_x)
        {
          curW = _max_text_x - x + 1;
        }
        if (is_foreground)
        {
          writeFillRect(x, y, curW, 1, color);
        }
        else if (bg != color)
        {
          writeFillRect(x, y, curW, 1, bg);
        }
      }
    }
    else
    {
      /* get target position */
      x = _u8g2_target_x + (lx * textsize_x);
      y = _u8g2_target_y + (ly * textsize_y);

      /* draw foreground and background (if required) */
      if (((x + textsize_x - 1) <= _max_text_x) && ((y + textsize_y - 1) <= _max_text_y))
      {
        curW = current * textsize_x;
        while ((x + curW - 1) > _max_text_x)
        {
          curW -= textsize_x;
        }
        if (is_foreground)
        {
          writeFillRect(x, y, curW - text_pixel_margin, textsize_y - text_pixel_margin, color);
        }
        else if (bg != color)
        {
          writeFillRect(x, y, curW - text_pixel_margin, textsize_y - text_pixel_margin, bg);
        }
      }
    }

    /* check, whether the end of the run length code has been reached */
    if (cnt < rem)
      break;
    cnt -= rem;
    lx = 0;
    ly++;
  }
  lx += cnt;

  _u8g2_dx = lx;
  _u8g2_dy = ly;
}

void Arduino_Canvas::drawBitmapToFramebuffer(const uint16_t* from_bitmap,
                                             int16_t bitmap_w,
                                             int16_t bitmap_h,
                                             uint16_t* framebuffer,
                                             int16_t x,
                                             int16_t y,
                                             int16_t framebuffer_w,
                                             int16_t framebuffer_h)
{
  int16_t max_X = framebuffer_w - 1;
  int16_t max_Y = framebuffer_h - 1;
  int16_t x_skip = 0;

  if ((y + bitmap_h - 1) > max_Y)
  {
    bitmap_h -= (y + bitmap_h - 1) - max_Y;
  }

  if (y < 0)
  {
    from_bitmap -= y * bitmap_w;
    bitmap_h += y;
    y = 0;
  }

  if ((x + bitmap_w - 1) > max_X)
  {
    x_skip = (x + bitmap_w - 1) - max_X;
    bitmap_w -= x_skip;
  }

  if (x < 0)
  {
    from_bitmap -= x;
    x_skip -= x;
    bitmap_w += x;
    x = 0;
  }

  uint16_t* row = framebuffer;
  row += y * framebuffer_w;  // shift framebuffer to y offset
  row += x;                  // shift framebuffer to x offset
  if (((framebuffer_w & 1) == 0) && ((x_skip & 1) == 0) && ((bitmap_w & 1) == 0))
  {
    uint32_t* row2 = (uint32_t*)row;
    uint32_t* from_bitmap2 = (uint32_t*)from_bitmap;
    int16_t framebuffer_w2 = framebuffer_w >> 1;
    int16_t xskip2 = x_skip >> 1;
    int16_t w2 = bitmap_w >> 1;

    int16_t j = bitmap_h;
    while (j--)
    {
      for (int16_t i = 0; i < w2; ++i)
      {
        row2[i] = *from_bitmap2++;
      }
      from_bitmap2 += xskip2;
      row2 += framebuffer_w2;
    }
  }
  else
  {
    int16_t j = bitmap_h;
    while (j--)
    {
      for (int i = 0; i < bitmap_w; ++i)
      {
        row[i] = *from_bitmap++;
      }
      from_bitmap += x_skip;
      row += framebuffer_w;
    }
  }
}

void Arduino_Canvas::convertToRGBA(const uint16_t* buf565, uint8_t* out_buf_rgba8, unsigned pixels)
{
  for (unsigned i = 0; i < pixels; ++i)
  {
    uint16_t pixel = buf565[i];
    uint8_t r = ((pixel >> 11) & 0x1F) << 3;
    uint8_t g = ((pixel >> 5) & 0x3F) << 2;
    uint8_t b = (pixel & 0x1F) << 3;
    out_buf_rgba8[i * 4 + 0] = r;
    out_buf_rgba8[i * 4 + 1] = g;
    out_buf_rgba8[i * 4 + 2] = b;
    out_buf_rgba8[i * 4 + 3] = 255;
  }
}

void Arduino_Canvas::renderWindow()
{
  _texture.update(_opengl_rgba_buff);
  _window->clear();
  _window->draw(_sprite);
  _window->display();
}

void Arduino_Canvas::drawEllipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint16_t color)
{
  writeEllipseHelper(x, y, rx, ry, 0xf, color);
}

void Arduino_Canvas::writeEllipseHelper(int32_t x, int32_t y, int32_t rx, int32_t ry, uint8_t cornername, uint16_t color)
{
  if (rx < 0 || ry < 0 || ((rx == 0) && (ry == 0)))
  {
    return;
  }
  if (ry == 0)
  {
    writeFastHLine(x - rx, y, (ry << 2) + 1, color);
    return;
  }
  if (rx == 0)
  {
    writeFastVLine(x, y - ry, (rx << 2) + 1, color);
    return;
  }

  int32_t xt, yt, s, i;
  int32_t rx2 = rx * rx;
  int32_t ry2 = ry * ry;

  i = -1;
  xt = 0;
  yt = ry;
  s = (ry2 << 1) + rx2 * (1 - (ry << 1));
  do
  {
    while (s < 0)
      s += ry2 * ((++xt << 2) + 2);
    if (cornername & 0x1)
    {
      writeFastHLine(x - xt, y - yt, xt - i, color);
    }
    if (cornername & 0x2)
    {
      writeFastHLine(x + i + 1, y - yt, xt - i, color);
    }
    if (cornername & 0x4)
    {
      writeFastHLine(x + i + 1, y + yt, xt - i, color);
    }
    if (cornername & 0x8)
    {
      writeFastHLine(x - xt, y + yt, xt - i, color);
    }
    i = xt;
    s -= (--yt) * rx2 << 2;
  } while (ry2 * xt <= rx2 * yt);

  i = -1;
  yt = 0;
  xt = rx;
  s = (rx2 << 1) + ry2 * (1 - (rx << 1));
  do
  {
    while (s < 0)
      s += rx2 * ((++yt << 2) + 2);
    if (cornername & 0x1)
    {
      writeFastVLine(x - xt, y - yt, yt - i, color);
    }
    if (cornername & 0x2)
    {
      writeFastVLine(x + xt, y - yt, yt - i, color);
    }
    if (cornername & 0x4)
    {
      writeFastVLine(x + xt, y + i + 1, yt - i, color);
    }
    if (cornername & 0x8)
    {
      writeFastVLine(x - xt, y + i + 1, yt - i, color);
    }
    i = yt;
    s -= (--xt) * ry2 << 2;
  } while (rx2 * yt <= ry2 * xt);
}

void Arduino_Canvas::writeFillEllipseHelper(int32_t x, int32_t y, int32_t rx, int32_t ry, uint8_t cornername, int16_t delta, uint16_t color)
{
  if (rx < 0 || ry < 0 || ((rx == 0) && (ry == 0)))
  {
    return;
  }
  if (ry == 0)
  {
    writeFastHLine(x - rx, y, (ry << 2) + 1, color);
    return;
  }
  if (rx == 0)
  {
    writeFastVLine(x, y - ry, (rx << 2) + 1, color);
    return;
  }

  int32_t xt, yt, i;
  int32_t rx2 = (int32_t)rx * rx;
  int32_t ry2 = (int32_t)ry * ry;
  int32_t s;

  writeFastHLine(x - rx, y, (rx << 1) + 1, color);
  i = 0;
  yt = 0;
  xt = rx;
  s = (rx2 << 1) + ry2 * (1 - (rx << 1));
  do
  {
    while (s < 0)
    {
      s += rx2 * ((++yt << 2) + 2);
    }
    if (cornername & 1)
    {
      writeFillRect(x - xt, y - yt, (xt << 1) + 1 + delta, yt - i, color);
    }
    if (cornername & 2)
    {
      writeFillRect(x - xt, y + i + 1, (xt << 1) + 1 + delta, yt - i, color);
    }
    i = yt;
    s -= (--xt) * ry2 << 2;
  } while (rx2 * yt <= ry2 * xt);

  xt = 0;
  yt = ry;
  s = (ry2 << 1) + rx2 * (1 - (ry << 1));
  do
  {
    while (s < 0)
    {
      s += ry2 * ((++xt << 2) + 2);
    }
    if (cornername & 1)
    {
      writeFastHLine(x - xt, y - yt, (xt << 1) + 1 + delta, color);
    }
    if (cornername & 2)
    {
      writeFastHLine(x - xt, y + yt, (xt << 1) + 1 + delta, color);
    }
    s -= (--yt) * rx2 << 2;
  } while (ry2 * xt <= rx2 * yt);
}

void Arduino_Canvas::drawArc(int16_t x, int16_t y, int16_t r1, int16_t r2, float start, float end, uint16_t color)
{
  if (r1 < r2)
  {
    _swap_int16_t(r1, r2);
  }
  if (r1 < 1)
  {
    r1 = 1;
  }
  if (r2 < 1)
  {
    r2 = 1;
  }
  bool equal = fabsf(start - end) < FLT_EPSILON;
  start = fmodf(start, 360);
  end = fmodf(end, 360);
  if (start < 0)
    start += 360.0;
  if (end < 0)
    end += 360.0;

  writeFillArcHelper(x, y, r1, r2, start, start, color);
  writeFillArcHelper(x, y, r1, r2, end, end, color);
  if (!equal && (fabsf(start - end) <= 0.0001))
  {
    start = .0;
    end = 360.0;
  }
  writeFillArcHelper(x, y, r1, r1, start, end, color);
  writeFillArcHelper(x, y, r2, r2, start, end, color);
}

void Arduino_Canvas::fillArc(int16_t x, int16_t y, int16_t r1, int16_t r2, float start, float end, uint16_t color)
{
  if (r1 < r2)
  {
    _swap_int16_t(r1, r2);
  }
  if (r1 < 1)
  {
    r1 = 1;
  }
  if (r2 < 1)
  {
    r2 = 1;
  }
  bool equal = fabsf(start - end) < FLT_EPSILON;
  start = fmodf(start, 360);
  end = fmodf(end, 360);
  if (start < 0)
    start += 360.0;
  if (end < 0)
    end += 360.0;
  if (!equal && (fabsf(start - end) <= 0.0001))
  {
    start = .0;
    end = 360.0;
  }

  writeFillArcHelper(x, y, r1, r2, start, end, color);
}

void Arduino_Canvas::writeFillArcHelper(int16_t cx, int16_t cy, int16_t oradius, int16_t iradius, float start, float end, uint16_t color)
{
  if ((start == 90.0) || (start == 180.0) || (start == 270.0) || (start == 360.0))
  {
    start -= 0.1;
  }

  if ((end == 90.0) || (end == 180.0) || (end == 270.0) || (end == 360.0))
  {
    end -= 0.1;
  }

  float s_cos = (cos(start * DEGTORAD));
  float e_cos = (cos(end * DEGTORAD));
  float sslope = s_cos / (sin(start * DEGTORAD));
  float eslope = e_cos / (sin(end * DEGTORAD));
  float swidth = 0.5 / s_cos;
  float ewidth = -0.5 / e_cos;
  --iradius;
  int32_t ir2 = iradius * iradius + iradius;
  int32_t or2 = oradius * oradius + oradius;

  bool start180 = !(start < 180.0);
  bool end180 = end < 180.0;
  bool reversed = start + 180.0 < end || (end < start && start < end + 180.0);

  int32_t xs = -oradius;
  int32_t y = -oradius;
  int32_t ye = oradius;
  int32_t xe = oradius + 1;
  if (!reversed)
  {
    if ((end >= 270 || end < 90) && (start >= 270 || start < 90))
    {
      xs = 0;
    }
    else if (end < 270 && end >= 90 && start < 270 && start >= 90)
    {
      xe = 1;
    }
    if (end >= 180 && start >= 180)
    {
      ye = 0;
    }
    else if (end < 180 && start < 180)
    {
      y = 0;
    }
  }
  do
  {
    int32_t y2 = y * y;
    int32_t x = xs;
    if (x < 0)
    {
      while (x * x + y2 >= or2)
      {
        ++x;
      }
      if (xe != 1)
      {
        xe = 1 - x;
      }
    }
    float ysslope = (y + swidth) * sslope;
    float yeslope = (y + ewidth) * eslope;
    int32_t len = 0;
    do
    {
      bool flg1 = start180 != (x <= ysslope);
      bool flg2 = end180 != (x <= yeslope);
      int32_t distance = x * x + y2;
      if (distance >= ir2 && ((flg1 && flg2) || (reversed && (flg1 || flg2))) && x != xe && distance < or2)
      {
        ++len;
      }
      else
      {
        if (len)
        {
          writeFastHLine(cx + x - len, cy + y, len, color);
          len = 0;
        }
        if (distance >= or2)
          break;
        if (x < 0 && distance < ir2)
        {
          x = -x;
        }
      }
    } while (++x <= xe);
  } while (++y <= ye);
}
