/*
 * Генератор віджетів.
 * Використовується тут для часткового очищення файлів вікон від однотипного коду.
 */

#pragma once

#include "../pixeler/driver/graphics/DisplayWrapper.h"
#include "../pixeler/ui/widget/image/Image.h"
#include "../pixeler/ui/widget/keyboard/Keyboard.h"
#include "../pixeler/ui/widget/layout/EmptyLayout.h"
#include "../pixeler/ui/widget/menu/DynamicMenu.h"
#include "../pixeler/ui/widget/menu/item/MenuItem.h"
#include "../pixeler/ui/widget/text/Label.h"
#include "resources/colors.h"
#include "resources/kb_btn_id.h"
#include "resources/strings.h"
#include "resources/ui_const.h"
#include "resources/ch32_pins_def.h"


using namespace pixeler;

class WidgetCreator
{
public:
  EmptyLayout* getEmptyLayout();
  Label* getItemLabel(const char* text, const uint8_t* font_ptr = font_unifont, uint8_t text_size = 1);
  MenuItem* getMenuItem(uint16_t id = 1);
  DynamicMenu* getDynamicMenu(uint16_t id);
  Label* getStatusMsgLable(uint16_t id, const char* text, uint8_t text_size = 1);
  Keyboard* getStandardEnKeyboard(uint16_t id);
  Keyboard* getCapsdEnKeyboard(uint16_t id);
};
