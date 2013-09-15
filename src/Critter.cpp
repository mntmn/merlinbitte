#include "Critter.h"

Critter::Critter(Zone* zone, int x, int y, int templateId) {
  
  this->zone = zone;
  this->x = x;
  this->y = y;
  
  sleepTurns = 0;
  hungerLevel = 0;
  thirstLevel = 0;

  switch (templateId) {
    case 0:
      name = "Random Critter";
      maxHealth = 10;
      strength = 5;
      intelligence = 1;
      agility = 2;
      willpower = 1;
      consoleChar = 'Z';
      fg = TCODColor::darkRed;
      bg = TCODColor::black;
      break;
    case 1:
      name = "Neonazi Scum";
      maxHealth = 10;
      strength = 6;
      intelligence = 1;
      agility = 2;
      willpower = 3;
      consoleChar = 'N';
      fg = TCODColor::white;
      bg = TCODColor::lightRed;
      break;
    case 2:
      name = "Neonazi Ueberscum";
      maxHealth = 20;
      strength = 9;
      intelligence = 1;
      agility = 1;
      willpower = 3;
      consoleChar = 'N';
      fg = TCODColor::white;
      bg = TCODColor::darkRed;
      break;
    case 3:
      name = "Drunk Tourist";
      maxHealth = 5;
      strength = 5;
      intelligence = 2;
      agility = 1;
      willpower = 1;
      consoleChar = 'T';
      fg = TCODColor::white;
      bg = TCODColor::blue;
      break;
    case 4:
      name = "Tilidinized Mouthfoamer";
      maxHealth = 5;
      strength = 6;
      intelligence = 1;
      agility = 1;
      willpower = 5;
      consoleChar = 'M';
      fg = TCODColor::white;
      bg = TCODColor::darkSepia;
      break;
  }

  health = maxHealth;
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
