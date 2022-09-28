#ifndef DOSRAI_H
#define DOSRAI_H
#ifdef __cplusplus
extern "C"{
#endif

enum
{
	style_Header = 0,
	style_Normal
};

extern char *retro_filename;

extern uint16_t get_keypress(void);
extern void print_title_bar(char *s);

extern void glk_exit(void);
extern void glk_put_char(uint8_t c);
extern void glk_put_string(char *s);
extern void glk_window_clear(void);
extern void glk_put_buffer(char *s, uint8_t length);
extern void glk_set_style(int style);
extern void clear_input_buffer(void);
extern void read_input_buffer(char *buff, uint16_t max_len);

#ifdef __cplusplus
};
#endif
#endif        //  #ifndef DOSRAI_H
