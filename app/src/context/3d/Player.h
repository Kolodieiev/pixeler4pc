#pragma once
#pragma GCC optimize("O3")
#include <cstdint>

#include "GameWorld.h"

class Player
{
public:
  Player();
  ~Player();

  void moveForward();
  void moveBack();
  void turnLeft();
  void turnRight();
  void jump();
  void crouch();

  float getDirX() const;
  float getDirY() const;

  float getPlaneX() const;
  float getPlaneY() const;

  float getPosX() const;
  float getPosY() const;
  float getPosZ() const;

  void setMap(GameWorld* map);
protected:
  void initLut();
  void move(float moveSpeed, float rotSpeed);

  GameWorld* _world_map{nullptr};

  // Винести з класу гравця
  float sinTable[1024];
  float cosTable[1024];

  float posX = 3.0f;
  float posY = 3.0f;
  float posZ = 0.5f;

  float dirX = -1.0f;
  float dirY = 0.0f;

  float planeX = 0.0f;
  float planeY = 0.66f;

  const float moveSpeed = 0.05f;
  const float rotSpeed = 0.06f;

  uint32_t angleIdx = 0;
};