#ifndef MAIN_H
#define MAIN_H
#ifdef __cplusplus
extern "C"{
#endif

/* Rocks to use to keep track of stuff */
#define ROCK_WINDOW          1
#define ROCK_SOURCEFILE      2


extern winid_t mainwin;
extern winid_t scorewin;

extern void gprintf(char *strX, ...);
extern void error(char *msg, ...);
extern glui32 get_keypress(void);

#ifdef __cplusplus
};
#endif
#endif
