#include "Critter.h"

Critter::Critter(Zone* zone, int x, int y) {
  this->name = "Random Critter";
  this->x = x;
  this->y = y;
  this->health = 10;
  this->strength = 5;
  this->intelligence = 1;
  this->agility = 2;
  this->willpower = 1;

  this->zone = zone;

  this->consoleChar = 'Z';
  this->fg = TCODColor::darkRed;
  this->bg = TCODColor::black;
}

bool Critter::isAlive() {
  return (this->health>0);
}

int Critter::armorValue() {
  int v = 0;
  for (Item c : this->clothes) {
    v += c.protection;
  }
  return v;
}

int Critter::damageValue() {
  int weaponFactor = 1;
  if (hasWeapon()) {
    weaponFactor = getWeapon().hazard;
  }

  return weaponFactor;
}

bool Critter::toggleClothing(Item item) {
  int idx = 0;
  for (Item c : clothes) {
    if (c.is(&item)) {
      clothes.erase(clothes.begin() + idx);
      return false;
    }
    idx++;
  }

  clothes.push_back(item);
  
  return true;
}

bool Critter::hasWeapon() {
  return this->weapons.size()>0;
}

Item Critter::getWeapon() {
  return this->weapons.at(0);
}

bool Critter::toggleWield(Item item) {
  if (!hasWeapon()) {
    this->weapons.push_back(item);
    return true;
  }

  if (getWeapon().is(&item)) {
    this->weapons.empty();
    return false;
  }

  this->weapons.empty();
  this->weapons.push_back(item);
  return true;
}

int Critter::attackBy(Critter* attacker) {
  int weaponFactor = attacker->damageValue();

  int maxDmg = attacker->strength * weaponFactor;
  int dmg = rand()%maxDmg - this->armorValue();
  if (dmg<1) dmg = 0;

  this->health -= dmg;

  return dmg;
}