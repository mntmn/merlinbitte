#ifndef MB_ZONE_H
#define MB_ZONE_H

#include "libtcod.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <fstream>
#include <stdio.h>
#include <assert.h>
#include <cstdlib>
#include "Item.h"

#define TILE_BLOCKS 1
#define TILE_LOCKED 2
#define TILE_CLOSED 4
#define TILE_OPEN 8
#define TILE_DESTRUCTIBLE 16

#define ITEM_WEAPON 1
#define ITEM_FOOD 2

using namespace std;

struct Teleport {
  int x;
  int y;
  string zoneId;
  int targetX;
  int targetY;
};

struct ZoneItem {
  int x;
  int y;
  Item item;
};

struct Tile {
  unsigned char consoleChar;
  unsigned int flags;
  string name;
  TCODColor fg;
  TCODColor bg;

  Tile unlock();
  Tile open();
  Tile close();
  Tile lock();
  Tile destroy();
};

class Zone {
  int width, height;

  vector<Teleport*> teleports;
  Tile* tiles;
  vector<ZoneItem> zoneItems;

  TCODMap* tcodMap;

public:
  string title;
  
  Zone(string title, int w, int h);
  ~Zone();

  void generate(char zoneType);
  void mutate(int x, int y, Tile t);
  void load(string filename);
  Tile tileAt(int x, int y);
  void addTeleport(Teleport* t);
  Teleport* teleportAt(int x, int y);
  vector<Item> itemsAt(int x, int y);
  vector<ZoneItem> getZoneItems();
  void eraseItem(int x, int y, string itemId);

  void generateHouses(int ox, int oy, int w, int h, int mind, int maxdx, int maxdy);

  void updateTcodMap();
  TCODMap* getTcodMap();
};

#endif