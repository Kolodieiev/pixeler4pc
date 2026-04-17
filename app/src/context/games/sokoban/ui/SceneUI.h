#pragma once

#include "pixeler/src/game/IGameObject.h"
#include "pixeler/src/game/ui/IGameUI.h"

using namespace pixeler;

namespace sokoban
{
  class SceneUI : public IGameUI
  {
  public:
    SceneUI() {}
    virtual ~SceneUI() {}
    virtual void onDraw() override;

  private:
  };
}  // namespace sokoban
