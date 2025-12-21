#pragma GCC optimize("O3")
#include "GameWorld.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "Player.h"
#include "pixeler/defines.h"
#include "pixeler/driver/graphics/DisplayWrapper.h"
#include "texture/texture_blue_block.h"
#include "texture/texture_brick.h"
#include "texture/texture_stone.h"
#include "texture/texture_tree.h"

using namespace pixeler;

#define MAX_RAY_NUM 64

constexpr int HORIZON_H = TFT_HEIGHT / 2;

GameWorld::GameWorld()
{
}

GameWorld::~GameWorld()
{
}

BlockType GameWorld::getBlockType(uint32_t tile_pos) const
{
  if (tile_pos > MAP_SIZE * MAP_SIZE)
    return BLOCK_EMPTY;

  return worldMap[tile_pos].type;
}

void GameWorld::update()
{
  renderWalls();
  renderSprites();
}

void GameWorld::setPlayer(Player* player)
{
  _player = player;
}

void GameWorld::init()
{
  // Очищуємо карту
  for (int i = 0; i < MAP_SIZE * MAP_SIZE; i++)
  {
    worldMap[i].type = BLOCK_EMPTY;
    worldMap[i].floorHeight = 0.0f;
    worldMap[i].ceilingHeight = 1.0f;
    worldMap[i].wallTextureId = 0;
  }

  // Стіни периметру (сірі)
  for (int x = 0; x < MAP_SIZE; x++)
  {
    worldMap[0 * MAP_SIZE + x].type = BLOCK_WALL;
    worldMap[0 * MAP_SIZE + x].wallTextureId = 1;  // Камінь
    worldMap[(MAP_SIZE - 1) * MAP_SIZE + x].type = BLOCK_WALL;
    worldMap[(MAP_SIZE - 1) * MAP_SIZE + x].wallTextureId = 1;
  }
  for (int y = 0; y < MAP_SIZE; y++)
  {
    worldMap[y * MAP_SIZE + 0].type = BLOCK_WALL;
    worldMap[y * MAP_SIZE + 0].wallTextureId = 1;
    worldMap[y * MAP_SIZE + (MAP_SIZE - 1)].type = BLOCK_WALL;
    worldMap[y * MAP_SIZE + (MAP_SIZE - 1)].wallTextureId = 1;
  }

  // Кімнати (цегла)
  for (int y = 2; y <= 3; y++)
  {
    for (int x = 4; x <= 5; x++)
    {
      worldMap[y * MAP_SIZE + x].type = BLOCK_WALL;
      worldMap[y * MAP_SIZE + x].wallTextureId = 0;  // Цегла
    }
  }

  // Сині блоки
  for (int y = 2; y <= 3; y++)
  {
    worldMap[y * MAP_SIZE + 10].type = BLOCK_WALL;
    worldMap[y * MAP_SIZE + 10].wallTextureId = 2;  // Сині
  }

  // Текстури
  textures[0].data = TEXTURE_BRICK;
  textures[0].width = TEXTURE_SIZE;
  textures[0].height = TEXTURE_SIZE;

  textures[1].data = TEXTURE_STONE;
  textures[1].width = TEXTURE_SIZE;
  textures[1].height = TEXTURE_SIZE;

  textures[2].data = TEXTURE_BLUE;
  textures[2].width = TEXTURE_SIZE;
  textures[2].height = TEXTURE_SIZE;

  textures[3].data = SPRITE_TREE;
  textures[3].width = TEXTURE_SIZE;
  textures[3].height = TEXTURE_SIZE;

  textureCount = 4;

  // Спрайти
  sprites[0] = {8.5f, 8.5f, 0.0f, 1.0f, 1.0f, 0.0f, 3};
  sprites[1] = {10.2f, 6.3f, 0.0f, 0.8f, 0.8f, 0.0f, 3};
  sprites[2] = {6.5f, 10.5f, 0.0f, 1.0f, 1.0f, 0.0f, 3};
  spriteCount = 3;
}

void GameWorld::renderWalls()
{
  // 1. ПІДГОТОВКА КАДРУ

  // Малюємо небо (верхня частина) та підлогу (нижня частина) суцільними кольорами
  _display.fillRect(0, 0, TFT_WIDTH, HORIZON_H, COLOR_BLUE);
  _display.fillRect(0, HORIZON_H, TFT_WIDTH, TFT_HEIGHT - HORIZON_H, COLOR_BROWN);

  // Очищуємо Z-буфер (потрібен, щоб спрайти/об'єкти не малювалися крізь стіни)
  std::fill(zBuffer, zBuffer + TFT_WIDTH, 1e30f);

  // 2. ГОЛОВНИЙ ЦИКЛ: Йдемо по кожному вертикальному ряду пікселів екрана (x)
  for (int x = 0; x < TFT_WIDTH; x++)
  {
    // Обчислюємо напрямок променя залежно від кута огляду (FOV)
    float cameraX = 2.0f * x / (float)TFT_WIDTH - 1.0f;
    float rayDirX = _player->getDirX() + _player->getPlaneX() * cameraX;
    float rayDirY = _player->getDirY() + _player->getPlaneY() * cameraX;

    // deltaDist: відстань, яку треба пройти променю, щоб перетнути одну лінію сітки карти
    float deltaDistX = (rayDirX == 0) ? 1e30f : std::abs(1.0f / rayDirX);
    float deltaDistY = (rayDirY == 0) ? 1e30f : std::abs(1.0f / rayDirY);

    float sideDistX, sideDistY;  // Відстань від гравця до найближчої лінії сітки
    int stepX, stepY;            // Напрямок кроку по карті (-1 або 1)

    // В яких клітинках карти ми зараз знаходимось
    int mapX = (int)_player->getPosX();
    int mapY = (int)_player->getPosY();

    // Визначаємо початкові sideDist та напрямок кроку
    if (rayDirX < 0)
    {
      stepX = -1;
      sideDistX = (_player->getPosX() - mapX) * deltaDistX;
    }
    else
    {
      stepX = 1;
      sideDistX = (mapX + 1.0f - _player->getPosX()) * deltaDistX;
    }
    if (rayDirY < 0)
    {
      stepY = -1;
      sideDistY = (_player->getPosY() - mapY) * deltaDistY;
    }
    else
    {
      stepY = 1;
      sideDistY = (mapY + 1.0f - _player->getPosY()) * deltaDistY;
    }

    // 3. АЛГОРИТМ DDA (Digital Differential Analyzer)
    // Промінь "стрибає" по клітинках сітки, поки не влучить у стіну
    int side;  // Яку сторону стіни зачепили (X чи Y)
    bool hit = false;

    MapCell hitCell;

    for (int i = 0; i < MAX_RAY_NUM && !hit; i++)
    {
      if (sideDistX < sideDistY)
      {
        sideDistX += deltaDistX;
        mapX += stepX;
        side = 0;  // Влучили у вертикальну стіну
      }
      else
      {
        sideDistY += deltaDistY;
        mapY += stepY;
        side = 1;  // Влучили у горизонтальну стіну
      }

      // Перевірка виходу за межі карти
      if (mapX < 0 || mapX >= MAP_SIZE || mapY < 0 || mapY >= MAP_SIZE)
        break;

      // Перевіряємо, чи є в цій клітинці стіна
      hitCell = worldMap[mapY * MAP_SIZE + mapX];
      if (hitCell.type != BLOCK_EMPTY)
        hit = true;
    }

    if (!hit)
      continue;

    // 4. РОЗРАХУНОК ВІЗУАЛІЗАЦІЇ
    float perpWallDist;
    if (side == 0)
      perpWallDist = sideDistX - deltaDistX;
    else
      perpWallDist = sideDistY - deltaDistY;

    zBuffer[x] = perpWallDist;

    // Корекція переспективи
    float wallX;
    if (side == 0)
      wallX = _player->getPosY() + perpWallDist * rayDirY;
    else
      wallX = _player->getPosX() + perpWallDist * rayDirX;
    wallX -= floor(wallX);

    // Обрізаємо дистанцію для розрахунку висоти стіни
    float visualDist = (perpWallDist < 0.1f) ? 0.1f : perpWallDist;

    int lineHeight = (int)(TFT_HEIGHT / visualDist);
    int drawStart = HORIZON_H - (int)(lineHeight * (hitCell.ceilingHeight - _player->getPosZ()));
    int drawEnd = HORIZON_H - (int)(lineHeight * (hitCell.floorHeight - _player->getPosZ()));

    // Обмежуємо малювання межами екрана
    int actualDrawStart = std::max(0, drawStart);
    int actualDrawEnd = std::min(TFT_HEIGHT - 1, drawEnd);
    int drawHeight = actualDrawEnd - actualDrawStart;

    if (drawHeight <= 0)
      continue;

    // 5. ТЕКСТУРУВАННЯ З ПЛАВНИМ КРОКОМ
    Texture& tex = textures[hitCell.wallTextureId];

    int texX = (int)(wallX * (float)TEXTURE_SIZE);
    if ((side == 0 && rayDirX > 0) || (side == 1 && rayDirY < 0))
      texX = TEXTURE_SIZE - texX - 1;
    texX &= (TEXTURE_SIZE - 1);  // Захист меж

    float step = 1.0f * TEXTURE_SIZE / lineHeight;

    // Розраховуємо початкову точку текстури з урахуванням обрізки екраном
    float texPos = (actualDrawStart - HORIZON_H + (lineHeight * (hitCell.ceilingHeight - _player->getPosZ()))) * step;

    for (int y = 0; y < drawHeight; y++)
    {
      int texY = (int)texPos & (TEXTURE_SIZE - 1);
      texPos += step;

      uint16_t color = tex.data[texY * TEXTURE_SIZE + texX];

      if (side == 1)
        color = (color >> 1) & 0x7BEF;

      texColumnBuffer[y] = color;
    }

    _display.drawBitmap(x, actualDrawStart, texColumnBuffer, 1, drawHeight);
  }
}

void GameWorld::renderSprites()
{
  // 1. ПЕРЕВІРКА НАЯВНОСТІ
  if (spriteCount == 0)
    return;

  // 2. РОЗРАХУНОК ВІДСТАНІ ДО КОЖНОГО СПРАЙТА
  for (int i = 0; i < spriteCount; i++)
  {
    // Використовуємо квадрат відстані (без sqrt), щоб зекономити ресурси процесора
    sprites[i].distance = (_player->getPosX() - sprites[i].x) * (_player->getPosX() - sprites[i].x) +
        (_player->getPosY() - sprites[i].y) * (_player->getPosY() - sprites[i].y);
  }

  // 3. СОРТУВАННЯ СПРАЙТІВ (Алгоритм художника)
  // Малюємо від найвіддаленіших до найближчих, щоб ближні об'єкти коректно перекривали дальні
  std::sort(sprites, sprites + spriteCount,
            [](const Sprite& a, const Sprite& b)
            { return a.distance > b.distance; });

  // 4. ЦИКЛ ВІЗУАЛІЗАЦІЇ КОЖНОГО СПРАЙТА
  for (int i = 0; i < spriteCount; i++)
  {
    Sprite& spr = sprites[i];

    // Відносна позиція спрайта щодо гравця
    float spriteX = spr.x - _player->getPosX();
    float spriteY = spr.y - _player->getPosY();

    // Математика трансформації: перетворюємо світові координати у координати камери
    // invDet - це зворотний визначник матриці камери
    float invDet = 1.0f / (_player->getPlaneX() * _player->getDirY() - _player->getDirX() * _player->getPlaneY());
    float transformX = invDet * (_player->getDirY() * spriteX - _player->getDirX() * spriteY);
    float transformY = invDet * (-_player->getPlaneY() * spriteX + _player->getPlaneX() * spriteY);  // Відстань до екрана(Z-глибина)

    // Якщо спрайт позаду гравця або надто близько - ігноруємо
    if (transformY <= 0.1f)
      continue;

    // Обчислюємо позицію центру спрайта на екрані та його масштаб (залежно від відстані transformY)
    int spriteScreenX = (int)((TFT_WIDTH / 2) * (1 + transformX / transformY));
    int spriteHeight = abs((int)(TFT_HEIGHT / transformY * spr.height));
    int spriteWidth = abs((int)(TFT_HEIGHT / transformY * spr.width));

    // Розраховуємо крайні точки малювання (з урахуванням висоти спрайта spr.z)
    int drawStartY = HORIZON_H - spriteHeight / 2 -
        (int)((spr.z - _player->getPosZ()) * TFT_HEIGHT / transformY);
    int drawStartX = spriteScreenX - spriteWidth / 2;

    // Перевірка, чи спрайт взагале потрапляє в межі екрана
    if (drawStartX >= TFT_WIDTH || drawStartX + spriteWidth < 0 ||
        drawStartY >= TFT_HEIGHT || drawStartY + spriteHeight < 0)
      continue;

    // 5. ПЕРЕВІРКА ВИДИМОСТІ
    // Швидка перевірка: чи є хоча б одна вертикальна смуга спрайта перед стіною (через zBuffer)
    bool visible = false;
    for (int sx = std::max(0, drawStartX);
         sx < std::min(TFT_WIDTH, drawStartX + spriteWidth); sx++)
    {
      if (transformY < zBuffer[sx])
      {
        visible = true;
        break;
      }
    }

    if (!visible)
      continue;

    // 6. ПОПІКСЕЛЬНЕ МАЛЮВАННЯ
    Texture& tex = textures[spr.textureId];

    // Обчислюємо крок: скільки пікселів текстури припадає на один піксель екрана
    // Використання float тут критично важливе для плавності при наближенні
    float stepY = (float)tex.height / (float)spriteHeight;
    float stepX = (float)tex.width / (float)spriteWidth;

    for (int sy = 0; sy < spriteHeight; sy++)
    {
      int screenY = drawStartY + sy;
      // Відсікання по вертикалі (екран)
      if (screenY < 0 || screenY >= TFT_HEIGHT)
        continue;

      // Обчислюємо вертикальну координату текстури
      int texY = (int)(sy * stepY);
      if (texY >= tex.height)
        texY = tex.height - 1;  // Захист від переповнення

      for (int sx = 0; sx < spriteWidth; sx++)
      {
        int screenX = drawStartX + sx;
        // Відсікання по горизонталі (екран)
        if (screenX < 0 || screenX >= TFT_WIDTH)
          continue;

        // Перевірка Z-буфера: якщо стіна ближче за спрайт — не малюємо піксель
        if (transformY > zBuffer[screenX])
          continue;

        // Обчислюємо горизонтальну координату текстури
        int texX = (int)(sx * stepX);
        if (texX >= tex.width)
          texX = tex.width - 1;  // Захист від переповненняAV

        uint16_t color = tex.data[texY * tex.width + texX];

        // Малюємо тільки якщо колір не прозорий
        if (color != COLOR_TRANSPARENT)
        {
          _display.drawPixel(screenX, screenY, color);
        }
      }
    }
  }
}
