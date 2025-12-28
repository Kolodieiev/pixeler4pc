#pragma GCC optimize("O3")
#include "lua_menu_item.h"

#include "./lua_image.h"
#include "./lua_iwidget.h"
#include "./lua_label.h"
#include "pixeler/plugin/lua/res/lua_strs.h"
#include "pixeler/ui/widget/IWidget.h"
#include "pixeler/ui/widget/image/Image.h"
#include "pixeler/ui/widget/menu/item/MenuItem.h"
#include "pixeler/ui/widget/text/Label.h"

using namespace pixeler;

int lua_menu_item_new(lua_State* L)
{
  uint16_t id = luaL_checkinteger(L, 2);
  MenuItem** ret_wid_ptr = static_cast<MenuItem**>(lua_newuserdata(L, sizeof(MenuItem*)));
  *ret_wid_ptr = new MenuItem(id);
  luaL_getmetatable(L, STR_TYPE_NAME_MENU_ITEM);
  lua_setmetatable(L, -2);
  return 1;
}

int lua_menu_item_clone(lua_State* L)
{
  MenuItem* menu_item = *static_cast<MenuItem**>(lua_touserdata(L, 1));
  uint16_t id = luaL_checkinteger(L, 2);
  MenuItem* clone = menu_item->clone(id);

  MenuItem** menu_item_clone = static_cast<MenuItem**>(lua_newuserdata(L, sizeof(MenuItem*)));
  *menu_item_clone = clone;

  luaL_getmetatable(L, STR_TYPE_NAME_MENU_ITEM);
  lua_setmetatable(L, -2);
  return 1;
}

int lua_menu_item_set_img(lua_State* L)
{
  MenuItem* item = *static_cast<MenuItem**>(lua_touserdata(L, 1));
  Image* image = *static_cast<Image**>(luaL_checkudata(L, 2, STR_TYPE_NAME_IMAGE));
  item->setImg(image);
  return 0;
}

int lua_menu_item_get_img(lua_State* L)
{
  MenuItem* item = *static_cast<MenuItem**>(lua_touserdata(L, 1));
  Image* image = item->getImg();

  if (!image)
  {
    lua_pushnil(L);
  }
  else
  {
    Image** img = static_cast<Image**>(lua_newuserdata(L, sizeof(Image*)));
    *img = image;

    luaL_getmetatable(L, STR_TYPE_NAME_IMAGE);
    lua_setmetatable(L, -2);
  }

  return 1;
}

int lua_menu_item_set_lbl(lua_State* L)
{
  MenuItem* item = *static_cast<MenuItem**>(lua_touserdata(L, 1));
  Label* label = *static_cast<Label**>(luaL_checkudata(L, 2, STR_TYPE_NAME_LABEL));
  item->setLbl(label);
  return 0;
}

int lua_menu_item_get_lbl(lua_State* L)
{
  MenuItem* item = *static_cast<MenuItem**>(lua_touserdata(L, 1));
  Label** lbl = static_cast<Label**>(lua_newuserdata(L, sizeof(Label*)));
  *lbl = item->getLbl();

  luaL_getmetatable(L, STR_TYPE_NAME_LABEL);
  lua_setmetatable(L, -2);
  return 1;
}

int lua_menu_item_set_text(lua_State* L)
{
  MenuItem* item = *static_cast<MenuItem**>(lua_touserdata(L, 1));
  const char* text = luaL_checkstring(L, 2);
  item->setText(text);
  return 0;
}

int lua_menu_item_get_text(lua_State* L)
{
  MenuItem* item = *static_cast<MenuItem**>(lua_touserdata(L, 1));
  lua_pushstring(L, item->getText().c_str());
  return 1;
}

const struct luaL_Reg TYPE_METH_MENU_ITEM[] = {
    {"setImg", lua_menu_item_set_img},
    {"getImg", lua_menu_item_get_img},
    {"setLbl", lua_menu_item_set_lbl},
    {"getLbl", lua_menu_item_get_lbl},
    {"setText", lua_menu_item_set_text},
    {"getText", lua_menu_item_get_text},
    {STR_LUA_WIDGET_CLONE, lua_menu_item_clone},
    {nullptr, nullptr},
};

void lua_init_menu_item(lua_State* L)
{
  luaL_newmetatable(L, STR_TYPE_NAME_MENU_ITEM);
  lua_newtable(L);
  luaL_setfuncs(L, TYPE_METH_MENU_ITEM, 0);
  luaL_getmetatable(L, STR_TYPE_NAME_IWIDGET);
  lua_setmetatable(L, -2);
  lua_setfield(L, -2, STR_LUA_INDEX);
  lua_pop(L, 1);

  lua_newtable(L);
  lua_pushcfunction(L, lua_menu_item_new);
  lua_setfield(L, -2, STR_LUA_NEW);
  lua_setglobal(L, STR_TYPE_NAME_MENU_ITEM);
}
