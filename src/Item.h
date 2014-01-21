#ifndef MB_ITEM_H
#define MB_ITEM_H

#include <string>
#include <vector>
#include <map>
#include "TDO.h"

using namespace std;

struct Item {
  int uniqueId;

  unsigned char consoleChar;
  unsigned int flags;
  string name;

  int weight; // grams
  int hazard;
  int protection;

  int hungerEffect;
  int thirstEffect;

  TDO* tdo;

  Item(unsigned char consoleChar, 
    int flags, string name, 
    int weight, int hazard, int protection,
    int hungerEffect, int thirstEffect,
    TDO* tdo);
  bool is(Item* otherItem);
};

#endif