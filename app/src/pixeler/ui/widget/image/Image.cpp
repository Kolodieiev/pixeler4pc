#pragma GCC optimize("O3")
#include "Image.h"

namespace pixeler
{
  Image::Image(uint16_t widget_ID) : IWidget(widget_ID, TYPE_ID_IMAGE) {}

  void Image::setTransparency(bool state)
  {
    _has_transparency = state;
    _is_changed = true;
  }

  Image* Image::clone(uint16_t id) const
  {
    try
    {
      Image* cln = new Image(id);
      cln->_has_border = _has_border;
      cln->_x_pos = _x_pos;
      cln->_y_pos = _y_pos;
      cln->_back_color = _back_color;
      cln->_border_color = _border_color;
      cln->_corner_radius = _corner_radius;
      cln->_is_transparent = _is_transparent;
      cln->_has_transparency = _has_transparency;
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

      cln->_width = _width;
      cln->_height = _height;
      cln->_img_ptr = _img_ptr;

      return cln;
    }
    catch (const std::bad_alloc& e)
    {
      log_e("%s", e.what());
      esp_restart();
    }
  }

  void Image::setSrc(const uint16_t* image_ptr)
  {
    _is_changed = true;
    _img_ptr = image_ptr;
  }

  Image::~Image()
  {
  }

  void Image::onDraw()
  {
    if (!_is_changed)
      return;

    _is_changed = false;

    if (_visibility == INVISIBLE)
    {
      hide();
      return;
    }

    uint16_t x_offset{0};
    uint16_t y_offset{0};

    if (_parent)
    {
      x_offset = _parent->getXPos();
      y_offset = _parent->getYPos();
    }

    if (_img_ptr)
    {
      if (!_has_transparency)
        _display.drawBitmap(_x_pos + x_offset, _y_pos + y_offset, _img_ptr, _width, _height);
      else
        _display.drawBitmapTransp(_x_pos + x_offset, _y_pos + y_offset, _img_ptr, _width, _height);
    }
    else
    {
      log_e("Не встановлено src");
      esp_restart();
    }
  }
}  // namespace pixeler