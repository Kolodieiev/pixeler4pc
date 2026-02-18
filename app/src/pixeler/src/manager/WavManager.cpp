#pragma GCC optimize("O3")
#include "WavManager.h"

#include <cmath>
#include <limits>

namespace pixeler
{
  WavManager::~WavManager()
  {
  }

  uint16_t WavManager::addToMix(WavTrack* track)
  {
    return 0;
  }

  void WavManager::removeFromMix(uint16_t id)
  {
  }

  void WavManager::startMix()
  {
  }

  void WavManager::pauseResume()
  {
  }

  void WavManager::mixTask(void* params)
  {
  }

  void WavTrack::play()
  {
  }

  int16_t WavTrack::getNextSample()
  {
    return 0;
  }

  void WavTrack::setVolume(uint8_t volume)
  {
  }

  WavTrack* WavTrack::clone()
  {
    return nullptr;
  }
}  // namespace pixeler