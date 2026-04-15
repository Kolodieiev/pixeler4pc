#pragma once

#include "pixeler/src/game/object/IGameObject.h"

using namespace pixeler;

namespace sokoban
{
  class BoxPointObj : public IGameObject
  {
  public:
    BoxPointObj(uint32_t id, IGameScene& game_scene, WavManager& audio);
    virtual ~BoxPointObj() {}

    virtual void __update() override;
    virtual void serialize(DataStream& ds) override;
    virtual void deserialize(DataStream& ds) override;
    virtual size_t getDataSize() const override;
  };
}  // namespace sokoban
