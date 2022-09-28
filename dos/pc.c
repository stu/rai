#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <i86.h>
#include <stdio.h>

#include "pc.h"

extern void glk_main(void);
extern uint16_t get_counter(int idx);
extern int save_data_length;
extern uint8_t *save_data;

extern uint8_t last_char_out;

char *retro_filename;
uint8_t colour_title = 0x70;
uint8_t colour_text = 0x17;
uint8_t colour_hl = 0x1F;

uint8_t text_colour;
uint8_t text_row;
uint8_t text_col;
uint8_t maxline_count;

enum eSCREEN_MODE
{
	SCREEN_MODE_DEFAULT = 0,
	SCREEN_MODE_80X25,
	SCREEN_MODE_80X28,
	SCREEN_MODE_80X43,
	SCREEN_MODE_80X50
};

void screen_clear(uint8_t colour);

extern uint16_t iskey(void);
#pragma aux iskey = \
	"mov ah,1"  \
	"int 0x16"  \
	"pushf" \
	"pop ax" \
	"and ax,0x40" \
	"xor ax,0x40" \
	value [ax] ;

extern uint16_t getkey(void);
#pragma aux getkey = \
	"xor ax,ax"  \
	"int 0x16"  \
	value [ax] ;


uint16_t get_keypress(void)
{
	return getkey() & 0xFF;
}


#ifdef __386__
uint8_t *p_screen = NULL;
uint8_t *p_backbuffer = NULL;
#else
uint8_t far *p_screen = NULL;
uint8_t far *p_backbuffer = NULL;
#endif

uint16_t screen_width;
uint16_t screen_height;

uint8_t cga_mda_flag;
uint8_t start_mode;


#if !(defined(_M_I386) || defined(__386__))
uint16_t get_curpos(void);
#pragma aux get_curpos =\
	"mov ah,0x3" \
	"xor bx,bx" \
	"int 0x10" \
	value [dx] \
	modify [ax bx cx];
#else
uint16_t get_curpos(void);
#pragma aux get_curpos =\
	"mov ah,0x3" \
	"xor ebx,ebx" \
	"int 0x10" \
	value [dx] \
	modify [eax ebx ecx];
#endif

#if !(defined(_M_I386) || defined(__386__))
void set_curpos(uint8_t row, uint8_t col);
#pragma aux set_curpos =\
	"mov ah,0x2" \
	"xor bx,bx" \
	"int 0x10" \
	parm [dh] [dl] \
	modify [ax bx cx];
#else
void set_curpos(uint8_t row, uint8_t col);
#pragma aux set_curpos =\
	"mov ah,0x2" \
	"xor ebx,ebx" \
	"int 0x10" \
	parm [dh] [dl] \
	modify [eax ebx ecx];
#endif

extern void hide_cursor(void);
#pragma aux hide_cursor = \
	"mov ah,1" \
	"mov cx,0x2607" \
	"int 0x10" \
	modify [ax bx cx dx];

extern void show_cursor(void);
#pragma aux show_cursor = \
	"mov ah,1" \
	"mov cx,0x0607" \
	"int 0x10" \
	modify [ax bx cx dx];

extern void disable_blink(void);
#pragma aux disable_blink = \
	"mov ax,0x1003"	\
	"xor bx,bx"		\
	"int 10h"		\
	modify    [ax bx];

extern void enable_blink(void);
#pragma aux enable_blink = \
	"mov ax,0x1003"	\
	"xor bx,bx"		\
	"mov bl,1"		\
	"int 0x10"		\
	modify    [ax bx];


extern void set_video_mode_80x25(void);
#pragma aux set_video_mode_80x25 = \
	"mov ax,3"	\
	"int 0x10"		\
	modify    [ax];

extern void set_video_mode_80x28(void);
#pragma aux set_video_mode_80x28 = \
	"mov ax,0x1202" \
	"mov bx,0x30" \
	"int 0x10" \
	"mov ax,3" \
	"int 0x10" \
	"mov ax,0x1111"	\
	"xor bx,bx"		\
	"int 0x10"		\
	modify    [ax bx];

extern void set_video_mode_80x50(void);
#pragma aux set_video_mode_80x50 = \
	"mov ax,0x1202" \
	"mov bx,0x30" \
	"int 0x10" \
	"mov ax,0x3"	\
	"int 0x10"		\
	"mov ax,0x1112"	\
	"xor bx,bx"		\
	"int 0x10"		\
	modify    [ax bx cx dx];

extern void set_video_mode_80x43(void);
#pragma aux set_video_mode_80x43 = \
	"mov ax,0x0003"	\
	"int 0x10"		\
	"mov ax,0x1114"	\
	"xor bx,bx"		\
	"int 0x10"		\
	"mov ax,0x1112"	\
	"xor bx,bx"		\
	"int 0x10"		\
	"mov ax,0x0003"	\
	"int 0x10"		\
	"mov ax,0x1112"	\
	"xor bx,bx"		\
	"int 0x10"		\
	modify    [ax bx];


void screen_set_startmode(uint8_t c)
{
	start_mode = c;
}

void screen_we_have_cga_mda(uint8_t c)
{
	cga_mda_flag = c;
}

// called by actions.c
void InitScreen(void)
{
#ifdef __386__
	uint8_t *low_seg = (uint8_t *)0x400;
	p_screen = (uint8_t *)0xB8000;
#else
	uint8_t far *low_seg = MK_FP(0x40, 0);
	p_screen = MK_FP(0xB800, 0);
#endif

	switch (start_mode)
	{
		case SCREEN_MODE_DEFAULT:
			break;

		case SCREEN_MODE_80X25:
			set_video_mode_80x25();
			break;

		case SCREEN_MODE_80X28:
			set_video_mode_80x28();
			break;

		case SCREEN_MODE_80X43:
			set_video_mode_80x43();
			break;

		case SCREEN_MODE_80X50:
			set_video_mode_80x50();
			break;
	}

	screen_width = low_seg[0x4A];
	screen_height = 1 + low_seg[0x84];

#ifdef __386__
	p_backbuffer = (uint8_t *)0xB8000 + ((screen_width * screen_height) * 2);
#else
	p_backbuffer = MK_FP(0xB800, ((screen_width * screen_height) * 2));
#endif

	// disable blink!
	if (cga_mda_flag == 0)
	{
		disable_blink();
	}

	screen_clear(0x7);

	text_col = 0;
	text_row = 0;
	text_colour = colour_text;

	show_cursor();
}

void ShutdownScreen(void)
{
	screen_clear(0x7);

	if (cga_mda_flag == 0)
	{
		enable_blink();
	}

	p_backbuffer = NULL;
}

uint16_t screen_get_width(void)
{
	return screen_width;
}

uint16_t screen_get_height(void)
{
	return screen_height;
}

void screen_line_clear(uint16_t row, uint8_t colour)
{
	int i;

	for (i = 0; i < screen_width; i++)
	{
		p_screen[(row * (screen_width << 1)) + (i << 1) + 0] = ' ';
		p_screen[(row * (screen_width << 1)) + (i << 1) + 1] = colour;
	}
}

void screen_print(uint16_t row, uint16_t col, uint8_t colour, uint8_t *s)
{
	uint16_t q;

	q = (row * (screen_width << 1)) + (col << 1);

	while (*s != NULL)
	{
		p_screen[q++] = *s++;
		p_screen[q++] = colour;
	}
}

void screen_print_center(uint16_t row, uint8_t colour, uint8_t *s)
{
	uint16_t w;

	w = screen_get_width();
	w -= strlen(s);
	w /= 2;
	screen_print(row, w, colour, s);
}


void screen_colour(uint16_t row, uint16_t col, uint8_t colour, uint8_t len)
{
	uint16_t q;

	q = (row * (screen_width << 1)) + (col << 1);

	while (len > 0)
	{
		q++;
		p_screen[q++] = colour;

		len -= 1;
	}
}

void screen_outch(uint16_t row, uint16_t col, uint8_t colour, uint8_t s)
{
	p_screen[(row * (screen_width << 1)) + (col << 1) + 0] = s;
	p_screen[(row * (screen_width << 1)) + (col << 1) + 1] = colour;
}

void screen_clear(uint8_t colour)
{
	int i;
	int j;

	for (i = 0; i < screen_height; i++)
	{
		for (j = 0; j < screen_width; j++)
		{
			p_screen[(i * (screen_width << 1)) + (j << 1) + 0] = ' ';
			p_screen[(i * (screen_width << 1)) + (j << 1) + 1] = colour;
		}
	}

	text_col = 0;
	text_row = 0;
}


void print_title_bar(char *s)
{
	int len;
	char score[32];
	char x[128];

	memset(x, ' ', screen_get_width());

	len = strlen(s);
	if (len >= screen_get_width() - 0)
	{
		len = screen_get_width() - 0;
	}
	memmove(x + 1, s, len);

	if (save_data != NULL)
	{
		// SCORE is always var 1!
		sprintf(score, "Score: %i", get_counter(1));
		memmove(x + screen_get_width() - (strlen(score) + 1), score, strlen(score));
	}

	x[screen_get_width()]=0;
	screen_print(0, 0, colour_title, x);
}



static void scroll_up(void)
{
	uint16_t i;
	uint16_t line_width = screen_width * 2;

	memmove(p_screen + line_width, p_screen + (line_width<<1), (((screen_height-2)*screen_width)*2));
	screen_line_clear(screen_get_height()-1, colour_text);
}

static void gotoxy(uint8_t c, uint8_t r)
{
	text_row = r;
	text_col = c;
	set_curpos(text_row, text_col);
}

static uint8_t wherey(void)
{
	return text_row;
}

static uint8_t wherex(void)
{
	return text_col;
}

void glk_put_char(uint8_t c)
{
	if (c == '\n' || c == '\r')
		last_char_out += 1;
	else
		last_char_out = 0;

	if (c == '\n' || c == '\r')
	{
		if (wherey() == screen_get_height() - 1)
		{
			scroll_up();
			gotoxy(0, screen_get_height() - 1);
		}
		else
		{
			gotoxy(0, wherey() + 1);
		}

		maxline_count += 1;
		if (maxline_count >= screen_get_height() - 2)
		{
			uint16_t saved_r;

			maxline_count = 0;

			saved_r = text_row;

			gotoxy(0, screen_get_height() - 1);
			screen_print_center(screen_get_height()-1, colour_hl, "--( press any key for more )--");
			get_keypress();

			screen_line_clear(screen_get_height()-1, colour_text);
			gotoxy(0, saved_r);
		}
	}
	else
	{
		screen_outch(text_row, text_col, text_colour, c);
		text_col += 1;
	}

	set_curpos(text_row, text_col);
}

void glk_put_string(char *s)
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

		if (wherex() + len >= screen_get_width())
			glk_put_char('\n');

		while (len > 0)
		{
			glk_put_char(*s++);
			len -= 1;
		}
	}
}

void glk_window_clear(void)
{
	screen_clear(colour_text);
	gotoxy(0, 1);
}

void glk_exit(void)
{
	ShutdownScreen();
	set_video_mode_80x25();
	exit(0);
}

void glk_set_style(int style)
{
	switch(style)
	{
		case style_Header:
			text_colour = colour_hl;
			break;

		default:
		case style_Normal:
			text_colour = colour_text;
			break;
	}
}

void clear_input_buffer(void)
{
	while(iskey() != 0)
		get_keypress();
}

void read_input_buffer(char *buff, uint16_t max_len)
{
	uint16_t key;
	int i;

	uint16_t r, c;

	memset(buff, 0, max_len);

	r = text_row;
	c = text_col;
	i = 0;

	maxline_count = 0;
	key = 0;

	do
	{
		key = get_keypress() & 0xFF;
		if(key >= 0x20 && key <= 0x7F)
		{
			screen_outch(r, c + i, text_colour, key);
			buff[i++] = key;
			buff[i] = 0;
			set_curpos(text_row, c + i);
		}
		else if(key == 0x08)
		{
			// bs
			if(i > 0)
			{
				i -= 1;
				buff[i] = 0;
				screen_outch(r, c + i, text_colour, ' ');
				set_curpos(text_row, c + i);
			}
		}
	}while (key != 0x0D && i < max_len-2);

	text_col = c + i;
	glk_put_string("\n");
	//last_char_out = 2;
}

static void header(void)
{
	printf("RAI - Retro Adventure Interpreter\n");
}

static void syntax(void)
{
	printf("\n");
	printf("rai filename.rai\n");
}

int main(int argc, char *argv[])
{
	int i;

	retro_filename = NULL;
	for(i=1; i < argc; i++)
	{
		if(strcmp(argv[i], "-?") == 0 || strcmp(argv[i], "/?") == 0)
		{
			header();
			syntax();
			exit(0);
		}
		else if(retro_filename == NULL)
		{
			retro_filename = strdup(argv[i]);
		}
		else
		{
			header();
			syntax();
			exit(0);
		}
	}

	if(retro_filename == NULL)
	{
		header();
		syntax();
		exit(0);
	}

	InitScreen();
	glk_set_style(style_Normal);
	glk_main();

	ShutdownScreen();
	set_video_mode_80x25();
	printf("Retro Adventure Interpreter\n");
	printf("\n- Thanks for playing!\n\n");

	return 0;
}
