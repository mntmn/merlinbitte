# merlinbitte Mac OSX makefile
SRCDIR=./src
INCDIR=include
TCODINCLUDE=../libtcod/include
CFLAGS=$(FLAGS) -I$(INCDIR) -I$(TCODINCLUDE) -I$(SRCDIR)
LDFLAGS= -framework Cocoa -lSDL -L./ -ltcodxx_debug -lm 
CC=gcc
CPP=clang++ -std=c++11 -stdlib=libc++ 
DMD=dmd
PROGNAME=merlinbitte
CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(patsubst src/%.cpp,obj/%.o,$(CPP_FILES))
.SUFFIXES: .o .h .c .hpp .cpp .m

obj/%.o : $(SRCDIR)/%.cpp
	$(CPP) $(CFLAGS) -g -o $@ -c $< 

obj/%.o : $(SRCDIR)/%.m
	$(CC) $(CFLAGS) -g -o $@ -c $< 

C_OBJS_DEBUG=$(OBJ_FILES) obj/SDLMain.o

all : debug

debug : $(PROGNAME)

$(TEMP) :
	\mkdir -p $@

$(PROGNAME) : $(TEMP) $(C_OBJS_DEBUG)
	$(CPP) $(C_OBJS_DEBUG) $(CFLAGS) -o $@ $(LDFLAGS) $(CFLAGS) -Wl,-rpath,.

clean :
	\rm -f $(PROGNAME) $(C_OBJS_DEBUG)

