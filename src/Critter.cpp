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

  this->weapon = NULL;

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
  for (Item* c : this->clothes) {
    v += c->protection;
    printf("clothes: %s %d prot\n",c->name.c_str(),c->protection);
  }
  return v;
}

int Critter::damageValue() {
  int weaponFactor = 1;
  if (weapon!=NULL) {
    weaponFactor = weapon->weight * weapon->hazard;
  }

  return weaponFactor;
}

bool Critter::toggleClothing(Item* item) {
  clothes.push_back(item);
  printf("pushing to clothes: %s\n",item->name.c_str());

  return true;
}

bool Critter::toggleWield(Item* item) {
  if (this->weapon == item) {
    this->weapon = NULL;
    return false;
  }

  this->weapon = item;
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