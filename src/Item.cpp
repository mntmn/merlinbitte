#include "Item.h"

static int lastUniqueId = 0;

Item::Item(unsigned char consoleChar, int flags, string name, int weight, int hazard, int protection, TCODColor fg, TCODColor bg) {
  
  printf("lastUniqueId: %d\n",lastUniqueId);

  this->uniqueId = ++lastUniqueId;

  this->consoleChar = consoleChar;
  this->flags = flags;
  this->name = name;
  this->weight = weight;
  this->hazard = hazard;
  this->protection = protection;
  this->fg = fg;
  this->bg = bg;
}

bool Item::is(Item* otherItem) {
  return this->uniqueId == otherItem->uniqueId;
}