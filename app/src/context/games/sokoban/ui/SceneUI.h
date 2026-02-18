#pragma once

#include "pixeler/src/game/IGameUI.h"
#include "pixeler/src/game/object/IGameObject.h"

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