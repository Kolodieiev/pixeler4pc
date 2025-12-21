#pragma once
#pragma GCC optimize("O3")
#include <cstdint>

#define MAP_SIZE 16
#define MAX_SPRITES 20

enum BlockType
{
  BLOCK_EMPTY = 0,
  BLOCK_WALL = 1,
  BLOCK_PLATFORM = 2,
  BLOCK_SLOPE_N = 3,
  BLOCK_SLOPE_S = 4,
  BLOCK_SLOPE_E = 5,
  BLOCK_SLOPE_W = 6
};

struct MapCell
{
  BlockType type;
  uint8_t wallTextureId;
  float floorHeight;
  float ceilingHeight;
};

struct Sprite
{
  float x;
  float y;
  float z;
  float width;
  float height;
  float distance;

  uint8_t textureId;
};

struct Texture
{
  const uint16_t* data;
  uint16_t width;
  uint16_t height;
};

class Player;

class GameWorld
{
public:
  GameWorld();
  ~GameWorld();

  void init();

  BlockType getBlockType(uint32_t tile_pos) const;

  void update();
  
  void setPlayer(Player* player);

private:
  void renderWalls();
  void renderSprites();

  // Z-buffer
  float zBuffer[240];

  // Буфер для колонки текстури
  uint16_t texColumnBuffer[320];

  MapCell worldMap[MAP_SIZE * MAP_SIZE];
  Sprite sprites[MAX_SPRITES];
  Texture textures[10];

  int spriteCount = 0;
  int textureCount = 0;

  Player* _player{nullptr};
};
