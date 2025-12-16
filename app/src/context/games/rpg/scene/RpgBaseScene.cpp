#include "RpgBaseScene.h"

#include "../ResID.h"
#include "../SceneID.h"
#include "../ui/SceneUI.h"

namespace rpg
{
  const char STR_TERRAIN_TMPL_FOLDER[] = "games/rpg/terrain_tmpl/";
  const char STR_TERRAIN_TILES_RES_FOLDER[] = "games/rpg/tiles/terrain/";

  // TODO Додати обробку рівня RpgContext.
  // TODO Додати власний клас сцени для конкретного рівня.
  // TODO Додати серіалізацію герою.
  // TODO Додати збереження сцени.
  // TODO Додати завантаження збереженої сцени з головного меню.
  // TODO Додати тригер переходу на іншу сцену.
  // TODO Додати відновлення даних героя на сцені.
  // TODO Додати відновлення сцени зі збереження.

  RpgBaseScene::RpgBaseScene(DataStream& stored_objs, bool is_loaded, uint8_t lvl) : IGameScene(stored_objs)
  {
    _level = --lvl;

    if (!loadMapRes())
    {
      _has_error = true;
      _is_finished = true;  // Просто закриваємо гру. Можна додати вивід повідомлення.
      log_e("Помилка завантаження ресурсів");
      return;
    }

    log_i("Ресурси завантажено");

    // Автоматично збираємо поверхню рівня з завантажених даних.
    _terrain_loader.buildTerrain(_terrain, 16);

    createHeroObj();

    _game_UI = new SceneUI();
  }

  RpgBaseScene::~RpgBaseScene()
  {
  }

  void RpgBaseScene::update()
  {
    if (_has_error)
    {
      if (_input.isReleased(BtnID::BTN_OK))
      {
        _input.lock(BtnID::BTN_OK, CLICK_LOCK);
        _is_finished = true;
      }
      return;
    }

    if (_input.isPressed(BtnID::BTN_OK))
    {
      // show ingame menu
      return;
    }
    else if (_input.isPressed(BtnID::BTN_BACK))
    {
      _input.lock(BtnID::BTN_BACK, PRESS_LOCK);
      _is_finished = true;
      return;
    }
    else if (_input.isReleased(BtnID::BTN_OK))
    {
      _input.lock(BtnID::BTN_OK, CLICK_LOCK);
      // якщо в сцені - переміщення, якщо в ігровому меню, обробка пункту меню
    }
    else
    {
      IGameObject::MovingDirection direction = IGameObject::DIRECTION_NONE;

      if (_input.isHolded(BtnID::BTN_UP))
      {
        _input.lock(BtnID::BTN_UP, HOLD_LOCK);
        direction = IGameObject::DIRECTION_UP;
      }
      else if (_input.isHolded(BtnID::BTN_DOWN))
      {
        _input.lock(BtnID::BTN_DOWN, HOLD_LOCK);
        direction = IGameObject::DIRECTION_DOWN;
      }
      else if (_input.isHolded(BtnID::BTN_RIGHT))
      {
        _input.lock(BtnID::BTN_RIGHT, HOLD_LOCK);
        direction = IGameObject::DIRECTION_RIGHT;
      }
      else if (_input.isHolded(BtnID::BTN_LEFT))
      {
        _input.lock(BtnID::BTN_LEFT, HOLD_LOCK);
        direction = IGameObject::DIRECTION_LEFT;
      }

      _hero_obj->move(direction);
    }

    IGameScene::update();
  }

  void RpgBaseScene::onTrigger(uint8_t id)
  {
  }

  bool RpgBaseScene::loadMapRes()
  {
    // TODO Додати прогрес завантаження ресурсів.
    // TODO Завантаження винести в окрему задачу.
    String path_to_csv = STR_TERRAIN_TMPL_FOLDER;
    path_to_csv += _level;
    path_to_csv += ".csv";

    String path_to_tile_type = STR_TERRAIN_TMPL_FOLDER;
    path_to_tile_type += "tiles.meta";

    String path_to_bmps = STR_TERRAIN_TILES_RES_FOLDER;

    return _terrain_loader.loadTerrain(path_to_csv.c_str(), path_to_tile_type.c_str(), path_to_bmps.c_str());
  }

  void RpgBaseScene::createHeroObj()
  {
    _hero_obj = createObject<HeroObj>();
    _hero_obj->init();
    _game_objs.emplace(_hero_obj->getId(), _hero_obj);
    _hero_obj->_x_global = 50;
    _hero_obj->_y_global = 120;
    _main_obj = _hero_obj;
  }

}  // namespace rpg
