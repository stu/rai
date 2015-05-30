#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>

#include <glk.h>

#define RAI_MAJOR	3
#define RAI_VERSION	"3.0"
#define RAI_COPYRIGHT "Copyright 2015 Stu George"

#define HAVE_RAMLOAD_RAMSAVE

//#define DEBUG_OPCODES

#ifdef BUILD_UNIXLIKE
#include "glkstart.h"
#include "unix_glkterm.h"
#ifdef GARGLK
#include <garversion.h>
#endif
#endif

#ifdef BUILD_WIN32
#include <winglk.h>
#include "win32_glk.h"
#endif


extern char *retro_filename;

#include "main.h"
#include "game_structs.h"

#ifdef BUILD_UNIXLIKE
#define stricmp strcasecmp
#endif

// use "it" hack.
#define USE_IT_HACK

uint8_t *save_name;

winid_t mainwin;
winid_t scorewin;

uint32_t game_length;
uGame *game_hdr;
uint8_t *data;

// ramsave
void *ramsave_data;


// consists of
int save_data_length;
uint8_t *save_data;

// global data that is also saved
uint8_t light_flag;

// internal use flags
uint8_t quit_restart_flag;

#define MAX_LINE_LENGTH 255
char line[MAX_LINE_LENGTH];

#define MAX_WORDS	8
uint8_t *words[MAX_WORDS];
int words_idx[MAX_WORDS];


uint8_t last_char_out;

// preallocated for string printing
char strLogBuffer3[1024];

// exit means we dont run any more codeblocks.
#define CODE_RAN_EXIT_OK		0
#define CODE_RAN_EXIT_FAIL		1
// continue means we run codeblocks+actions
#define CODE_RAN_CONTINUE_OK	2
#define CODE_RAN_CONTINUE_FAIL	3
#define CODE_QUIT_RESTART		5

static int run_codeblock(uint8_t *ptr);
static uint16_t get_counter(int idx);



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

static void set_bit(uint8_t *bits, int pos, int state)
{
	int mask = (0x80 >> (pos % 8));

	if (state != 0)
		bits[pos / 8] = bits[pos / 8] | mask;
	else
		bits[pos / 8] = bits[pos / 8] & (~mask);

}

static int get_bit(uint8_t *bits, int pos)
{
	int mask;
	mask = (0x80 >> (pos % 8));
	return (bits[pos / 8] & mask) == mask ? 1 : 0;
}


static uint16_t get_pointer(uint8_t *ptr)
{
	return (ptr[0] << 8) + ptr[1];
}


static void printstring(char *s)
{
	char *p;

	p = s;

	while (*p != 0x0)
	{
		switch (*p)
		{
			case 0x0A:
			case 0x0D:
				glk_put_string("\n");
				last_char_out += 1;
				p++;
				break;

			case '\t':
				glk_put_string("    ");
				last_char_out = 0;
				p++;
				break;

			default:
				last_char_out = 0;
				glk_put_char(*p);
				p++;
				break;
		}
	}
}

void gprintf(char *strX, ...)
{
	va_list args;

	va_start(args, strX);
	vsprintf(strLogBuffer3, strX, args);

	printstring(strLogBuffer3);

	va_end(args);
}

static void new_paragraph(void)
{
	switch (last_char_out)
	{
		case 0:
			gprintf("\n\n");
			break;
		case 1:
			gprintf("\n");
			break;
		default:
			break;
	}
}

void error(char *msg, ...)
{
	char strLogBuffer[128];

	va_list args;

	va_start(args, msg);
	vsprintf(strLogBuffer, msg, args);
	printstring(strLogBuffer);
	va_end(args);

	glk_exit();
}


glui32 get_keypress(void)
{
	event_t ev;
	glui32 key;

	/* cancel all input events pending. */
	glk_cancel_char_event(mainwin);
	glk_cancel_line_event(mainwin, &ev);

	glk_request_char_event(mainwin);
	do
	{
		glk_select(&ev);

		if (ev.type == evtype_CharInput)
			key = ev.val1;

	}while (ev.type != evtype_CharInput);

	return key;
}

void print_title_bar(char *s)
{
	glui32 width, height;          /* Status window dimensions */
	strid_t status_stream;          /* Status window stream */
	char *x;
	int len;
	char score[32];

	/* Measure the status window, and do nothing if height is zero. */
	glk_window_get_size(scorewin, &width, &height);

	x = calloc(1, width + 32);
	memset(x, ' ', width);

	len = strlen(s);
	if (len >= width - 0)
	{
		len = width - 0;
	}
	memmove(x + 1, s, len);

	if (save_data != NULL)
	{
		// SCORE is always var 1!
		sprintf(score, "Score: %i", get_counter(1));
		memmove(x + width - (strlen(score) + 1), score, strlen(score));
	}

	glk_set_window(scorewin);
	//glk_window_move_cursor(scorewin, 0, 0);
	glk_set_style(style_User1);
	glk_window_clear(scorewin);
	glk_put_buffer(x, width);

	//glk_window_move_cursor(scorewin, 0, 0);
	glk_set_window(mainwin);

	free(x);
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

static void load_game(strid_t realfd)
{
	glk_stream_set_position(realfd, 0x0, seekmode_Start);
	glk_stream_set_position(realfd, 0x0, seekmode_End);
	game_length = glk_stream_get_position(realfd);

	if (game_length == 0)
		error("Game file size is incorrect\n");

	glk_stream_set_position(realfd, 0x0, seekmode_Start);

	data = calloc(1, game_length + 16);
	if (data == NULL)
		error("Not enough memory for game file (%ikb)", (game_length / 1024) + 1);

	if (glk_get_buffer_stream(realfd, (void *)data, game_length) != game_length)
		error("bad data file");

	game_hdr = (void *)data;
	discombobulate_header(game_hdr);

	if (memcmp(game_hdr->magic, "XADV", 4) != 0)
		error("bad magic in file header");

	if (game_hdr->interp_required_version > RAI_MAJOR)
	{
		error("This game requires an minimum interpreter version v%i, we are v%i\n", game_hdr->interp_required_version, RAI_MAJOR);
	}

	/* bounds checking */
	if (game_hdr->offs_string_table > game_length)
		error("offset outside of file");
	if (game_hdr->offs_verb_table > game_length)
		error("offset outside of file");
	if (game_hdr->offs_room_data > game_length)
		error("offset outside of file");
	if (game_hdr->offs_item_data > game_length)
		error("offset outside of file");
	if (game_hdr->offs_noun_table > game_length)
		error("offset outside of file");
	if (game_hdr->offs_global_codeblocks > game_length)
		error("offset outside of file");
	if (game_hdr->offs_action_codeblocks > game_length)
		error("offset outside of file");

	game_hdr->game[0x1F] = 0;
	game_hdr->author[0x1F] = 0;
	game_hdr->version[0xF] = 0;
}

static char* get_string(int idx)
{
	uint8_t *ptr;

	ptr = data + game_hdr->offs_string_table + (idx * 2);
	ptr = data + get_pointer(ptr);

	return ptr;
}


static inline void set_current_player(int p)
{
	save_data[0] = p;
}

static inline uint8_t get_current_player(void)
{
	return save_data[0];
}

static uint8_t* get_flag_offset(void)
{
	uint8_t *ptr;

	ptr = save_data + 1;                    // current player
	ptr += game_hdr->item_count;
	ptr += 2 * game_hdr->counter_count;

	return ptr;
}

static uint16_t* get_counter_offset(void)
{
	uint8_t *ptr;

	assert(save_data != NULL);

	ptr = save_data + 1;        // current player
	ptr += game_hdr->item_count;

	return(uint16_t *)ptr;
}

static uint8_t* get_item_location_offset(void)
{
	uint8_t *ptr;

	ptr = save_data + 1;        // current player

	return ptr;
}

static uint8_t* get_base_player_offs(void)
{
	uint8_t *ptr;

	ptr = save_data + 1;        // current player
	ptr += game_hdr->item_count;
	ptr += 2 * game_hdr->counter_count;
	ptr += (game_hdr->flag_count + 7) / 8;

	return ptr;
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

	return ptr;
}

static uint8_t get_player_inventory_room(void)
{
	uint8_t *ptr;

	ptr = get_player_offs();

	return ptr[1];
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
	return ptr[0];
}

static void add_counter(int idx, int num)
{
	uint16_t *x;

	x = get_counter_offset();
	x[idx] += num;
}

static void set_counter(int idx, int num)
{
	uint16_t *x;

	x = get_counter_offset();
	x[idx] = num;
}

static uint16_t get_counter(int idx)
{
	uint16_t *x;

	x = get_counter_offset();
	return x[idx];
}

static uint8_t* get_stringX(int idx)
{
	uint8_t * ptr,*x,*xx;
	int offs;
	int len;
	uint16_t z;

	x = get_string(idx);
	ptr = x;
	len = 0;

	while (get_pointer(x) != 0)
	{
		z = get_pointer(x);
		x += 2;

		if (game_hdr->interp_required_version > 2)
		{
			len += strlen(data + z);
		}
		else
		{
			len += z >> 12;
			if (len == 15)
			{
				z = get_pointer(x);
				x += 2;
				len += z >> 12;
			}
		}

		len += 1;
	}

	x = ptr;
	xx = calloc(1, len + 4);
	ptr = xx;

	while (get_pointer(x) != 0)
	{
		z = get_pointer(x);

		x += 2;
		if (game_hdr->interp_required_version > 2)
		{
			len = strlen(data + z);
			offs = z;
		}
		else
		{
			offs = z & 0xFFF;
			len = z >> 12;
		}

		memmove(ptr, data + offs, len);
		ptr += len;

		if (len == 15 && game_hdr->interp_required_version < 3)
		{
			z = get_pointer(x);
			x += 2;
			offs = z & 0xFFF;
			len = z >> 12;
			memmove(ptr, data + offs, len);
			ptr += len;
		}

		*ptr = ' ';
		ptr++;
	}

	// trim trailing space
	ptr--;
	if (*ptr == ' ')
	{
		*ptr = 0;
	}

	return (xx);
}

static void print_string(int idx)
{
	uint8_t * ptr,*x,*xx,*optr;
	int offs;
	int len;
	uint16_t z;
	char *qq;

	optr = get_stringX(idx);
	ptr = optr;
	x = malloc(128);

	qq = x;
	z = 0;
	qq[z] = 0;

	while (*ptr != 0)
	{
		if (*ptr == 0x01)
		{
			gprintf(x);
			z = 0;
			qq = x;
			qq[z] = 0;

			ptr++;

			sprintf(qq, "%i", get_counter(ptr[0]));

			ptr++;

			gprintf(x);
			z = 0;
			qq = x;
			qq[z] = 0;
		}
		else if (*ptr == 0x02)
		{
			gprintf(x);
			z = 0;
			qq = x;
			qq[z] = 0;

			sprintf(qq, "%s", words[0]);
			ptr++;
			gprintf(x);
			z = 0;
			qq = x;
			qq[z] = 0;
		}
		else if (*ptr == 0x03)
		{
			gprintf(x);
			z = 0;
			qq = x;
			qq[z] = 0;

			sprintf(qq, "%s", words[1]);
			ptr++;
			gprintf(x);
			z = 0;
			qq = x;
			qq[z] = 0;
		}
		else if (*ptr == 0x04)
		{
			gprintf(x);
			z = 0;
			qq = x;
			qq[z] = 0;
			gprintf("\n");
			ptr++;
		}
		else
		{
			qq[z++] = *ptr;
			qq[z] = 0;

			if (*ptr == ' ')
			{
				gprintf(x);
				z = 0;
				qq = x;
				qq[z] = 0;
			}

			ptr++;
		}
	}

	gprintf(x);
	free(x);

	free(optr);
}


static int call_global_codeblock(int id)
{
	uint8_t *ptr;

	ptr = data + game_hdr->offs_global_codeblocks;
	ptr = data + get_pointer(ptr + id * 2);
	if (ptr != data)
		run_codeblock(ptr);
}

static uint8_t get_item_flags(int idx)
{
	uint8_t *ptr = data + game_hdr->offs_item_data;
	int i;

	for (i = 0; i < game_hdr->item_count; i++)
	{
		if (ptr[0] == idx)
		{
			return ptr[2];
		}
		else
		{
			ptr += 6; // skip flags, id etc.
		}
	}

	return 0;
}

static uint8_t* get_player_name(int idx)
{
	uint8_t *ptr = data + game_hdr->offs_item_data;
	int i;

	ptr += 6 * (game_hdr->item_count);

	for (i = 0; i < game_hdr->player_count; i++)
	{
		if (ptr[0] == idx)
		{
			ptr += 4;
			return get_stringX(get_pointer(ptr));
		}
		else
		{
			ptr += 6; // skip flags, id etc.
		}
	}

	return strdup("??");
}

static uint8_t* get_item_name(int idx)
{
	uint8_t *ptr = data + game_hdr->offs_item_data;
	int i;

	for (i = 0; i < game_hdr->item_count; i++)
	{
		if (ptr[0] == idx)
		{
			ptr += 4;
			return get_stringX(get_pointer(ptr));
		}
		else
		{
			ptr += 6; // skip flags, id etc.
		}
	}

	return strdup("??");
}

static int do_can_see(int room, int flags)
{
	int i, j;
	uint8_t * ptr,*q;

	ptr = get_item_location_offset();

	for (i = 0, j = 0; i < game_hdr->item_count; i++)
	{
		if (ptr[i] == room)
		{
			if (flags == 0 || (get_item_flags(i) & flags) == 0)
			{

				if (j > 0)
				{
					gprintf(", ");
				}

				q = get_item_name(i);
				gprintf("%s", q);
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
			if (j > 0)
			{
				gprintf(", ");
			}
			q = get_player_name(i);
			gprintf("%s", q);
			free(q);
			j++;
		}

		ptr += 2;
	}

	return j;
}

static uint8_t is_light_in_room_x(uint8_t room)
{
	uint8_t *x;
	uint8_t i;

	// test whats in room.
	x = get_item_location_offset();

	for (i = 0; i < game_hdr->item_count; i++)
	{
		// for each item, does it exist in player inventory or current room
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
	uint8_t *ptr;
	uint8_t i;

	// naturally lit?
	if (light_flag == 1)
	{
		return (1);
	}

	// check contents of all players inventory for light.
	//x = get_item_location_offset();
	z = get_player_current_room();
	x = get_base_player_offs();

	for (i = 0; i < game_hdr->player_count; i++)
	{
		// another player in same room as active player?
		if (x[0] == z)
		{
			if (is_light_in_room_x(get_character_inventory_room(i)) != 0)
			{
				return (1);
			}
		}

		x += 2;
	}

	// check items in current room
	if (is_light_in_room_x(get_player_current_room()) != 0)
	{
		return (1);
	}

	return (0);
}


static void you_can_see(void)
{
	int i, j;
	uint8_t *x, y;

	x = get_item_location_offset();
	y = get_player_current_room();

	for (i = 0, j = 0; i < game_hdr->item_count; i++)
	{
		if (x[i] == y)
		{
			if ((get_item_flags(i) & ITEM_SCENERY) == 0)
			{
				j++;
			}
		}
	}

	x = get_base_player_offs();
	for (i = 0; i < game_hdr->player_count; i++)
	{
		if (x[0] == y && i != get_current_player())
		{
			j++;
		}
		x += 2;
	}

	if (j > 0)
	{
		new_paragraph();
		gprintf("You can see : ");
		if (do_can_see(get_player_current_room(), ITEM_SCENERY) == 0)
			gprintf("Nothing special\n");
	}
}

static void show_inventory(void)
{
	new_paragraph();
	gprintf("Your inventory : ");
	if (do_can_see(get_player_inventory_room(), 0) == 0)
		gprintf("Is empty!\n");
}


static int call_room_codeblock(int id)
{
	uint8_t *ptr;
	int offs;

#ifdef DEBUG_OPCODES
	gprintf("call codeblock %i\n", id);
#endif

	ptr = data + game_hdr->offs_room_data;
	ptr += 2 * get_player_current_room();

	offs = get_pointer(ptr);
	ptr = data + offs;

	// ptr now points to room structure.

	ptr += 1; // room id
	ptr += 2; // string id
	ptr += id * 2; // codeblock offset

	offs = get_pointer(ptr);

	if (offs != 0)
	{
		run_codeblock(data + offs + 1);
	}

	if (id == XRM_LOOK)
	{
		you_can_see();
	}
}

static void header_line(void)
{
	uint8_t * ptr,*q;

	ptr = data + game_hdr->offs_room_data;
	ptr += get_player_current_room() * 2;
	ptr = data + get_pointer(ptr);

	// skip room ID#
	ptr++;

	q = get_stringX(get_pointer(ptr));
	print_title_bar(q);
	free(q);
}

static void print_room_title(void)
{
	uint8_t * ptr,*q;

	ptr = data + game_hdr->offs_room_data;
	ptr += get_player_current_room() * 2;
	ptr = data + get_pointer(ptr);

	// skip room ID#
	ptr++;

	glk_set_style(style_Header);

	new_paragraph();
	q = get_stringX(get_pointer(ptr));
	print_title_bar(q);
	free(q);
	print_string(get_pointer(ptr));
	gprintf("\n");

	glk_set_style(style_Normal);
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
				gprintf(" Quit");
				break;

			case 'R':
			case 'r':
				gprintf(" Restart");
				quit_restart_flag = 1;
				break;

			default:
				k = 0;
				break;
		}
	}
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

static void pause_time(uint8_t t)
{
	time_t xx;

	xx = t + 1 + time(NULL);
	while (xx > time(NULL));
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

static int run_codeblock(uint8_t *ptr)
{
	// MAX TRY DEPTH
	uint8_t *try_stack[8];
	int try_idx = 0;
	uint8_t *item_locn;
	uint8_t qflag;
	uint8_t do_not;

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
		"X_NOT",
	};
#endif

	if (quit_restart_flag != 0)
		return CODE_RAN_EXIT_FAIL;

	if (ptr - data >= game_length || ptr < data)
	{
		error("wild pointer! ptr = %p, data is %p, max is %p", ptr, data, data + game_length);
	}

	item_locn = get_item_location_offset();

	while (*ptr != X_ENDOPCODES)
	{
		qflag = 0;

#ifdef DEBUG_OPCODES
		gprintf("%s. ", ops[*ptr]);
#endif
		do_not = 0;
		if (*ptr == X_NOT)
		{
			ptr++;
			do_not = 1;
		}

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

				ptr += 3;
				break;

			case X_COUNTERGT:
				ptr++;
				if (!(get_counter(ptr[0]) > (get_pointer(1 + ptr))))
				{
					qflag = 1;
				}
				ptr += 3;
				break;

			case X_COUNTERLT:
				ptr++;
				if (!(get_counter(ptr[0]) < (get_pointer(1 + ptr))))
				{
					qflag = 1;
				}
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
				if (item_locn[ptr[0]] != get_player_current_room())
				{
					qflag = 1;
				}

				ptr++;
				break;

			case X_ISPRESENT:
				ptr++;
				if (item_locn[ptr[0]] != get_player_current_room() && item_locn[ptr[0]] != get_player_inventory_room())
				{
					qflag = 1;
				}

				ptr++;
				break;

			case X_CANCARRY:
				{
					int i = 0;
					int j = 0;
					int z;

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
				if (item_locn[ptr[0]] != get_player_inventory_room())
				{
					qflag = 1;
				}

				ptr++;
				break;

			case X_HERE:
				ptr++;
				item_locn[ptr[0]] = get_player_current_room();
				update_item_count();
				ptr++;
				break;

			case X_IN:
				ptr++;
				if (ptr[0] != get_player_current_room())
				{
					qflag = 1;
				}

				ptr++;
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
					gprintf("You can't carry anymore.\n");
					return CODE_RAN_CONTINUE_FAIL;
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
				//ignores dark flag.
				//call_room_codeblock(XRM_LOOK);
				do_look();
				break;

			case X_GOTO:
				ptr++;
				//set_player_current_room(*ptr);
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
				return CODE_QUIT_RESTART;
				break;

			case X_ISIMTEINROOM:
				ptr++;

				if (item_locn[ptr[0]] != ptr[1])
				{
					qflag = 1;
				}

				ptr += 2;
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
					return CODE_RAN_EXIT_OK;
				else
					return CODE_RAN_EXIT_FAIL;
				break;

			case X_CONTINUE:
				ptr += 2;
				if (ptr[-1] == 1)
					return CODE_RAN_CONTINUE_OK;
				else
					return CODE_RAN_CONTINUE_FAIL;
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
				return CODE_QUIT_RESTART;
				break;

			case X_NOUNIS:
				ptr++;
				if (ptr[0] != words_idx[1])
				{
					qflag = 1;
				}

				ptr++;
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

			case X_PAUSE:
				ptr++;
				pause_time(ptr[0]);
				ptr++;
				break;

			case X_ISFLAG:
				ptr++;

				if (get_bit(get_flag_offset(), ptr[0]) != ptr[1])
				{
					qflag = 1;
				}

				ptr += 2;
				break;

			default:
				error("Unknown OPCODE %i", *ptr);
				break;
		}

		if ((qflag ^ do_not) == 1)
		{
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

	return CODE_RAN_CONTINUE_OK;
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
	event_t event;
	glui32 key;
	uint8_t *p;
	int i;

	for (i = 0; i < MAX_WORDS; i++)
	{
		words[i] = NULL;
		words_idx[i] = -1;
	}


	glk_cancel_char_event(mainwin);
	glk_cancel_line_event(mainwin, &event);

	do
	{
		memset(line, 0, MAX_LINE_LENGTH);
		glk_request_line_event(mainwin, line, MAX_LINE_LENGTH - 2, 0);
		glk_select(&event);

		switch (event.type)
		{
			case evtype_LineInput:
				line[event.val1] = 0;   /* Null terminate it */
				break;
		};
	} while (event.type != evtype_LineInput);

	p = line;
	i = 0;

	while (*p != 0x0 && *p != 0x0D && *p != 0x0A && i < MAX_WORDS)
	{
		while ((*p == 0x20 || *p == 0x09) && *p != 0x0)
			p++;

		words[i++] = p;

		while (*p != 0x0 && *p != 0x20 && *p != 0x09)
			p++;

		if (*p != 0x0)
		{
			*p = 0;
			p++;
		}
	}

	last_char_out = 2;
}

// convert into verb index
static int scan_word(uint32_t offs, uint8_t *word)
{
	uint8_t *ptr;
	int v;

	ptr = data + offs;

	do
	{
		v = *ptr++;

		do
		{
			if (stricmp(word, ptr) == 0)
			{
				return v;
			}

			ptr = strchr(ptr, 0);
			ptr++;

		}while (*ptr != 0);

		ptr++;
	}while (*ptr != 0);

	return -1;
}

static int parse_action_blocks(void)
{
	uint8_t * ptr,*x;
	int i;
	int offs;

	ptr = data + game_hdr->offs_action_codeblocks;

	while (*ptr != 0)
	{
		//gprintf("v=%i(%i), n=%i(%i/%i)\n", ptr[0], words_idx[0], ptr[1], words_idx[1], ptr[1]);

		if (ptr[0] == words_idx[0] && (ptr[1] == words_idx[1] || ptr[1] == 0))
		{
			int rc;

			rc = run_codeblock(ptr + 4);

			switch (rc)
			{
				case CODE_QUIT_RESTART:
				case CODE_RAN_EXIT_FAIL:
				case CODE_RAN_EXIT_OK:
					return rc;

				case CODE_RAN_CONTINUE_OK:
				case CODE_RAN_CONTINUE_FAIL:
					break;
			}
		}

		ptr += 2;
		ptr += get_pointer(ptr) + 2;
	}

	return CODE_RAN_CONTINUE_FAIL;
}

static int parse_room_blocks(void)
{
	uint8_t * ptr,*x;
	int i;
	uint32_t offs;

	ptr = data + game_hdr->offs_room_data;
	ptr += (get_player_current_room() * 2);
	ptr = data + get_pointer(ptr);

	ptr += 1;   // skip room id
	ptr += 2;   // skip string id

	ptr += 4; // skip enter + look. leaves us with directional commands only.

	// look + n/s/e/w/nw/ne/sw/se/u/d = 11
	for (i = 0; i < 11; i++)
	{
		offs = get_pointer(ptr);
		ptr += 2;

		if (offs != 0)
		{
			x = data + offs;
			if (x[0] == words_idx[0])
			{

				i = run_codeblock(x + 1);
				// run a directional codeblock
				switch (i)
				{
					case CODE_QUIT_RESTART:
						return CODE_QUIT_RESTART;
						break;

						// if continue ok, parse room direction to new room.
					case CODE_RAN_CONTINUE_FAIL:
					case CODE_RAN_CONTINUE_OK:
					case CODE_RAN_EXIT_FAIL:
					case CODE_RAN_EXIT_OK:
						return i;
						break;

				}
			}
		}
	}

	return CODE_RAN_CONTINUE_OK;
}

static int load_saved_game(void)
{
	FILE *fp;
	fp = fopen(save_name, "rb");
	if (fp == NULL)
		return 1;

	fread(save_data, 1, save_data_length, fp);
	fclose(fp);

	return 0;
}

static int save_game(void)
{
	FILE *fp;
	fp = fopen(save_name, "wb");
	if (fp == NULL)
		return 1;

	fwrite(save_data, 1, save_data_length, fp);
	fclose(fp);
	return 0;
}

static void make_save_name(uint8_t *s)
{
	uint8_t *p;

	save_name = malloc(strlen(s) + 16);
	strcpy(save_name, s);

	p = strchr(save_name, 0x0);

	while (p != s && *p != '.')
	{
		p--;
	}

	if (p == save_name)
	{
		strcpy(save_name, "save.bin");
	}
	else
		sprintf(p, ".sav");
}

static void initial_clear_reset(void)
{
	if (quit_restart_flag == 1)
	{
		glk_set_window(mainwin);
		glk_window_clear(mainwin);
		print_title_bar(game_hdr->game);
		reset_game();
	}

}

void glk_main(void)
{
	strid_t sid = 0;
	frefid_t fid = 0;
	glui32 res;
#ifdef USE_IT_HACK
	char *old_word2 = NULL;
#endif
	reset_rnd_seed();

	glk_stylehint_set(wintype_TextGrid, style_User1, stylehint_ReverseColor, 1);

	mainwin = glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
	assert(mainwin != NULL);
	scorewin = glk_window_open(mainwin, winmethod_Above | winmethod_Fixed, 1, wintype_TextGrid, 0);

	glk_set_window(mainwin);
	glk_window_clear(mainwin);
	glk_set_style(style_Header);

#ifdef WINGLK_H_
	winglk_app_set_name("Retro Adventure Interpreter");
	winglk_window_set_title("RAI v" RAI_VERSION " - " RAI_COPYRIGHT "");
#endif

#ifdef GARGLK
	garglk_set_program_name("Retro Adventure Interpreter");
	garglk_set_program_info("RAI v" RAI_VERSION " - " RAI_COPYRIGHT "\n"
							"Gargoyle tweaks by Tor Andersson");
#endif

	gprintf("RAI v" RAI_VERSION " - Copyright (c) 2011 by Stu George\n");
	glk_set_style(style_Normal);

	if (retro_filename == NULL)
	{
		error("Missing game filename");
	}

	make_save_name(retro_filename);


	res = glk_gestalt(gestalt_Version, 0);
#ifndef GARGLK
	gprintf("Running on Glk v%i.%i.%i\n", (res >> 16), (res >> 8) & 0xFF, res & 0xFF);
#else
	gprintf("GarGlk version " VERSION "; Glk v%i.%i.%i\n", (res >> 16), (res >> 8) & 0xFF, res & 0xFF);
#endif

#ifdef HAVE_RAMLOAD_RAMSAVE
	gprintf("\n#ramsave and #ramload supported\n");
#endif
#ifdef USE_IT_HACK
	gprintf("'IT' hack supported (last verb used inplace of 'it')\n");
#endif

#if defined(WINGLK_H_)
	fid = winglk_fileref_create_by_name(fileusage_BinaryMode, retro_filename, 0, 0);
	if (fid == 0)
		error("Error creating fileref");

	sid = glk_stream_open_file(fid, filemode_Read, 0);
#elif defined(GARGLK) //|| defined(GT_START_H)
//#ifdef GARGLK
//fid = garglk_fileref_create_by_name(fileusage_BinaryMode, retro_filename, 0);
//sid = glk_stream_open_file(fid, filemode_Read, 0);
//#endif
	sid = glkunix_stream_open_pathname(retro_filename, 0, 0);
#else
	fid = glk_fileref_create_by_name(fileusage_BinaryMode, retro_filename, 0);
	if (fid == 0)
		error("Error creating fileref");

	sid = glk_stream_open_file(fid, filemode_Read, 0);
#endif

	if (sid == 0)
		error("Error opening stream [%s]", retro_filename);

	load_game(sid);
	glk_stream_close(sid, NULL);

	print_title_bar(game_hdr->game);

	if (fid != 0)
		glk_fileref_destroy(fid);

	gprintf("\n%s by %s\nVersion %s\n\n", game_hdr->game, game_hdr->author, game_hdr->version);


	/* allocate save buffer */
	save_data_length = 1; // current player
	save_data_length += game_hdr->item_count;
	save_data_length += game_hdr->counter_count * 2;
	save_data_length += (game_hdr->flag_count + 7) / 8;
	save_data_length += game_hdr->player_count * 2;

	save_data = calloc(1, save_data_length);
	ramsave_data = calloc(1, save_data_length);

	// resets game & local data, calls pregame
	reset_game();

	res = 0;

	while (quit_restart_flag == 0)
	{
		new_paragraph();
		call_global_codeblock(XGC_PROMPT);

		if (get_counter(1) != res && is_light_present() == 1)
		{
			header_line();
			res = get_counter(1);
		}

		if (quit_restart_flag == 0)
		{
			read_input();
			new_paragraph();

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

#ifdef USE_IT_HACK
			if (old_word2 != NULL && stricmp(words[1], "it") == 0)
			{
				memmove(words[1], old_word2, strlen(old_word2) + 1);
			}

			if (words[1] != NULL)
			{
				if (old_word2 != NULL)
					free(old_word2);

				old_word2 = strdup(words[1]);
			}
#endif

			if (stricmp("save", (char *)words[0]) == 0)
			{
				if (save_game() == 0)
					gprintf("\nGame was saved to disk.\n");
				else
					gprintf("\nError saving game.");
			}
			else if (stricmp("restore", (char *)words[0]) == 0)
			{
				if (load_saved_game() == 0)
				{
					gprintf("\nGame was restored from disk.\n");
					do_look();
				}
				else
					gprintf("\nError loading game.");
			}
#ifdef HAVE_RAMLOAD_RAMSAVE
			else if (stricmp("#ramload", words[0]) == 0)
			{
				memmove(save_data, ramsave_data, save_data_length);
				do_look();
			}
			else if (stricmp("#ramsave", words[0]) == 0)
			{
				memmove(ramsave_data, save_data, save_data_length);
				gprintf("\nGame was saved in ram.\n");
			}
#endif
			else
			{
				words_idx[0] = scan_word(game_hdr->offs_verb_table, words[0]);
				words_idx[1] = scan_word(game_hdr->offs_noun_table, words[1]);

				// run room blocks (directions), then global blocks then fail routine.
				switch (parse_room_blocks())
				{
					case CODE_RAN_CONTINUE_OK:
						switch (parse_action_blocks())
						{
							case CODE_RAN_CONTINUE_FAIL:

								if (words_idx[0] >= 1 && words_idx[0] <= 10)
								{
									gprintf("You can't go that way.");
									call_global_codeblock(XGC_ON_SUCCESS);
									break;
								}
								else
									call_global_codeblock(XGC_ON_FAIL);
								break;
							case CODE_RAN_CONTINUE_OK:
								call_global_codeblock(XGC_ON_SUCCESS);
								break;
							case CODE_RAN_EXIT_OK:
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

					case CODE_RAN_CONTINUE_FAIL:
						call_global_codeblock(XGC_ON_FAIL);
						break;

					case CODE_RAN_EXIT_OK:
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
			}
		}
		else
		{
			initial_clear_reset();
		}
	}

#ifdef HAVE_RAMLOAD_RAMSAVE
	free(ramsave_data);
#endif
	free(save_data);
	free(data);

	free(save_name);
}

