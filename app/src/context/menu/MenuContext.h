#pragma once

#include "pixeler/ui/context/IContext.h"
#include "pixeler/ui/widget/menu/FixedMenu.h"
#include "pixeler/ui/widget/scrollbar/ScrollBar.h"

using namespace pixeler;

class MenuContext : public IContext
{
public:
  MenuContext();
  virtual ~MenuContext();

protected:
  virtual bool loop() override;
  virtual void update() override;

private:
  enum Widget_ID : uint8_t
  {
    ID_MENU = 1,
    ID_SCROLLBAR,
  };

  std::vector<IWidget*> _bin;

  FixedMenu* _menu;
  ScrollBar* _scrollbar;

  static uint8_t _last_sel_item_pos;

  void up();
  void down();
  void ok();
};
