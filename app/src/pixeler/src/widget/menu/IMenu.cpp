#pragma GCC optimize("O3")
#include "IMenu.h"

namespace pixeler
{
  Menu::Menu(uint16_t widget_ID, TypeID type_ID) : IWidgetContainer(widget_ID, type_ID) {}

  void Menu::delWidgets()
  {
    IWidgetContainer::delWidgets();
    _first_item_index = 0;
    _cur_focus_pos = 0;
  }

  bool Menu::delWidgetByID(uint16_t widget_ID)
  {
    if (IWidgetContainer::delWidgetByID(widget_ID))
      return false;

    if (_widgets.empty())
    {
      _first_item_index = 0;
      _cur_focus_pos = 0;
    }
    else
    {
      if (_first_item_index >= _widgets.size())
      {
        if (getItemsNumOnScreen() <= _widgets.size())
          _first_item_index = 0;
        else
          _first_item_index = _widgets.size() - getItemsNumOnScreen();
      }

      if (_cur_focus_pos >= _widgets.size())
      {
        _cur_focus_pos = _widgets.size() - 1;
        _widgets[_cur_focus_pos]->setFocus();
      }
    }

    return true;
  }

  void Menu::setItemHeight(uint16_t height)
  {
    _item_height = height > 0 ? height : 1;
    _is_changed = true;
  }

  uint16_t Menu::getItemHeight() const
  {
    return _orientation == VERTICAL ? _item_height : _height - 4;
  }

  void Menu::setItemWidth(uint16_t width)
  {
    _item_width = width > 0 ? width : 1;
    _is_changed = true;
  }

  uint16_t Menu::getItemWidth() const
  {
    return _orientation == HORIZONTAL ? _item_width : _width - 4;
  }

  void Menu::setOrientation(const Orientation orientation)
  {
    _orientation = orientation;
    _is_changed = true;
  }

  IWidget::Orientation Menu::getOrientation() const
  {
    return _orientation;
  }

  uint16_t Menu::getCurrItemID() const
  {
    if (_widgets.empty())
      return 0;

    return _widgets[_cur_focus_pos]->getID();
  }

  uint16_t Menu::getCurrFocusPos() const
  {
    return _cur_focus_pos;
  }

  String Menu::getCurrItemText() const
  {
    if (_widgets.empty())
      return emptyString;

    const MenuItem* item = static_cast<MenuItem*>(_widgets[_cur_focus_pos]);
    return item->getText();
  }

  IWidget* Menu::getCurrItem()
  {
    if (_widgets.empty())
      return nullptr;

    return _widgets[_cur_focus_pos];
  }

  void Menu::setItemsSpacing(uint16_t items_spacing)
  {
    _items_spacing = items_spacing;
  }

  uint16_t Menu::getItemsSpacing() const
  {
    return _items_spacing;
  }

  void Menu::addItem(MenuItem* item_ptr)
  {
    addWidget(item_ptr);
  }

  uint16_t Menu::getItemsNumOnScreen() const
  {
    return (float)_height / _item_height;
  }

  void Menu::onDraw()
  {
    if (!_is_changed)
    {
      if (_visibility != INVISIBLE && _is_enabled)
      {
        uint16_t cycles_count = getCyclesCount();
        for (uint16_t i{_first_item_index}; i < _first_item_index + cycles_count; ++i)
          _widgets[i]->onDraw();
      }

      return;
    }

    _is_changed = false;

    if (_visibility == INVISIBLE)
    {
      hide();
      return;
    }

    if (_widgets.empty())
    {
      clear();
      return;
    }

    uint16_t cyclesCount = getCyclesCount();

    if (_first_item_index >= _widgets.size())
      _first_item_index = _widgets.size() - 1;

    if (_cur_focus_pos >= _widgets.size())
      _cur_focus_pos = _widgets.size() - 1;

    IWidget* item = _widgets[_cur_focus_pos];
    item->setFocus();

    drawItems(_first_item_index, cyclesCount);
  }

  void Menu::drawItems(uint16_t start, uint16_t count)
  {
    clear();

    if (_widgets.empty())
      return;

    uint16_t itemXPos = 2;
    uint16_t itemYPos = 2;

    for (uint16_t i{start}; i < start + count; ++i)
    {
      _widgets[i]->setPos(itemXPos, itemYPos);

      if (_orientation == VERTICAL)
      {
        _widgets[i]->setHeight(_item_height);
        _widgets[i]->setWidth(_width - 4);
        itemYPos += (_item_height + _items_spacing);
      }
      else
      {
        _widgets[i]->setHeight(_height - 4);
        _widgets[i]->setWidth(_item_width);
        itemXPos += (_item_width + _items_spacing);
      }

      _widgets[i]->onDraw();
    }
  }

  uint16_t Menu::getCyclesCount() const
  {
    uint16_t cyclesCount;

    if (_orientation == VERTICAL)
      cyclesCount = (float)_height / (_item_height + _items_spacing);
    else
      cyclesCount = (float)_width / (_item_width + _items_spacing);

    uint16_t itemsToEndNum = _widgets.size() - _first_item_index;

    if (cyclesCount > itemsToEndNum)
      cyclesCount = itemsToEndNum;

    return cyclesCount;
  }
}  // namespace pixeler
