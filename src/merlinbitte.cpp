#include <string>
#include <map>
#include <vector>
#include "libtcod.hpp"
#include "Player.h"
#include "Zone.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 60
#define SPAWN_ZONE "apartment"
#define MAX_CONSOLE_MSGS 10

#define HSTATE_MOVE 0
#define HSTATE_USE_SELECT 1
#define HSTATE_USE_DIRECTION 2

using namespace std;

TCODConsole tc(SCREEN_WIDTH, SCREEN_HEIGHT);

map<string, Zone*> zones;
Player hero{"mntmn", 10, 10};
vector<Item> inventory;
Zone* currentZone;
int heroState = HSTATE_MOVE;
int selectedItemId = 0;

void renderZone(Zone* zone) {

	tc.clear();
	tc.setDefaultForeground(TCODColor::white);

	// tiles
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			Tile t = zone->tileAt(x, y);

			tc.putCharEx(x, y, t.consoleChar, t.fg, t.bg);
		}
	}

	// items
	for (ZoneItem zi : zone->getZoneItems()) {
		Item item = zi.item;
		tc.putCharEx(zi.x, zi.y, item.consoleChar, item.fg, item.bg);
	}

	tc.setDefaultForeground(TCODColor::white);

	tc.putChar(hero.x,hero.y,'@',TCOD_BKGND_NONE);

	auto playerTile = zone->tileAt(hero.x,hero.y);

	tc.print(1,1,zone->title.c_str());
	tc.print(20,1,playerTile.name.c_str());
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
		tc.print(SCREEN_WIDTH-20,y++, (to_string(idx++)+": "+item.name).c_str());
	}
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
	auto zoneIds={"apartment","backyard"};
	for (auto id : zoneIds) {
		auto z = new Zone(id,40,30);
		z->load(string("zones/") + string(id) + string(".txt"));
		zones.emplace(id, z);
	}
}

Zone* getZone(string zoneName) {
	return zones.at(zoneName);
}

vector<string> processInput(TCOD_key_t key, TCOD_mouse_t mouse) {

	vector<string> responses;

	int newHeroX = hero.x;
	int newHeroY = hero.y;
	bool moveHero = false;

	if (heroState == HSTATE_MOVE) {
		// move state

		if (key.vk == TCODK_DOWN) {
			newHeroY++;
		} else if (key.vk == TCODK_UP) {
			newHeroY--;
		} else if (key.vk == TCODK_LEFT) {
			newHeroX--;
		} else if (key.vk == TCODK_RIGHT) {
			newHeroX++;
		}

		if (key.c == 't') {
			// take item
			vector<Item> heroItems = currentZone->itemsAt(hero.x, hero.y);
			for (Item item : heroItems) {
				responses.push_back(string("You take the ") + item.name);
				inventory.push_back(item);
				currentZone->eraseItem(hero.x, hero.y, item.name);
			}
		}

		if (key.c == 'u') {
			// use item
			responses.push_back(string("Which item do you want to use? (0-9)"));
			heroState = HSTATE_USE_SELECT;
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
				responses.push_back(string("Where do you want to use ")+item.name+"?");
				heroState = HSTATE_USE_DIRECTION;
			}
		}

	} else if (heroState == HSTATE_USE_DIRECTION) {
		int useX = hero.x;
		int useY = hero.y;

		if (key.vk == TCODK_DOWN) {
			useY++;
		} else if (key.vk == TCODK_UP) {
			useY--;
		} else if (key.vk == TCODK_LEFT) {
			useX--;
		} else if (key.vk == TCODK_RIGHT) {
			useX++;
		}

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
		}
	}

	Tile newHeroTile = currentZone->tileAt(newHeroX, newHeroY);

	if (newHeroTile.flags & TILE_BLOCKS || newHeroTile.flags & TILE_LOCKED) {
		responses.push_back(string("Your path is blocked by a ") + newHeroTile.name + string("."));
	} else if (newHeroTile.flags & TILE_CLOSED) {
		responses.push_back(string("You open the ") + newHeroTile.name + string("."));
		Tile openedTile = newHeroTile.open();

		currentZone->mutate(newHeroX, newHeroY, openedTile);
	} else {
		moveHero = true;
	}

	if (moveHero && (hero.x!=newHeroX || hero.y!=newHeroY)) {
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

	return responses;
}


extern "C" int SDL_main(int argc, char** argv) {
	initTCod();

	TCOD_key_t key{TCODK_NONE,0};
	TCOD_mouse_t mouse;

	initZones();
	currentZone = getZone(SPAWN_ZONE);

	vector<string> messages = {};

	while (!TCODConsole::isWindowClosed()) {
		renderZone(currentZone);
		renderMessages(&messages);
		renderInventory();
		TCODConsole::blit(&tc,0,0,SCREEN_WIDTH,SCREEN_HEIGHT, 
			TCODConsole::root,0,0);

		TCODConsole::flush();

		TCODSystem::checkForEvent((TCOD_event_t)(TCOD_EVENT_KEY_PRESS|TCOD_EVENT_MOUSE),&key,&mouse);

		auto newMessages = processInput(key, mouse);
		for (string msg : newMessages) {
			messages.push_back(msg);

			if (messages.size() > MAX_CONSOLE_MSGS) {
				messages.erase(messages.begin()+messages.size()-MAX_CONSOLE_MSGS);
			}
		}
	}

	return 0;
}