#pragma once
//
#include "pixeler/game/DataStream.h"
//
#include "scene/RpgBaseScene.h"

#include "pixeler/ui/context/IContext.h"
#include "pixeler/ui/widget/menu/FixedMenu.h"

using namespace pixeler;

namespace rpg
{
  class RpgContext : public IContext
  {
  public:
    RpgContext();
    virtual ~RpgContext();

  protected:
    virtual bool loop() override;
    virtual void update() override;

  private:
    enum Widget_ID : uint8_t
    {
      ID_MENU_GAME = 1,
      ID_ITEM_LOAD_GAME,
      ID_ITEM_NEW_GAME,
      ID_ITEM_HELP,
    };

    enum Mode : uint8_t
    {
      MODE_MAIN_MENU = 0,
      MODE_HELP,
      MODE_GAME,
    };

    void showGameMenuTmpl();
    void showHelpTmpl();

    DataStream _stored_objs{1024};

    FixedMenu* _game_menu{nullptr};
    RpgBaseScene* _scene{nullptr};

    Mode _mode{MODE_MAIN_MENU};
  };

}  // namespace rpg