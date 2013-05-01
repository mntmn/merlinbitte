merlinbitte
===========

merlinbitte is a roguelike.

It is based on [libtcod] and written in C++11.

Building
--------

To clone libtcod, you need [mercurial] (hg). Clone libtcod into a directory that is parallel to the
merlinbitte directory:

    hg clone ssh://hg@bitbucket.org/jice/libtcod

For OSX 10.8, I included a custom makefile for libtcod 1.5.2: libtcod_makefile_osx. 
    
    cd libtcod
    brew install sdl
    make -f libtcod_makefile_osx
    cd ../merlinbitte
    make -f Makefile_osx

On Linux, you need at least GCC 4.7. This was tested with Debian 7.0 testing.

    cd libtcod
    sudo apt-get install libsdl-dev
    make -f makefiles/makefile-linux
    cd ../merlinbitte
    make -f Makefile_linux

Playing
-------

- cursor keys to move
- t key to take items
- u key to use items from inventory

License
-------

MIT

  [libtcod]: http://doryen.eptalys.net/libtcod/
