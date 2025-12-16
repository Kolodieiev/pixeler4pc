#include "GameListContext.h"

#include "../WidgetCreator.h"
#include "./icons/sokoban_ico.h"
#include "pixeler/ui/widget/layout/EmptyLayout.h"
#include "pixeler/ui/widget/menu/item/MenuItem.h"

const char STR_SOKOBAN_ITEM[] = "Комірник";
const char STR_RPG_ITEM[] = "RPG";

GameListContext::GameListContext()
{
  //------ Налаштування зовнішнього вигляду віджетів

  WidgetCreator creator;
  //
  EmptyLayout* layout = creator.getEmptyLayout();
  setLayout(layout);
  //
  _menu = new FixedMenu(ID_MENU);
  layout->addWidget(_menu);
  _menu->setBackColor(COLOR_MENU_ITEM);
  _menu->setWidth(TFT_WIDTH - SCROLLBAR_WIDTH - 2);
  _menu->setHeight(TFT_HEIGHT - 2);
  _menu->setItemHeight((_menu->getHeight() - 2) / 4);
  //
  _scrollbar = new ScrollBar(ID_SCROLLBAR);
  layout->addWidget(_scrollbar);
  _scrollbar->setWidth(SCROLLBAR_WIDTH);
  _scrollbar->setHeight(TFT_HEIGHT);
  _scrollbar->setPos(TFT_WIDTH - SCROLLBAR_WIDTH, 0);
  
  //

  MenuItem* sokoban_item = creator.getMenuItem(ID_CONTEXT_SOKOBAN);
  _menu->addItem(sokoban_item);

  Image* soko_img = new Image(1);
  sokoban_item->setImg(soko_img);
  soko_img->setWidth(32);
  soko_img->setHeight(32);
  soko_img->setCornerRadius(5);
  soko_img->setTransparency(true);
  soko_img->setSrc(ICO_SOKOBAN);

  Label* soko_lbl = creator.getItemLabel(STR_SOKOBAN_ITEM, font_10x20);
  sokoban_item->setLbl(soko_lbl);
  soko_lbl->setAutoscrollInFocus(true);

  //

  MenuItem* rpg_item = creator.getMenuItem(ID_CONTEXT_RPG);
  _menu->addItem(rpg_item);

  Label* rpg_lbl = creator.getItemLabel(STR_RPG_ITEM, font_10x20);
  rpg_item->setLbl(rpg_lbl);
  rpg_lbl->setAutoscrollInFocus(true);

  //

  _bin.reserve(_menu->getSize());
  _bin.push_back(soko_img);

  _scrollbar->setMax(_menu->getSize());
}

GameListContext::~GameListContext()
{
  for (auto b_it = _bin.begin(), e_it = _bin.end(); b_it != e_it; ++b_it)
    delete *b_it;
}

bool GameListContext::loop()
{
  return true;
}

void GameListContext::update()
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
  else if (_input.isReleased(BtnID::BTN_BACK))
  {
    _input.lock(BtnID::BTN_BACK, CLICK_LOCK);
    openContextByID(ID_CONTEXT_MENU);
  }
  else if (_input.isReleased(BtnID::BTN_OK))
  {
    _input.lock(BtnID::BTN_OK, CLICK_LOCK);
    openContextByID((ContextID)_menu->getCurrItemID());
  }
}

void GameListContext::up()
{
  _menu->focusUp();
  _scrollbar->scrollUp();
}

void GameListContext::down()
{
  _menu->focusDown();
  _scrollbar->scrollDown();
}
