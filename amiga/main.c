#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>
#include <proto/utility.h>
#include <proto/gadtools.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>


#include "dlist.h"
#include "ini.h"
#include "amiga_missing.h"
#include "queue.h"


/*#define DEBUG_OPCODES*/
#define USE_IT_HACK					/* use "it" hack.*/
#define HAVE_RAMLOAD_RAMSAVE		/* enable ramsave/ramrestore*/


#define DEFAULT_GAME_FILE	"mystery.rai"	/*"game.rai"*/

#define RAI_MAJOR	3
#define RAI_VERSION	"3.0"
#define HEAD_TITLE "RAI v" RAI_VERSION ""
#define RAI_COPYRIGHT "Copyright 2015 Stu George"


#include "main.h"
#include "game_structs.h"


/* amiga defs */
#define NORMAL_FLAGS (WINDOWSIZING|WINDOWDRAG|WINDOWCLOSE|WINDOWDEPTH)

struct NewWindow myWindow =
{
	30, /* LeftEdge for window measured in pixels, */
	/* at the current horizontal resolution, */
	/* from the leftmost edge of the screen */
	30, /* TopEdge for window is measured in lines */
	/* from the top of the screen. */
	500, 150, /* width, height of this window */
	-1, /* DetailPen - what pen number is to be */
	/* used to draw the borders of the window */
	-1, /* BlockPen - what pen number is to be */
	/* used to draw system-generated window */
	/* gadgets */
	/* (for DetailPen and BlockPen, the value */
	/* of -1 uses default settings) */
	CLOSEWINDOW | NEWSIZE | REFRESHWINDOW | MENUPICK | VANILLAKEY,
	/* IDCMP flags */
	SIMPLE_REFRESH | NORMAL_FLAGS | GIMMEZEROZERO,
	/* window flags */
	NULL, /* FirstGadget ..\u2022 explained in Chapter 5 */
	NULL, /* CheckMark ... explained in Chapter 5 */
	"Retro Adventure Interpreter",
	NULL,
	NULL,
	400, 100,
	640, 200,
	WBENCHSCREEN
};

#define _MSG_QUIT	1
#define _MSG_REDRAW 2
#define _MSG_RESIZE 3
#define _MSG_KEY 	4
#define _MSG_LOAD	5
#define _MSG_SAVE	6


struct TextAttr fntTopaz = { "topaz.font", 8, 0, 0 };

/*
struct NewMenu rai_menu[] =
{
	{ NM_TITLE, "File" },
	{ NM_ITEM, "Save", "S"},
	{ NM_ITEM, "Load", "L"},
	{ NM_ITEM, NM_BARLABEL},
	{ NM_ITEM, "Quit", "Q"},
	{ NM_TITLE, "Help"},
	{ NM_ITEM, "About", "A"},
	{ NM_END}
};
*/

struct NewMenu rai_menu[] =
{
	{ NM_TITLE, "File" },
	{ NM_ITEM, "Save", "S" },
	{ NM_ITEM, "Load", "L" },
	{ NM_ITEM, NM_BARLABEL },
	{ NM_ITEM, "Quit", "Q" },
/*
	{ NM_TITLE, "Help"},
	{ NM_ITEM, "About", "A"},*/
	{ NM_END }
};


struct Window *win;
struct RastPort *rport;
struct Screen *screen;
struct TextFont *tf;
struct Menu *menus;

extern struct Window* OpenWindow();

struct GfxBase *GfxBase;
struct IntuitionBase *IntuitionBase;

struct Library *DiskfontBase;
struct Library *LayersBase;

void *vi;

Queue *qKeys;

/**************/


/* consists of*/
uint16_t save_data_length;
uint8_t *save_data;

/* global data that is also saved*/
uint8_t light_flag;

/* internal use flags*/
uint8_t quit_restart_flag;

#define MAX_LINE_LENGTH 255
char line[MAX_LINE_LENGTH];
int line_idx;

#define MAX_WORDS	2
uint8_t *words[MAX_WORDS];
int words_idx[MAX_WORDS];

uint8_t *save_name;

/* preallocated for string printing*/
#define LOG_BUFFER_LEN 256
uint8_t strLogBuffer3[LOG_BUFFER_LEN];

uGame *game_hdr;
uint8_t *xdata;

uint16_t maxline_count;

#ifdef HAVE_RAMLOAD_RAMSAVE
/* ramsave*/
void *ramsave_data;
#endif


#ifdef USE_IT_HACK
char *old_word2 = NULL;
#endif

#define AMIGA_MISSING_SHIT

#define XBUFFER_SIZE		(1024*64)

/* exit means we dont run any more codeblocks.*/
#define CODE_RAN_EXIT_OK		0
#define CODE_RAN_EXIT_FAIL		1
/* continue means we run codeblocks+actions*/
#define CODE_RAN_CONTINUE_OK	2
#define CODE_RAN_CONTINUE_FAIL	3
#define CODE_QUIT_RESTART		5


static uint16_t width;
static uint16_t height;
static uint8_t last_char_out;

static void print_title_bar(char *msg);
static void internal_string_print(char *strX, ...);
static uint8_t run_codeblock(uint8_t *ptr);
static uint16_t get_pointer(uint8_t *ptr);
static void amiga_shutdown(void);
static int handle_inputs(void);
static void CleanupLibs(void);
static uint8_t get_keypress(void);



#define RNG_M 2147483647L	/* m = 2^31 - 1 */
#define RNG_A 48271L
#define RNG_Q 127773L		/* m div a */
#define RNG_R 2836L			/* m mod a */

long rnd_seed;

static void set_rnd_seed(long seedval)
{
	rnd_seed = (seedval % (RNG_M - 1)) + 1;
}

static void reset_rnd_seed(void)
{
	set_rnd_seed((time(NULL) % (RNG_M - 1)) + 1);
}

static long rnd(void)
{
	long low, high, test;

	high = rnd_seed / RNG_Q;
	low = rnd_seed % RNG_Q;
	test = RNG_A * low - RNG_R * high;
	if (test > 0)
		rnd_seed = test;
	else
		rnd_seed = test + RNG_M;

	return rnd_seed;
}

long xrnd(long maxrnd)
{
	if (maxrnd == 0L)
		return rnd();
	else
		return ((rnd()) % maxrnd);
}

static void ZClearEOL(struct RastPort *rp)
{
	ClearEOL(rp);
}

static void XClearEOL(struct RastPort *rp)
{
	RectFill(rp,
			 rp->cp_x, rp->cp_y - rp->TxBaseline,
			 win->GZZWidth - 1, (rp->cp_y - rp->TxBaseline) + (rp->Font->tf_YSize - 1));
}

static void gotoxy(uint16_t x, uint16_t y)
{
	Move(rport, x * rport->TxWidth, (y * rport->TxHeight) + rport->TxBaseline);
}

static void cputc(int c)
{
	char x[2];

	x[0] = c;
	x[1] = 0;

	Text(rport, x, 1);
}

static int wherey(void)
{
	return ((rport->cp_y - rport->TxBaseline) / rport->TxHeight);
}

static int wherex(void)
{
	return (rport->cp_x / rport->TxWidth);
}

static void clrscr(void)
{
	int i;

	SetAPen(rport, 0);
	SetDrMd(rport, JAM2 | INVERSVID);

	for (i = 0; i < height; i++)
	{
		gotoxy(0, i);
		XClearEOL(rport);
	}

	gotoxy(0, 0);

	SetAPen(rport, 1);
	SetDrMd(rport, JAM1);
}

static int cgetc(void)
{
	uint32_t c;

	c = 0;

	if (queue_size(qKeys) > 0)
	{
		queue_dequeue(qKeys, (void *)&c);
	}

	return (c);
}

static void revers(int x)
{
	if (x == 1)
	{
		SetDrMd(rport, JAM1 | INVERSVID);
	}
	else
	{
		SetDrMd(rport, JAM1);
	}
}

static void screensize(uint16_t *w, uint16_t *h)
{
	*w = (win->GZZWidth / rport->TxWidth);
	*h = (win->GZZHeight / rport->TxHeight);
}

static void cursor(int x)
{
}

static int handle_inputs(void)
{
	struct IntuiMessage *imsg;
	int status;
	uint32_t signals;
	uint32_t mpbit;

	uint32_t mclass;
	uint16_t code;

	mpbit = 1L << win->UserPort->mp_SigBit;

	/* go into suspend mode and wait for event */
	signals = Wait(mpbit);

	status = 0;

	if ((signals & mpbit) == mpbit)
	{
		/* process events.. */
		while ((imsg = (struct IntuiMessage *)GetMsg(win->UserPort)) != NULL)
		{
			mclass = imsg->Class;
			code = imsg->Code;
			ReplyMsg((struct Message *)imsg);

			switch (mclass)
			{
				case IDCMP_CLOSEWINDOW:
					status = _MSG_QUIT;
					quit_restart_flag = 1;
					break;

				case IDCMP_NEWSIZE:
					status = _MSG_RESIZE;
					break;

				case IDCMP_REFRESHWINDOW:
					status = 0;
					BeginRefresh(win);
					EndRefresh(win, TRUE);
					break;

				case IDCMP_MENUPICK:
					status = 0;
					while (code != MENUNULL)
					{
						struct MenuItem *item;

						if ((item = ItemAddress(menus, code)) != NULL)
						{
							if (MENUNUM(code) == 0)
							{
								switch (ITEMNUM(code))
								{
									case 0: /* save */
										status = _MSG_SAVE;
										break;

									case 1: /* load */
										status = _MSG_LOAD;
										break;

									case 2: /* spacer */
										break;

									case 3: /* quit */
										status = _MSG_QUIT;
										break;
								}
							}

							code = item->NextSelect;
						}
						else
						{
							code = MENUNULL;
						}
					}

					break;

				case IDCMP_VANILLAKEY:
					status = _MSG_KEY;
					if (queue_size(qKeys) < 32)
					{
						queue_enqueue(qKeys, (void *)code);
					}

					break;
			}

			if (status == _MSG_RESIZE)
			{
				screensize(&width, &height);
				status = _MSG_REDRAW;
			}

			if (status == _MSG_REDRAW)
			{
				/* recompute all text?? */
				status = 0;
			}
		}
	}

	return (status);
}

static void set_bit(uint8_t *bits, int pos, int state)
{
	uint8_t mask = (0x80 >> (pos % 8));

	if (state != 0)
		bits[pos / 8] = bits[pos / 8] | mask;
	else
		bits[pos / 8] = bits[pos / 8] & (~mask);

}

static int get_bit(uint8_t *bits, int pos)
{
	uint8_t mask;
	mask = (0x80 >> (pos % 8));
	return (bits[pos / 8] & mask) == mask ? 1 : 0;
}

static void make_save_name(uint8_t *s)
{
	uint8_t *p;

	save_name = malloc(strlen(s) + 16);
	strcpy(save_name, s);

	p = strchr(save_name, 0x0);

	while (p > save_name && *p != '.')
	{
		p--;
	}

	if (*p != '.')
		strcat(save_name, ".sav");
	else
		memmove(p, ".sav", 4);
}

static void scroll_up(void)
{
	ScrollRaster(rport,
				 0, rport->TxHeight,
				 0, rport->TxHeight,
				 win->Width, win->Height);

	gotoxy(0, height - 1);
	ZClearEOL(rport);
	gotoxy(0, height - 1);
}

static void glk_put_char(uint8_t c)
{
	if (c == '\n' || c == '\r')
		last_char_out += 1;
	else
		last_char_out = 0;

	if (c == '\n' || c == '\r')
	{
		if (wherey() == height - 1)
		{
			scroll_up();
			gotoxy(0, height - 1);
		}
		else
		{
			gotoxy(0, wherey() + 1);
		}

		maxline_count += 1;
		if (maxline_count == height - 2)
		{
			maxline_count = 0;

			gotoxy(0, height - 1);
			Text(rport, "PRESS ANY KEY", 13);
			get_keypress();

			gotoxy(0, height - 1);
			SetAPen(rport, 0);
			SetDrMd(rport, JAM2 | INVERSVID);
			XClearEOL(rport);

			SetAPen(rport, 1);
			SetDrMd(rport, JAM1);

			gotoxy(0, height - 1);
		}
	}
	else
	{
		char x[2];
		x[0] = c;
		Text(rport, x, 1);
	}
}

static void glk_put_string(char *s)
{
	uint8_t len;

	while (*s != 0)
	{
		len = 0;
		while (s[len] != 0x20 && s[len] != 0x0)
		{
			len++;
		}

		if (s[len] == 0x20)
			len++;

		if (wherex() + len >= width)
			glk_put_char('\n');

		while (len > 0)
		{
			glk_put_char(*s++);
			len -= 1;
		}
	}
}

static void glk_window_clear(void)
{
	clrscr();
}

static void glk_put_buffer(char *s, uint8_t length)
{
	while (length > 0)
	{
		glk_put_char(*s++);
		length -= 1;
	}
}

static void internal_string_print(char *strX, ...)
{
	va_list args;

	va_start(args, strX);
	vsprintf(strLogBuffer3, strX, args);
	va_end(args);

	glk_put_string(strLogBuffer3);
}

static void new_paragraph(void)
{
	switch (last_char_out)
	{
		case 0:
			glk_put_string("\n\n");
			break;

		case 1:
			glk_put_string("\n");
			break;

		default:
			break;
	}

	last_char_out = 2;
}

void error(char *msg, ...)
{
	va_list args;

	va_start(args, msg);
	vsprintf(strLogBuffer3, msg, args);
	glk_put_string(strLogBuffer3);
	va_end(args);

	get_keypress();

	CleanupLibs();

	exit(0);
}

static uint8_t get_keypress(void)
{
	int status;

	while (1)
	{
		status = handle_inputs();
		if (status == _MSG_KEY)
		{
			queue_dequeue(qKeys, (void *)&status);
			return (status);
		}
		else if (status == _MSG_QUIT)
		{
			amiga_shutdown();
			exit(1);
		}
	};
}

static uint16_t get_pointer(uint8_t *ptr)
{
	return(uint16_t)((ptr[0] << 8) + ptr[1]);
}

static void discombobulate_header(uGame *g)
{
	g->string_count = get_pointer((void *)&g->string_count);
	g->offs_string_table = get_pointer((void *)&g->offs_string_table);
	g->offs_verb_table = get_pointer((void *)&g->offs_verb_table);
	g->offs_room_data = get_pointer((void *)&g->offs_room_data);
	g->offs_item_data = get_pointer((void *)&g->offs_item_data);
	g->offs_noun_table = get_pointer((void *)&g->offs_noun_table);
	g->offs_global_codeblocks = get_pointer((void *)&g->offs_global_codeblocks);
	g->offs_action_codeblocks = get_pointer((void *)&g->offs_action_codeblocks);
	g->size_in_kb = get_pointer((void *)&g->size_in_kb);
}

static void load_game(char *game_file)
{
	FILE *fp;
	uint16_t p;
	uint16_t game_length;
	uGame game_headx;

	sprintf(line, "Loading %s", game_file);
	print_title_bar(line);

	make_save_name(game_file);

	fp = fopen(game_file, "rb");
	if (fp == NULL)
		error("\n\nFailed to load %s. Try \"rai datafile.rai\"\n", game_file);

	fread(&game_headx, 1, sizeof(uGame), fp);
	fclose(fp);

	if (memcmp(game_headx.magic, "XADV", 4) != 0)
	{
		error("bad game file");
	}

	discombobulate_header(&game_headx);

	if (game_headx.interp_required_version > RAI_MAJOR)
	{
		error("This game requires an minimum interpreter version %i.00\n", game_headx.interp_required_version);
	}

	/* 25k game file SHOULD give us enough*/
	/* code space. CC65 gives us D000-0800 = C800 = 50kb*/
	/* interpreter currentl runs 18k. That gives 25-18, 7kb for stack+heap space...*/
	game_length = game_headx.size_in_kb * 1024;       /* give it 25k*/

	/* bounds checking */
	if (game_headx.offs_string_table > game_length)
		error("bad game file");

	if (game_headx.offs_verb_table > game_length)
		error("bad game file");

	if (game_headx.offs_room_data > game_length)
		error("bad game file");

	if (game_headx.offs_item_data > game_length)
		error("bad game file");

	if (game_headx.offs_noun_table > game_length)
		error("bad game file");

	if (game_headx.offs_global_codeblocks > game_length)
		error("bad game file");

	if (game_headx.offs_action_codeblocks > game_length)
		error("bad game file");

	if (game_headx.size_in_kb > (XBUFFER_SIZE / 1024))
	{
		error("Not enough memory for game file (%ukb)", game_headx.size_in_kb);
	}

	/* c64 has no fseek*/
	fp = fopen(game_file, "rb");
	assert(fp != NULL);

	xdata = calloc(1, XBUFFER_SIZE);
	if (xdata == NULL)
	{
		fclose(fp);
		error("Not enough memory");
	}

	p = fread(xdata, 1, XBUFFER_SIZE, fp);

	fclose(fp);

	game_hdr = (uGame *)xdata;
	discombobulate_header(game_hdr);

	game_hdr->game[0x1F] = 0;
	game_hdr->author[0x1F] = 0;
	game_hdr->version[0xF] = 0;
}

static uint8_t* get_string(uint16_t idx)
{
	uint8_t *ptr;
	ptr = xdata + game_hdr->offs_string_table + (idx * 2);
	return (xdata + get_pointer(ptr));
}

static void set_current_player(uint8_t p)
{
	save_data[0] = p;
}

static uint8_t get_current_player(void)
{
	return (save_data[0]);
}

static uint8_t* get_flag_offset(void)
{
	uint8_t *ptr;

	ptr = save_data + 1;                    /* current player*/
	ptr += game_hdr->item_count;
	ptr += 2 * game_hdr->counter_count;

	return (ptr);
}

static uint16_t* get_counter_offset(void)
{
	uint8_t *ptr;

	ptr = save_data + 1;        /* current player*/
	ptr += game_hdr->item_count;

	return(uint16_t *)ptr;
}

static uint8_t* get_item_location_offset(void)
{
	uint8_t *ptr;

	ptr = save_data + 1;        /* current player*/

	return (ptr);
}

static uint8_t* get_base_player_offs(void)
{
	uint8_t *ptr;

	ptr = save_data + 1;        /* current player*/
	ptr += game_hdr->item_count;
	ptr += 2 * game_hdr->counter_count;
	ptr += (game_hdr->flag_count + 7) / 8;

	return (ptr);
}

static uint8_t get_charcter_location(uint8_t c)
{
	uint8_t *ptr;

	ptr = get_base_player_offs();
	ptr += 2 * c;

	return (ptr[0]);
}

static uint8_t get_character_inventory_room(uint8_t c)
{
	uint8_t *ptr;

	ptr = get_base_player_offs();
	ptr += 2 * c;

	return (ptr[1]);
}

static uint8_t* get_player_offs(void)
{
	uint8_t *ptr;

	ptr = get_base_player_offs();
	ptr += 2 * get_current_player();

	return (ptr);
}

static uint8_t get_player_inventory_room(void)
{
	uint8_t *ptr;

	ptr = get_player_offs();

	return (ptr[1]);
}

static void set_player_inventory_room(uint8_t p, uint8_t r)
{
	uint8_t *ptr;

	ptr = get_base_player_offs();
	ptr += 2 * p;
	ptr[1] = r;
}

static void transport_player(uint8_t p, uint8_t r)
{
	uint8_t *ptr;

	ptr = get_base_player_offs();
	ptr += 2 * p;
	ptr[0] = r;
}

static uint8_t get_player_current_room(void)
{
	uint8_t *ptr;

	ptr = get_player_offs();

	return (ptr[0]);
}

static void add_counter(uint8_t idx, int num)
{
	uint16_t *x;

	x = get_counter_offset();
	x[idx] += num;
}

static void set_counter(uint8_t idx, int num)
{
	uint16_t *x;

	x = get_counter_offset();
	x[idx] = num;
}

static uint16_t get_counter(uint8_t idx)
{
	uint16_t *x;

	x = get_counter_offset();
	return (x[idx]);
}

static uint8_t* get_stringX(int idx)
{
	uint8_t * ptr,*x,*xx;
	uint16_t len;
	uint16_t z;
	uint16_t s1, s2, s3;

	s1 = get_pointer(data + game_hdr->offs_string_table + (idx * 2) + 0);
	s2 = get_pointer(data + game_hdr->offs_string_table + (idx * 2) + 2);


	x = get_string(idx);
	xx = x;
	len = 0;

	s3 = s2 - s1;
	while (s3>0)
	{
		z = get_pointer(x);
		x += 2;
		s3 -= 2;

		if (game_hdr->interp_required_version > 2)
		{
			len += strlen(xdata + z);
		}
		else
		{
			len += 1 + (z >> 12);
			if (z >= 0xF000)
			{
				z = get_pointer(x);
				x += 2;
				s3 -= 2;
				len += (z >> 12);
			}
		}
	}

	/* pointer comes into page_buffer, we have a 1kb buffer to play with.*/
	x = xx;

	xx = calloc(1, len + 8);
	ptr = xx;

	s3 = s2 - s1;
	while (s3>0)
	{
		uint32_t offs;

		z = get_pointer(x);
		x += 2;
		s3 -= 2;

		if (game_hdr->interp_required_version > 2)
		{
			len = strlen(xdata + z);
			offs = z;
		}
		else
		{
			offs = z & 0xFFF;
			len = z >> 12;
		}

		memmove(ptr, xdata + offs, len);
		ptr += len;

		while (z >= 0xF000 && game_hdr->interp_required_version < 3)
		{
			z = get_pointer(x);
			x += 2;
			s3 -= 2;
			memmove(ptr, xdata + (z & 0xFFF), (z >> 12));
			ptr += (z >> 12);
		}

		*ptr = ' ';
		ptr++;
		*ptr = 0;
	}

	/* trim trailing space*/
	ptr--;
	if (*ptr == ' ')
	{
		*ptr = 0;
	}

	return (xx);
}

static void print_string(uint16_t idx)
{
	uint8_t *ptr,*optr,*x;
	uint8_t z;


	optr = get_stringX(idx);
	ptr = optr;
	x = strLogBuffer3;

	z = 0;
	x[z] = 0;

	while (*ptr != 0)
	{
		if (*ptr == 0x01)
		{
			glk_put_string((char *)x);

			z = 0;
			x[z] = 0;

			ptr++;

			sprintf((char *)x, "%i", (int)get_counter(ptr[0]));

			ptr++;

			glk_put_string((char *)x);

			z = 0;
			x[z] = 0;
		}
		else if (*ptr == 0x02)
		{
			char *q;

			glk_put_string((char *)x);

			z = 0;
			x[z] = 0;

			q = (char *)strdup(words[0]);
			q = (char *)strlower(q);

			sprintf((char *)x, "%s", q);
			ptr++;
			glk_put_string((char *)x);
			free(q);

			z = 0;
			x[z] = 0;
		}
		else if (*ptr == 0x03)
		{
			char *q;

			glk_put_string((char *)x);

			z = 0;
			x[z] = 0;

			q = (char *)strdup(words[1]);
			q = (char *)strlower(q);

			sprintf((char *)x, "%s", q);
			ptr++;
			glk_put_string((char *)x);

			free(q);
			z = 0;
			x[z] = 0;
		}
		else if (*ptr == 0x04)
		{
			glk_put_string((char *)x);

			z = 0;
			x[z] = 0;

			glk_put_string("\n");
			ptr++;
		}
		else
		{
			x[z++] = *ptr;
			x[z] = 0;

			if (*ptr == ' ')
			{
				ptr++;

				glk_put_string((char *)x);
				z = 0;
				x[z] = 0;

				while (*ptr == ' ')
				{
					ptr++;
				}
			}
			else
			{
				ptr++;
			}
		}
	}

	glk_put_string((char *)x);

	ptr = strchr(x, 0);
	if(ptr != x)
		ptr -= 1;
	if(*ptr == '.')
		glk_put_string(" ");

	free(optr);
}

static void call_global_codeblock(uint8_t id)
{
	uint8_t *ptr;
	ptr = xdata + game_hdr->offs_global_codeblocks;
	ptr = xdata + get_pointer(ptr + id * 2);
	if (ptr != xdata)
		run_codeblock(ptr);
}

static uint8_t get_item_flags(uint8_t idx)
{
	uint8_t *ptr; /* = data + game_hdr->offs_item_data;*/
	uint8_t i;

	ptr = xdata + game_hdr->offs_item_data;

	for (i = 0; i < game_hdr->item_count; i++)
	{
		if (ptr[0] == idx)
		{
			i = ptr[2];
			return (i & 0xFF);
		}
		else
		{
			ptr += 6; /* skip flags, id etc.*/
		}
	}

	return (0);
}

static char* get_player_name(uint8_t idx)
{
	uint8_t *ptr;
	uint8_t i;

	ptr = xdata + game_hdr->offs_item_data;
	ptr += 6 * (game_hdr->item_count);


	for (i = 0; i < game_hdr->player_count; i++)
	{
		if (ptr[0] == idx)
		{
			ptr += 4;
			ptr = get_stringX(get_pointer(ptr));
			return (ptr);
		}
		else
		{
			ptr += 6; /* skip flags, id etc.*/
		}
	}

	return ((char *)strdup("??"));
}

static char* get_item_name(uint8_t idx)
{
	uint8_t *ptr;
	uint8_t i;

	ptr = xdata + game_hdr->offs_item_data;

	for (i = 0; i < game_hdr->item_count; i++)
	{
		if (ptr[0] == idx)
		{
			ptr += 4;
			ptr = get_stringX(get_pointer(ptr));
			return (ptr);
		}
		else
		{
			ptr += 6; /* skip flags, id etc.*/
		}
	}

	return ((char *)strdup("??"));
}

static uint8_t do_can_see(uint8_t room, uint8_t flags, uint8_t count)
{
	uint8_t i, j;
	uint8_t * ptr,*q;

	ptr = get_item_location_offset();

	for (i = 0, j = 0; i < game_hdr->item_count; i++)
	{
		if (ptr[i] == room)
		{
			if (flags == 0 || (get_item_flags(i) & flags) == 0)
			{
				q = (uint8_t *)get_item_name(i);

				if (j < count - 1)
					sprintf(line, "%s, ", q);
				else
					sprintf(line, "%s.", q);

				internal_string_print(line);
				free(q);
				j++;
			}
		}
	}

	ptr = get_base_player_offs();
	for (i = 0; i < game_hdr->player_count; i++)
	{
		if (ptr[0] == room && i != get_current_player())
		{
			q = (uint8_t *)get_player_name(i);

			if (j < count - 1)
				sprintf(line, "%s, ", q);
			else
				sprintf(line, "%s.", q);

			internal_string_print(line);
			free(q);
			j++;
		}

		ptr += 2;
	}

	return (j);
}

static uint8_t is_light_in_room_x(uint8_t room)
{
	uint8_t *x;
	uint8_t i;

	/* test whats in room.*/
	x = get_item_location_offset();

	for (i = 0; i < game_hdr->item_count; i++)
	{
		/* for each item, does it exist in player inventory or current room*/
		if (x[i] == room)
		{
			if ((get_item_flags(i) & ITEM_LIGHT) == ITEM_LIGHT)
			{
				return (1);
			}
		}
	}

	return (0);
}

static uint8_t is_light_present(void)
{
	uint8_t *x, z;
	uint8_t i;


	/* naturally lit?*/
	if (light_flag == 1)
	{
		return (1);
	}

	/* check contents of all players inventory for light.*/
	/*x = get_item_location_offset();*/
	z = get_player_current_room();
	x = get_base_player_offs();

	for (i = 0; i < game_hdr->player_count; i++)
	{
		/* another player in same room as active player?*/
		if (x[0] == z)
		{
			if (is_light_in_room_x(get_character_inventory_room(i)) != 0)
			{
				return (1);
			}
		}

		x += 2;
	}

	/* check items in current room*/
	if (is_light_in_room_x(get_player_current_room()) != 0)
	{
		return (1);
	}

	return (0);
}

static uint8_t count_items_in_room(uint8_t room)
{
	uint8_t i, j;

	uint8_t *x;

	x = get_item_location_offset();

	for (i = 0, j = 0; i < game_hdr->item_count; i++)
	{
		if (x[i] == room)
		{
			if ((get_item_flags(i) & ITEM_SCENERY) != ITEM_SCENERY)
			{
				j++;
			}
		}
	}

	x = get_base_player_offs();
	for (i = 0; i < game_hdr->player_count; i++)
	{
		if (x[0] == room && i != get_current_player())
		{
			j++;
		}

		x += 2;
	}

	return (j);
}

static void you_can_see(void)
{
	uint8_t j;

	j = count_items_in_room(get_player_current_room());

	if (j > 0)
	{
		new_paragraph();
		internal_string_print("You can see : ");
		if (do_can_see(get_player_current_room(), ITEM_SCENERY, j) == 0)
			internal_string_print("Nothing special\n");
	}
}

static void show_inventory(void)
{
	uint8_t j;

	j = count_items_in_room(get_player_inventory_room());

	new_paragraph();
	internal_string_print("Your inventory : ");
	if (do_can_see(get_player_inventory_room(), 0, j) == 0)
		internal_string_print("Is empty!\n");
}

static void update_item_count(void)
{
	uint8_t i, z;
	uint8_t *item_locn;

	item_locn = get_item_location_offset();

	z = get_player_inventory_room();
	set_counter(COUNTER_INVENTORY, 0);

	for (i = 0; i < game_hdr->item_count; i++)
	{
		if (item_locn[i] == z)
		{
			add_counter(COUNTER_INVENTORY, 1);
		}
	}
}

static void call_room_codeblock(uint8_t id)
{
	uint8_t *ptr;
	uint16_t offs;

	ptr = xdata + game_hdr->offs_room_data;
	ptr += 2 * get_player_current_room();
	offs = get_pointer(ptr);
	ptr = xdata + offs;
	/* ptr now points to room structure.*/

	/*ptr += 1; // room id*/
	/*ptr += 2; // string id*/
	/*ptr += id * 2; // codeblock offset*/

	ptr += 3 + (id * 2);
	offs = get_pointer(ptr);

	if (offs != 0)
	{
		ptr = xdata + offs;
		run_codeblock(ptr + 1);
	}

	if (id == XRM_LOOK)
	{
		you_can_see();
	}
}

static uint16_t get_room_title(void)
{
	uint8_t *ptr;

	ptr = xdata + game_hdr->offs_room_data;
	ptr += get_player_current_room() * 2;
	ptr = xdata + get_pointer(ptr);
	/* skip room ID#*/
	ptr++;
	return (get_pointer(ptr));
}

static void print_room_title(void)
{
	uint16_t tid;

	tid = get_room_title();

	new_paragraph();

	SetSoftStyle(rport, FSF_BOLD, FSF_BOLD);

	print_string(tid);

	SetSoftStyle(rport, FS_NORMAL, 0xFF);

	glk_put_string("\n");
}

static void print_title_bar(char *msg)
{
	uint8_t *q;
	char *x;
	uint8_t len;
	uint8_t cx, cy;
	char score[32];
	uint16_t tid;

	if (msg == NULL)
	{
		tid = get_room_title();
		q = get_stringX(tid);
	}
	else
		q = msg;

	x = strLogBuffer3;
	memset(x, ' ', width);

	len = strlen(q);
	if (len >= width - 0)
	{
		len = width - 0;
	}

	memmove(x + 1, q, len);

	if (msg == NULL && save_data != NULL)
	{
		/* SCORE is always var 1!*/
		sprintf(score, "Score: %i", (int)get_counter(1));
		memmove(x + (width - (strlen(score) + 1)), score, strlen(score));
	}

	cx = wherex();
	cy = wherey();


	gotoxy(0, 0);

	SetAPen(rport, 0);
	SetDrMd(rport, JAM2 | INVERSVID);
	XClearEOL(rport);
	SetAPen(rport, 1);
	SetDrMd(rport, JAM1 | INVERSVID);
	Text(rport, x, width);

	SetAPen(rport, 1);
	SetDrMd(rport, JAM1);

	gotoxy(cx, cy);

	if (msg == NULL)
	{
		free(q);
	}
}

static void do_quitrestart(void)
{
	int k;

	k = 0;
	while (k == 0)
	{
		k = get_keypress();
		switch (k)
		{
			case 'q':
			case 'Q':
				quit_restart_flag = -1;
				internal_string_print(" Quit");
				break;

			case 'R':
			case 'r':
				internal_string_print(" Restart");
				quit_restart_flag = 1;
				break;

			default:
				k = 0;
				break;
		}
	}
}

static void do_look(void)
{
	if (is_light_present() == 1)
	{
		print_room_title();
		call_room_codeblock(XRM_LOOK);
	}
	else
	{
		call_global_codeblock(XGC_IN_DARK);
	}
}

static void pause_time(uint8_t t)
{
	/*time_t xx;*/

	/*xx = t + 1 + time(NULL);*/
	unsigned int xx;
	xx = 10000;
	while (xx-- != 0);
}

static uint8_t run_codeblock(uint8_t *ptr)
{
	/* MAX TRY DEPTH*/
	uint8_t *try_stack[8];

	uint8_t try_idx = 0;
	uint8_t *item_locn;
	uint8_t qflag;

#ifdef DEBUG_OPCODES
	char *ops[] =
	{
		"(NULLOP)",
		"X_ADDCOUNTER",
		"X_SUBCOUNTER",
		"X_SETCOUNTER",
		"X_SETLIGHT",
		"X_MSG",
		"X_QUITRESTARTGAME",
		"X_TRY",
		"X_ISIMTEINROOM",
		"X_WINLOOSEGAME",
		"X_CONTINUE",
		"X_EXIT",
		"X_MOVE",
		"X_SWAP",
		"X_CANPLAYERSEE",
		"X_ISPRESENT",
		"X_GOTO",
		"X_NOUNIS",
		"X_SHOWINVENTORY",
		"X_CANCARRY",
		"X_HAS",
		"X_HERE",
		"X_ENDTRY",
		"X_ISNOTPRESENT",
		"X_LOOK",
		"X_IN",
		"X_TRANSPORT",
		"X_SWITCH",
		"X_COUNTEREQUALS",
		"X_COUNTERNOTEQUALS",
		"X_TAKE",
		"X_COUNTERGT",
		"X_COUNTERLT",
		"X_ISIMTEMHERE",
		"X_ISFLAG",
		"X_SETFLAG",
		"X_SETPLAYERINVENTORY",
		"X_NPCHERE",
		"X_PAUSE",
		"X_NOTIN",
		"X_RANDOM",
		"X_HASNOT",
		"X_BYTECODE_JUMP",
	};
#endif

	assert(ptr != NULL);

	if (quit_restart_flag != 0)
		return (CODE_RAN_EXIT_FAIL);

	item_locn = get_item_location_offset();

	while (*ptr != X_ENDOPCODES)
	{
		qflag = 0;

#ifdef DEBUG_OPCODES
		internal_string_print("%s. ", ops[*ptr]);
#endif
		switch (*ptr)
		{
			case X_SWAP:
				ptr++;
				{
					uint8_t k;

					k = item_locn[ptr[0]];
					item_locn[ptr[0]] = item_locn[ptr[1]];
					item_locn[ptr[1]] = k;
				}
				update_item_count();
				ptr += 2;
				break;

			case X_SHOWINVENTORY:
				ptr++;
				show_inventory();
				break;

			case X_CANPLAYERSEE:
				ptr++;
				if (is_light_present() == 0)
				{
					qflag = 1;
				}

				break;

			case X_COUNTEREQUALS:
				ptr++;
				if (!(get_counter(ptr[0]) == get_pointer(1 + ptr)))
				{
					qflag = 1;
				}
				else
					ptr += 3;

				break;

			case X_COUNTERGT:
				ptr++;
				if (!(get_counter(ptr[0]) > (get_pointer(1 + ptr))))
				{
					qflag = 1;
				}
				else
					ptr += 3;

				break;

			case X_COUNTERLT:
				ptr++;
				if (!(get_counter(ptr[0]) < (get_pointer(1 + ptr))))
				{
					qflag = 1;
				}
				else
					ptr += 3;

				break;

			case X_COUNTERNOTEQUALS:
				ptr++;
				if (!(get_counter(ptr[0]) != get_pointer(1 + ptr)))
				{
					qflag = 1;
				}
				else
					ptr += 3;

				break;

			case X_ADDCOUNTER:
				ptr++;
				add_counter(ptr[0], get_pointer(1 + ptr));
				ptr += 3;
				break;

			case X_SUBCOUNTER:
				ptr++;
				add_counter(ptr[0], 0 - get_pointer(1 + ptr));
				ptr += 3;
				break;

			case X_ISIMTEMHERE:
				ptr++;
				if (item_locn[ptr[0]] == get_player_current_room())
				{
					ptr++;
				}
				else
				{
					qflag = 1;
				}

				break;

			case X_ISPRESENT:
				ptr++;
				if (item_locn[ptr[0]] == get_player_current_room() || item_locn[ptr[0]] == get_player_inventory_room())
				{
					ptr++;
				}
				else
				{
					qflag = 1;
				}

				break;

			case X_ISNOTPRESENT:
				ptr++;
				if (item_locn[ptr[0]] != get_player_current_room() && item_locn[ptr[0]] != get_player_inventory_room())
				{
					ptr++;
				}
				else
				{
					qflag = 1;
				}

				break;

			case X_CANCARRY:
				{
					uint8_t i = 0;
					uint8_t j = 0;
					uint8_t z;

					ptr++;

					z = get_player_inventory_room();
					for (i = 0; i < game_hdr->item_count; i++)
					{
						if (item_locn[i] == z)
						{
							j++;
						}
					}

					if (j > game_hdr->max_carry)
					{
						qflag = 1;
					}
				}
				break;

			case X_HAS:
				ptr++;
				if (item_locn[ptr[0]] == get_player_inventory_room())
				{
					ptr++;
				}
				else
				{
					qflag = 1;
				}

				break;

			case X_HASNOT:
				ptr++;
				if (item_locn[ptr[0]] != get_player_inventory_room())
				{
					ptr++;
				}
				else
				{
					qflag = 1;
				}

				break;

			case X_HERE:
				ptr++;
				item_locn[ptr[0]] = get_player_current_room();
				update_item_count();
				ptr++;
				break;

			case X_IN:
				ptr++;
				if (ptr[0] == get_player_current_room())
				{
					ptr++;
				}
				else
				{
					qflag = 1;
				}

				break;

			case X_NOTIN:
				ptr++;
				if (ptr[0] != get_player_current_room())
				{
					ptr++;
				}
				else
				{
					qflag = 1;
				}

				break;

			case X_NPCHERE:
				ptr++;
				if (get_player_current_room() != get_charcter_location(*ptr++))
				{
					qflag = 1;
				}

				break;

			case X_SWITCH:
				ptr++;
				set_current_player(*ptr);
				update_item_count();
				call_room_codeblock(XRM_ENTER);
				ptr++;
				break;

			case X_TRANSPORT:
				ptr++;
				transport_player(ptr[0], ptr[1]);
				ptr += 2;
				break;

			case X_MOVE:
				ptr++;
				item_locn[ptr[0]] = ptr[1];
				ptr += 2;
				update_item_count();
				break;

			case X_TAKE:
				ptr++;
				if (get_counter(COUNTER_INVENTORY) < game_hdr->max_carry)
				{
					item_locn[ptr[0]] = get_player_inventory_room();
					ptr++;
					update_item_count();
				}
				else
				{
					internal_string_print("You can't carry anymore.\n");
					return (CODE_RAN_CONTINUE_FAIL);
				}

				break;

			case X_SETCOUNTER:
				ptr++;
				set_counter(ptr[0], get_pointer(1 + ptr));
				ptr += 3;
				break;

			case X_SETLIGHT:
				ptr++;
				light_flag = ptr[0];
				ptr++;
				break;

			case X_MSG:
				ptr++;
				print_string(get_pointer(ptr));
				ptr += 2;
				break;

			case X_LOOK:
				ptr++;
				/*ignores dark flag.*/
				/*call_room_codeblock(XRM_LOOK);*/
				do_look();
				break;

			case X_GOTO:
				ptr++;
				/*set_player_current_room(*ptr);*/
				transport_player(get_current_player(), *ptr);
				ptr++;

				call_room_codeblock(XRM_ENTER);
				if (quit_restart_flag == 0)
				{
					do_look();
				}

				break;

			case X_TRY:
				ptr++;
				try_stack[try_idx++] = ptr + 2 + get_pointer(ptr);
				ptr += 2;
				break;

			case X_QUITRESTARTGAME:
				new_paragraph();
				ptr++;
				print_string(get_pointer(ptr));
				ptr += 2;
				do_quitrestart();
				return (CODE_QUIT_RESTART);
				break;

			case X_ISIMTEINROOM:
				ptr++;

				if (item_locn[ptr[0]] == ptr[1])
				{
					ptr += 2;
				}
				else
				{
					qflag = 1;
				}

				break;

			case X_RANDOM:
				ptr++;
				if (xrnd(100) > ptr[0])
				{
					qflag = 1;
				}

				ptr++;
				break;

			case X_EXIT:
				ptr += 2;
				if (ptr[-1] == 1)
					return (CODE_RAN_EXIT_OK);
				else
					return (CODE_RAN_EXIT_FAIL);

				break;

			case X_CONTINUE:
				ptr += 2;
				if (ptr[-1] == 1)
					return (CODE_RAN_CONTINUE_OK);
				else
					return (CODE_RAN_CONTINUE_FAIL);

				break;

			case X_ENDTRY:
				ptr++;
				if (try_idx > 0)
				{
					try_idx -= 1;
				}

				break;

			case X_WINLOOSEGAME:
				ptr++;
				if (*ptr == 1)
				{
					call_global_codeblock(XGC_ON_GAMEWIN);
					quit_restart_flag = -1;
				}
				else
				{
					call_global_codeblock(XGC_ON_GAMEFAIL);
					quit_restart_flag = 1;
				}

				return (CODE_QUIT_RESTART);
				break;

			case X_NOUNIS:
				ptr++;
				if (ptr[0] != words_idx[1])
				{
					qflag = 1;
				}
				else
				{
					ptr++;
				}

				break;

			case X_SETFLAG:
				ptr++;
				set_bit(get_flag_offset(), ptr[0], ptr[1]);
				ptr += 2;
				break;

			case X_SETPLAYERINVENTORY:
				ptr++;
				set_player_inventory_room(ptr[0], ptr[1]);
				ptr += 2;
				break;

			case X_ISFLAG:
				ptr++;

				if (get_bit(get_flag_offset(), ptr[0]) == ptr[1])
				{
					ptr += 2;
				}
				else
				{
					qflag = 1;
				}

				break;

			case X_PAUSE:
				ptr++;
				pause_time(ptr[0]);
				ptr++;
				break;

			case X_BYTECODE_GOTO:
				ptr += 1;
				ptr = data + get_pointer(ptr);
				break;

			default:
				error("Unknown OPCODE %i", *ptr);
				break;
		}

		if (qflag == 1)
		{
			qflag = 0;
			if (try_idx > 0)
			{
				try_idx -= 1;

				ptr = try_stack[try_idx];

				if (*ptr != X_ENDTRY)
				{
					error("PTR is not endtry!");
				}
			}
			else
			{
				return (CODE_RAN_CONTINUE_FAIL);
			}
		}
	}

	return (CODE_RAN_CONTINUE_OK);
}

static void reset_game(void)
{
	memset(save_data, 0x0, save_data_length);

	quit_restart_flag = 0;
	light_flag = 1;

	last_char_out = 0;

	call_global_codeblock(XGC_RESET);
	update_item_count();
	call_global_codeblock(XGC_PREGAME);
}

static void read_input(void)
{
	char *p;
	uint8_t i;

	maxline_count = 0;

	for (i = 0; i < MAX_WORDS; i++)
	{
		words[i] = NULL;
		words_idx[i] = -1;
	}

	p = line;
	while (*p != 0x0)
	{
		*p = tolower(*p);

		if (*p == 0x0A)
			*p = 0;

		if (*p == 0x0D)
			*p = 0;

		p++;
	}

	p = line;
	i = 0;

	while (*p != 0x0 && *p != 0x0D && *p != 0x0A && i < MAX_WORDS)
	{
		while ((*p == 0x20 || *p == 0x09) && *p != 0x0)
			p++;

		words[i++] = (uint8_t *)p;

		while (*p != 0x0 && *p != 0x20 && *p != 0x09)
			p++;

		if (*p != 0x0)
		{
			*p = 0;
			p++;
		}
	}

	last_char_out = 0;
}

static uint8_t scan_word(uint16_t offs, uint8_t *word)
{
	int8_t v;
	uint8_t *ptr;

	ptr = xdata + offs;

	do
	{
		v = *ptr++;
		do
		{
			if (stricmp((char *)word, (char *)ptr) == 0)
			{
				return (v);
			}

			ptr = strchr(ptr, 0);
			ptr++;

		}while (*ptr != 0);
		ptr++;

	}while (*ptr != 0);

	return (-1);
}

static uint8_t parse_action_blocks(void)
{
	uint8_t *ptr;

	ptr = xdata + game_hdr->offs_action_codeblocks;

	while (*ptr != 0)
	{
		/*internal_string_print("v=%i(%i), n=%i(%i)\n", ptr[0], words_idx[0], ptr[1], words_idx[1]);*/
		if (ptr[0] == words_idx[0] && (ptr[1] == words_idx[1] || ptr[1] == 0))
		{
			int rc;

			rc = run_codeblock(ptr + 4);
			switch (rc)
			{
				case CODE_QUIT_RESTART:
				case CODE_RAN_EXIT_FAIL:
				case CODE_RAN_EXIT_OK:
					return (rc);

				case CODE_RAN_CONTINUE_OK:
				case CODE_RAN_CONTINUE_FAIL:
					break;
			}
		}

		/*ptr += 2;*/
		ptr += get_pointer(2 + ptr) + 4;
	}

	return (CODE_RAN_CONTINUE_FAIL);
}

static uint8_t parse_room_blocks(void)
{
	uint8_t * ptr,*x;
	uint8_t i;
	uint16_t offs;

	ptr = xdata + game_hdr->offs_room_data;
	ptr += (get_player_current_room() * 2);
	ptr = xdata + get_pointer(ptr);

	/*ptr += 1;	// skip room id*/
	/*ptr += 2;	// skip string id*/
	/*ptr += 4;	// skip enter + look. leaves us with directional commands only.*/

	ptr += 7;

	/* look + n/s/e/w/nw/ne/sw/se/u/d = 11*/
	for (i = 0; i < 11; i++)
	{
		offs = get_pointer(ptr);
		ptr += 2;
		if (offs != 0)
		{
			x = xdata + offs;

			if (x[0] == words_idx[0])
			{
				/* TODO: Fix page buffer for codeblock...*/
				i = run_codeblock(x + 1);
				/* run a directional codeblock*/
				switch (i)
				{
					case CODE_QUIT_RESTART:
						/* todo*/
						return (CODE_QUIT_RESTART);
						break;

						/* if continue ok, parse room direction to new room.*/
					case CODE_RAN_CONTINUE_FAIL:
					case CODE_RAN_CONTINUE_OK:
					case CODE_RAN_EXIT_FAIL:
					case CODE_RAN_EXIT_OK:
						return (i);
						break;

				}
			}

		}
	}

	return (CODE_RAN_CONTINUE_OK);
}

static int load_saved_game(void)
{
	FILE *fp;
	fp = fopen(save_name, "rb");
	if (fp == NULL)
		return (1);

	fread(save_data, 1, save_data_length, fp);
	fclose(fp);

	return (0);
}

static int save_game(void)
{
	FILE *fp;
	fp = fopen(save_name, "wb");
	if (fp == NULL)
		return (1);

	fwrite(save_data, 1, save_data_length, fp);
	fclose(fp);

	return (0);
}

static void initial_clear_reset(void)
{
	if (quit_restart_flag == 1)
	{
		glk_window_clear();
		print_title_bar(game_hdr->game);
		gotoxy(0, 1);
		reset_game();
	}
}

static void CleanupLibs(void)
{
	if (qKeys != NULL)
	{
		queue_destroy(qKeys);
		free(qKeys);
	}

	if (win != NULL)
	{
		ClearMenuStrip(win);
		CloseWindow(win);
	}

	if (screen != NULL)
	{
		UnlockPubScreen(0, screen);
	}

	if (menus != NULL)
	{
		FreeMenus(menus);
	}

	if (vi != NULL)
	{
		FreeVisualInfo(vi);
	}

	if (DiskfontBase != NULL)
	{
		CloseLibrary(DiskfontBase);
	}

	if (LayersBase != NULL)
	{
		CloseLibrary(LayersBase);
	}

	if (GfxBase != NULL)
	{
		CloseLibrary((struct Library *)GfxBase);
	}

	if (IntuitionBase != NULL)
	{
		CloseLibrary((struct Library *)IntuitionBase);
	}
}

static void Cleanup(int rc, char *msg)
{
	printf("%s\n", msg);
	CleanupLibs();
	exit(rc);
}

static void amiga_setup_openlibs(void)
{
	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0);
	if (GfxBase == NULL)
	{
		Cleanup(20, "Cant open graphics.library!\n");
	}

	IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 0);
	if (IntuitionBase == NULL)
	{
		Cleanup(22, "Cant open intuition.library!");
	}

	LayersBase = OpenLibrary("layers.library", 0);
	if (LayersBase == NULL)
	{
		Cleanup(24, "Cant open layers.library!");
	}

	DiskfontBase = OpenLibrary("diskfont.library", 0);
	if (DiskfontBase == NULL)
	{
		Cleanup(26, "Cant open diskfont.library!");
	}

}

static void amiga_setup_visuals(void)
{
	screen = LockPubScreen(0);
	if (screen == NULL)
	{
		Cleanup(10, "Can't lock screen!");
	}

	vi = GetVisualInfo(screen, 0);
	if (vi == NULL)
	{
		Cleanup(11, "Can't get visual info!");
	}

	menus = CreateMenusA(rai_menu, NULL);
	if (menus == NULL)
	{
		Cleanup(30, "Didnt create menus!");
	}

	if (!LayoutMenusA(menus, vi, NULL))
	{
		Cleanup(32, "Can't layout menus!");
	}
}

static void amiga_setup_window(void)
{
	INIFILE *ini;
	char *fnt_name = NULL;
	int fnt_size = 0;

	ini = INI_load("PROGDIR:rai.ini");
	if (ini != NULL)
	{
		char *p;

		fnt_name = INI_get(ini, "display", "font.name");
		p = INI_get(ini, "display", "font.size");
		if (p != NULL)
		{
			fnt_size = atoi(p);
		}
		else
			fnt_size = 8;
	}

	if (((screen->Height / 3) * 2) > 200)
	{
		myWindow.Height = ((screen->Height / 3) * 2);
	}

	if (((screen->Width / 3) * 2) > 500)
	{
		myWindow.Width = ((screen->Width / 3) * 2);
	}

	if (myWindow.Width > myWindow.MaxWidth)
	{
		myWindow.MaxWidth = screen->Width;
	}

	if (myWindow.Height > myWindow.MaxHeight)
	{
		myWindow.MaxHeight = screen->Height;
	}

	win = OpenWindow(&myWindow);
	if (win == NULL)
	{
		Cleanup(28, "Window didn't open!");
	}

	rport = win->RPort;

	/* load topaz and set to rastport */
	if (fnt_name == NULL)
	{
		fnt_name = "topaz.font";
		fnt_size = 8;
	}

	fntTopaz.ta_Name = strdup(fnt_name);
	fntTopaz.ta_YSize = fnt_size;

	tf = OpenDiskFont(&fntTopaz);
	if (tf == NULL)
	{
		printf("could not open font %s, size %i\n", (char *)fntTopaz.ta_Name, (int)fntTopaz.ta_YSize);
		Cleanup(34, "Font fail");
	}

	SetFont(rport, tf);

	if (ini != NULL)
	{
		INI_unload(ini);
	}

	SetMenuStrip(win, menus);
	/*ScreenToFront(win->WScreen);*/
}

static void amiga_setup(void)
{
	qKeys = malloc(sizeof(SList));
	queue_init(qKeys, NULL);

	amiga_setup_openlibs();
	amiga_setup_visuals();
	amiga_setup_window();

	SetAPen(rport, 1);
	SetBPen(rport, 0);
	SetDrMd(rport, JAM1);
}

static void amiga_shutdown(void)
{
	CleanupLibs();
}

static void part1(void)
{
	new_paragraph();
	print_title_bar(NULL);
	call_global_codeblock(XGC_PROMPT);
}

static void part2(void)
{
	char *w0;
	char *w1;

	read_input();
	if (words[0] == NULL)
	{
		/*words[0] = (uint8_t *)"any";*/
		strcpy(line, "any");
		words[0] = line;
	}

	if (words[1] == NULL)
	{
		/*words[1] = (uint8_t *)"any";*/
		strcpy(strchr(line, 0) + 1, "any");
		words[1] = strchr(line, 0) + 1;
	}

	new_paragraph();

#ifdef USE_IT_HACK
	if (old_word2 != NULL && stricmp((char *)words[1], "it") == 0)
	{
		memmove(words[1], old_word2, strlen(old_word2) + 1);
	}

	if (words[1] != NULL)
	{
		if (old_word2 != NULL)
			free(old_word2);

		old_word2 = (char *)strdup((char *)words[1]);
	}

#endif

	w0 = words[0];
	w1 = words[1];

	if (stricmp("save", w0) == 0)
	{
		maxline_count = 0;
		if (save_game() == 0)
			internal_string_print("\nGame saved\n");
		else
			internal_string_print("\nError saving");
	}
	else if (stricmp("restore", w0) == 0)
	{
		maxline_count = 0;
		if (load_saved_game() == 0)
		{
			internal_string_print("\nGame restored\n");
			do_look();
		}
		else
			internal_string_print("\nError restoring");
	}

#ifdef HAVE_RAMLOAD_RAMSAVE
	else if (stricmp("#ramrestore", w0) == 0)
	{
		maxline_count = 0;
		internal_string_print("\nGame restored\n");
		memmove((void *)save_data, (void *)ramsave_data, save_data_length);
		do_look();
	}
	else if (stricmp("#ramsave", w0) == 0)
	{
		maxline_count = 0;
		memmove((void *)ramsave_data, (void *)save_data, save_data_length);
		internal_string_print("\nGame saved\n");
	}

#endif
	else
	{

		words_idx[0] = scan_word(game_hdr->offs_verb_table, w0);
		words_idx[1] = scan_word(game_hdr->offs_noun_table, w1);

		/* run room blocks (directions), then global blocks then fail routine.*/
		switch (parse_room_blocks())
		{
			case CODE_RAN_CONTINUE_OK:
				switch (parse_action_blocks())
				{
					case CODE_RAN_CONTINUE_FAIL:
						if (words_idx[0] >= 1 && words_idx[0] <= 10)
						{
							internal_string_print("You can't go that way.");
							call_global_codeblock(XGC_ON_SUCCESS);
							break;
						}
						else
							call_global_codeblock(XGC_ON_FAIL);

						break;

					case CODE_RAN_EXIT_OK:
					case CODE_RAN_CONTINUE_OK:
						call_global_codeblock(XGC_ON_SUCCESS);
						break;

					case CODE_RAN_EXIT_FAIL:
						call_global_codeblock(XGC_ON_FAIL);
						break;

					case CODE_QUIT_RESTART:
						initial_clear_reset();
						if (quit_restart_flag == 1)
							quit_restart_flag = 0;

						break;
				}

				break;

			case CODE_RAN_EXIT_FAIL:
			case CODE_RAN_CONTINUE_FAIL:
				call_global_codeblock(XGC_ON_FAIL);
				break;

			case CODE_RAN_EXIT_OK:
				call_global_codeblock(XGC_ON_SUCCESS);
				break;

			case CODE_QUIT_RESTART:
				initial_clear_reset();
				if (quit_restart_flag == 1)
					quit_restart_flag = 0;

				break;
		}
	}
}

int main(int argc, char *argv[])
{
	int status;

	maxline_count = 0;

	amiga_setup();
	ScreenToFront(win->WScreen);

	screensize(&width, &height);

	glk_window_clear();
	gotoxy(0, 1);
	print_title_bar(HEAD_TITLE);
	internal_string_print("\n" HEAD_TITLE "\n" RAI_COPYRIGHT "\n");

#ifdef HAVE_RAMLOAD_RAMSAVE
	internal_string_print("\n#ramsave and #ramrestore supported\n");
#endif
#ifdef USE_IT_HACK
	internal_string_print("'IT' hack supported (last verb used inplace of 'it')\n");
#endif

	reset_rnd_seed();

	/* load game does a close on fp*/
	if (argc >= 2)
		load_game(argv[1]);
	else
		load_game(DEFAULT_GAME_FILE);

	/* allocate save buffer */
	save_data_length = 1; /* current player*/
	save_data_length += game_hdr->item_count;
	save_data_length += game_hdr->counter_count * 2;
	save_data_length += (game_hdr->flag_count + 7) / 8;
	save_data_length += game_hdr->player_count * 2;

	save_data = calloc(1, save_data_length);
#ifdef HAVE_RAMLOAD_RAMSAVE
	ramsave_data = calloc(1, save_data_length);
#endif

	print_title_bar(game_hdr->game);

	internal_string_print("\n");
	internal_string_print("%s\n", game_hdr->game);
	internal_string_print("by ");
	internal_string_print("%s\n", game_hdr->author);
	internal_string_print("Version ");
	internal_string_print("%s\n\n", game_hdr->version);

	/* resets game & local data, calls pregame*/
	reset_game();

	status = 0;

	while (quit_restart_flag == 0 && status != _MSG_QUIT)
	{
		line_idx = 0;
		memset(line, 0, MAX_LINE_LENGTH);

		part1();

		while (quit_restart_flag == 0 && status == 0)
		{
			status = handle_inputs();

			if (status == _MSG_KEY)
			{
				while (queue_dequeue(qKeys, (void *)&status) != -1)
				{
					switch (status)
					{
						case 0x08:
							if (line_idx > 0)
							{
								line_idx -= 1;
								line[line_idx] = 0;

								gotoxy(wherex() - 1, wherey());

								SetAPen(rport, 0);
								SetDrMd(rport, JAM2 | INVERSVID);
								XClearEOL(rport);
								SetAPen(rport, 1);
								SetDrMd(rport, JAM1);

								status = 0;
							}

							break;

						case 0x0D:
						case 0x0A:
							if (line_idx < width - 10 && (status >= 0x20 && status <= 0x7F))
							{
								line[line_idx++] = status & 0xFF;
								line[line_idx] = 0;
							}

							/* empty queue. */
							while (queue_dequeue(qKeys, (void *)&status) != -1)
							{
							};

							status = 0x0D;
							break;

						default:
							if (line_idx < width - 10 && (status >= 0x20 && status <= 0x7F))
							{
								line[line_idx++] = status & 0xFF;
								line[line_idx] = 0;
								cputc(status);
								status = 0;
							}

							break;
					}
				}

				if (status == 0x0D || status == 0x0A)
				{
					part2();

					line_idx = 0;
					memset(line, 0, MAX_LINE_LENGTH);

					status = 0;
					break;
				}

				status = 0;
			}
			else if (status == _MSG_LOAD)
			{
				status = 0;
				maxline_count = 0;
				if (load_saved_game() == 0)
				{
					internal_string_print("\nGame restored\n");
					do_look();
				}
				else
					internal_string_print("\nError restoring");
			}
			else if (status == _MSG_SAVE)
			{
				status = 0;
				maxline_count = 0;
				if (save_game() == 0)
					internal_string_print("\nGame saved\n");
				else
					internal_string_print("\nError saving");
			}
		}

		if (quit_restart_flag == 1)
		{
			quit_restart_flag = 0;

			glk_window_clear();
			maxline_count = 0;
			gotoxy(0, height - 1);

			print_title_bar(game_hdr->game);

			internal_string_print("\n");
			internal_string_print("%s\n", game_hdr->game);
			internal_string_print("by ");
			internal_string_print("%s\n", game_hdr->author);
			internal_string_print("Version ");
			internal_string_print("%s\n\n", game_hdr->version);

			reset_game();
		}
	}

	free(save_name);
#ifdef HAVE_RAMLOAD_RAMSAVE
	free(ramsave_data);
#endif
	free(save_data);
	/*free(data);*/

	glk_window_clear();

	amiga_shutdown();

	/*return(0);*/
	exit(1);
}

