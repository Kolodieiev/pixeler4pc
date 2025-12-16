#pragma once

#include "pixeler/game/object/IGameObject.h"

using namespace pixeler;

namespace sokoban
{
  class BoxObj : public IGameObject
  {
  public:
    BoxObj(WavManager& audio,
           TerrainManager& terrain,
           std::unordered_map<uint32_t, IGameObject*>& game_objs) : IGameObject(audio, terrain, game_objs) {}
    virtual ~BoxObj() {}
    virtual void init() override;
    virtual void update() override;
    virtual void serialize(DataStream& ds) override;
    virtual void deserialize(DataStream& ds) override;
    virtual size_t getDataSize() const override;

    // Якщо можливо, переміститися в задані координати. Повертає true в разі успіху
    bool moveTo(uint16_t x, uint16_t y);

    inline bool isOk() const
    {
      return _is_ok;
    }

  private:
    bool _is_ok{false};  // Вказує, чи встановлено ящик в ключову точку
  };
}  // namespace sokoban