#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>

#include <em.h>
#include <time.h>

//#define DEBUG_OPCODES
#define USE_IT_HACK					// use "it" hack.
#define HAVE_RAMLOAD_RAMSAVE		// enable ramsave/ramrestore

/****************************************************************************/

#if defined(__C64__) || defined(__C128__)
#define USE_EXTRA_RAM
#endif

#define DEFAULT_GAME_FILE	"mystery.rai"	//"game.rai"

#define RAI_MAJOR	3
#define RAI_VERSION	"3.0"
#define HEAD_TITLE "RAI v" RAI_VERSION ""
#define RAI_COPYRIGHT "Copyright 2015 Stu George"


#include "main.h"
#include "game_structs.h"


// consists of
uint16_t save_data_length;
uint8_t *save_data;

// global data that is also saved
uint8_t light_flag;

// internal use flags
uint8_t quit_restart_flag;

// how many lines have scrolled
uint8_t scrolled_lines;

#if defined(__C128__)
#define MAX_LINE_LENGTH 80
#else
#define MAX_LINE_LENGTH 40
#endif
char line[MAX_LINE_LENGTH];

#define MAX_WORDS	2
uint8_t *words[MAX_WORDS];
int words_idx[MAX_WORDS];

uint8_t *save_name;

// preallocated for string printing
#define LOG_BUFFER_LEN 256
uint8_t strLogBuffer3[LOG_BUFFER_LEN];

uGame *game_hdr;
uint8_t *xdata;

#ifdef HAVE_RAMLOAD_RAMSAVE
// ramsave
void *ramsave_data;
#endif

// our app is about 30kb
// c128 -> 40kb free 40 - 30 -> 10
// c64 -> 50kb free 50 - 30 -> 20
// +4 -> 55kb free 55 - 30 -> 25
#if defined(__C64__)
#define XBUFFER_SIZE	(15 * 1024)
#elif defined(__C128__)
#define XBUFFER_SIZE	(2 * 1024)
#elif defined(__PLUS4__)
#define XBUFFER_SIZE	(20 * 1024)
#else
#error "Unknown machine type"
#endif

// exit means we dont run any more codeblocks.
#define CODE_RAN_EXIT_OK		0
#define CODE_RAN_EXIT_FAIL		1
// continue means we run codeblocks+actions
#define CODE_RAN_CONTINUE_OK	2
#define CODE_RAN_CONTINUE_FAIL	3
#define CODE_QUIT_RESTART		5


static uint8_t width;
static uint8_t height;
static uint8_t last_char_out;

static void c64_glk_put_string(char *s, uint8_t ascii_translate);
static void print_title_bar(char *msg);
static void internal_string_print(char *strX, ...);
static uint8_t run_codeblock(uint8_t *ptr);
static uint16_t get_pointer(uint8_t *ptr);
uint8_t get_keypress(void);

static void dump_verbs(void);

#if defined (__C64__) || defined (__C128__)
#define TEXT_RAM		0x400
#else
#define TEXT_RAM		0xC00
#endif



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
#ifdef USE_EXTRA_RAM
#ifdef __C64__
extern char x_reu;
#endif
extern char x_ram;

// which parts of the story are loaded where...
uint8_t ram_page_lo, ram_page_hi;
uint8_t reu_page_lo, reu_page_hi;


// total pages in reu system
uint16_t page_count;

// 1kb buffer
uint16_t last_page;

#define PAGE_BUFFER_SIZE	(EM_PAGE_SIZE * 2)
#define PAGE_BUFFER_SIZE_UP	(EM_PAGE_SIZE * 1)

uint8_t page_buffer[PAGE_BUFFER_SIZE];

static void fill_buffer_n(uint16_t page, uint16_t offs)
{
	if (page < ram_page_hi)
	{
		memmove(page_buffer + offs, xdata + (page * EM_PAGE_SIZE), EM_PAGE_SIZE);
	}
	else
	{
		uint8_t *p;
		p = em_map(page - ram_page_hi);
		memmove(page_buffer + offs, p, EM_PAGE_SIZE);

		/*
		struct em_copy cpy;

		cpy.buf = &page_buffer + offs;
		cpy.count = EM_PAGE_SIZE;
		cpy.offs = 0;
		cpy.page = page - ram_page_hi;
		em_copyfrom(&cpy);
		*/
	}
}

static uint8_t* fill_buffer(uint16_t page)
{
	if (last_page == page)
		return page_buffer;

	last_page = page;

	fill_buffer_n(page + 0, 0);
	fill_buffer_n(page + 1, EM_PAGE_SIZE);

	return (page_buffer);
}

static uint8_t* shift_up_buffer(uint8_t *buff)
{
	//if (buff >= (page_buffer + PAGE_BUFFER_SIZE_UP) && buff < (page_buffer + PAGE_BUFFER_SIZE))
	if (buff - page_buffer >= EM_PAGE_SIZE)
	{
		fill_buffer(last_page + 1);
		return (buff - EM_PAGE_SIZE);
	}

	return (buff);
}

static void bootstrap_reu(void)
{
	int rc;

#ifdef __C64__
	uint8_t *page;
	uint8_t zbuff[16];


	// nonstandard
	_randomize();

	for (rc = 0; rc < 16; rc++)
		zbuff[rc] = (rand() >> 3) & 0xFF;

	page_count = 0;

	rc = em_install(&x_reu);
	if (rc == EM_ERR_OK)
	{
		while (rc == EM_ERR_OK)     // em_err_ok = 0
		{
			cprintf("\rTest REU %ukb\r", page_count >> 2);

			page = em_map(page_count);

			if (memcmp(page, zbuff, 16) == 0)
			{
				rc = 1;
			}
			else
			{
				memmove(page, zbuff, 16);
				em_commit();
				memset(page, 0x55, EM_PAGE_SIZE);

				page = em_map(page_count);
				if (memcmp(page, zbuff, 16) == 0)
				{
					// 256 does 64kb at a time.
					// 512 does 128kb at a time.
					page_count += (uint16_t)256; //16;

					if (page_count == (uint16_t)0)
					{
						page_count = 0xFFFF;
						rc = 1;
					}
				}
				else
				{
					rc = 1;
				}
			}
		}

		return;
	}
	else

	{
		em_uninstall();
		cprintf("REU (NO). IO+Kernel RAM ");
		rc = em_install(&x_ram);
		if (rc != EM_ERR_OK)
		{
			em_uninstall();
			internal_string_print("(NO).\n");

			return;
		}
		else
		{
			page_count = em_pagecount();
			internal_string_print("%ukb\n", page_count >> 2);
		}
	}

#else
	//cprintf("Using IO+Kernel RAM ");
	rc = em_install(&x_ram);
	page_count = em_pagecount();
#endif
}
#endif

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
#ifdef __C64__
	memmove((uint8_t *)COLOR_RAM + width, (uint8_t *)COLOR_RAM + width * 2, (width * height) - width * 2);
	memmove((uint8_t *)TEXT_RAM + width, (uint8_t *)TEXT_RAM + width * 2, (width * height) - width * 2);
	cclearxy(0, height - 1, width);
#elif defined(__C128__)
	if (width < 80)
	{
		memmove((uint8_t *)COLOR_RAM + width, (uint8_t *)COLOR_RAM + width * 2, (width * height) - width * 2);
		memmove((uint8_t *)TEXT_RAM + width, (uint8_t *)TEXT_RAM + width * 2, (width * height) - width * 2);
		cclearxy(0, height - 1, width);
	}
	else
	{
		printf("\n");
	}

#elif defined(__PLUS4__)
	memmove((uint8_t *)COLOR_RAM + width, (uint8_t *)COLOR_RAM + width * 2, (width * height) - width * 2);
	memmove((uint8_t *)TEXT_RAM + width, (uint8_t *)TEXT_RAM + width * 2, (width * height) - width * 2);
	cclearxy(0, height - 1, width);
#else
	printf("\n");
#endif
}

static void c64_glk_put_char(uint8_t c, uint8_t ascii_translate)
{
	if (c == '\n' || c == '\r')
	{
		last_char_out += 1;
		scrolled_lines += 1;
	}
	else
	{
		last_char_out = 0;
	}

	if (ascii_translate == 1)
	{
		if (c >= 0x60 && c < 0x80)
			c -= 0x20;
		else if (c >= 0x40 && c < 0x60)
			c += 0x20;
	}

	if (c == '\n' || c == '\r')
	{
		if (wherey() == height - 1)
		{
			scroll_up();
			gotoxy(0, height - 1);
		}
		else
			cputs("\r\n");

		if (scrolled_lines >= height - 1)
		{
			gotoxy(0, height - 1);
			cputs("-- PAUSE --");
			get_keypress();
			gotoxy(0, height - 1);
			cputs("             ");
			gotoxy(0, height - 1);
			scrolled_lines = 0;
		}
	}
	else
		cputc(c);
}

static void c64_glk_put_string(char *s, uint8_t ascii_translate)
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
			c64_glk_put_char('\n', ascii_translate);

		//assert(wherex() + len < width);

		while (len > 0)
		{
			c64_glk_put_char(*s++, ascii_translate);
			len -= 1;
		}
	}
}

static void glk_exit(void)
{
	exit(0);
}

static void glk_window_clear(void)
{
	clrscr();
}

static void c64_glk_put_buffer(char *s, uint8_t length, uint8_t ascii_translate)
{
	while (length > 0)
	{
		c64_glk_put_char(*s++, ascii_translate);
		length -= 1;
	}
}

static void internal_string_print(char *strX, ...)
{
	va_list args;

	va_start(args, strX);
	vsprintf(strLogBuffer3, strX, args);
	va_end(args);

	c64_glk_put_string(strLogBuffer3, 0);
}

// internal shift print
static void datafile_print_string(uint8_t *strX, ...)
{
	va_list args;

	va_start(args, strX);
	vsprintf(strLogBuffer3, strX, args);
	va_end(args);

	c64_glk_put_string(strLogBuffer3, 1);
}

static void new_paragraph(void)
{
	switch (last_char_out)
	{
		case 0:
			c64_glk_put_string("\n\n", 0);
			break;

		case 1:
			c64_glk_put_string("\n", 0);
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
	c64_glk_put_string(strLogBuffer3, 0);
	va_end(args);

	glk_exit();
}

uint8_t get_keypress(void)
{
	return (cgetc());
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
	uint16_t left;

#ifdef USE_EXTRA_RAM
	uint16_t cpage;
	//uint8_t *page;
#endif


	sprintf(line, "Loading %s", game_file);
	print_title_bar(line);

	make_save_name(game_file);

	fp = fopen(game_file, "rb");
	if (fp == NULL)
		error("\n\nFailed to load %s. Try \"RUN: REM DATAFILENAME\"\n", game_file);

	fread(&game_headx, 1, sizeof(uGame), fp);
	fclose(fp);

	if (memcmp(game_headx.magic, "xadv", 4) != 0)
	{
		error("bad game file");
	}

	discombobulate_header(&game_headx);

	if (game_headx.interp_required_version > RAI_MAJOR)
	{
		error("This game requires an minimum interpreter version %i.00\n", game_headx.interp_required_version);
	}

	// 25k game file SHOULD give us enough
	// code space. CC65 gives us D000-0800 = C800 = 50kb
	// interpreter currentl runs 25k.
	game_length = game_headx.size_in_kb * 1024;       // give it 25k

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

	left = (game_headx.size_in_kb * 1024);

#ifdef USE_EXTRA_RAM
	// use 20kb low ram, plus reu ram.
	//game_length = page_count / (1024 / EM_PAGE_SIZE);
	//game_length += (XBUFFER_SIZE / EM_PAGE_SIZE);

	// convert buffer + REU code into kb, compare against game.
	game_length = page_count / 4; //(page_count * EM_PAGE_SIZE) / 1024;
	game_length += XBUFFER_SIZE / 1024;

	if (game_headx.size_in_kb > game_length)
	{
		error("Not enough memory for game file %ukb vs %ukb", game_headx.size_in_kb, game_length);
	}

#else
	if (game_headx.size_in_kb > (XBUFFER_SIZE / 1024))
	{
		error("Not enough memory for game file %ukb vs %ukb", game_headx.size_in_kb, game_length);
	}

#endif

	// c64 has no fseek
	fp = fopen(game_file, "rb");
	assert(fp != NULL);

	xdata = calloc(1, XBUFFER_SIZE);
	if (xdata == NULL)
	{
		fclose(fp);
		error("Not enough memory");
	}

	//left = XBUFFER_SIZE / 1024;
	game_length = 0;
	while (game_length < XBUFFER_SIZE)
	{
		sprintf(line, "Loading %s, %ukb ", game_file, left / 1024);
		print_title_bar(line);

		p = fread(xdata + game_length, 1, 1024, fp);
		left -= p;
		game_length += 1024;

		if (p != 1024)
		{
			game_length = XBUFFER_SIZE;
		}
	}

#ifdef USE_EXTRA_RAM
	ram_page_lo = 0;
	reu_page_lo = 0;
	ram_page_hi = XBUFFER_SIZE / EM_PAGE_SIZE;

	cpage = 0;

	do
	{
		struct em_copy cpy;

		sprintf(line, "Loading %s, %ukb ", game_file, left / 1024);
		print_title_bar(line);

		memset(page_buffer, 0x0, EM_PAGE_SIZE);
		p = fread(page_buffer, 1, EM_PAGE_SIZE, fp);
		left -= p;

		cpy.buf = &page_buffer;
		cpy.count = EM_PAGE_SIZE;
		cpy.offs = 0;
		cpy.page = cpage;
		em_copyto(&cpy);

		cpage += 1;
	}while (cpage < page_count && p == EM_PAGE_SIZE);

	// mark our top page
	reu_page_hi = cpage;
#endif

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
#ifdef USE_EXTRA_RAM
	uint16_t page;

	page = game_hdr->offs_string_table + 2 * idx;
	ptr = fill_buffer(page / EM_PAGE_SIZE) + (page % EM_PAGE_SIZE);
	page = get_pointer(ptr);

	return (fill_buffer(page / EM_PAGE_SIZE) + (page % EM_PAGE_SIZE));
#else
	ptr = xdata + game_hdr->offs_string_table + (idx * 2);
	return (xdata + get_pointer(ptr));
#endif
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

	ptr = save_data + 1;                    // current player
	ptr += game_hdr->item_count;
	ptr += 2 * game_hdr->counter_count;

	return (ptr);
}

static uint16_t* get_counter_offset(void)
{
	uint8_t *ptr;

	ptr = save_data + 1;        // current player
	ptr += game_hdr->item_count;

	return(uint16_t *)ptr;
}

static uint8_t* get_item_location_offset(void)
{
	uint8_t *ptr;

	ptr = save_data + 1;        // current player

	return (ptr);
}

static uint8_t* get_base_player_offs(void)
{
	uint8_t *ptr;

	ptr = save_data + 1;        // current player
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
	uint8_t *ptr;
	uint8_t *x;
	uint8_t *xx;
	uint16_t len;
	uint16_t z;

#ifdef USE_EXTRA_RAM
	uint16_t lp = last_page;
	uint16_t lp2;
	uint16_t lp3;
#endif

	x = get_string(idx);
	xx = x;
	len = 0;

#ifdef USE_EXTRA_RAM
	lp2 = last_page;
#endif

	while ((z = get_pointer(x)) != 0)
	{
		x += 2;
		if (game_hdr->interp_required_version > 2)
		{
#ifdef USE_EXTRA_RAM
			lp3 = last_page;
			ptr = fill_buffer(z / EM_PAGE_SIZE) + (z % EM_PAGE_SIZE);
#else
			ptr = xdata + z;
#endif
			len += strlen(ptr);
#ifdef USE_EXTRA_RAM
			fill_buffer(lp3);
#endif
		}
		else
		{
			len += 1 + (z >> 12);
			if (z >= 0xF000)
			{
				z = get_pointer(x);
				x += 2;
				len += (z >> 12);
			}
		}

#ifdef USE_EXTRA_RAM
		x = shift_up_buffer(x);
#endif
	}

#ifdef USE_EXTRA_RAM
	fill_buffer(lp2);
#endif
	// pointer comes into page_buffer, we have a 1kb buffer to play with.
	x = xx;

	xx = calloc(1, len + 16);
	ptr = xx;

	while ((z = get_pointer(x)) != 0)
	{
#ifdef USE_EXTRA_RAM
		uint8_t *pbuff;

		len = last_page;

		x += 2;

		if (game_hdr->interp_required_version > 2)
		{
			pbuff = fill_buffer(z / EM_PAGE_SIZE) + (z % EM_PAGE_SIZE);
			memmove(ptr, pbuff, strlen(pbuff));
			ptr += strlen(pbuff);
		}
		else
		{
			pbuff = fill_buffer((z & 0xFFF) / EM_PAGE_SIZE) + ((z & 0xFFF) % EM_PAGE_SIZE);
			memmove(ptr, pbuff, (z >> 12));
			ptr += (z >> 12);

			while (z >= 0xF000)
			{
				fill_buffer(len);
				z = get_pointer(x);
				x += 2;
				pbuff = fill_buffer((z & 0xFFF) / EM_PAGE_SIZE) + ((z & 0xFFF) % EM_PAGE_SIZE);
				memmove(ptr, pbuff, (z >> 12));
				ptr += (z >> 12);
			}
		}

		fill_buffer(len);
		x = shift_up_buffer(x);
#else
		x += 2;

		if (game_hdr->interp_required_version > 2)
		{
			memmove(ptr, xdata + z, strlen(xdata + z));
			ptr += strlen(xdata + z);
		}
		else
		{
			memmove(ptr, xdata + (z & 0xFFF), (z >> 12));
			ptr += (z >> 12);

			while (z >= 0xF000)
			{
				z = get_pointer(x);
				x += 2;
				memmove(ptr, xdata + (z & 0xFFF), (z >> 12));
				ptr += (z >> 12);
			}
		}

#endif
		*ptr = ' ';
		ptr++;
		*ptr = 0;
	}

	// trim trailing space
	ptr--;
	if (*ptr == ' ')
	{
		*ptr = 0;
	}

#ifdef USE_EXTRA_RAM
	fill_buffer(lp);
#endif

	return (xx);
}

static void print_string(uint16_t idx)
{
	uint8_t * ptr,*optr,*x;
	uint8_t z;

#ifdef USE_EXTRA_RAM
	uint16_t lp;
	lp = last_page;
#endif

	optr = get_stringX(idx);
	ptr = optr;
	x = strLogBuffer3;

	z = 0;
	x[z] = 0;

	while (*ptr != 0)
	{
		if (*ptr >= 0x01 && *ptr <= 0x4)
		{
			c64_glk_put_string((char *)x, 1);

			switch (*ptr)
			{
				case 0x01:
					ptr++;
					sprintf((char *)x, "%i", get_counter(ptr[0]));
					c64_glk_put_string((char *)x, 0);
					break;

				case 0x02:
					c64_glk_put_string(words[0], 0);
					break;

				case 0x03:
					c64_glk_put_string(words[1], 0);
					break;

				case 0x04:
					c64_glk_put_string("\n", 0);
					break;
			}

			ptr++;
			z = 0;
			x[z] = 0;
		}
		else
		{
			x[z++] = *ptr;
			x[z] = 0;

			if (*ptr == ' ')
			{
				ptr++;

				c64_glk_put_string((char *)x, 1);
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

	c64_glk_put_string((char *)x, 1);

	free(optr);
#ifdef USE_EXTRA_RAM
	fill_buffer(lp);
#endif
}

static void call_global_codeblock(uint8_t id)
{
#ifdef USE_EXTRA_RAM
	uint16_t ptr;
	uint8_t *p;
	uint16_t lp;

	lp = last_page;

	ptr = game_hdr->offs_global_codeblocks;

	p = fill_buffer(ptr / EM_PAGE_SIZE) + (ptr % EM_PAGE_SIZE);
	ptr = get_pointer(p + (id * 2));

	if (ptr != 0)
	{
		p = fill_buffer(ptr / EM_PAGE_SIZE) + (ptr % EM_PAGE_SIZE);

		run_codeblock(p);
	}

	fill_buffer(lp);
#else
	uint8_t *ptr;
	ptr = xdata + game_hdr->offs_global_codeblocks;
	ptr = xdata + get_pointer(ptr + id * 2);
	if (ptr != xdata)
		run_codeblock(ptr);

#endif
}

static uint8_t get_item_flags(uint8_t idx)
{
	uint8_t *ptr; // = data + game_hdr->offs_item_data;
	uint8_t i;

#ifdef USE_EXTRA_RAM
	uint16_t lp;
	lp = last_page;
	ptr = fill_buffer(game_hdr->offs_item_data / EM_PAGE_SIZE) + (game_hdr->offs_item_data % EM_PAGE_SIZE);
#else
	ptr = xdata + game_hdr->offs_item_data;
#endif

	for (i = 0; i < game_hdr->item_count; i++)
	{
		if (ptr[0] == idx)
		{
			i = ptr[2];
#ifdef USE_EXTRA_RAM
			fill_buffer(lp);
#endif
			return (i & 0xFF);
		}
		else
		{
			ptr += 6; // skip flags, id etc.
		}

#ifdef USE_EXTRA_RAM
		ptr = shift_up_buffer(ptr);
#endif
	}

#ifdef USE_EXTRA_RAM
	fill_buffer(lp);
#endif
	return (0);
}

static char* get_player_name(uint8_t idx)
{
	uint8_t *ptr;
	uint8_t i;

#ifdef USE_EXTRA_RAM
	uint16_t lp = last_page;
	ptr = fill_buffer(game_hdr->offs_item_data / EM_PAGE_SIZE) + (game_hdr->offs_item_data % EM_PAGE_SIZE);
#else
	ptr = xdata + game_hdr->offs_item_data;
#endif
	ptr += 6 * (game_hdr->item_count);
#ifdef USE_EXTRA_RAM
	ptr = shift_up_buffer(ptr);
#endif

	for (i = 0; i < game_hdr->player_count; i++)
	{
		if (ptr[0] == idx)
		{
			ptr += 4;
			ptr = get_stringX(get_pointer(ptr));
#ifdef USE_EXTRA_RAM
			fill_buffer(lp);
#endif
			return ptr;
		}
		else
		{
			ptr += 6; // skip flags, id etc.
		}

#ifdef USE_EXTRA_RAM
		ptr = shift_up_buffer(ptr);
#endif
	}

#ifdef USE_EXTRA_RAM
	fill_buffer(lp);
#endif
	return (strdup("??"));
}

static char* get_item_name(uint8_t idx)
{
	uint8_t *ptr;
	uint8_t i;

#ifdef USE_EXTRA_RAM
	uint16_t lp = last_page;
	ptr = fill_buffer(game_hdr->offs_item_data / EM_PAGE_SIZE) + (game_hdr->offs_item_data % EM_PAGE_SIZE);
#else
	ptr = xdata + game_hdr->offs_item_data;
#endif

	for (i = 0; i < game_hdr->item_count; i++)
	{
		if (ptr[0] == idx)
		{
			ptr += 4;
			ptr = get_stringX(get_pointer(ptr));
#ifdef USE_EXTRA_RAM
			fill_buffer(lp);
#endif
			return ptr;
		}
		else
		{
			ptr += 6; // skip flags, id etc.
		}

#ifdef USE_EXTRA_RAM
		ptr = shift_up_buffer(ptr);
#endif
	}

#ifdef USE_EXTRA_RAM
	fill_buffer(lp);
#endif
	return (strdup("??"));
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

				datafile_print_string(q);
				free(q);

				if (j < count - 1)
					datafile_print_string(", ");
				else
					datafile_print_string(".");

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

			datafile_print_string(line);
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
	uint8_t i;

#ifdef USE_EXTRA_RAM
	uint16_t lp;

	lp = last_page;
#endif

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
#ifdef USE_EXTRA_RAM
				fill_buffer(lp);
#endif
				return (1);
			}
		}

		x += 2;
	}

	// check items in current room
	if (is_light_in_room_x(get_player_current_room()) != 0)
	{
#ifdef USE_EXTRA_RAM
		fill_buffer(lp);
#endif
		return (1);
	}

#ifdef USE_EXTRA_RAM
	fill_buffer(lp);
#endif
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

#ifdef USE_EXTRA_RAM
	uint16_t lp;

	lp = last_page;

	//ptr = data + game_hdr->offs_room_data;
	//ptr += 2 * get_player_current_room();
	ptr = fill_buffer(game_hdr->offs_room_data / EM_PAGE_SIZE) + (game_hdr->offs_room_data % EM_PAGE_SIZE);
	ptr += 2 * get_player_current_room();
	ptr = shift_up_buffer(ptr);

	offs = get_pointer(ptr);
	ptr = fill_buffer(offs / EM_PAGE_SIZE) + (offs % EM_PAGE_SIZE);
#else
	ptr = xdata + game_hdr->offs_room_data;
	ptr += 2 * get_player_current_room();
	offs = get_pointer(ptr);
	ptr = xdata + offs;
#endif
	// ptr now points to room structure.

	//ptr += 1; // room id
	//ptr += 2; // string id
	//ptr += id * 2; // codeblock offset

	ptr += 3 + (id * 2);
#ifdef USE_EXTRA_RAM
	ptr = shift_up_buffer(ptr);
#endif
	offs = get_pointer(ptr);

	if (offs != 0)
	{
#ifdef USE_EXTRA_RAM
		ptr = fill_buffer(offs / EM_PAGE_SIZE) + (offs % EM_PAGE_SIZE);
#else
		ptr = xdata + offs;
#endif
		run_codeblock(ptr + 1);
	}

	if (id == XRM_LOOK)
	{
		you_can_see();
	}

#ifdef USE_EXTRA_RAM
	fill_buffer(lp);
#endif
}

static uint16_t get_room_title(void)
{
	uint8_t *ptr;

#ifdef USE_EXTRA_RAM
	uint16_t offs;

	ptr = fill_buffer(game_hdr->offs_room_data / EM_PAGE_SIZE) + (game_hdr->offs_room_data % EM_PAGE_SIZE);
	ptr += 2 * get_player_current_room();
	ptr = shift_up_buffer(ptr);
	offs = get_pointer(ptr);
	ptr = fill_buffer(offs / EM_PAGE_SIZE) + offs % EM_PAGE_SIZE;
#else
	ptr = xdata + game_hdr->offs_room_data;
	ptr += get_player_current_room() * 2;
	ptr = xdata + get_pointer(ptr);
#endif
	// skip room ID#
	ptr++;
	return (get_pointer(ptr));
}

static void print_room_title(void)
{
	uint16_t tid;

	tid = get_room_title();

	new_paragraph();
#ifdef __C128__
	if (width == 80)
	{
		textcolor(COLOR_WHITE);
	}
	else
	{
		textcolor(COLOR_WHITE);
	}

#elif defined(__C64__)
	textcolor(COLOR_WHITE);
#elif defined(__PLUS4__)
	textcolor(121);
#endif

	print_string(tid);

#ifdef __C128__
	if (width == 80)
	{
		textcolor(COLOR_GRAY3);
	}
	else
	{
		textcolor(COLOR_GRAY3);
	}

#elif defined(__C64__)
	textcolor(COLOR_GRAY3);
#elif defined(__PLUS4__)
	textcolor(81);
#endif
	c64_glk_put_string("\n", 0);
}

static void print_title_bar(char *msg)
{
	uint8_t *q;
	char *x;
	uint8_t len;
	uint8_t cx, cy;
	char score[32];
	uint16_t tid;

	if (msg == NULL && save_data != NULL)
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
		// SCORE is always var 1!
		sprintf(score, "Score: %i", get_counter(1));
		memmove(x + width - (strlen(score) + 1), score, strlen(score));
	}

	cx = wherex();
	cy = wherey();

	gotoxy(0, 0);
	revers(1);
	c64_glk_put_buffer(x, width, 1);
	revers(0);

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
#ifdef USE_EXTRA_RAM
	uint16_t lp;

	lp = last_page;
#endif
	if (is_light_present() == 1)
	{
		print_room_title();
		call_room_codeblock(XRM_LOOK);
	}
	else
	{
		call_global_codeblock(XGC_IN_DARK);
	}

#ifdef USE_EXTRA_RAM
	fill_buffer(lp);
#endif
}

static void pause_time(uint8_t t)
{
	time_t xx;

	xx = t + 1 + time(NULL);
	while (xx > time(NULL));
}

static uint8_t run_codeblock(uint8_t *ptr)
{
	// MAX TRY DEPTH
#ifdef USE_EXTRA_RAM
	uint16_t try_stack[8];
#else
	uint8_t *try_stack[8];
#endif
	uint8_t try_idx = 0;
	uint8_t *item_locn;
	uint8_t qflag;
	uint8_t do_not;

#ifdef USE_EXTRA_RAM
	uint16_t lp = last_page;
#endif

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
		"X_NOT"
	};
#endif

#ifdef USE_EXTRA_RAM
	ptr = shift_up_buffer(ptr);
#endif

	assert(ptr != NULL);

	if (quit_restart_flag != 0)
		return (CODE_RAN_EXIT_FAIL);

	item_locn = get_item_location_offset();

	while (*ptr != X_ENDOPCODES)
	{
		qflag = 0;

#ifdef USE_EXTRA_RAM
		ptr = shift_up_buffer(ptr);
#endif
#ifdef DEBUG_OPCODES
		//internal_string_print("%s. ", ops[*ptr]);
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
#ifdef USE_EXTRA_RAM
				try_stack[try_idx++] = ((ptr - page_buffer) + 2 + get_pointer(ptr)) + (last_page * EM_PAGE_SIZE);
#else
				try_stack[try_idx++] = ptr + 2 + get_pointer(ptr);
#endif
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

#ifdef USE_EXTRA_RAM
				ptr = shift_up_buffer(ptr);
#endif
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

			case X_ISFLAG:
				ptr++;

				if (get_bit(get_flag_offset(), ptr[0]) != ptr[1])
				{
					qflag = 1;
				}

				ptr += 2;
				break;

			case X_PAUSE:
				ptr++;
				pause_time(ptr[0]);
				ptr++;
				break;

			default:
				error("Unknown OPCODE %i", *ptr);
				break;
		}

		if ((qflag ^ do_not) == 1)
		{
			qflag = 0;
			if (try_idx > 0)
			{
				try_idx -= 1;

#ifdef USE_EXTRA_RAM
				ptr = fill_buffer(try_stack[try_idx] / EM_PAGE_SIZE) + try_stack[try_idx] % EM_PAGE_SIZE;
#else
				ptr = try_stack[try_idx];
#endif
				if (*ptr != X_ENDTRY)
				{
					error("PTR is not endtry!");
				}
			}
			else
			{
#ifdef USE_EXTRA_RAM
				fill_buffer(lp);
#endif
				return (CODE_RAN_CONTINUE_FAIL);
			}
		}
	}

#ifdef USE_EXTRA_RAM
	fill_buffer(lp);
#endif
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

	for (i = 0; i < MAX_WORDS; i++)
	{
		words[i] = NULL;
		words_idx[i] = -1;
	}

#ifdef __C128__
	if (width == 80)
	{
		textcolor(COLOR_WHITE);
	}
	else
	{
		textcolor(COLOR_WHITE);
	}

#elif defined(__C64__)
	textcolor(COLOR_WHITE);
#elif defined(__PLUS4__)
	textcolor(121);
#endif

	memset(line, 0, MAX_LINE_LENGTH);
	cursor(1);

	i = 0;
	while (line[i] != 0x0D)
	{
		line[i] = cgetc();
		switch (line[i])
		{
			case 17:
			case 29:
				break;

			case 0x7F:
			case 20:
				if (i > 0)
				{
					i--;
					gotoxy(wherex() - 1, wherey());
					cputc(' ');
					gotoxy(wherex() - 1, wherey());
					line[i] = 0;
				}

				break;

			case 0x0D:
			case 0x0A:
				break;

			default:
				if (i < MAX_LINE_LENGTH && (line[i] >= 0x20 && line[i] <= 0x7F))
				{
					cputc(line[i]);
					i++;
				}

				break;
		}
	}

	p = line;
	while (*p != 0x0)
	{
		*p = toupper(*p);

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


#ifdef __C128__
	if (width == 80)
	{
		textcolor(COLOR_GRAY3);
	}
	else
	{
		textcolor(COLOR_GRAY3);
	}

#elif defined(__C64__)
	textcolor(COLOR_GRAY3);
#elif defined(__PLUS4__)
	textcolor(81);
#endif
}

static int xcmp_petscii_ascii(uint8_t *petscii, uint8_t *ascii)
{
	uint8_t c;

	do
	{
		c = tolower(*petscii);
		if (c >= 0x40 && c <= 0x6F)
		{
			c += 0x20;
		}

		if (c != *ascii)
		{
			return (c > *ascii ? -1 : 1);
		}

		petscii++;
		ascii++;
	}while (*petscii != 0);

	if (*ascii == 0)
		return (0);

	return (c > *ascii ? -1 : 1);
}

static uint8_t scan_word(uint16_t offs, uint8_t *word)
{
	int8_t v;
	uint8_t *ptr;

#ifdef USE_EXTRA_RAM
	uint16_t lp;
	lp = last_page;

	ptr = fill_buffer(offs / EM_PAGE_SIZE) + (offs % EM_PAGE_SIZE);
#else
	ptr = xdata + offs;
#endif

	do
	{
		v = *ptr++;
		do
		{
			if (xcmp_petscii_ascii((char *)word, (char *)ptr) == 0)
			{
#ifdef USE_EXTRA_RAM
				fill_buffer(lp);
#endif
				return (v);
			}

			ptr = strchr(ptr, 0);
			ptr++;

		}while (*ptr != 0);
		ptr++;

#ifdef USE_EXTRA_RAM
		ptr = shift_up_buffer(ptr);
#endif

	}while (*ptr != 0);

#ifdef USE_EXTRA_RAM
	fill_buffer(lp);
#endif

	return (-1);
}

static uint8_t parse_action_blocks(void)
{
	uint8_t *ptr;

#ifdef USE_EXTRA_RAM
	uint16_t lp;
	uint16_t lp2;

	lp = last_page;
	ptr = fill_buffer(game_hdr->offs_action_codeblocks / EM_PAGE_SIZE) + (game_hdr->offs_action_codeblocks % EM_PAGE_SIZE);
	lp2 = last_page;
#else
	ptr = xdata + game_hdr->offs_action_codeblocks;
#endif

	while (*ptr != 0)
	{
		//internal_string_print("v=%i(%i), n=%i(%i/%i)\n", ptr[0], words_idx[0], ptr[1], words_idx[1], ptr[1]);
		if (ptr[0] == words_idx[0] && (ptr[1] == words_idx[1] || ptr[1] == 0))
		{
			int rc;

#ifdef USE_EXTRA_RAM
			lp2 = last_page;
			//ptr = shift_up_buffer(ptr);
#endif
			rc = run_codeblock(ptr + 4);
			switch (rc)
			{
				case CODE_QUIT_RESTART:
				case CODE_RAN_EXIT_FAIL:
				case CODE_RAN_EXIT_OK:
#ifdef USE_EXTRA_RAM
					fill_buffer(lp);
#endif
					return (rc);

				case CODE_RAN_CONTINUE_OK:
				case CODE_RAN_CONTINUE_FAIL:
#ifdef USE_EXTRA_RAM
					fill_buffer(lp2);
#endif
					break;
			}
		}

		//ptr += 2;
		ptr += get_pointer(2 + ptr) + 4;
#ifdef USE_EXTRA_RAM
		last_page = lp2;
		ptr = shift_up_buffer(ptr);
		lp2 = last_page;
#endif
	}

#ifdef USE_EXTRA_RAM
	fill_buffer(lp);
#endif

	return (CODE_RAN_CONTINUE_FAIL);
}

static uint8_t parse_room_blocks(void)
{
	uint8_t * ptr,*x;
	uint8_t i;
	uint16_t offs;

#ifdef USE_EXTRA_RAM
	uint16_t lp, lpx;

	lp = last_page;
	//ptr = data + game_hdr->offs_room_data;
	//ptr += (get_player_current_room() * 2);
	//ptr = data + get_pointer(ptr);

	ptr = fill_buffer(game_hdr->offs_room_data / EM_PAGE_SIZE) + (game_hdr->offs_room_data % EM_PAGE_SIZE);
	ptr += 2 * get_player_current_room();
	ptr = shift_up_buffer(ptr);

	offs = get_pointer(ptr);
	ptr = fill_buffer(offs / EM_PAGE_SIZE) + (offs % EM_PAGE_SIZE);
#else
	ptr = xdata + game_hdr->offs_room_data;
	ptr += (get_player_current_room() * 2);
	ptr = xdata + get_pointer(ptr);
#endif
	//ptr += 1;	// skip room id
	//ptr += 2;	// skip string id
	//ptr += 4;	// skip enter + look. leaves us with directional commands only.

	ptr += 7;

	// look + n/s/e/w/nw/ne/sw/se/u/d = 11
	for (i = 0; i < 11; i++)
	{
		offs = get_pointer(ptr);
		ptr += 2;
#ifdef USE_EXTRA_RAM
		ptr = shift_up_buffer(ptr);
		lpx = last_page;
#endif
		if (offs != 0)
		{
#ifdef USE_EXTRA_RAM
			x = fill_buffer(offs / EM_PAGE_SIZE) + (offs % EM_PAGE_SIZE);
#else
			x = xdata + offs;
#endif

			if (x[0] == words_idx[0])
			{
				// TODO: Fix page buffer for codeblock...
				i = run_codeblock(x + 1);
				// run a directional codeblock
				switch (i)
				{
					case CODE_QUIT_RESTART:
						// todo
#ifdef USE_EXTRA_RAM
						fill_buffer(lp);
#endif
						return (CODE_QUIT_RESTART);
						break;

						// if continue ok, parse room direction to new room.
					case CODE_RAN_CONTINUE_FAIL:
					case CODE_RAN_CONTINUE_OK:
					case CODE_RAN_EXIT_FAIL:
					case CODE_RAN_EXIT_OK:
						return (i);
						break;

				}
			}

#ifdef USE_EXTRA_RAM
			fill_buffer(lpx);
#endif
		}
	}

#ifdef USE_EXTRA_RAM
	fill_buffer(lp);
#endif
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

void main(int argc, char *argv[])
{
#ifdef USE_IT_HACK
	char *old_word2 = NULL;
#endif

	screensize(&width, &height);

#ifdef __C128__
	if (width == 80)
	{
		bgcolor(COLOR_BLACK);
		bordercolor(COLOR_BLACK);
		textcolor(COLOR_GRAY3);
		fast();
	}
	else
	{
		bgcolor(COLOR_BLUE);
		bordercolor(COLOR_BLUE);
		textcolor(COLOR_GRAY3);
	}

#elif defined(__C64__)
	bgcolor(COLOR_BLUE);
	bordercolor(COLOR_BLUE);
	textcolor(COLOR_GRAY3);
#elif defined(__PLUS4__)
	bgcolor(29);
	bordercolor(29);
	textcolor(81);
#endif

	glk_window_clear();
	print_title_bar(HEAD_TITLE);
	gotoxy(0, 1);

	reset_rnd_seed();

	internal_string_print("\n" HEAD_TITLE "\n" RAI_COPYRIGHT "\n");

#ifdef USE_EXTRA_RAM
	bootstrap_reu();
#endif
#ifdef HAVE_RAMLOAD_RAMSAVE
	internal_string_print("\n#ramsave and #ramrestore supported\n");
#endif
#ifdef USE_IT_HACK
	internal_string_print("'IT' hack supported (last verb used inplace of 'it')\n");
#endif
	// load game does a close on fp
	if (argc >= 2)
		load_game(argv[1]);
	else
		load_game(DEFAULT_GAME_FILE);

	print_title_bar(game_hdr->game);

	/* allocate save buffer */
	save_data_length = 1; // current player
	save_data_length += game_hdr->item_count;
	save_data_length += game_hdr->counter_count * 2;
	save_data_length += (game_hdr->flag_count + 7) / 8;
	save_data_length += game_hdr->player_count * 2;

	save_data = calloc(1, save_data_length);
#ifdef HAVE_RAMLOAD_RAMSAVE
	ramsave_data = calloc(1, save_data_length);
#endif

	internal_string_print("\n");
	datafile_print_string("%s\n", game_hdr->game);
	internal_string_print("by ");
	datafile_print_string("%s\n", game_hdr->author);
	internal_string_print("Version ");
	datafile_print_string("%s\n\n", game_hdr->version);

	// resets game & local data, calls pregame
	reset_game();

	while (quit_restart_flag == 0)
	{
		//print_title_bar(NULL);
		new_paragraph();
		scrolled_lines = 0;
		call_global_codeblock(XGC_PROMPT);

		if (quit_restart_flag == 0)
		{
			print_title_bar(NULL);
			read_input();
			if (words[0] == NULL)
			{
#ifdef USE_IT_HACK
				words[0] = 1 + strchr(words[1], 0x0);
				strcpy(words[0], "any");
#else
				words[0] = (uint8_t *)"any";
#endif
			}

			if (words[1] == NULL)
			{
#ifdef USE_IT_HACK
				words[1] = 1 + strchr(words[1], 0x0);
				strcpy(words[1], "any");
#else
				words[1] = (uint8_t *)"any";
#endif
			}

			new_paragraph();

#ifdef USE_IT_HACK
			if (old_word2 != NULL && stricmp((char *)words[1], "it") == 0)
			{
				// words[1] SHOULD be in input line buffer (except if its 'any')
				memmove(words[1], old_word2, strlen(old_word2) + 1);
			}
			else
			{
				if (old_word2 != NULL)
					free(old_word2);

				old_word2 = strdup((char *)words[1]);
			}

#endif

			if (stricmp("save", (char *)words[0]) == 0)
			{
				if (save_game() == 0)
					internal_string_print("\nGame saved\n");
				else
					internal_string_print("\nError saving");
			}
			else if (stricmp("restore", (char *)words[0]) == 0)
			{
				if (load_saved_game() == 0)
				{
					internal_string_print("\nGame restored\n");
					/*memmove(save_data, ramsave_data, save_data_length);*/
					do_look();
				}
				else
					internal_string_print("\nError restoring");
			}

#ifdef HAVE_RAMLOAD_RAMSAVE
			else if (stricmp("#ramrestore", (char *)words[0]) == 0)
			{
				internal_string_print("\nGame restored\n");
				memmove(save_data, ramsave_data, save_data_length);
				do_look();
			}
			else if (stricmp("#ramsave", (char *)words[0]) == 0)
			{
				memmove(ramsave_data, save_data, save_data_length);
				internal_string_print("\nGame saved\n");
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
		else
		{
			initial_clear_reset();
		}
	}

	free(save_name);
#ifdef HAVE_RAMLOAD_RAMSAVE
	free(ramsave_data);
#endif
	free(save_data);
	//free(data);

	glk_window_clear();
	gotoxy(0, 0);

#ifdef USE_EXTRA_RAM
	em_uninstall();
#endif

#ifdef __C64__
	// c64 reboot!
	asm("jmp $fce2");
#endif
#ifdef __C128__
	if (width > 40)
	{
		slow();
	}

	asm("jmp $E000");
#endif
}



