#include "HeroObj.h"

#include "../ClassID.h"

const char STR_HERO_BMP_PATH[] = "games/rpg/hero.bmp";

namespace rpg
{

  HeroObj::HeroObj(WavManager& audio, TerrainManager& terrain, std::unordered_map<uint32_t, IGameObject*>& game_objs)
      : IGameObject(audio, terrain, game_objs)
  {
  }

  HeroObj::~HeroObj()
  {
    _res.deleteBmpRes(_bmp_id);
  }

  void HeroObj::init()
  {
    _type_ID = ClassID::CLASS_HERO;
    _layer = 1;

    _bmp_id = _res.loadBmpRes(STR_HERO_BMP_PATH);

    if (_bmp_id == 0)
    {
      _sprite.has_img = false;
    }
    else
    {
      _sprite.has_img = true;
      _sprite.img_ptr = _res.getBmpRes(_bmp_id).data_ptr;
      _sprite.width = 20;
      _sprite.height = 30;
      _sprite.rigid_offsets.top = 25;
      _sprite.rigid_offsets.left = 9;
      _sprite.rigid_offsets.right = 9;
    }

    // _sprite.pass_abillity_mask = TILE_TYPE_ALL;
    _sprite.pass_abillity_mask = TILE_TYPE_GROUND;
  }

  void HeroObj::update()
  {
  }

  void HeroObj::serialize(DataStream& ds)
  {
  }

  void HeroObj::deserialize(DataStream& ds)
  {
  }

  size_t HeroObj::getDataSize() const
  {
    return 0;
  }

  void HeroObj::onDraw()
  {
    IGameObject::onDraw();
  }

  void HeroObj::move(MovingDirection direction)
  {
    if (direction == DIRECTION_NONE)
      return;

    if (direction == DIRECTION_UP)
      stepTo(_x_global, _y_global - PIX_PER_STEP);
    else if (direction == DIRECTION_DOWN)
      stepTo(_x_global, _y_global + PIX_PER_STEP);
    else if (direction == DIRECTION_LEFT)
      stepTo(_x_global - PIX_PER_STEP, _y_global);
    else if (direction == DIRECTION_RIGHT)
      stepTo(_x_global + PIX_PER_STEP, _y_global);
  }

  void HeroObj::stepTo(uint16_t x, uint16_t y)
  {
    // std::list<IGameObject*> objs = getObjAt(x, y);  // Вибрати всі об'єкти на плитці куди повинен здійснюватися наступний крок

    if (_terrain.canPass(_x_global, _y_global, x, y, _sprite))
    {
      _x_global = x;
      _y_global = y;
    }
  }

};  // namespace rpg