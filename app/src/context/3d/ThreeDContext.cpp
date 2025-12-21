#include "ThreeDContext.h"

#include "../WidgetCreator.h"
#include "pixeler/manager/SettingsManager.h"
#include "pixeler/util/img/BmpUtil.h"

ThreeDContext::ThreeDContext()
{
  WidgetCreator creator;
  EmptyLayout* layout = creator.getEmptyLayout();
  setLayout(layout);

  _game.init();
  _player.setMap(&_game);
  _game.setPlayer(&_player);
}

ThreeDContext::~ThreeDContext()
{
}

bool ThreeDContext::loop()
{
  return true;
}

void ThreeDContext::update()
{
  if (_input.isPressed(BtnID::BTN_BACK))
  {
    _input.lock(BtnID::BTN_BACK, PRESS_LOCK);
    openContextByID(ID_CONTEXT_MENU);
  }

  if (_input.isHolded(BtnID::BTN_UP))
  {
    _player.moveForward();
  }
  if (_input.isHolded(BtnID::BTN_DOWN))
  {
    _player.moveBack();
  }
  if (_input.isHolded(BtnID::BTN_LEFT))
  {
    _player.turnLeft();
  }
  if (_input.isHolded(BtnID::BTN_RIGHT))
  {
    _player.turnRight();
  }

  _game.update();
}