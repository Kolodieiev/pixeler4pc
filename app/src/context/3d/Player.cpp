#pragma GCC optimize("O3")
#include "Player.h"

#include <algorithm>
#include <cmath>

#include "pixeler/defines.h"

Player::Player()
{
  initLut();
}

Player::~Player()
{
}

void Player::moveForward()
{
  move(moveSpeed, 0);
}

void Player::moveBack()
{
  move(-moveSpeed, 0);
}

void Player::turnLeft()
{
  move(0, -rotSpeed);
}

void Player::turnRight()
{
  move(0, rotSpeed);
}

void Player::jump()
{
  // if (_input.isHolded(BtnID::BTN_BACK))
  // {
  //   if (_player.posZ > 0.5f)
  //     _player.posZ -= 0.1f;
  // }
  // if (_input.isHolded(BtnID::BTN_OK))
  // {
  //   if (_player.posZ < 3.0f)
  //     _player.posZ += 0.1f;
  // }
}

void Player::crouch()
{
}

float Player::getDirX() const
{
  return dirX;
}

float Player::getDirY() const
{
  return dirY;
}

float Player::getPlaneX() const
{
  return planeX;
}

float Player::getPlaneY() const
{
  return planeY;
}

float Player::getPosX() const
{
  return posX;
}

float Player::getPosY() const
{
  return posY;
}

float Player::getPosZ() const
{
  return posZ;
}

void Player::setMap(GameWorld* map)
{
  if (!map)
    esp_restart();

  _world_map = map;
}

void Player::initLut()
{
  for (int i = 0; i < 1024; i++)
  {
    float angle = (i / 1024.0f) * 2.0f * PI;
    sinTable[i] = sin(angle);
    cosTable[i] = cos(angle);
  }
}

void Player::move(float moveSpeed, float rotSpeed)
{
  // 1. ПОВОРОТ КАМЕРИ
  if (rotSpeed != 0)
  {
    // 1. Додаємо зсув до поточного кута
    angleIdx = (angleIdx + (int)(rotSpeed * 162.97f)) & 1023;

    // 2. Просто беремо готові значення для напрямку (довжина вектора завжди 1.0!)
    dirX = cosTable[angleIdx];
    dirY = sinTable[angleIdx];

    // 3. Площина камери (plane) — це завжди +90 градусів до погляду
    // 90 градусів у таблиці на 1024 — це зсув на 256 позицій (1024 / 4)
    int planeIdx = (angleIdx + 256) & 1023;
    planeX = cosTable[planeIdx] * 0.66f;
    planeY = sinTable[planeIdx] * 0.66f;
  }

  // 2. РУХ З ФІЗИЧНИМ РАДІУСОМ
  if (moveSpeed != 0)
  {
    float nextX = posX + dirX * moveSpeed;
    float nextY = posY + dirY * moveSpeed;

    // Радіус для перевірки стін
    float r = 0.2f;

    // Перевірка X з відступом (залежно від напрямку руху)
    float checkX = (nextX > posX) ? (nextX + r) : (nextX - r);
    if (_world_map->getBlockType((int)posY * MAP_SIZE + (int)checkX) == BLOCK_EMPTY)
    {
      posX = nextX;
    }

    // Перевірка Y з відступом
    float checkY = (nextY > posY) ? (nextY + r) : (nextY - r);
    if (_world_map->getBlockType((int)checkY * MAP_SIZE + (int)posX) == BLOCK_EMPTY)
    {
      posY = nextY;
    }
  }

  // 3. ВЕРТИКАЛЬНІСТЬ ТА ЗГЛАДЖУВАННЯ ВИСОТИ
  // int mapX = (int)posX, mapY = (int)posY;
  // if (mapX >= 0 && mapX < MAP_SIZE && mapY >= 0 && mapY < MAP_SIZE)
  // {
  //   MapCell& cell = worldMap[mapY * MAP_SIZE + mapX];

  //   // fracX/fracY — це позиція гравця всередині однієї клітинки (від 0.0 до 1.0)
  //   float fracX = posX - mapX, fracY = posY - mapY;

  //   // Вираховуємо цільову висоту підлоги в цій точці + додаємо 0.5 (висота очей персонажа)
  //   float targetZ = getFloorHeight(cell, fracX, fracY) + 0.5f;

  //   // Лінійна інтерполяція (Lerp) для плавності:
  //   // Замість миттєвого стрибка на сходинку, posZ плавно "підтягується" до цілі (коефіцієнт 0.2)
  //   posZ += (targetZ - posZ) * 0.2f;
  // }
}
