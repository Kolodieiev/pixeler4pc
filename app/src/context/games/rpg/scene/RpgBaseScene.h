#pragma once

#include "../obj/hero/HeroObj.h"
#include "pixeler/game/IGameScene.h"

using namespace pixeler;

namespace rpg
{
  class RpgBaseScene : public IGameScene
  {
  public:
    explicit RpgBaseScene(DataStream& stored_objs, bool is_loaded = false, uint8_t lvl = 1);
    virtual ~RpgBaseScene();
    virtual void update() override;
    virtual void onTrigger(uint8_t id) override;

  private:
    bool loadMapRes();
    void createHeroObj();

  private:
    TerrainLoader _terrain_loader;
    String _err_message;
    HeroObj* _hero_obj{nullptr};
    bool _has_error{false};
    uint8_t _level{0};
  };

}  // namespace rpg
