#ifndef MB_CRITTER_H
#define MB_CRITTER_H

#include "libtcod.hpp"
#include <string>
#include "Zone.h"

using namespace std;

struct Critter {
public:
  string name;

  Zone* zone;
  int x;
  int y;

  int health;
  int state;

  int strength;
  int intelligence;
  int agility;
  int willpower;

  unsigned char consoleChar;
  TCODColor fg;
  TCODColor bg;

  std::vector<Item> weapons;
  std::vector<Item> clothes;

  Critter(Zone* zone, int x, int y);
  bool isAlive();

  int attackBy(Critter* attacker);
  int armorValue();
  int damageValue();

  bool toggleClothing(Item item);
  bool toggleWield(Item item);

  bool hasWeapon();
  Item getWeapon();
};

#endif