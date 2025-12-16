#pragma once
//
#include "pixeler/game/DataStream.h"
//
#include "pixeler/game/IGameScene.h"
#include "pixeler/ui/context/IContext.h"
#include "pixeler/ui/widget/menu/FixedMenu.h"
#include "pixeler/ui/widget/scrollbar/ScrollBar.h"

using namespace pixeler;

namespace sokoban
{
  class SokobanContext : public IContext
  {
  public:
    SokobanContext();
    virtual ~SokobanContext();

  protected:
    virtual bool loop() override;
    virtual void update() override;

  private:
    enum Widget_ID : uint8_t
    {
      ID_LVL_LIST = 1,
      ID_SCROLL
    };

    void showLvlMenu();

  private:
    DataStream _stored_objs{1024};

    FixedMenu* _lvl_menu{nullptr};
    ScrollBar* _scrollbar{nullptr};
    IGameScene* _scene{nullptr};

    uint8_t _prev_lvl{1};

    bool _is_game_started{false};
  };
}  // namespace sokoban