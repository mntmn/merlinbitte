#include "Zone.h"

using namespace std;

TDO* defaultTDO = NULL;

map<char,Tile> tileDefs = {
  {' ', Tile{' ', 0, "asphalt", defaultTDO}},
  {'w', Tile{' ', 0, "sidewalk", defaultTDO}},

  {'|', Tile{'|', TILE_BLOCKS, "wall", defaultTDO}},
  {'.', Tile{'.', TILE_BLOCKS, "corner", defaultTDO}},
  {',', Tile{',', 0, "rubble", defaultTDO}},

  {'#', Tile{'#', TILE_BLOCKS | TILE_DESTRUCTIBLE, "window", defaultTDO}},
  {'=', Tile{'=', TILE_BLOCKS, "wall", defaultTDO}},
  {'+', Tile{'+', TILE_BLOCKS, "fence", defaultTDO}},
  {';', Tile{';', 0, "smashed window", defaultTDO}},
  
  {'_', Tile{'-', 0, "stairs", defaultTDO}},
  {'>', Tile{'>', 0, "stairs leading up", defaultTDO}},
  {'<', Tile{'<', 0, "stairs leading down", defaultTDO}},

  {'T', Tile{'T', TILE_DESTRUCTIBLE, "table", defaultTDO}},
  {'C', Tile{'#', TILE_DESTRUCTIBLE, "couch", defaultTDO}},
  
  {'o', Tile{'o', TILE_DESTRUCTIBLE, "toilet", defaultTDO}},
  {'s', Tile{'s', TILE_DESTRUCTIBLE, "sink", defaultTDO}},
  
  {'[', Tile{'[', TILE_LOCKED, "locked door", defaultTDO}},
  {']', Tile{']', TILE_CLOSED, "closed door", defaultTDO}},
  {'/', Tile{'/', TILE_OPEN, "open door", defaultTDO}},
  
  {'-', Tile{'-', 0, "wooden flooring", defaultTDO}},
  {'0', Tile{' ', 0, "marble flooring", defaultTDO}},

  {'*', Tile{'*', TILE_BLOCKS, "tree", defaultTDO}}
};

map<string,Item> itemDefs = {
  {"apartment keys", 
    Item{',', 0, "apartment keys", 
      10, 1, 0, 0, 0,
      defaultTDO}},
  {"1 euro coin", 
    Item{'.', 0, "1 euro coin", 
      5, 0, 0, 0, 0,
      defaultTDO}},
  {"cigarette butt", 
    Item{'.', 0, "cigarette butt", 
      2, 0, 0, 0, 0,
      defaultTDO}},
  {"empty blister of ibuprofen", 
    Item{',', 0, "empty blister of ibuprofen", 
      2, 0, 0, 0, 0,
      defaultTDO}},
  {"newspaper", 
    Item{'n', 0, "newspaper", 
      50, 0, 1, 1, 0,
      defaultTDO}},
  {"ballpen", 
    Item{'.', 0, "ballpen", 
      8, 2, 0, 0, 0,
      defaultTDO}},
  {"mug", 
    Item{'u', 0, "mug", 
      100, 1, 0, 0, 0,
      defaultTDO}},
  {"plate", 
    Item{'_', 0, "plate", 
      100, 1, 0, 0, 0,
      defaultTDO}},
  {"phonebook", 
    Item{'_', 0, "phonebook", 
      250, 1, 0, 1, 0,
      defaultTDO}},
  {"ibuprofen", 
    Item{'.', 0, "ibuprofen", 
      2, 0, 0, 0, 0,
      defaultTDO}},
  {"apple", 
    Item{'o', 0, "apple", 
      75, 0, 0, 3, 0,
      defaultTDO}},
  {"tomato", 
    Item{'o', 0, "tomato", 
      75, 0, 0, 2, 0,
      defaultTDO}},
  {"slice of white bread", 
    Item{'n', 0, "slice of white bread", 
      30, 0, 0, 2, 0,
      defaultTDO}},
  {"blue jeans", 
    Item{'n', 0, "blue jeans", 
      800, 0, 3, 0, 0,
      defaultTDO}},
  {"black jeans", 
    Item{'n', 0, "black jeans", 
      800, 0, 4, 0, 0,
      defaultTDO}},
  {"t-shirt", 
    Item{'T', 0, "t-shirt", 
      300, 0, 1, 0, 0,
      defaultTDO}},
  {"kitchen knife", 
    Item{'k', 0, "kitchen knife", 
      300, 3, 0, 0, 0,
      defaultTDO}},
  {"baseball bat", 
    Item{'b', 0, "baseball bat", 
      1000, 3, 0, 0, 0,
      defaultTDO}},
  {"steelcap doc martens", 
    Item{'k', 0, "steelcap doc martens", 
      1000, 0, 2, 0, 0,
      defaultTDO}},
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

Tile Tile::close() {
  if (name == "open door") {
    return tileDefs.at(']');
  }
  return *this;
}

Tile Tile::lock() {
  if (name == "open door" || name == "closed door") {
    return tileDefs.at('[');
  }
  return *this;
}

Tile Tile::destroy() {
  if (flags & TILE_DESTRUCTIBLE) {
    if (name == "window") {
      return tileDefs.at(';');
    }
    return tileDefs.at(','); // rubble
  }
  return *this;
}

Zone::Zone(string title, int w, int h) {
  this->title = title;
  this->width = w;
  this->height = h;
  this->tiles = new Tile[w*h];

  for (int i=0; i<w*h; i++) {
    tiles[i] = Tile{' ', TILE_BLOCKS, "void", defaultTDO};
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
    //printf("[Zone] Read line: %s\n",line.c_str());

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

        //printf("Adding Teleport to %s at %d:%d.\n", zoneId.c_str(), x, y);
        Teleport* t = new Teleport{x,y,zoneId,tx,ty};

        teleports.push_back(t);
      }
    }
  }
}

void Zone::addTeleport(Teleport* t) {
  teleports.push_back(t);
}

void Zone::generateHouses(int ox, int oy, int w, int h, int mind, int maxdx, int maxdy) {
  int ydiv = rand()%maxdy + mind;
  int pad = 2;
  
  for (int y=0; y<ydiv; y++) {
    int xdiv = rand()%maxdx + mind;
    for (int x=0; x<xdiv; x++) {

      int x1 = ox + ((w-1)/xdiv)*x+pad + rand()%2;
      int y1 = oy + ((h-1)/ydiv)*y+pad + rand()%2;
      int x2 = x1+(w-2)/xdiv-2*pad - rand()%2;
      int y2 = y1+(h-2)/ydiv-2*pad - rand()%2;

      if (rand()%4) {

        // fill with junk

        int floorType = rand()%2;
        for (int hy=y1+1; hy<y2; hy++) {
          for (int hx=x1+1; hx<x2; hx++) {
            if (floorType) {
              mutate(hx,hy,tileDefs.at('0')); // marble
            } else {
              mutate(hx,hy,tileDefs.at('-')); // wood
            }

            string items[] = {
              "ballpen",
              "mug",
              "plate",
              "phonebook",
              "ibuprofen",
              "blue jeans",
              "black jeans",
              "steelcap doc martens",
              "t-shirt",
              "tomato",
              "apple",
              "slice of white bread",
              "kitchen knife",
              "baseball bat"
            };

            const int itemCount = sizeof(items)/sizeof(*items);
            const int itemR = rand()%(itemCount+80);
            if (itemR<itemCount) {

              //printf("item: %d %s\n",itemR,items[itemR].c_str());

              Item item = itemDefs.at(items[itemR]);

              ZoneItem zi = ZoneItem{hx,hy,item};
              zoneItems.push_back(zi);
            }
          }
        }

        int WINDOW_CHANCE = 8;

        // house rect
        for (int hx=x1; hx<=x2; hx++) {
          auto tile = tileDefs.at('=');
          if (rand()%10>WINDOW_CHANCE) tile = tileDefs.at('#');
          mutate(hx,y1,tile);
          tile = tileDefs.at('=');
          if (rand()%10>WINDOW_CHANCE) tile = tileDefs.at('#');
          mutate(hx,y2,tile);
        }

        for (int hy=y1; hy<=y2; hy++) {
          auto tile = tileDefs.at('|');
          if (rand()%10>WINDOW_CHANCE) tile = tileDefs.at('#');
          mutate(x1,hy,tile);
          tile = tileDefs.at('|');
          if (rand()%10>WINDOW_CHANCE) tile = tileDefs.at('#');
          mutate(x2,hy,tile);
        }

        // corners
        mutate(x1,y1,tileDefs.at('.'));
        mutate(x2,y1,tileDefs.at('.'));
        mutate(x1,y2,tileDefs.at('.'));
        mutate(x2,y2,tileDefs.at('.'));

        // door
        mutate((x1+x2)/2,y2,tileDefs.at(']'));
      }

    }
  }
}

void Zone::generate(char zoneType) {

  //printf("[Zone] generating zone of type %c\n",zoneType);

  for (int y=0; y<height; y++) {
    for (int x=0; x<width; x++) {
      mutate(x,y,tileDefs.at('w'));
    }
  }

  int roadWidth=8;

  if (zoneType=='|') {
    generateHouses(0,0,width/2-roadWidth/2,height,1,1,2);
    generateHouses(width/2+roadWidth/2,0,width/2-roadWidth/2,height,1,1,2);
  }

  if (zoneType=='-') {
    generateHouses(0,0,width,height/2-roadWidth/2,1,2,1);
    generateHouses(0,height/2+roadWidth/2,width,height/2-roadWidth/2,1,2,1);
  }

  if (zoneType=='+') {
    generateHouses(0,0,width/2-roadWidth/2,height/2-roadWidth/2,1,1,1);
    generateHouses(width/2+roadWidth/2,0,width/2-roadWidth/2,height/2-roadWidth/2,1,1,1);
    generateHouses(0,height/2+roadWidth/2,width/2-roadWidth/2,height/2-roadWidth/2,1,1,1);
    generateHouses(width/2+roadWidth/2,height/2+roadWidth/2,width/2-roadWidth/2,height/2-roadWidth/2,1,1,1);
  }

  if (zoneType=='|' || zoneType=='+') {
    for (int y=0; y<height; y++) {
      for (int x=width/2-roadWidth/2; x<width/2+roadWidth/2; x++) {
        mutate(x,y,tileDefs.at(' '));
      }
    }
  }

  if (zoneType=='-' || zoneType=='+') {

    for (int x=0; x<width; x++) {
      for (int y=height/2-roadWidth/2; y<height/2+roadWidth/2; y++) {
        mutate(x,y,tileDefs.at(' '));
      }
    }
  }

  if (zoneType=='R') {
    // generate residential houses
    generateHouses(0,0,width,height,1,3,3);
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

/*
void Zone::updateTcodMap() {
  this->tcodMap = new TCODMap(this->width, this->height);

  for (int x=0; x<this->width; x++) {
    for (int y=0; y<this->height; y++) {
      Tile t = this->tileAt(x,y);

      bool walkable = !(t.flags & (TILE_BLOCKS|TILE_CLOSED|TILE_LOCKED));

      this->tcodMap->setProperties(x, y, walkable, walkable);
    }
  }
}

TCODMap* Zone::getTcodMap() {
  if (!this->tcodMap) {
    updateTcodMap();
  }
  return this->tcodMap;
}*/

Zone::~Zone() {
  delete tiles;
}