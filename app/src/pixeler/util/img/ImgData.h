#pragma once
#include <stdint.h>

namespace pixeler
{
  struct ImgData
  {
    uint16_t* data_ptr{nullptr};
    uint16_t width{0};
    uint16_t height{0};
  };
}  // namespace pixeler