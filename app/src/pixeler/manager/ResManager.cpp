#pragma GCC optimize("O3")
#include "ResManager.h"

#include "../util/img/BmpUtil.h"

namespace pixeler
{
  uint16_t ResManager::loadBmpRes(const char* path_to_bmp)
  {
    if (!path_to_bmp)
      return 0;

    ImgData data = BmpUtil::loadBmp(path_to_bmp);

    if (data.width == 0)
    {
      log_e("Помилка завантаження: %s", path_to_bmp);
      return 0;
    }

    _bmp_res.insert(std::pair<uint16_t, ImgData>(_img_res_id_i, data));
    return _img_res_id_i++;
  }

  uint16_t ResManager::loadWavRes(const char* path_to_wav)
  {
    // stub
    return 0;
  }

  ImgData ResManager::getBmpRes(uint16_t res_id)
  {
    const auto it = _bmp_res.find(res_id);
    if (it == _bmp_res.end())
    {
      log_e("Bmp-ресурс не знайдено по ID: %u", res_id);
      return _empty_img;
    }

    return it->second;
  }

  void ResManager::deleteBmpRes(uint16_t res_id)
  {
    if (res_id == 0)
      return;
      
    const auto it = _bmp_res.find(res_id);

    if (it != _bmp_res.end())
    {
      free(it->second.data_ptr);
      _bmp_res.erase(it);
    }
  }

  void ResManager::deleteWavRes(uint16_t res_id)
  {
    // stub
  }

  void ResManager::clearBmpRes()
  {
    for (auto it = _bmp_res.begin(), last_it = _bmp_res.end(); it != last_it; ++it)
      free(it->second.data_ptr);

    _bmp_res.clear();
  }

  void ResManager::clearWavRes()
  {
    // stub
  }

  ResManager _res;
}  // namespace pixeler
