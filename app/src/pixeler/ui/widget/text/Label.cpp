#pragma GCC optimize("O3")
#include "Label.h"

#define RESERVE_PIX_WIDTH 7
#define RESERVE_PIX_HEIGHT 4

namespace pixeler
{
  Label::Label(uint16_t widget_ID, IWidget::TypeID type_ID) : IWidget(widget_ID, type_ID)
  {
    updateHeight();
  }

  Label* Label::clone(uint16_t id) const
  {
    try
    {
      Label* cln = new Label(id);
      cln->_has_border = _has_border;
      cln->_x_pos = _x_pos;
      cln->_y_pos = _y_pos;
      cln->_width = _width;
      cln->_height = _height;
      cln->_back_color = _back_color;
      cln->_border_color = _border_color;
      cln->_corner_radius = _corner_radius;
      cln->_is_transparent = _is_transparent;
      cln->_visibility = _visibility;
      cln->_has_focus = _has_focus;
      cln->_old_border_state = _old_border_state;
      cln->_need_clear_border = _need_clear_border;
      cln->_need_change_border = _need_change_border;
      cln->_need_change_back = _need_change_back;
      cln->_focus_border_color = _focus_border_color;
      cln->_old_border_color = _old_border_color;
      cln->_focus_back_color = _focus_back_color;
      cln->_old_back_color = _old_back_color;
      cln->_parent = _parent;

      cln->_is_multiline = _is_multiline;
      cln->setText(_text);
      cln->_text_size = _text_size;
      cln->_text_color = _text_color;
      cln->_font_ptr = _font_ptr;
      cln->_char_hgt = _char_hgt;
      cln->_h_padding = _h_padding;
      cln->_text_gravity = _text_gravity;
      cln->_text_alignment = _text_alignment;
      cln->_has_autoscroll = _has_autoscroll;
      cln->_temp_has_autoscroll = _temp_has_autoscroll;
      cln->_has_autoscroll_in_focus = _has_autoscroll_in_focus;
      cln->_temp_has_autoscroll_in_focus = _temp_has_autoscroll_in_focus;
      cln->_temp_width = _temp_width;

      if (_back_img)
      {
        cln->_back_img = _back_img->clone(_back_img->getID());
      }

      return cln;
    }
    catch (const std::bad_alloc& e)
    {
      log_e("%s", e.what());
      esp_restart();
    }
  }

  void Label::initWidthToFit(uint16_t add_width_value)
  {
    _width = calcTextPixels();
    _width += RESERVE_PIX_WIDTH + add_width_value + _h_padding * 2;

    _is_changed = true;
  }

  void Label::updateWidthToFit(uint16_t add_width_value)
  {
    _temp_width = calcTextPixels();
    _temp_width += RESERVE_PIX_WIDTH + add_width_value + _h_padding * 2;

    if (_temp_width == _width)
      _temp_width = 0;

    _is_changed = true;
  }

  void Label::setText(const String& text)
  {
    try
    {
      _text = text;
      _text_len = calcRealStrLen(_text);
      _has_autoscroll = _temp_has_autoscroll;
      _has_autoscroll_in_focus = _temp_has_autoscroll_in_focus;
    }
    catch (const std::bad_alloc& e)
    {
      log_e("%s", e.what());
      _text = "err";
      _text_len = 0;
    }

    _first_draw_char_pos = 0;
    _is_changed = true;
  }

  String Label::getText() const
  {
    return _text;
  }

  void Label::setTextSize(uint8_t size)
  {
    if (size == 0)
      size = 1;

    _text_size = size;
    _is_changed = true;
    updateHeight();
  }

  void Label::setTextColor(uint16_t textColor)
  {
    _text_color = textColor;
    _is_changed = true;
  }

  uint16_t Label::getCharHgt() const
  {
    return _char_hgt;
  }

  void Label::setBackImg(Image* back_img)
  {
    _back_img = back_img;
    _is_changed = true;
  }

  void Label::setMultiline(bool state)
  {
    _is_multiline = state;

    if (_is_multiline)
      _width = TFT_WIDTH;

    _is_changed = true;
  }

  void Label::setAutoscrollDelay(uint16_t delay)
  {
    _autoscroll_update_delay = delay;
    _is_changed = true;
  }

  void Label::setAutoscroll(bool state)
  {
    _has_autoscroll = state;
    _temp_has_autoscroll = state;

    if (!state)
      _first_draw_char_pos = 0;

    _is_changed = true;
  }

  void Label::setAutoscrollInFocus(bool state)
  {
    _has_autoscroll_in_focus = state;
    _temp_has_autoscroll_in_focus = state;

    if (!state && _has_autoscroll)
      _first_draw_char_pos = 0;

    _is_changed = true;
  }

  void Label::setFont(const uint8_t* font_ptr)
  {
    if (!font_ptr)
      font_ptr = font_unifont;

    _font_ptr = font_ptr;
    _is_changed = true;

    updateHeight();
  }

  const uint8_t* Label::getFont() const
  {
    return _font_ptr;
  }

  void Label::setGravity(const Gravity gravity)
  {
    _text_gravity = gravity;
    _is_changed = true;
  }

  void Label::setAlign(const Alignment alignment)
  {
    _text_alignment = alignment;
    _is_changed = true;
  }

  void Label::setHPadding(uint8_t padding)
  {
    _h_padding = padding;
    _is_changed = true;
  }

  void Label::setFullAutoscroll(bool state)
  {
    _is_changed = true;
    _first_draw_char_pos = 0;
    _has_full_autoscroll = state;
    _is_reverse_autoscroll = false;
  }

  uint16_t Label::calcXStrOffset(uint16_t str_pix_num) const
  {
    if (_width - RESERVE_PIX_WIDTH <= str_pix_num + _h_padding * 2)
      return 1;

    if (_has_autoscroll)
      return _h_padding + 1;

    switch (_text_alignment)
    {
      case ALIGN_START:
        return _h_padding + 1;
      case ALIGN_CENTER:
        return static_cast<uint16_t>((static_cast<float>(_width - str_pix_num)) / 2);
      case ALIGN_END:
        return _width - str_pix_num - _h_padding - RESERVE_PIX_WIDTH / 2;
      default:
        return 1;
    }
  }

  uint16_t Label::calcYStrOffset() const
  {
    if (_height > _char_hgt)
    {
      switch (_text_gravity)
      {
        case GRAVITY_TOP:
          return 1;
        case GRAVITY_CENTER:
          return (_height - _char_hgt) / 2;
        case GRAVITY_BOTTOM:
          return _height - _char_hgt - 1;
      }
    }

    return 0;
  }

  uint32_t Label::calcRealStrLen(const String& str) const
  {
    if (str.isEmpty())
      return 0;

    uint32_t length{0};

    for (const char c : str)
    {
      if ((c & 0xC0) != 0x80)
        ++length;
    }
    return length;
  }

  uint16_t Label::calcTextPixels(uint16_t char_pos) const
  {
    int16_t x1{0};
    int16_t y1{0};
    uint16_t w{0};
    uint16_t h{0};

    uint32_t byte_pos{0};
    if (char_pos > 0)
    {
      const uint8_t* ch_str_p8 = reinterpret_cast<uint8_t*>(const_cast<char*>(_text.c_str()));
      byte_pos = charPosToByte(ch_str_p8, char_pos);
    }

    _display.setFont(_font_ptr);
    _display.setTextSize(_text_size);
    _display.calcTextBounds(_text.c_str() + byte_pos, 0, 0, x1, y1, w, h);

    return w;
  }

  uint16_t Label::charPosToByte(const uint8_t* buf, uint16_t char_pos) const
  {
    if (char_pos == 0)
      return 0;

    uint16_t char_num = 0;
    uint16_t i = 0;

    while (char_num < char_pos && buf[i] != '\0')
    {
      if ((buf[i] & 0x80) == 0x00)
        ++i;
      else if ((buf[i] & 0xE0) == 0xC0)
        i += 2;
      else if ((buf[i] & 0xF0) == 0xE0)
        i += 3;
      else if ((buf[i] & 0xF8) == 0xF0)
        i += 4;
      else
        ++i;

      ++char_num;
    }

    return i;
  }

  void Label::updateHeight()
  {
    int16_t x1, y1;
    uint16_t w;
    _display.setTextSize(_text_size);
    _display.setFont(_font_ptr);
    _display.calcTextBounds("I", 0, 0, x1, y1, w, _char_hgt);

    if (_char_hgt + RESERVE_PIX_HEIGHT > _height)
      _height = _char_hgt + RESERVE_PIX_HEIGHT;
  }

  uint32_t Label::utf8ToUnicode(const uint8_t* buf, uint16_t& byte_pos, uint16_t remaining) const
  {
    uint32_t codepoint = 0;

    if ((buf[byte_pos] & 0x80) == 0x00)
    {
      // 1-byte sequence
      codepoint = buf[byte_pos];
      ++byte_pos;
    }
    else if ((buf[byte_pos] & 0xE0) == 0xC0 && remaining > 1)
    {
      // 2-byte sequence
      codepoint = (buf[byte_pos] & 0x1F) << 6;
      ++byte_pos;
      codepoint |= (buf[byte_pos] & 0x3F);
      ++byte_pos;
    }
    else if ((buf[byte_pos] & 0xF0) == 0xE0 && remaining > 2)
    {
      // 3-byte sequence
      codepoint = (buf[byte_pos] & 0x0F) << 12;
      ++byte_pos;
      codepoint |= (buf[byte_pos] & 0x3F) << 6;
      ++byte_pos;
      codepoint |= (buf[byte_pos] & 0x3F);
      ++byte_pos;
    }
    else if ((buf[byte_pos] & 0xF8) == 0xF0 && remaining > 3)
    {
      // 4-byte sequence
      codepoint = (buf[byte_pos] & 0x07) << 18;
      ++byte_pos;
      codepoint |= (buf[byte_pos] & 0x3F) << 12;
      ++byte_pos;
      codepoint |= (buf[byte_pos] & 0x3F) << 6;
      ++byte_pos;
      codepoint |= (buf[byte_pos] & 0x3F);
      ++byte_pos;
    }
    else
      ++byte_pos;

    return codepoint;
  }

  uint16_t Label::getFitStr(String& ret_str, uint16_t start_pos) const
  {
    if (_text.isEmpty())
    {
      ret_str = "";
      return 0;
    }

    uint16_t len = _text.length();

    if (start_pos >= len)
    {
      ret_str = "";
      return 0;
    }

    const uint8_t* ch_str_p8 = reinterpret_cast<uint8_t*>(const_cast<char*>(_text.c_str()));
    uint16_t cur_byte_pos = charPosToByte(ch_str_p8, start_pos);
    const uint16_t start_byte_pos{cur_byte_pos};

    uint16_t pix_sum{0};
    uint16_t chars_counter{0};

    int16_t x1, y1;
    uint16_t w, h;
    String new_str;
    while (cur_byte_pos < len)
    {
      uint32_t code = utf8ToUnicode(ch_str_p8, cur_byte_pos, len - cur_byte_pos);
      if (code == 0)
        continue;

      new_str = _text.substring(start_byte_pos, cur_byte_pos);
      _display.calcTextBounds(new_str.c_str(), 0, 0, x1, y1, w, h);

      if (w < _width - _h_padding * 2 - 2)
      {
        pix_sum = w;
        ++chars_counter;
      }
      else
      {
        break;
      }
    }

    ret_str = getSubStr(_text, start_pos, chars_counter);

    return pix_sum;
  }

  String Label::getSubStr(const String& str, uint16_t start, uint16_t length) const
  {
    if (length == 0)
      return emptyString;

    int bytePos = 0;
    int charIndex = 0;
    int startByte = -1;
    int endByte = -1;

    while (bytePos < str.length())
    {
      if (charIndex == start)
        startByte = bytePos;

      if (charIndex == start + length)
      {
        endByte = bytePos;
        break;
      }

      unsigned char c = str[bytePos];
      int charLen = 1;

      if ((c & 0x80) == 0x00)
        charLen = 1;
      else if ((c & 0xE0) == 0xC0)
        charLen = 2;
      else if ((c & 0xF0) == 0xE0)
        charLen = 3;
      else if ((c & 0xF8) == 0xF0)
        charLen = 4;
      else
        charLen = 1;  // Невідомий символ — вважаємо за 1 байт

      bytePos += charLen;
      ++charIndex;
    }

    if (startByte == -1)
      return emptyString;

    if (endByte == -1)
      endByte = str.length();

    return str.substring(startByte, endByte);
  }

  void Label::onDraw()
  {
    if (!_is_changed)
    {
      if ((!_has_autoscroll && !(_has_autoscroll_in_focus && _has_focus)) || _visibility == INVISIBLE)
        return;
      else if ((millis() - _last_autoscroll_ts) < _autoscroll_update_delay)
        return;
    }

    _is_changed = false;

    if (_visibility == INVISIBLE)
    {
      hide();
      return;
    }

    if (_has_autoscroll_in_focus && !_has_focus)
      _first_draw_char_pos = 0;

    if (_temp_width > 0)
    {
      if (_temp_width > _width)
      {
        _width = _temp_width;
        clear();
      }
      else
      {
        clear(false);
        _width = _temp_width;

        if (_has_border)
          clear();
      }

      _temp_width = 0;
    }
    else
    {
      clear();
    }

    _display.setTextColor(_text_color);

    uint16_t txtYPos = calcYStrOffset();
    uint16_t str_pix_num = calcTextPixels();

    uint16_t x_offset{0};
    uint16_t y_offset{0};

    if (_parent)
    {
      x_offset = _parent->getXPos();
      y_offset = _parent->getYPos();
    }

    if (_back_img)
    {
      _back_img->setParent(this);
      _back_img->forcedDraw();
    }

    if (str_pix_num + _h_padding * 2 - 2 < _width)
    {
      _has_autoscroll = false;
      _has_autoscroll_in_focus = false;
      uint16_t txt_x_pos = calcXStrOffset(str_pix_num);
      _display.setCursor(_x_pos + x_offset + txt_x_pos, _y_pos + y_offset + txtYPos + _char_hgt);
      _display.print(_text.c_str());
    }
    else
    {
      if (!_is_multiline)
      {
        String sub_str;
        uint16_t sub_str_pix_num;
        uint16_t txt_x_pos;

        sub_str_pix_num = getFitStr(sub_str, _first_draw_char_pos);
        txt_x_pos = calcXStrOffset(sub_str_pix_num);

        if (_has_autoscroll || (_has_autoscroll_in_focus && _has_focus))
        {
          if ((millis() - _last_autoscroll_ts) > _autoscroll_update_delay)
          {
            if (_has_full_autoscroll)
            {
              if (_first_draw_char_pos == _text_len - 1 || sub_str_pix_num == 0)
                _first_draw_char_pos = 0;
              else
                ++_first_draw_char_pos;
            }
            else
            {
              if (!_is_reverse_autoscroll)
              {
                if (sub_str_pix_num + _h_padding > (_width * 3) >> 2)  // 3/4 від ширини
                  ++_first_draw_char_pos;
                else
                  _is_reverse_autoscroll = true;
              }
              else
              {
                if (_first_draw_char_pos > 0)
                  --_first_draw_char_pos;
                else
                  _is_reverse_autoscroll = false;
              }
            }

            _last_autoscroll_ts = millis();
          }
        }

        _display.setCursor(_x_pos + x_offset + txt_x_pos, _y_pos + y_offset + txtYPos + _char_hgt);
        _display.print(sub_str.c_str());
      }
      else
      {
        _first_draw_char_pos = 0;
        _has_autoscroll = false;
        _has_autoscroll_in_focus = false;

        _display.setTextWrap(true);
        _display.setCursor(0, _y_pos + y_offset + _char_hgt);
        _display.print(_text.c_str());
        _display.setTextWrap(false);
      }
    }
  }
}  // namespace pixeler