#pragma once
#include <stdint.h>

#include <functional>
#include <unordered_map>

#include "../driver/graphics/DisplayWrapper.h"
#include "context_id_setup.h"

// -------------------------------- Підключи нижче заголовкові файли контекстів першого рівня
#include "context/files/FilesContext.h"
#include "context/games/GameListContext.h"
#include "context/games/rpg/RpgContext.h"
#include "context/games/sokoban/SokobanContext.h"
#include "context/home/HomeContext.h"
#include "context/menu/MenuContext.h"

namespace pixeler
{
  // -------------------------------- Додай перемикання контексту за прикладом
  std::unordered_map<ContextID, std::function<IContext*()>> _context_id_map = {
      {ContextID::ID_CONTEXT_HOME, []()
       { return new HomeContext(); }},
      {ContextID::ID_CONTEXT_MENU, []()
       { return new MenuContext(); }},
      {ContextID::ID_CONTEXT_FILES, []()
       { return new FilesContext(); }},
      {ContextID::ID_CONTEXT_GAMES, []()
       { return new GameListContext(); }},
      {ContextID::ID_CONTEXT_SOKOBAN, []()
       { return new sokoban::SokobanContext(); }},
      {ContextID::ID_CONTEXT_RPG, []()
       { return new rpg::RpgContext(); }},
  };
}  // namespace pixeler

// -------------------------------- Стартовий контекст
#define START_CONTEXT HomeContext
