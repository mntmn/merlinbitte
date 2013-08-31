#ifndef MB_ITEM_H
#define MB_ITEM_H

#include "libtcod.hpp"
#include <string>
#include <vector>
#include <map>

using namespace std;

struct Item {
  unsigned char consoleChar;
  unsigned int flags;
  string name;

  int weight; // grams
  int hazard;
  int protection;

  TCODColor fg;
  TCODColor bg;
};

#endif