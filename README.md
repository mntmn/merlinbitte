merlinbitte
===========

merlinbitte is a roguelike.

It is based on [libtcod] and written in C++11.

Building
--------

For OSX 10.8, I included binaries of libtcod 1.5.2 (.so files) because I had to create a custom makefile to create them etc. (find it in libtcod_makefile_osx). So just do:
````brew install sdl
make````

On Linux (untested, you probably need GCC 4.7+, or change to clang++ in Makefile):
````Somehow install libtcod
make -f Makefile_linux````

Playing
-------

- cursor keys to move
- t key to take items
- u key to use items from inventory

License
-------

MIT

  [libtcod]: http://doryen.eptalys.net/libtcod/