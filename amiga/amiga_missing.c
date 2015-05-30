
#include <string.h>

#include "amiga_missing.h"

char *strlower(char *s1)
{
	char *c;
	int ch;

	c = s1;

	while (*c != 0x0)
	{
		ch = c[0];
		c[0] = tolower(ch);
		c++;
	}

	return(s1);
}

char *strdup(const char *str)
{
	size_t len;
	char *newstr;

	if (str == NULL)
		return(char *)NULL;

	len = strlen(str);

	if (len >= ((size_t)-1) / sizeof(char))
		return(char *)NULL;

	newstr = (char *) malloc((len+1)*sizeof(char));
	if (!newstr)
		return(char *)NULL;

	memcpy(newstr,str,(len+1)*sizeof(char));

	return(newstr);
}

int stricmp(const char *cs, const char *ct)
{
	int c1, c2;

	do
	{
		c1 = *cs++;
		c2 = *ct++;
		c1 = tolower(c1);
		c2 = tolower(c2);
	}while ((c1 == c2) && (c1 != 0x0));

	return(c1-c2);
}

