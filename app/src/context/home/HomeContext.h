#pragma once

#include "pixeler/src/context/IContext.h"
#include "pixeler/src/manager/res/BmpLoader.h"
#include "pixeler/src/widget/text/Label.h"

using namespace pixeler;

class HomeContext : public IContext
{
public:
  HomeContext();
  virtual ~HomeContext();

protected:
  virtual bool loop() override;
  virtual void update() override;
  //
private:
  enum Widget_ID : uint8_t
  {
    ID_WALLPAPER = 1,
    ID_BAT_LVL,
    ID_TIME_LBL,
    ID_DATE_LBL,
    ID_DAY_LBL,
    ID_MESSAGE_LBL
  };

  void updateBattVoltage();

private:
  Image* _batt_ico;
  Label* _batt_volt_lbl{nullptr};
  ImageResource* _wall_res{nullptr};
  unsigned long _upd_timer{0};
};
