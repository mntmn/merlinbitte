#include <string>
#include <map>
#include <vector>
#include "libtcod.hpp"
#include "Zone.h"
#include "Critter.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 60
#define SPAWN_ZONE "apartment"
#define MAX_CONSOLE_MSGS 10

#define HSTATE_MOVE 0
#define HSTATE_USE_SELECT 1
#define HSTATE_USE_DIRECTION 2
#define HSTATE_MANIPULATE_DIRECTION 3
#define HSTATE_WIELD_SELECT 4
#define HSTATE_SMASH_DIRECTION 5
#define HSTATE_SLEEP 500
#define HSTATE_DEAD 666

using namespace std;

TCODConsole tc(SCREEN_WIDTH, SCREEN_HEIGHT);

map<string, Zone*> zones;
Zone* currentZone;

Critter hero{NULL,0,0,0};
vector<string> consoleMessages = {};
vector<Item> inventory;
vector<Critter*> critters;

int heroState = HSTATE_MOVE;
int selectedItemId = 0;
int yOffset = 2;

int turn = 0;

void restart();


vector<string> moveCritters(Zone* playerZone) {
  vector<string> responses;

  for (Critter* c : critters) {

    if (c->zone == playerZone) {

      auto path = new TCODPath(playerZone->getTcodMap(), 1.0f); 
      if (path->compute(c->x, c->y, hero.x, hero.y)) {

        // trial against willpower
        bool enoughWillPower = (c->willpower > rand()%MAX_WILLPOWER);
          
        if (enoughWillPower && !path->isEmpty()) {

          int nextX, nextY;
          path->walk(&nextX, &nextY, true);

          if (nextX == hero.x && nextY == hero.y) {
            // attack hero

            int dmg = hero.attackBy(c);

            responses.push_back(string("The ") + c->name + string(" claws at you causing ") + to_string(dmg) + string(" damage."));
          } else {
            c->x = nextX;
            c->y = nextY;
          }
        }
      }
    }
  }

  return responses;
}

void renderCritters(Zone* zone) {
  for (Critter* c : critters) {
    if (c->zone == zone) {
      tc.putCharEx(c->x, c->y + yOffset, c->consoleChar, c->fg, c->bg);
    }
  }
}

void renderZone(Zone* zone) {
  tc.clear();
  tc.setDefaultForeground(TCODColor::white);

  // tiles
  for (int y = 0; y < SCREEN_HEIGHT-yOffset; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      Tile t = zone->tileAt(x, y);
      if (t.consoleChar>0) {
        tc.putCharEx(x, y + yOffset, t.consoleChar, t.fg, t.bg);
      }
    }
  }

  // items
  for (ZoneItem zi : zone->getZoneItems()) {
    Item item = zi.item;
    tc.putCharEx(zi.x, zi.y+yOffset, item.consoleChar, item.fg, item.bg);
  }

  tc.setDefaultForeground(TCODColor::white);

  tc.putChar(hero.x,hero.y+yOffset,'@',TCOD_BKGND_NONE);

  auto playerTile = zone->tileAt(hero.x,hero.y);

  tc.print(1,1,zone->title.c_str());
  tc.print(20,1,playerTile.name.c_str());

  renderCritters(zone);
}

void renderMessages(vector<string>* messages) {
  tc.setDefaultForeground(TCODColor::white);

  int y = SCREEN_HEIGHT-MAX_CONSOLE_MSGS-1;
  for (string m : *messages) {
    tc.print(1,y++,m.c_str());
  }
}

void renderInventory() {
  tc.setDefaultForeground(TCODColor::white);

  tc.print(SCREEN_WIDTH-20,1,"Inventory");
  int y = 2;
  int idx = 0;
  for (Item item : inventory) {
    string mod = "";
    if (hero.hasWeapon() && hero.getWeapon().is(&item)) {
      mod += "[W] ";
    } else {
      for (Item c : hero.clothes) {
        if (c.is(&item)) {
          mod += "[w] ";
        }
      }
    }
    tc.print(SCREEN_WIDTH-20,y++, (to_string(idx++)+": "+mod+item.name).c_str());
  }
}

void renderStats() {
  tc.setDefaultForeground(TCODColor::white);

  int x = SCREEN_WIDTH-35;

  tc.print(x,1,"Stats");

  int minutesTotal = turn/5;
  int hoursTotal = (minutesTotal/60);
  int hours = hoursTotal%24;
  int day = hoursTotal/24 + 1;

  tc.print(x,3,(string("Day ") + to_string(day) + " " + to_string(hoursTotal%24) + ":" + to_string(minutesTotal%60)).c_str());
  tc.print(x,4,(string("Health: ") + to_string(hero.health)).c_str());
  tc.print(x,5,(string("INT: ") + to_string(hero.intelligence)).c_str());
  tc.print(x,6,(string("STR: ") + to_string(hero.strength)).c_str());
  tc.print(x,7,(string("AGL: ") + to_string(hero.agility)).c_str());
  tc.print(x,8,(string("WPR: ") + to_string(hero.willpower)).c_str());

  tc.print(x,10,(string("ARM: ") + to_string(hero.armorValue())).c_str());
  tc.print(x,11,(string("DMG: ") + to_string(hero.damageValue())).c_str());
}

void initTCod() {
  auto fullscreen = false;

  const char *font="data/fonts/consolas12x12_gs_tc.png";

  int fontFlags=TCOD_FONT_TYPE_GREYSCALE|TCOD_FONT_LAYOUT_TCOD, fontNewFlags=0;

  TCODConsole::setCustomFont(font,fontFlags,0,0);

  TCODConsole::initRoot(SCREEN_WIDTH,SCREEN_HEIGHT,"merlin bitte",fullscreen,TCOD_RENDERER_SDL);

  TCODConsole::root->setDefaultForeground(TCODColor::white);
  TCODConsole::root->setDefaultBackground(TCODColor::lightBlue);

  TCODSystem::setFps(30);
}

void initZones() {

  // hard-coded special zones
  auto zoneIds={"apartment","apt-stairs","backyard","torstrasse"};
  for (auto id : zoneIds) {
    auto z = new Zone(id,40,40);
    z->load(string("zones/") + string(id) + string(".txt"));
    zones.insert(make_pair(id, z));
  }

  // dynamic generated zones

  ifstream input("data/mapseed.txt");
  printf("[initZones] Loading mapseed.txt\n");

  int y=0;
  int x=0;
  int w=40;
  int h=40;
  int seedW = 20;
  int seedH = 20;
  int lc=0;

  vector<string> lines;

  for (string line; getline(input, line);) {
    //printf("Read line: %d\n",lc++);
    lines.push_back(line);
  }

  for (string line : lines) {
    for (int x=0; x<seedW; x++) {
      string zid = "Sector " + to_string(x) + "/" + to_string(y);
      auto z = new Zone(zid,w,h);
      z->generate(line[x]);

      char toLeft=0, toRight=0, toTop=0, toBottom=0;

      if (x>0) toLeft = line[x-1];
      if (x<seedW-1) toRight = line[x+1];
      if (y>0) toTop = lines[y-1][x];
      if (y<seedH-1) toBottom = lines[y+1][x];

      string targetId;
      // add north, south teleports
      for (int tx=0; tx<w; tx++) {
        if (toTop!=0) {
          if (toTop == '1') {
            targetId = "backyard";
            printf("zid %s top linked to %s\n",zid.c_str(), targetId.c_str());
          } else {
            targetId = "Sector " + to_string(x) + "/" + to_string(y-1);
          }

          Teleport* t = new Teleport{tx,0,targetId,tx,h-2};
          z->addTeleport(t);
        }
        if (toBottom!=0) {
          if (toBottom == '1') {
            targetId = "backyard";
            printf("zid %s bottom linked to %s\n",zid.c_str(), targetId.c_str());
          } else {
            targetId = "Sector " + to_string(x) + "/" + to_string(y+1);
          }

          Teleport* t = new Teleport{tx,h-1,targetId,tx,1};
          z->addTeleport(t);
        } else {
          // edge of map
        }
      }

      // add east, west teleports
      for (int ty=0; ty<h; ty++) {
        if (toLeft!=0) {
          if (toLeft == '1') {
            targetId = "backyard";
            printf("zid %s left linked to %s\n",zid.c_str(), targetId.c_str());
          } else {
            targetId = "Sector " + to_string(x-1) + "/" + to_string(y);
          }
          Teleport* t = new Teleport{0,ty,targetId,w-2,ty};
          z->addTeleport(t);
        }
        if (toRight!=0) {
          if (toRight == '1') {
            targetId = "backyard";
            printf("zid %s right linked to %s\n",zid.c_str(), targetId.c_str());
          } else {
            targetId = "Sector " + to_string(x+1) + "/" + to_string(y);
          }
          Teleport* t = new Teleport{w-2,ty,targetId,1,ty};
          z->addTeleport(t);
        } else {
          // edge of map
        }
      }

      zones.insert(make_pair(zid,z));

      // sprinkle with critters

      for (int i=0; i<4; i++) {
        int pad = 2;
        int tx, ty;
        bool done = false;
        int tries = 10;
        while (!done && tries>0) {
          tx = rand()%(w-2*pad)+pad;
          ty = rand()%(h-2*pad)+pad;
          if (!(z->tileAt(tx,ty).flags & TILE_BLOCKS)) {
            done = true;
          }
          tries--;
        }

        int templateId = rand()%NUM_CRITTER_TEMPLATES;

        Critter* c = new Critter(z,tx,ty,templateId);
        critters.push_back(c);
      }
    }
    y++;
  }
}

void initCritters() {
  printf("Initializing Critters...");

  for (Critter* critter : critters) {
    //printf("Critter: %s at %d %d\n", critter->name.c_str(), critter->x, critter->y);
  }
}

void initHero() {

  hero=Critter{NULL, 17, 15, 0};
  hero.name = "mntmn";
  hero.maxHealth = 20;
  hero.health = 20;
  hero.strength = 3;
}

Zone* getZone(string zoneName) {
  return zones.at(zoneName);
}

int getKeyX(TCOD_key_t key) {
  if (key.vk == TCODK_LEFT) return -1;
  if (key.vk == TCODK_RIGHT) return 1;
  return 0;
}

int getKeyY(TCOD_key_t key) {
  if (key.vk == TCODK_UP) return -1;
  if (key.vk == TCODK_DOWN) return 1;
  return 0;
}

vector<string> processInput(TCOD_key_t key, TCOD_mouse_t mouse) {

  vector<string> responses;

  int newHeroX = hero.x;
  int newHeroY = hero.y;
  bool moveHero = false;

  int prevTurn = turn;

  if (heroState == HSTATE_SLEEP) {
    hero.sleepTurns--;
    turn++;
    responses.push_back(string("You sleep, dreamless."));
    
    if (hero.health < hero.maxHealth) {
      // trial for healing
      if (rand()%10 > 6) {
        hero.health++;
        responses.push_back(string("Your body heals for 1 point."));
      }
    }

    if (hero.sleepTurns<1) {
      heroState = HSTATE_MOVE;
      responses.push_back(string("You wake up."));
    }

  } else if (heroState == HSTATE_MOVE) {
    // move state

    newHeroY += getKeyY(key);
    newHeroX += getKeyX(key);

    if (key.c == 't') {
      // take item
      vector<Item> heroItems = currentZone->itemsAt(hero.x, hero.y);
      for (Item item : heroItems) {
        responses.push_back(string("You take the ") + item.name);
        inventory.push_back(item);
        currentZone->eraseItem(hero.x, hero.y, item.name);
      }

      turn++;
    }

    if (key.c == 'u') {
      // use item
      responses.push_back(string("Which item do you want to use? (0-9)"));
      heroState = HSTATE_USE_SELECT;
    }

    if (key.c == 'm') {
      // manipulate tile
      responses.push_back(string("What do you want to manipulate (direction)?"));
      heroState = HSTATE_MANIPULATE_DIRECTION;
    }

    if (key.c == 's') {
      // smash tile
      responses.push_back(string("What do you want to smash (direction)?"));
      heroState = HSTATE_SMASH_DIRECTION;
    }

    if (key.c == 'S') {
      // smash tile
      responses.push_back(string("You fall asleep..."));
      heroState = HSTATE_SLEEP;
      hero.sleepTurns = 10;
    }

    if (key.c == 'w') {
      // manipulate tile
      responses.push_back(string("What do you want wield/wear (0-9)?"));
      heroState = HSTATE_WIELD_SELECT;
    }

  } else if (heroState == HSTATE_USE_SELECT) {

    if (key.c >= '0' && key.c <= '9') {
      int itemNumber = key.c - '0';

      if (inventory.size()<=itemNumber) {
        responses.push_back(string("No such item."));
        heroState = HSTATE_MOVE;
      } else {
        Item item = inventory.at(itemNumber);
        selectedItemId = itemNumber;

        if (item.hungerEffect > 0) {
          // food item

          responses.push_back(string("You eat the ")+item.name+", quelling your hunger.");
          hero.hungerLevel -= item.hungerEffect;
          if (hero.hungerLevel<0) hero.hungerLevel = 0;

          inventory.erase(inventory.begin() + selectedItemId);
          heroState = HSTATE_MOVE;
        } else {
          responses.push_back(string("Where do you want to use ")+item.name+"?");
          heroState = HSTATE_USE_DIRECTION;
        }
      }
    }

  } else if (heroState == HSTATE_WIELD_SELECT) {

    if (key.c >= '0' && key.c <= '9') {
      int itemNumber = key.c - '0';

      if (inventory.size()<=itemNumber) {
        responses.push_back(string("No such item."));
        heroState = HSTATE_MOVE;
      } else {
        Item item = inventory.at(itemNumber);
        selectedItemId = itemNumber;

        if (item.protection > 0) {
          // wear
          if (hero.toggleClothing(item)) {
            responses.push_back(string("You put on the ") + item.name);
          } else {
            responses.push_back(string("You take off the ") + item.name);
          }
        } else if (item.hazard > 0) {
          // wield
          if (hero.toggleWield(item)) {
            responses.push_back(string("You wield the ") + item.name);
          } else {
            responses.push_back(string("You no longer wield the ") + item.name);
          }
        }
        heroState = HSTATE_MOVE;
        turn++;
      }
    }

  } else if (heroState == HSTATE_USE_DIRECTION) {
    int useX = hero.x;
    int useY = hero.y;

    useY += getKeyY(key);
    useX += getKeyX(key);

    if (useX!=hero.x || useY!=hero.y) {
      Item useItem = inventory.at(selectedItemId);
      Tile useTile = currentZone->tileAt(useX, useY);

      if (useItem.name == "apartment keys" && useTile.name == "locked door") {
        responses.push_back(string("You use the ") + useItem.name + string(" with the ")+useTile.name+"!");

        currentZone->mutate(useX, useY, useTile.unlock());
      } else {
        responses.push_back(string("That didn't work."));
      }

      heroState = HSTATE_MOVE;
      turn++;
    }
  } else if (heroState == HSTATE_MANIPULATE_DIRECTION) {
    int useX = hero.x;
    int useY = hero.y;

    useY += getKeyY(key);
    useX += getKeyX(key);

    if (useX!=hero.x || useY!=hero.y) {

      Tile useTile = currentZone->tileAt(useX, useY);

      if (useTile.flags & TILE_OPEN) {
        responses.push_back(string("You close the ") + useTile.name + string("."));
        Tile closedTile = useTile.close();

        currentZone->mutate(useX, useY, closedTile);
        currentZone->updateTcodMap();
      }

      heroState = HSTATE_MOVE;
      turn++;
    }
  } else if (heroState == HSTATE_SMASH_DIRECTION) {
    int useX = hero.x + getKeyX(key);
    int useY = hero.y + getKeyY(key);

    if (useX!=hero.x || useY!=hero.y) {
      Tile useTile = currentZone->tileAt(useX, useY);

      if (useTile.flags & TILE_DESTRUCTIBLE) {
        responses.push_back(string("You smash the ") + useTile.name + string(" into pieces."));
        Tile smashedTile = useTile.destroy();

        currentZone->mutate(useX, useY, smashedTile);
        currentZone->updateTcodMap();
      } else {
        responses.push_back(string("You can't seem to destroy the ") + useTile.name + string("."));
      }

      heroState = HSTATE_MOVE;
      turn++;
    }
  } else if (heroState == HSTATE_DEAD) {
    if (key.c == 'r') {
      restart();
      return responses;
    }
  }

  Tile newHeroTile = currentZone->tileAt(newHeroX, newHeroY);

  if (newHeroTile.flags & TILE_BLOCKS || newHeroTile.flags & TILE_LOCKED) {
    responses.push_back(string("Your path is blocked by a ") + newHeroTile.name + string("."));
  } else if (newHeroTile.flags & TILE_CLOSED) {
    responses.push_back(string("You open the ") + newHeroTile.name + string("."));
    Tile openedTile = newHeroTile.open();
    
    currentZone->mutate(newHeroX, newHeroY, openedTile);
    currentZone->updateTcodMap();
    turn++;
  } else {
    moveHero = true;
  }

  if (moveHero && (hero.x!=newHeroX || hero.y!=newHeroY)) {

    // check for critters

    bool attacked = false;
    int idx = 0;
    for (Critter* critter : critters) {
      if (critter->zone == currentZone && critter->x==newHeroX && critter->y==newHeroY) {
        // move onto critter -> attack
        int attackResult = critter->attackBy(&hero);

        if (attackResult>0) {
          int dmg = attackResult;
          responses.push_back(string("You swing at the ") + critter->name.c_str() + " and cause "+to_string(dmg)+" damage!");
        } else {
          responses.push_back(string("You swing at the ") + critter->name.c_str() + " but miss.");
        }

        critter->willpower++; // attacked enemy wants revenge

        if (!critter->isAlive()) {
          responses.push_back(string("The ") + critter->name.c_str() + " dies.");
          critters.erase(critters.begin() + idx);
        }

        attacked = true;
        break;
      }
      idx++;
    }

    if (!attacked) {
      hero.x = newHeroX;
      hero.y = newHeroY;
    
      vector<Item> heroItems = currentZone->itemsAt(hero.x, hero.y);
      if (heroItems.size()==1) {
        responses.push_back(string("There is an item here (t=take): ") + heroItems.at(0).name);
      } else if (heroItems.size()>1) {
        responses.push_back(string("There are several items here:"));
        for (Item item : heroItems) {
          responses.push_back(item.name);
        }
      }

      Teleport* teleport = currentZone->teleportAt(hero.x, hero.y);
      if (teleport != NULL) {
        responses.push_back(string("You leave the ")+currentZone->title + string("."));

        currentZone = getZone(teleport->zoneId);
        hero.x = teleport->targetX;
        hero.y = teleport->targetY;
      }
    }

    turn++;
  }

  if (hero.health<1) {
    if (heroState != HSTATE_DEAD) {
      hero.health = 0;
      responses.push_back(string("You're dead. Game over. (r=restart)"));
      heroState = HSTATE_DEAD;
    }
  }

  // turm passed
  if (turn!=prevTurn) {
    printf("Turn passed! (%d)\n", turn);

    if (rand()%10>8) hero.hungerLevel++;

    if (hero.hungerLevel > 4) {
      if (hero.hungerLevel > 8) {
        responses.push_back(string("You're very hungry."));
        if (rand()%hero.hungerLevel>8) {
          responses.push_back(string("You're starving and lose 1 HP."));
          hero.health--;
        }
      } else {
        responses.push_back(string("Your stomach grumbles."));
      }
    }

    vector<string> critterResponses = moveCritters(currentZone);

    for (auto s : critterResponses) {
      responses.push_back(s);
    }
  }

  return responses;
}

void restart() {
  turn = 0;
  consoleMessages.clear();
  inventory.clear();
  critters.clear();
  zones.clear();

  initHero();
  initZones();
  initCritters();
  currentZone = getZone(SPAWN_ZONE);
  hero.zone = currentZone;

  heroState = HSTATE_MOVE;
}

extern "C" int SDL_main(int argc, char** argv) {
  initTCod();

  TCOD_key_t key{TCODK_NONE,0};
  TCOD_mouse_t mouse;

  restart();

  while (!TCODConsole::isWindowClosed()) {
    renderZone(currentZone);
    renderMessages(&consoleMessages);
    renderInventory();
    renderStats();
    TCODConsole::blit(&tc,0,0,SCREEN_WIDTH,SCREEN_HEIGHT, 
      TCODConsole::root,0,0);

    TCODConsole::flush();

    TCODSystem::checkForEvent((TCOD_event_t)(TCOD_EVENT_KEY_PRESS|TCOD_EVENT_MOUSE),&key,&mouse);

    auto newMessages = processInput(key, mouse);
    for (string msg : newMessages) {
      consoleMessages.push_back(msg);

      if (consoleMessages.size() > MAX_CONSOLE_MSGS) {
        consoleMessages.erase(consoleMessages.begin()+consoleMessages.size()-MAX_CONSOLE_MSGS);
      }
    }
  }

  return 0;
}
