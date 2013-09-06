#ifndef MB_CRITTER_H
#define MB_CRITTER_H

#include "libtcod.hpp"
#include <string>
#include "Zone.h"

#define NUM_CRITTER_TEMPLATES 5
#define MAX_WILLPOWER 10

using namespace std;

struct Critter {
public:
  string name;

  Zone* zone;
  int x;
  int y;

  int maxHealth;
  int health;
  int state;

  int strength;
  int intelligence;
  int agility;
  int willpower;

  int sleepTurns;

  unsigned char consoleChar;
  TCODColor fg;
  TCODColor bg;

  std::vector<Item> weapons;
  std::vector<Item> clothes;

  Critter(Zone* zone, int x, int y, int templateId);
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