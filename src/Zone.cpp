
#include "Zone.h"

using namespace std;

map<char,Tile> tileDefs = {
  {' ', Tile{' ', 0, "asphalt", TCODColor::black, TCODColor::black}},
  {'w', Tile{' ', 0, "sidewalk", TCODColor::darkestGrey, TCODColor::darkestGrey}},

  {'|', Tile{'|', TILE_BLOCKS, "wall", TCODColor::lightGrey, TCODColor::darkestGrey}},
  {'.', Tile{'.', TILE_BLOCKS, "corner", TCODColor::lightGrey, TCODColor::darkestGrey}},

  {'=', Tile{'=', TILE_BLOCKS, "wall", TCODColor::lightGrey, TCODColor::darkestGrey}},
  {'+', Tile{'+', TILE_BLOCKS, "fence", TCODColor::lightGreen, TCODColor::darkestGrey}},
  
  {'_', Tile{'-', 0, "stairs", TCODColor::darkGrey, TCODColor::black}},
  {'>', Tile{'>', 0, "stairs leading up", TCODColor::yellow, TCODColor::black}},
  {'<', Tile{'<', 0, "stairs leading down", TCODColor::yellow, TCODColor::black}},

  {'T', Tile{'T', 0, "table", TCODColor::darkSepia, TCODColor::darkestSepia}},
  {'#', Tile{'#', 0, "couch", TCODColor::black, TCODColor::darkSepia}},
  
  {'o', Tile{'o', 0, "toilet", TCODColor::white, TCODColor::darkGrey}},
  {'s', Tile{'s', 0, "sink", TCODColor::white, TCODColor::darkGrey}},
  
  {'[', Tile{'[', TILE_LOCKED, "locked door", TCODColor::white, TCODColor::black}},
  {']', Tile{']', TILE_CLOSED, "closed door", TCODColor::white, TCODColor::black}},
  {'/', Tile{'/', 0, "open door", TCODColor::white, TCODColor::black}},
  
  {'-', Tile{'-', 0, "wooden flooring", TCODColor::lightGrey, TCODColor::darkestGrey}},
  {'0', Tile{' ', 0, "marble flooring", TCODColor::darkGrey, TCODColor::darkGrey}},

  {'*', Tile{'*', TILE_BLOCKS, "tree", TCODColor::darkGreen, TCODColor::darkGrey}}
};

map<string,Item> itemDefs = {
  {"apartment keys", Item{',', 0, "apartment keys", 10, TCODColor::white, TCODColor::black}},
  {"1 euro coin", Item{'.', 0, "1 euro coin", 5, TCODColor::yellow, TCODColor::black}},
  {"cigarette butt", Item{'.', 0, "cigarette butt", 2, TCODColor::lightSepia, TCODColor::black}},
  {"empty blister of ibuprofen", Item{',', 0, "empty blister of ibuprofen", 2, TCODColor::lightGrey, TCODColor::black}}
};

Tile Tile::unlock() {
  if (name == "locked door") {
    return tileDefs.at(']');
  }
  return *this;
}

Tile Tile::open() {
  if (name == "closed door") {
    return tileDefs.at('/');
  }
  return *this;
}

Zone::Zone(string title, int w, int h) {
  this->title = title;
  this->width = w;
  this->height = h;
  this->tiles = new Tile[w*h];

  for (int i=0; i<w*h; i++) {
    tiles[i] = Tile{' ', TILE_BLOCKS, "void", TCODColor::black, TCODColor::black};
  }
}

void Zone::mutate(int x, int y, Tile t) {
  tiles[y*width + x] = t;
}

// lifted from http://stackoverflow.com/a/236803
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
      elems.push_back(item);
  }
  return elems;
}
std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

void Zone::load(string filename) {
  tiles = new Tile[width*height];

  ifstream input(filename);

  printf("[Zone] Loading Zone %s: %s\n", title.c_str(), filename.c_str());

  int y = 0;

  #define ZL_TILES 0
  #define ZL_ITEMS 1
  #define ZL_TELEPORTS 2

  int mode = ZL_TILES;

  for (string line; getline(input, line);) {
    printf("[Zone] Read line: %s\n",line.c_str());

    if (line=="!items") {
      mode = ZL_ITEMS;
    } else if (line=="!teleports") {
      mode = ZL_TELEPORTS;
    } else {
      if (mode == ZL_TILES) {
        int x = 0;
        for (char c : line) {
          if (x<width && y<height) {
            if (tileDefs.count(c)==1) {
              Tile t = tileDefs.at(c);
              tiles[y*width + x] = t;
            } else {
              printf("Warning: no tileDef for [%c]!\n",c);
            }
          }
          x++;
        }
        y++;
      }

      if (mode == ZL_ITEMS) {
        // x,y,item name
        auto parts = split(line, ',');
        int x = atoi(parts.at(0).c_str());
        int y = atoi(parts.at(1).c_str());
        string itemId = parts.at(2);

        printf("Adding ZoneItem %s at %d:%d.\n", itemId.c_str(), x, y);
        ZoneItem zi = ZoneItem{x,y,itemDefs.at(itemId)};

        zoneItems.push_back(zi);
      }

      if (mode == ZL_TELEPORTS) {
        // x,y,zone name
        auto parts = split(line, ',');
        int x = atoi(parts.at(0).c_str());
        int y = atoi(parts.at(1).c_str());
        string zoneId = parts.at(2);
        int tx = atoi(parts.at(3).c_str());
        int ty = atoi(parts.at(4).c_str());

        printf("Adding Teleport to %s at %d:%d.\n", zoneId.c_str(), x, y);
        Teleport* t = new Teleport{x,y,zoneId,tx,ty};

        teleports.push_back(t);
      }
    }
  }
}

vector<ZoneItem> Zone::getZoneItems() {
  return zoneItems;
}

vector<Item> Zone::itemsAt(int x, int y) {
  vector<Item> results;

  for (ZoneItem i : zoneItems) {
    if (i.x == x && i.y == y) {
      results.push_back(i.item);
    }
  }
  return results;
}

Tile Zone::tileAt(int x, int y) {
  if (x>=width) return tileDefs.at(' ');
  if (y>=height) return tileDefs.at(' ');
  
  return tiles[y*width + x];
}

Teleport* Zone::teleportAt(int x, int y) {
  if (x>=width || y>=height) return NULL;

  for (Teleport* t : teleports) {
    if (t->x == x && t->y == y) {
      return t;
    }
  }
  return NULL;
}

void Zone::eraseItem(int x, int y, string itemId) {
  int idx = 0;
  for (ZoneItem i : zoneItems) {
    if (i.x == x && i.y == y && i.item.name == itemId) {
      zoneItems.erase(zoneItems.begin() + idx);
      break;
    }
    idx++;
  }
}

Zone::~Zone() {
  delete tiles;
}