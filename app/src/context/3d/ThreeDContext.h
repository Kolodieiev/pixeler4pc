#pragma once

#include "GameWorld.h"
#include "Player.h"
#include "pixeler/ui/context/IContext.h"
#include "pixeler/ui/widget/text/Label.h"

using namespace pixeler;

class ThreeDContext : public IContext
{
public:
  ThreeDContext();
  virtual ~ThreeDContext();

protected:
  virtual bool loop() override;
  virtual void update() override;
  //
private:
  GameWorld _game;
  Player _player;
};
