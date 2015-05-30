#ifndef AMIGA_MISSING_H
#define AMIGA_MISSING_H

#ifdef __cplusplus
extern "C"{
#endif

extern int stricmp(const char *cs, const char *ct);
extern char *strdup(const char *str);
extern char *strlower(char *s1);

#ifdef __cplusplus
};
#endif
#endif
