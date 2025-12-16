#pragma once
#include "../../defines.h"
#include "TileType.h"

namespace pixeler
{
  class Tile
  {
  public:
    Tile(TileType type, const uint16_t* img_ptr) : _img_ptr{img_ptr}, _type{type} {}

    const uint16_t* _img_ptr;
    const TileType _type;
  };
}  // namespace pixeler