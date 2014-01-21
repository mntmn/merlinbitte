#include <string>
#include <string.h>
#include <map>
#include <vector>
#include "Zone.h"
#include "Critter.h"
#include <GL/glut.h>

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

      /*auto path = new TCODPath(playerZone->getTcodMap(), 1.0f); 
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
      }*/
    }
  }

  return responses;
}

void renderCritters(Zone* zone) {
  for (Critter* c : critters) {
    if (c->zone == zone) {
      //tc.putCharEx(c->x, c->y + yOffset, c->consoleChar, c->fg, c->bg);
    }
  }
}

void renderZone(Zone* zone) {

  // tiles
  for (int y = 0; y < SCREEN_HEIGHT-yOffset; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      Tile t = zone->tileAt(x, y);
      if (t.consoleChar>0) {
        //tc.putCharEx(x, y + yOffset, t.consoleChar, t.fg, t.bg);
      }
    }
  }

  // items
  for (ZoneItem zi : zone->getZoneItems()) {
    Item item = zi.item;
    //tc.putCharEx(zi.x, zi.y+yOffset, item.consoleChar, item.fg, item.bg);
  }

  //tc.putChar(hero.x,hero.y+yOffset,'@',TCOD_BKGND_NONE);

  auto playerTile = zone->tileAt(hero.x,hero.y);

  //tc.print(1,1,zone->title.c_str());
  //tc.print(20,1,playerTile.name.c_str());

  renderCritters(zone);
}

void renderMessages(vector<string>* messages) {
  int y = SCREEN_HEIGHT-MAX_CONSOLE_MSGS-1;
  for (string m : *messages) {
    //tc.print(1,y++,m.c_str());
  }
}

void renderInventory() {
  //tc.print(SCREEN_WIDTH-20,1,"Inventory");
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
    //tc.print(SCREEN_WIDTH-20,y++, (to_string(idx++)+": "+mod+item.name).c_str());
  }
}

void renderStats() {
  int x = SCREEN_WIDTH-35;

  //tc.print(x,1,"Stats");

  int minutesTotal = turn/5;
  int hoursTotal = (minutesTotal/60);
  int hours = hoursTotal%24;
  int day = hoursTotal/24 + 1;

  /*tc.print(x,3,(string("Day ") + to_string(day) + " " + to_string(hoursTotal%24) + ":" + to_string(minutesTotal%60)).c_str());
  tc.print(x,4,(string("Health: ") + to_string(hero.health)).c_str());
  tc.print(x,5,(string("INT: ") + to_string(hero.intelligence)).c_str());
  tc.print(x,6,(string("STR: ") + to_string(hero.strength)).c_str());
  tc.print(x,7,(string("AGL: ") + to_string(hero.agility)).c_str());
  tc.print(x,8,(string("WPR: ") + to_string(hero.willpower)).c_str());

  tc.print(x,10,(string("ARM: ") + to_string(hero.armorValue())).c_str());
  tc.print(x,11,(string("DMG: ") + to_string(hero.damageValue())).c_str());*/
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
            //printf("zid %s top linked to %s\n",zid.c_str(), targetId.c_str());
          } else {
            targetId = "Sector " + to_string(x) + "/" + to_string(y-1);
          }

          Teleport* t = new Teleport{tx,0,targetId,tx,h-2};
          z->addTeleport(t);
        }
        if (toBottom!=0) {
          if (toBottom == '1') {
            targetId = "backyard";
            //printf("zid %s bottom linked to %s\n",zid.c_str(), targetId.c_str());
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
            //printf("zid %s left linked to %s\n",zid.c_str(), targetId.c_str());
          } else {
            targetId = "Sector " + to_string(x-1) + "/" + to_string(y);
          }
          Teleport* t = new Teleport{0,ty,targetId,w-2,ty};
          z->addTeleport(t);
        }
        if (toRight!=0) {
          if (toRight == '1') {
            targetId = "backyard";
            //printf("zid %s right linked to %s\n",zid.c_str(), targetId.c_str());
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

/*
int getKeyX(TCOD_key_t key) {
  if (key.vk == TCODK_LEFT) return -1;
  if (key.vk == TCODK_RIGHT) return 1;
  return 0;
}

int getKeyY(TCOD_key_t key) {
  if (key.vk == TCODK_UP) return -1;
  if (key.vk == TCODK_DOWN) return 1;
  return 0;
}*/

vector<string> processInput() {

  vector<string> responses;

  int newHeroX = hero.x;
  int newHeroY = hero.y;
  bool moveHero = false;

  int prevTurn = turn;

  int keyCode = 0;

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

    //newHeroY += getKeyY(key);
    //newHeroX += getKeyX(key);

    if (keyCode == 't') {
      // take item
      vector<Item> heroItems = currentZone->itemsAt(hero.x, hero.y);
      for (Item item : heroItems) {
        responses.push_back(string("You take the ") + item.name);
        inventory.push_back(item);
        currentZone->eraseItem(hero.x, hero.y, item.name);
      }

      turn++;
    }

    if (keyCode == 'u') {
      // use item
      responses.push_back(string("Which item do you want to use? (0-9)"));
      heroState = HSTATE_USE_SELECT;
    }

    if (keyCode == 'm') {
      // manipulate tile
      responses.push_back(string("What do you want to manipulate (direction)?"));
      heroState = HSTATE_MANIPULATE_DIRECTION;
    }

    if (keyCode == 's') {
      // smash tile
      responses.push_back(string("What do you want to smash (direction)?"));
      heroState = HSTATE_SMASH_DIRECTION;
    }

    if (keyCode == 'S') {
      // smash tile
      responses.push_back(string("You fall asleep..."));
      heroState = HSTATE_SLEEP;
      hero.sleepTurns = 10;
    }

    if (keyCode == 'w') {
      // manipulate tile
      responses.push_back(string("What do you want wield/wear (0-9)?"));
      heroState = HSTATE_WIELD_SELECT;
    }

  } else if (heroState == HSTATE_USE_SELECT) {

    if (keyCode >= '0' && keyCode <= '9') {
      int itemNumber = keyCode - '0';

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

    if (keyCode >= '0' && keyCode <= '9') {
      int itemNumber = keyCode - '0';

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

    //useY += getKeyY(key);
    //useX += getKeyX(key);

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

    //useY += getKeyY(key);
    //useX += getKeyX(key);

    if (useX!=hero.x || useY!=hero.y) {

      Tile useTile = currentZone->tileAt(useX, useY);

      if (useTile.flags & TILE_OPEN) {
        responses.push_back(string("You close the ") + useTile.name + string("."));
        Tile closedTile = useTile.close();

        currentZone->mutate(useX, useY, closedTile);
        //currentZone->updateTcodMap();
      }

      heroState = HSTATE_MOVE;
      turn++;
    }
  } else if (heroState == HSTATE_SMASH_DIRECTION) {
    int useX = hero.x; // + getKeyX(key);
    int useY = hero.y; // + getKeyY(key);

    if (useX!=hero.x || useY!=hero.y) {
      Tile useTile = currentZone->tileAt(useX, useY);

      if (useTile.flags & TILE_DESTRUCTIBLE) {
        responses.push_back(string("You smash the ") + useTile.name + string(" into pieces."));
        Tile smashedTile = useTile.destroy();

        currentZone->mutate(useX, useY, smashedTile);
        //currentZone->updateTcodMap();
      } else {
        responses.push_back(string("You can't seem to destroy the ") + useTile.name + string("."));
      }

      heroState = HSTATE_MOVE;
      turn++;
    }
  } else if (heroState == HSTATE_DEAD) {
    if (keyCode == 'r') {
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
    //currentZone->updateTcodMap();
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

GLuint vShaderProg;
GLuint vShaderOb;
GLuint fShaderOb;

void compileShader(const GLchar* text, GLuint shob, GLuint shprog) {

  GLint success;
  GLchar glLog[1024];

  const GLchar* p[1];
  p[0] = text;
  GLint lengths[1];
  lengths[0] = strlen(text);
  glShaderSource(shob, 1, p, lengths);

  glCompileShader(shob);

  glGetShaderiv(shob, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shob, sizeof(glLog), NULL, glLog);
    fprintf(stderr, "Error compiling shader: '%s'\n", glLog);
    exit(1);
  }

  glAttachShader(shprog, shob);
}

void initShaders() {
  vShaderProg = glCreateProgram();
  vShaderOb = glCreateShader(GL_VERTEX_SHADER);
  compileShader("\n\
    #version 100\n\
    attribute vec4 pos;\n\
    varying float tile;\n\
    void main() {\n\
      gl_Position = vec4(pos.x-0.75, (1.0-pos.y)-0.5, pos.z, 1.0);\n\
      tile = pos.w;\n\
    }\n\
    ", vShaderOb, vShaderProg);

  //fShaderProg = glCreateProgram();
  fShaderOb = glCreateShader(GL_FRAGMENT_SHADER);
  compileShader("\n\
    #version 100\n\
    precision lowp float;\n\
    varying float tile;\n\
    void main(void) {\n\
      vec2 res = vec2(1024.0,768.0);\n\
      gl_FragColor = vec4(0,0, tile/128.0, 1.0);\n\
    }\n\
    ", fShaderOb, vShaderProg);


  GLint success;
  GLchar glLog[1024];
  glLinkProgram(vShaderProg);

  glGetProgramiv(vShaderProg, GL_LINK_STATUS, &success);
  if (success == 0) {
    glGetProgramInfoLog(vShaderProg, sizeof(glLog), NULL, glLog);
    fprintf(stderr, "Error linking shader program: '%s'\n", glLog);
    exit(1);
  }

  glValidateProgram(vShaderProg);

  glUseProgram(vShaderProg);
}


#define NUM_TILE_VECS SCREEN_WIDTH*SCREEN_HEIGHT*4
Vector4f tileVecs[NUM_TILE_VECS];


void renderScene() {
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  float z = 0.0f;
  float tw = 0.02f;
  float th = 0.02f;
  float pad = 0.005f;

  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      Tile t = currentZone->tileAt(x, y);
      //if (t!=NULL) {
        //t.consoleChar;
      //}

      float c = (float)t.consoleChar;

      int o = (y*SCREEN_WIDTH + x)*4;

      tileVecs[o+0] = {tw*x,th*y,z, c};
      tileVecs[o+1] = {tw*x+tw-pad,th*y,z, c};
      tileVecs[o+2] = {tw*x+tw-pad,th*y+th-pad,z, c};
      tileVecs[o+3] = {tw*x,th*y+th-pad,z, c};
    }
  }

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(tileVecs), tileVecs, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

  glDrawArrays(GL_QUADS, 0, NUM_TILE_VECS);

  glDisableVertexAttribArray(0);

  glutSwapBuffers();
}

void handleMouseMove(int x, int y) {
  
}

int main(int argc, char** argv) {

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowSize(1024, 768);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("merlinbitte");


  GLenum res = glewInit();
  if (res != GLEW_OK)
  {
      fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
      return 1;
  }

  initShaders();

  glutDisplayFunc(renderScene);
  
  restart();

  glutMainLoop();

  while (true) {
    renderZone(currentZone);
    renderMessages(&consoleMessages);
    renderInventory();
    renderStats();

    /*auto newMessages = processInput();
    for (string msg : newMessages) {
      consoleMessages.push_back(msg);

      if (consoleMessages.size() > MAX_CONSOLE_MSGS) {
        consoleMessages.erase(consoleMessages.begin()+consoleMessages.size()-MAX_CONSOLE_MSGS);
      }
    }*/
  }

  return 0;
}
