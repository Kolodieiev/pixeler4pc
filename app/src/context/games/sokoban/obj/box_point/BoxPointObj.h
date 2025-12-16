#pragma once

#include "pixeler/game/object/IGameObject.h"

using namespace pixeler;

namespace sokoban
{
  class BoxPointObj : public IGameObject
  {
  public:
    BoxPointObj(WavManager& audio,
                TerrainManager& terrain,
                std::unordered_map<uint32_t, IGameObject*>& game_objs) : IGameObject(audio, terrain, game_objs) {}
    virtual ~BoxPointObj() {}

    virtual void init() override;
    virtual void update() override;
    virtual void serialize(DataStream& ds) override;
    virtual void deserialize(DataStream& ds) override;
    virtual size_t getDataSize() const override;
  };
}  // namespace sokoban