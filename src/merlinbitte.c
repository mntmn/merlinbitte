/*
 * libtcod C samples
 * This code demonstrates various usages of libtcod modules
 * It's in the public domain.
 */

// uncomment this to disable SDL sample (might cause compilation issues on some systems)
//#define NO_SDL_SAMPLE

#include <stdlib.h> /* for NULL */
#include <string.h>
#include <stdio.h>
#include "libtcod.h"
#define _SDL_main_h
#include <SDL/SDL.h>
#include <math.h>

/* a sample has a name and a rendering function */
typedef struct {
	char name[64];
	void (*render)(bool first, TCOD_key_t*key, TCOD_mouse_t *mouse);
} sample_t;

/* sample screen size */
#define SAMPLE_SCREEN_WIDTH 46
#define SAMPLE_SCREEN_HEIGHT 20
/* sample screen position */
#define SAMPLE_SCREEN_X 4
#define SAMPLE_SCREEN_Y 4

/* ***************************
 * samples rendering functions
 * ***************************/

/* the offscreen console in which the samples are rendered */
TCOD_console_t sample_console;

/* ***************************
 * fov sample
 * ***************************/
void render_fov(bool first, TCOD_key_t*key, TCOD_mouse_t *mouse) {
	static char *smap[] = {
		"##############################################",
		"##############################################",
		"##############################################",
		"##############################################",
		"##############################################",
		"##############################################",
		"##############################################",
		"####################   ###                  ##",
		"####################   ###   ;;;            ##",
		"####################   ###                  ##",
		"####################   ###        ##        ##",
		"#####################=####        ##        ##",
		"###################=                        ##",
		"##############################################",
		"##############################################",
		"##############################################",
		"##############################################",
		"##############################################",
		"##############################################",
		"##############################################",
	};

	#define TORCH_RADIUS 5.0f
	#define SQUARED_TORCH_RADIUS (TORCH_RADIUS*TORCH_RADIUS)

	static int px=38,py=10; /* player position */
	static bool recompute_fov=true;
	static bool torch=true;
	static bool light_walls=true;
	static TCOD_map_t map=NULL;
	static TCOD_color_t dark_wall={0,0,100};
	static TCOD_color_t light_wall={130,110,50};
	static TCOD_color_t dark_ground={50,50,150};
	static TCOD_color_t light_ground={200,180,50};
	static TCOD_noise_t noise;
	static int algonum=0;
	static float torchx=0.0f; /* torch light position in the perlin noise */
	int x,y;
	/* torch position & intensity variation */
	float dx=0.0f,dy=0.0f,di=0.0f;

	if ( ! map) {
		map = TCOD_map_new(SAMPLE_SCREEN_WIDTH,SAMPLE_SCREEN_HEIGHT);
		for (y=0; y < SAMPLE_SCREEN_HEIGHT; y++ ) {
			for (x=0; x < SAMPLE_SCREEN_WIDTH; x++ ) {
				if ( smap[y][x] == ' ' ) TCOD_map_set_properties(map,x,y,true,true); /* ground */
				else if ( smap[y][x] == '=' ) TCOD_map_set_properties(map,x,y,true,false); /* window */
			}
		}
		noise=TCOD_noise_new(1,1.0f,1.0f,NULL); /* 1d noise for the torch flickering */
	}
	
	if ( first ) {
		TCOD_sys_set_fps(30); /* limited to 30 fps */
		/* we draw the foreground only the first time.
		   during the player movement, only the @ is redrawn.
		   the rest impacts only the background color */
		/* draw the help text & player @ */
		TCOD_console_clear(sample_console);
		TCOD_console_set_default_foreground(sample_console,TCOD_white);
		/*TCOD_console_print(sample_console,1,0,"IJKL : move around\nT : torch fx %s\nW : light walls %s\n+-: algo %s",
			torch ? "on " : "off", light_walls ? "on "  : "off", algo_names[algonum]);*/
		TCOD_console_set_default_foreground(sample_console,TCOD_black);
		TCOD_console_put_char(sample_console,px,py,'@',TCOD_BKGND_NONE);
		/* draw windows */
		for (y=0; y < SAMPLE_SCREEN_HEIGHT; y++ ) {
			for (x=0; x < SAMPLE_SCREEN_WIDTH; x++ ) {
				if ( smap[y][x] == '=' ) {
					TCOD_console_put_char(sample_console,x,y,TCOD_CHAR_DHLINE,TCOD_BKGND_NONE);
				} else if ( smap[y][x] != '#' ) {
					TCOD_console_put_char(sample_console,x,y,smap[y][x],TCOD_BKGND_NONE);
				}
			}
		}
	}
	
	if ( recompute_fov ) {
		recompute_fov=false;
		TCOD_map_compute_fov(map,px,py,torch ? (int)(TORCH_RADIUS) : 0, light_walls, (TCOD_fov_algorithm_t)algonum);
	}

	if ( torch ) {
	    float tdx;
		/* slightly change the perlin noise parameter */
		torchx+=0.2f;
		/* randomize the light position between -1.5 and 1.5 */
		tdx=torchx+20.0f;
		dx = TCOD_noise_get(noise,&tdx)*1.5f;
		tdx += 30.0f;
		dy = TCOD_noise_get(noise,&tdx)*1.5f;
		di = 0.2f * TCOD_noise_get(noise,&torchx);
	}

	for (y=0; y < SAMPLE_SCREEN_HEIGHT; y++ ) {
		for (x=0; x < SAMPLE_SCREEN_WIDTH; x++ ) {
			bool visible = TCOD_map_is_in_fov(map,x,y);
			bool wall=smap[y][x]=='#';
			if (! visible ) {
				TCOD_console_set_char_background(sample_console,x,y,
					wall ? dark_wall:dark_ground,TCOD_BKGND_SET);

			} else {
				if ( !torch ) {
					TCOD_console_set_char_background(sample_console,x,y,
					 wall ? light_wall : light_ground, TCOD_BKGND_SET );
				} else {
					TCOD_color_t base=(wall ? dark_wall : dark_ground);
					TCOD_color_t light=(wall ? light_wall : light_ground);
					float r=(x-px+dx)*(x-px+dx)+(y-py+dy)*(y-py+dy); /* cell distance to torch (squared) */
					if ( r < SQUARED_TORCH_RADIUS ) {
						float l = (SQUARED_TORCH_RADIUS-r)/SQUARED_TORCH_RADIUS+di;
						l=CLAMP(0.0f,1.0f,l);
						base=TCOD_color_lerp(base,light,l);
					}
					TCOD_console_set_char_background(sample_console,x,y,base,TCOD_BKGND_SET);
				}
			}
		}
	}

	if ( key->c == 'I' || key->c == 'i' || key->vk == TCODK_UP) {
		if ( smap[py-1][px] == ' ' ) {
			TCOD_console_put_char(sample_console,px,py,' ',TCOD_BKGND_NONE);
			py--;
			TCOD_console_put_char(sample_console,px,py,'@',TCOD_BKGND_NONE);
			recompute_fov=true;
		}
	} else if ( key->c == 'K' || key->c == 'k' || key->vk == TCODK_DOWN) {
		if ( smap[py+1][px] == ' ' ) {
			TCOD_console_put_char(sample_console,px,py,' ',TCOD_BKGND_NONE);
			py++;
			TCOD_console_put_char(sample_console,px,py,'@',TCOD_BKGND_NONE);
			recompute_fov=true;
		}
	} else if ( key->c == 'J' || key->c == 'j' || key->vk == TCODK_LEFT ) {
		if ( smap[py][px-1] == ' ' ) {
			TCOD_console_put_char(sample_console,px,py,' ',TCOD_BKGND_NONE);
			px--;
			TCOD_console_put_char(sample_console,px,py,'@',TCOD_BKGND_NONE);
			recompute_fov=true;
		}
	} else if ( key->c == 'L' || key->c == 'l' || key->vk == TCODK_RIGHT ) {
		if ( smap[py][px+1] == ' ' ) {
			TCOD_console_put_char(sample_console,px,py,' ',TCOD_BKGND_NONE);
			px++;
			TCOD_console_put_char(sample_console,px,py,'@',TCOD_BKGND_NONE);
			recompute_fov=true;
		}
	} else if ( key->c == 'T' || key->c == 't' ) {
		torch=!torch;
		TCOD_console_set_default_foreground(sample_console,TCOD_white);
		TCOD_console_set_default_foreground(sample_console,TCOD_black);
	} else if ( key->c == 'W' || key->c == 'w' ) {
		light_walls=!light_walls;
		TCOD_console_set_default_foreground(sample_console,TCOD_white);
		TCOD_console_set_default_foreground(sample_console,TCOD_black);
		recompute_fov=true;
	} else if ( key->c == '+' || key->c == '-' ) {
		algonum+= key->c == '+' ? 1 : -1;
		algonum = CLAMP(0,NB_FOV_ALGORITHMS-1,algonum);
		TCOD_console_set_default_foreground(sample_console,TCOD_white);
		TCOD_console_set_default_foreground(sample_console,TCOD_black);
		recompute_fov=true;
	}
}


/* ***************************
 * the list of samples
 * ***************************/
sample_t samples[] = {
	{"  Field of view      ",render_fov}
};
int nb_samples = sizeof(samples)/sizeof(sample_t); /* total number of samples */

/* ***************************
 * the main function
 * ***************************/

int SDL_main( int argc, char *argv[] ) {
	printf("SDLMain!\n");

	int cur_sample=0; /* index of the current sample */
	bool first=true; /* first time we render a sample */
	int i;
	TCOD_key_t key = {TCODK_NONE,0};
	TCOD_mouse_t mouse;
	char *font="data/fonts/consolas10x10_gs_tc.png";
	int nb_char_horiz=0,nb_char_vertic=0;
	int argn;
	int fullscreen_width=0;
	int fullscreen_height=0;
	int font_flags=TCOD_FONT_TYPE_GREYSCALE|TCOD_FONT_LAYOUT_TCOD;
	int font_new_flags=0;
	TCOD_renderer_t renderer=TCOD_RENDERER_SDL;
	bool fullscreen=false;
	int cur_renderer=0;

	/* initialize the root console (open the game window) */
	for (argn=1; argn < argc; argn++) {
		if ( strcmp(argv[argn],"-font") == 0 && argn+1 < argc) {
			argn++;
			font=argv[argn];
			font_flags=0;
		} else if ( strcmp(argv[argn],"-font-nb-char") == 0 && argn+2 < argc ) {
			argn++;
			nb_char_horiz=atoi(argv[argn]);
			argn++;
			nb_char_vertic=atoi(argv[argn]);
			font_flags=0;
		} else if ( strcmp(argv[argn],"-fullscreen-resolution") == 0 && argn+2 < argc ) {
			argn++;
			fullscreen_width=atoi(argv[argn]);
			argn++;
			fullscreen_height=atoi(argv[argn]);
		} else if ( strcmp(argv[argn],"-renderer") == 0 && argn+1 < argc ) {
			argn++;
			renderer=(TCOD_renderer_t)atoi(argv[argn]);
		} else if ( strcmp(argv[argn],"-fullscreen") == 0 ) {
			fullscreen=true;
		} else if ( strcmp(argv[argn],"-font-in-row") == 0 ) {
			font_flags=0;
			font_new_flags |= TCOD_FONT_LAYOUT_ASCII_INROW;
		} else if ( strcmp(argv[argn],"-font-greyscale") == 0 ) {
			font_flags=0;
			font_new_flags |= TCOD_FONT_TYPE_GREYSCALE;
		} else if ( strcmp(argv[argn],"-font-tcod") == 0 ) {
			font_flags=0;
			font_new_flags |= TCOD_FONT_LAYOUT_TCOD;
		} else if ( strcmp(argv[argn],"-help") == 0 || strcmp(argv[argn],"-?") == 0) {
			printf ("options :\n");
			printf ("-font <filename> : use a custom font\n");
			printf ("-font-nb-char <nb_char_horiz> <nb_char_vertic> : number of characters in the font\n");
			printf ("-font-in-row : the font layout is in row instead of columns\n");
			printf ("-font-tcod : the font uses TCOD layout instead of ASCII\n");
			printf ("-font-greyscale : antialiased font using greyscale bitmap\n");
			printf ("-fullscreen : start in fullscreen\n");
			printf ("-fullscreen-resolution <screen_width> <screen_height> : force fullscreen resolution\n");
			printf ("-renderer <num> : set renderer. 0 : GLSL 1 : OPENGL 2 : SDL\n");
			exit(0);
		} else {
			/* ignore parameter */
		}
	}
	if ( font_flags == 0 ) font_flags=font_new_flags;
	TCOD_console_set_custom_font(font,font_flags,nb_char_horiz,nb_char_vertic);
	if ( fullscreen_width > 0 ) {
		TCOD_sys_force_fullscreen_resolution(fullscreen_width,fullscreen_height);
	}
	TCOD_console_init_root(80,50,"Merlin Bitte!",fullscreen, renderer);
	/* initialize the offscreen console for the samples */
	sample_console = TCOD_console_new(SAMPLE_SCREEN_WIDTH,SAMPLE_SCREEN_HEIGHT);
	do {
		
		/* print the help message */
		TCOD_console_set_default_foreground(NULL,TCOD_grey);
		//TCOD_console_print_ex(NULL,79,46,TCOD_BKGND_NONE,TCOD_RIGHT,"last frame : %3d ms (%3d fps)", (int)(TCOD_sys_get_last_frame_length()*1000), TCOD_sys_get_fps());
		//TCOD_console_print_ex(NULL,79,47,TCOD_BKGND_NONE,TCOD_RIGHT,"elapsed : %8dms %4.2fs", TCOD_sys_elapsed_milli(),TCOD_sys_elapsed_seconds());
		//TCOD_console_print(NULL,2,47,"%c%c : select a sample", TCOD_CHAR_ARROW_N,TCOD_CHAR_ARROW_S);


		TCOD_console_print(NULL,2,40,"You are inside mntmns apartment. It is very neat.");

		TCOD_console_print(NULL,2,48,"ALT-ENTER : switch to %s",
			TCOD_console_is_fullscreen() ? "windowed mode  " : "fullscreen mode");

		/* render current sample */
		samples[cur_sample].render(first,&key,&mouse);
		first=false;

		/* blit the sample console on the root console */
		TCOD_console_blit(sample_console,0,0,SAMPLE_SCREEN_WIDTH,SAMPLE_SCREEN_HEIGHT, /* the source console & zone to blit */
							NULL,SAMPLE_SCREEN_X,SAMPLE_SCREEN_Y, /* the destination console & position */
							1.0f,1.0f /* alpha coefs */
						 );

		/* update the game screen */
		TCOD_console_flush();

		/* did the user hit a key ? */
		TCOD_sys_check_for_event((TCOD_event_t)(TCOD_EVENT_KEY_PRESS|TCOD_EVENT_MOUSE),&key,&mouse);
		
		if ( key.vk == TCODK_ENTER && key.lalt ) {
			/* ALT-ENTER : switch fullscreen */
			TCOD_console_set_fullscreen(!TCOD_console_is_fullscreen());
		} else if (key.vk==TCODK_F1) {
			/* switch renderers with F1,F2,F3 */
			TCOD_sys_set_renderer(TCOD_RENDERER_GLSL);
		} else if (key.vk==TCODK_F2) {
			TCOD_sys_set_renderer(TCOD_RENDERER_OPENGL);
		} else if (key.vk==TCODK_F3) {
			TCOD_sys_set_renderer(TCOD_RENDERER_SDL);
		}
	} while (!TCOD_console_is_window_closed());
	return 0;
}


