#include "MenuContext.h"
//
#include "../../pixeler/ui/widget/layout/EmptyLayout.h"
#include "../../pixeler/ui/widget/menu/item/MenuItem.h"
#include "../WidgetCreator.h"
#include "./res/book.h"
#include "./res/chip.h"
#include "./res/headphones.h"
#include "./res/joystick.h"
#include "./res/sd.h"
#include "./res/settings.h"
#include "./res/wifi_ico.h"

#define ICO_WH 35

const char STR_3D_ITEM[] = "3D";
const char STR_READER_ITEM[] = "Читалка";
const char STR_FILES_ITEM[] = "Файли";
const char STR_GAME_ITEM[] = "Ігри";
const char STR_WIFI_ITEM[] = "Підключення";
const char STR_FIRMWARE_ITEM[] = "Прошивка";

uint8_t MenuContext::_last_sel_item_pos;

MenuContext::MenuContext()
{
  WidgetCreator creator;
  //
  EmptyLayout* layout = creator.getEmptyLayout();
  setLayout(layout);
  //
  _menu = new FixedMenu(ID_MENU);
  layout->addWidget(_menu);
  _menu->setBackColor(COLOR_MAIN_BACK);
  _menu->setWidth(TFT_WIDTH - SCROLLBAR_WIDTH - DISPLAY_PADDING * 2);
  _menu->setHeight(TFT_HEIGHT);
  _menu->setItemHeight(TFT_HEIGHT / 4 - 2);
  _menu->setPos(DISPLAY_PADDING, 0);
  //
  _scrollbar = new ScrollBar(ID_SCROLLBAR);
  layout->addWidget(_scrollbar);
  _scrollbar->setWidth(SCROLLBAR_WIDTH);
  _scrollbar->setHeight(_menu->getHeight());
  _scrollbar->setPos(_menu->getWidth() + _menu->getXPos(), _menu->getYPos());
  _scrollbar->setBackColor(COLOR_MAIN_BACK);

  // Файли
  MenuItem* files_item = creator.getMenuItem(ID_CONTEXT_FILES);
  _menu->addItem(files_item);

  Image* files_img = new Image(1);
  files_item->setImg(files_img);
  files_img->setTransparency(true);
  files_img->setWidth(ICO_WH);
  files_img->setHeight(ICO_WH);
  files_img->setSrc(SD_IMG);

  Label* files_lbl = creator.getItemLabel(STR_FILES_ITEM, font_10x20);
  files_item->setLbl(files_lbl);

  // Музика
  MenuItem* seudo3d_item = creator.getMenuItem(ID_CONTEXT_MP3);
  _menu->addItem(seudo3d_item);

  Image* mp3_img = new Image(1);
  seudo3d_item->setImg(mp3_img);
  mp3_img->setTransparency(true);
  mp3_img->setWidth(ICO_WH);
  mp3_img->setHeight(ICO_WH);
  mp3_img->setSrc(HEADPHONES_IMG);

  Label* pseudo3d_lbl = creator.getItemLabel(STR_3D_ITEM, font_10x20);
  seudo3d_item->setLbl(pseudo3d_lbl);

  // Ігри
  MenuItem* game_item = creator.getMenuItem(ID_CONTEXT_GAMES);
  _menu->addItem(game_item);

  Image* game_img = new Image(1);
  game_item->setImg(game_img);
  game_img->setTransparency(true);
  game_img->setWidth(ICO_WH);
  game_img->setHeight(ICO_WH);
  game_img->setSrc(JOYSTICK_IMG);

  Label* game_lbl = creator.getItemLabel(STR_GAME_ITEM, font_10x20);
  game_item->setLbl(game_lbl);

  // Читалка
  MenuItem* read_item = creator.getMenuItem(ID_CONTEXT_READER);
  _menu->addItem(read_item);

  Image* read_img = new Image(1);
  read_item->setImg(read_img);
  read_img->setTransparency(true);
  read_img->setWidth(ICO_WH);
  read_img->setHeight(ICO_WH);
  read_img->setSrc(BOOK_IMG);

  Label* read_lbl = creator.getItemLabel(STR_READER_ITEM, font_10x20);
  read_item->setLbl(read_lbl);

  // WiFi
  MenuItem* wifi_item = creator.getMenuItem(ID_CONTEXT_WIFI);
  _menu->addItem(wifi_item);

  Image* wifi_img = new Image(1);
  wifi_item->setImg(wifi_img);
  wifi_img->setTransparency(true);
  wifi_img->setWidth(ICO_WH);
  wifi_img->setHeight(ICO_WH);
  wifi_img->setSrc(WIFI_IMG);

  Label* wifi_lbl = creator.getItemLabel(STR_WIFI_ITEM, font_10x20);
  wifi_item->setLbl(wifi_lbl);

  // Налаштування
  MenuItem* pref_item = creator.getMenuItem(ID_CONTEXT_PREF_SEL);
  _menu->addItem(pref_item);

  Image* pref_img = new Image(1);
  pref_item->setImg(pref_img);
  pref_img->setTransparency(true);
  pref_img->setWidth(ICO_WH);
  pref_img->setHeight(ICO_WH);
  pref_img->setSrc(SETTINGS_IMG);

  Label* pref_lbl = creator.getItemLabel(STR_PREFERENCES, font_10x20);
  pref_item->setLbl(pref_lbl);

  // Прошивка
  MenuItem* firm_item = creator.getMenuItem(ID_CONTEXT_FIRMWARE);
  _menu->addItem(firm_item);

  Image* firm_img = new Image(1);
  firm_item->setImg(firm_img);
  firm_img->setTransparency(true);
  firm_img->setWidth(ICO_WH);
  firm_img->setHeight(ICO_WH);
  firm_img->setSrc(CHIP_IMG);

  Label* firm_lbl = creator.getItemLabel(STR_FIRMWARE_ITEM, font_10x20);
  firm_item->setLbl(firm_lbl);
  //
  // Зображення не видаляються віджетами автоматично задля можливості превикористання.
  // Тому нам потрібно додати механізм для видалення зображень після виходу з контексту.
  _bin.reserve(_menu->getSize());
  _bin.push_back(mp3_img);
  _bin.push_back(read_img);
  _bin.push_back(game_img);
  _bin.push_back(files_img);
  _bin.push_back(pref_img);
  _bin.push_back(firm_img);

  _scrollbar->setMax(_menu->getSize());

  _menu->setCurrFocusPos(_last_sel_item_pos);
  _scrollbar->setValue(_last_sel_item_pos);
}

MenuContext::~MenuContext()
{
  for (auto b_it = _bin.begin(), e_it = _bin.end(); b_it != e_it; ++b_it)
    delete *b_it;
}

bool MenuContext::loop()
{
  return true;
}

void MenuContext::update()
{
  if (_input.isHolded(BtnID::BTN_UP))
  {
    _input.lock(BtnID::BTN_UP, HOLD_LOCK);
    up();
  }
  else if (_input.isHolded(BtnID::BTN_DOWN))
  {
    _input.lock(BtnID::BTN_DOWN, HOLD_LOCK);
    down();
  }
  else if (_input.isReleased(BtnID::BTN_OK))
  {
    _input.lock(BtnID::BTN_OK, CLICK_LOCK);
    ok();
  }
  else if (_input.isReleased(BtnID::BTN_BACK))
  {
    _input.lock(BtnID::BTN_BACK, CLICK_LOCK);
    _last_sel_item_pos = 0;
    openContextByID(ID_CONTEXT_HOME);
  }
}

void MenuContext::up()
{
  _menu->focusUp();
  _scrollbar->scrollUp();
}

void MenuContext::down()
{
  _menu->focusDown();
  _scrollbar->scrollDown();
}

void MenuContext::ok()
{
  ContextID id = static_cast<ContextID>(_menu->getCurrItemID());
  _last_sel_item_pos = _menu->getCurrFocusPos();

  if (id == ID_CONTEXT_FILES || id == ID_CONTEXT_GAMES)
  {
    openContextByID(id);
  }
  else
  {
    log_i("Не реалізовано");
  }
}
