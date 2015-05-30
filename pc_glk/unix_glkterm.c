
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include <glk.h>

#include "main.h"
#include "glkstart.h"
#include "unix_glkterm.h"

char *retro_filename = NULL;

glkunix_argumentlist_t 	glkunix_arguments[] =
{
	{ "",   glkunix_arg_ValueFollows,  "filename: The game file to load."},
	{ NULL, glkunix_arg_End, NULL }
};

int glkunix_startup_code(glkunix_startup_t *data)
{
	int i;

	if (data->argc == 2)
		retro_filename = data->argv[1];

	return TRUE;
}


