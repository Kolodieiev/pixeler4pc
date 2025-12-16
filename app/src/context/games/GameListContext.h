#pragma once
#include <stdint.h>

#include "pixeler/ui/context/IContext.h"
#include "pixeler/ui/widget/menu/FixedMenu.h"
#include "pixeler/ui/widget/scrollbar/ScrollBar.h"

using namespace pixeler;

class GameListContext : public IContext
{
public:
  GameListContext();
  virtual ~GameListContext();

protected:
  virtual bool loop() override;
  virtual void update() override;

private:
  enum Widget_ID : uint8_t
  {
    ID_MENU = 1,
    ID_SCROLLBAR,
  };

  void up();
  void down();

private:
  std::vector<IWidget*> _bin;
  FixedMenu* _menu;
  ScrollBar* _scrollbar;
};
