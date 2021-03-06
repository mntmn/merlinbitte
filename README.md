merlinbitte
===========

merlinbitte is a roguelike.

It is based on [libtcod] and written in C++11.

Building
--------

To clone libtcod, you need [mercurial]. Clone libtcod into a directory that is parallel to the
merlinbitte directory:

    hg clone https://bitbucket.org/jice/libtcod
    git clone git://github.com/mntmn/merlinbitte.git

OSX
---

For OSX 10.9, I included a custom makefile for libtcod 1.5.2: libtcod_makefile_osx. 
    
    brew install sdl
    
    cp ./merlinbitte/libtcod_makefile_osx ./libtcod/
    cd libtcod

Now, comment out line 652 in src/sys_sdl_c.c, the one saying
    
        CustomSDLMain();

So it says

        //CustomSDLMain();

Then, compile libtcod:

    make -f libtcod_makefile_osx
    
    cd ../merlinbitte
    make -f Makefile_osx

Linux
-----

On Linux, you need at least GCC 4.7. This was tested with Debian 7.0 testing and on Arch Linux.

    sudo apt-get install libsdl-dev  # (debian linux)
    sudo pacman install sdl          # (arch linux)
    
    cd libtcod
    make -f makefiles/makefile-linux
    
    cd ../merlinbitte
    make -f Makefile_linux all

Playing
-------

- cursor keys to move
- t key to take items
- u key to use items from inventory (also use for eating)
- m key to manipulate (close doors)
- s key to smash (windows)
- S key to sleep
- w key to wield/wear

Hint to survive:
Find your way out of the apartment and go north in the staircase. Then immediately go left. Don't waste time, find food quickly and eat it. Find clothes and weapons. Find more food. Don't starve!

License
-------

MIT

  [libtcod]: http://doryen.eptalys.net/libtcod/
  [mercurial]: http://mercurial.selenic.com/
