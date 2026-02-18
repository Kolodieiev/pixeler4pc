#pragma GCC optimize("O3")
#include "Arduino_Canvas.h"

#include <cstring>
#include <iostream>

#ifndef _ordered_in_range
#define _ordered_in_range(v, a, b) ((a <= v) && (v <= b))
#endif

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) \
  {                         \
    int16_t t = a;          \
    a = b;                  \
    b = t;                  \
  }
#endif

#ifndef _diff
#define _diff(a, b) ((a > b) ? (a - b) : (b - a))
#endif

namespace pixeler
{
  Arduino_Canvas::Arduino_Canvas(uint16_t width, uint16_t height) : _framebuffer{new uint16_t[TFT_WIDTH * TFT_HEIGHT]},
                                                                    _opengl_rgba_buff{new uint8_t[TFT_WIDTH * TFT_HEIGHT * 4]},
                                                                    BUFF_SIZE{TFT_WIDTH * TFT_HEIGHT * sizeof(uint16_t)},
                                                                    MAX_X{TFT_WIDTH - 1},
                                                                    MAX_Y{TFT_HEIGHT - 1},
                                                                    _max_text_x{static_cast<int16_t>(MAX_X)},
                                                                    _max_text_y{static_cast<int16_t>(MAX_Y)}
  {
  }

  Arduino_Canvas::~Arduino_Canvas()
  {
    delete[] _framebuffer;
    delete[] _opengl_rgba_buff;
  }

  bool Arduino_Canvas::begin(sf::RenderWindow* window)
  {
    if (!window)
    {
      _is_inited = false;
      printf("Помилка ініціалізації Canvas\n");
      return false;
    }

    _window = window;

#if SFML_VERSION_MAJOR < 3
    _image.create(TFT_WIDTH, TFT_HEIGHT, _opengl_rgba_buff);
    _texture.loadFromImage(_image);
    _sprite.setTexture(_texture);

    sf::View view = _window->getDefaultView();
    uint32_t view_w = view.getSize().x;
    uint32_t view_h = view.getSize().y;

    if (view_h > view_w)
    {
      _sprite.setScale(view_w / TFT_WIDTH, view_w / TFT_WIDTH);
      _sprite.setPosition((view_w - (TFT_WIDTH * view_w / TFT_WIDTH)) / 2.0f, 40);
    }
    else
    {
      _sprite.setScale(view_h / TFT_HEIGHT, view_h / TFT_HEIGHT);
      _sprite.setPosition((view_w - (TFT_WIDTH * view_h / TFT_HEIGHT)) / 2.0f, 40);
    }

#endif  // #if SFML_VERSION_MAJOR < 3

    _is_inited = true;
    printf("Canvas успішно ініціалізовано\n");
    return true;
  }

  uint16_t* Arduino_Canvas::getFramebuffer()
  {
    return _framebuffer;
  }

  void Arduino_Canvas::flushMainBuff()
  {
    convertToRGBA(_framebuffer, _opengl_rgba_buff, TFT_WIDTH * TFT_HEIGHT);
    renderWindow();
  }

  void Arduino_Canvas::convertToRGBA(const uint16_t* buf_rgb565, uint8_t* out_buf_rgba8, unsigned pixels)
  {
    for (unsigned i = 0; i < pixels; ++i)
    {
      uint16_t pixel = buf_rgb565[i];
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

  void Arduino_Canvas::writePixelPreclipped(int16_t x, int16_t y, uint16_t color)
  {
    uint16_t* fb = _framebuffer;
    fb += (int32_t)y * TFT_WIDTH;
    fb += x;
    *fb = color;
  }

  void Arduino_Canvas::writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
  {
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

          uint16_t* fb = _framebuffer + ((int32_t)y * TFT_WIDTH) + x;
          while (h--)
          {
            *fb = color;
            fb += TFT_WIDTH;
          }
        }
      }
    }
  }

  void Arduino_Canvas::writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
  {
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

          uint16_t* fb = _framebuffer + ((int32_t)y * TFT_WIDTH) + x;
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
    uint16_t* row = _framebuffer;
    row += y * TFT_WIDTH;
    row += x;
    for (int j = 0; j < h; j++)
    {
      for (int i = 0; i < w; i++)
      {
        row[i] = color;
      }
      row += TFT_WIDTH;
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
    {
      return;
    }
    else
    {
      int16_t x_skip = 0;
      if ((y + h - 1) > MAX_Y)
      {
        h -= (y + h - 1) - MAX_Y;
      }
      if (y < 0)
      {
        bitmap -= y * w;
        h += y;
        y = 0;
      }
      if ((x + w - 1) > MAX_X)
      {
        x_skip = (x + w - 1) - MAX_X;
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
      row += y * TFT_WIDTH;  // shift _framebuffer to y offset
      row += x;              // shift _framebuffer to x offset
      if (((TFT_WIDTH & 1) == 0) && ((x_skip & 1) == 0) && ((w & 1) == 0))
      {
        uint32_t* row2 = (uint32_t*)row;
        uint32_t* bitmap2 = (uint32_t*)bitmap;
        int16_t TFT_WIDTH2 = TFT_WIDTH >> 1;
        int16_t xskip2 = x_skip >> 1;
        int16_t w2 = w >> 1;

        int16_t j = h;
        while (j--)
        {
          for (int16_t i = 0; i < w2; ++i)
          {
            row2[i] = *bitmap2++;
          }
          bitmap2 += xskip2;
          row2 += TFT_WIDTH2;
        }
      }
      else
      {
        int16_t j = h;
        while (j--)
        {
          for (int i = 0; i < w; ++i)
          {
            row[i] = *bitmap++;
          }
          bitmap += x_skip;
          row += TFT_WIDTH;
        }
      }
    }
  }

  void Arduino_Canvas::draw16bitRGBBitmapWithTranColor(int16_t x, int16_t y, const uint16_t* bitmap, uint16_t transparent_color, int16_t w, int16_t h)
  {
    if (
        ((x + w - 1) < 0) ||  // Outside left
        ((y + h - 1) < 0) ||  // Outside top
        (x > MAX_X) ||        // Outside right
        (y > MAX_Y)           // Outside bottom
    )
    {
      return;
    }
    else
    {
      int16_t x_skip = 0;
      if ((y + h - 1) > MAX_Y)
      {
        h -= (y + h - 1) - MAX_Y;
      }
      if (y < 0)
      {
        bitmap -= y * w;
        h += y;
        y = 0;
      }
      if ((x + w - 1) > MAX_X)
      {
        x_skip = (x + w - 1) - MAX_X;
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
      row += y * TFT_WIDTH;
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
        row += TFT_WIDTH;
      }
    }
  }

  uint16_t Arduino_Canvas::width() const
  {
    return TFT_WIDTH;
  }

  uint16_t Arduino_Canvas::height() const
  {
    return TFT_HEIGHT;
  }

  void Arduino_Canvas::setTextWrap(bool state)
  {
    wrap = state;
  }

  void Arduino_Canvas::setTextColor(uint16_t c)
  {
    textcolor = textbgcolor = c;
  }

  void Arduino_Canvas::setTextColor(uint16_t c, uint16_t bg)
  {
    textcolor = c;
    textbgcolor = bg;
  }

  void Arduino_Canvas::fillScreen(uint16_t color)
  {
    fillRect(0, 0, TFT_WIDTH, TFT_HEIGHT, color);
  }

  void Arduino_Canvas::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
  {
    writeFillRect(x, y, w, h, color);
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
      if (x <= MAX_X)
      {  // Not off right
        if (h < 0)
        {              // If negative height...
          y += h + 1;  //   Move Y to top edge
          h = -h;      //   Use positive height
        }
        if (y <= MAX_Y)
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
              if (x2 > MAX_X)
              {
                w = MAX_X - x + 1;
              }  // Clip right
              if (y2 > MAX_Y)
              {
                h = MAX_Y - y + 1;
              }  // Clip bottom
              writeFillRectPreclipped(x, y, w, h, color);
            }
          }
        }
      }
    }
  }

  void Arduino_Canvas::setCursor(int16_t x, int16_t y)
  {
    cursor_x = x;
    cursor_y = y;
  }

  void Arduino_Canvas::setFont(const uint8_t* font)
  {
    u8g2Font = font;

    // extract from u8g2_read_font_info()
    /* offset 0 */
    _u8g2_glyph_cnt = (*(const unsigned char*)(font));
    // uint8_t bbx_mode = (*(const unsigned char*)(font + 1);
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
    // int8_t x_offset = (*(const unsigned char*)(font + 11);
    // int8_t y_offset = (*(const unsigned char*)(font + 12);
    // log_d("_u8g2_max_char_width: %d, _u8g2_max_char_height: %d, x_offset: %d, y_offset: %d",
    //       _u8g2_max_char_width, _u8g2_max_char_height, x_offset, y_offset);

    /* offset 13 */
    // int8_t ascent_A = (*(const unsigned char*)(font + 13);
    // int8_t descent_g = (*(const unsigned char*)(font + 14);
    // int8_t ascent_para = (*(const unsigned char*)(font + 15);
    // int8_t descent_para = (*(const unsigned char*)(font + 16);
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

  void Arduino_Canvas::setTextSize(uint8_t s)
  {
    setTextSize(s, s, 0);
  }

  void Arduino_Canvas::setTextSize(uint8_t s_x, uint8_t s_y)
  {
    setTextSize(s_x, s_y, 0);
  }

  void Arduino_Canvas::setTextSize(uint8_t s_x, uint8_t s_y, uint8_t pixel_margin)
  {
    text_pixel_margin = ((pixel_margin < s_x) && (pixel_margin < s_y)) ? pixel_margin : 0;
    textsize_x = (s_x > 0) ? s_x : 1;
    textsize_y = (s_y > 0) ? s_y : 1;
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

  void Arduino_Canvas::drawPixel(int16_t x, int16_t y, uint16_t color)
  {
    if (_ordered_in_range(y, 0, MAX_Y))
    {
      writePixelPreclipped(x, y, color);
    }
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
        drawPixel(y0, x0, color);
      }
      else
      {
        drawPixel(x0, y0, color);
      }
      err -= dy;
      if (err < 0)
      {
        err += dx;
        y0 += step;
      }
    }
  }

  void Arduino_Canvas::drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color)
  {
    writeEllipseHelper(x, y, r, r, 0xf, color);
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

  void Arduino_Canvas::fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color)
  {
    writeFillEllipseHelper(x, y, r, r, 3, 0, color);
  }

  void Arduino_Canvas::writeFillEllipseHelper(int32_t x, int32_t y, int32_t rx, int32_t ry, uint8_t corners, int16_t delta, uint16_t color)
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
      if (corners & 1)
      {
        writeFillRect(x - xt, y - yt, (xt << 1) + 1 + delta, yt - i, color);
      }
      if (corners & 2)
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
      if (corners & 1)
      {
        writeFastHLine(x - xt, y - yt, (xt << 1) + 1 + delta, color);
      }
      if (corners & 2)
      {
        writeFastHLine(x - xt, y + yt, (xt << 1) + 1 + delta, color);
      }
      s -= (--yt) * rx2 << 2;
    } while (ry2 * xt <= rx2 * yt);
  }

  void Arduino_Canvas::drawEllipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint16_t color)
  {
    writeEllipseHelper(x, y, rx, ry, 0xf, color);
  }

  void Arduino_Canvas::fillEllipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint16_t color)
  {
    writeFillEllipseHelper(x, y, rx, ry, 3, 0, color);
  }

  void Arduino_Canvas::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
  {
    writeFastHLine(x, y, w, color);
    writeFastHLine(x, y + h - 1, w, color);
    writeFastVLine(x, y, h, color);
    writeFastVLine(x + w - 1, y, h, color);
  }

  void Arduino_Canvas::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color)
  {
    int16_t max_radius = ((w < h) ? w : h) / 2;  // 1/2 minor axis
    if (r > max_radius)
      r = max_radius;
    // smarter version
    writeFastHLine(x + r, y, w - 2 * r, color);          // Top
    writeFastHLine(x + r, y + h - 1, w - 2 * r, color);  // Bottom
    writeFastVLine(x, y + r, h - 2 * r, color);          // Left
    writeFastVLine(x + w - 1, y + r, h - 2 * r, color);  // Right
    // draw four corners
    writeEllipseHelper(x + r, y + r, r, r, 1, color);
    writeEllipseHelper(x + w - r - 1, y + r, r, r, 2, color);
    writeEllipseHelper(x + w - r - 1, y + h - r - 1, r, r, 4, color);
    writeEllipseHelper(x + r, y + h - r - 1, r, r, 8, color);
  }

  void Arduino_Canvas::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color)
  {
    int16_t max_radius = ((w < h) ? w : h) / 2;  // 1/2 minor axis
    if (r > max_radius)
      r = max_radius;
    // smarter version
    writeFillRect(x, y + r, w, h - (r << 1), color);
    // draw four corners
    writeFillEllipseHelper(x + r, y + r, r, r, 1, w - 2 * r - 1, color);
    writeFillEllipseHelper(x + r, y + h - r - 1, r, r, 2, w - 2 * r - 1, color);
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

  size_t Arduino_Canvas::print(const char* str)
  {
    if (!str)
      return 0;

    return write((const uint8_t*)str, strlen(str));
  }

  size_t Arduino_Canvas::print(char c)
  {
    return write(c);
  }

  size_t Arduino_Canvas::write(const uint8_t* buffer, size_t size)
  {
    size_t n = 0;
    while (size--)
    {
      n += write(*buffer++);
    }
    return n;
  }

  size_t Arduino_Canvas::write(uint8_t c)
  {
    _u8g2_decode_ptr = 0;

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

          drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor);
          cursor_x += (int16_t)textsize_x * _u8g2_delta_x;
        }
      }
    }
    return 1;
  }

  void Arduino_Canvas::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg)
  {
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

  uint16_t Arduino_Canvas::color565(uint8_t red, uint8_t green, uint8_t blue)
  {
    return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
  }

  int16_t Arduino_Canvas::getCursorX(void) const
  {
    return cursor_x;
  }

  int16_t Arduino_Canvas::getCursorY(void) const
  {
    return cursor_y;
  };

}  // namespace pixeler
