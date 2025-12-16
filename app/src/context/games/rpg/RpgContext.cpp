#include "RpgContext.h"

#include "../../WidgetCreator.h"
#include "./SceneID.h"

#define COLOR_MAIN_RPG_BACK 0x30A0
#define COLOR_ITEM_RPG_BACK 0x0885
#define COLOR_ITEM_RPG_BORDER 0x73A6

#define GAME_MENU_H_PADDINGS 30
#define GAME_MENU_V_PADDINGS 5
#define GAME_MENU_ITEM_SPACING 3
#define GAME_MENU_ITEM_ADD_HEIGHT 20

namespace rpg
{
  const char STR_LOAD_GAME[] = "Завантажити";
  const char STR_NEW_GAME[] = "Нова гра";
  const char STR_HELP[] = "Допомога";

  RpgContext::~RpgContext()
  {
  }

  RpgContext::RpgContext()
  {
    showGameMenuTmpl();
  }

  void RpgContext::showGameMenuTmpl()
  {
    _mode = MODE_MAIN_MENU;

    WidgetCreator creator;
    EmptyLayout* layout = creator.getEmptyLayout();
    setLayout(layout);

    layout->setBackColor(COLOR_MAIN_RPG_BACK);

    _game_menu = new FixedMenu(ID_MENU_GAME);
    layout->addWidget(_game_menu);
    _game_menu->setBackColor(COLOR_MAIN_RPG_BACK);
    _game_menu->setWidth(TFT_WIDTH - GAME_MENU_H_PADDINGS * 2);
    _game_menu->setHeight(TFT_HEIGHT / 2 - GAME_MENU_V_PADDINGS * 2);
    _game_menu->setPos(GAME_MENU_H_PADDINGS, TFT_HEIGHT / 2 + GAME_MENU_V_PADDINGS);
    _game_menu->setItemsSpacing(GAME_MENU_ITEM_SPACING);
    _game_menu->setLoopState(true);

    //

    // TODO перевірити збереження, якщо наявне додати ITEM_LOAD

    MenuItem* item_load_game = creator.getMenuItem(ID_ITEM_LOAD_GAME);
    _game_menu->addItem(item_load_game);
    item_load_game->setBackColor(COLOR_ITEM_RPG_BACK);
    item_load_game->setBorderColor(COLOR_ITEM_RPG_BORDER);
    item_load_game->setBorder(true);
    item_load_game->setCornerRadius(4);
    item_load_game->setChangingBorder(false);
    item_load_game->setChangingBack(true);

    Label* lbl_load_game = creator.getItemLabel(STR_LOAD_GAME, font_10x20);
    item_load_game->setLbl(lbl_load_game);
    lbl_load_game->setGravity(IWidget::GRAVITY_CENTER);
    lbl_load_game->setAlign(IWidget::ALIGN_CENTER);

    //

    _game_menu->setItemHeight(lbl_load_game->getHeight() + GAME_MENU_ITEM_ADD_HEIGHT);

    //

    MenuItem* item_new_game = item_load_game->clone(ID_ITEM_NEW_GAME);
    _game_menu->addItem(item_new_game);

    Label* lbl_new_game = lbl_load_game->clone(1);
    item_new_game->setLbl(lbl_new_game);
    lbl_new_game->setText(STR_NEW_GAME);

    //

    MenuItem* item_help = item_load_game->clone(ID_ITEM_HELP);
    _game_menu->addItem(item_help);

    Label* lbl_help = lbl_load_game->clone(1);
    item_help->setLbl(lbl_help);
    lbl_help->setText(STR_HELP);
  }

  void RpgContext::showHelpTmpl()
  {
    _mode = MODE_HELP;

    WidgetCreator creator;
    EmptyLayout* layout = creator.getEmptyLayout();
    setLayout(layout);
  }

  bool RpgContext::loop()
  {
    return true;
  }

  void RpgContext::update()
  {
    if (_mode == MODE_GAME)
    {
      if (!_scene->isFinished())
      {
        if (!_scene->isReleased())
        {
          _scene->update();
        }
        else
        {
          uint8_t next_lvl = _scene->getNextSceneID();
          delete _scene;
          _scene = new RpgBaseScene(_stored_objs, false, next_lvl);
        }
      }
      else
      {
        delete _scene;
        _scene = nullptr;
        showGameMenuTmpl();
      }
    }
    else
    {
      if (_input.isReleased(BtnID::BTN_BACK))
      {
        _input.lock(BtnID::BTN_BACK, CLICK_LOCK);
        openContextByID(ID_CONTEXT_GAMES);
      }
      else if (_input.isHolded(BtnID::BTN_UP))
      {
        _input.lock(BtnID::BTN_UP, HOLD_LOCK);
        _game_menu->focusUp();
      }
      else if (_input.isHolded(BtnID::BTN_DOWN))
      {
        _input.lock(BtnID::BTN_DOWN, HOLD_LOCK);
        _game_menu->focusDown();
      }
      else if (_input.isReleased(BtnID::BTN_OK))
      {
        _input.lock(BtnID::BTN_OK, CLICK_LOCK);

        uint8_t item_id = _game_menu->getCurrItemID();

        if (item_id == ID_ITEM_LOAD_GAME)
        {
        }
        else if (item_id == ID_ITEM_NEW_GAME)
        {
          _mode = MODE_GAME;
          getLayout()->delWidgets();
          _scene = new RpgBaseScene(_stored_objs, false);
        }
        else if (item_id == ID_ITEM_HELP)
        {
        }
      }
    }
  }

}  // namespace rpg