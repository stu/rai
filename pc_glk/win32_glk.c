#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include <glk.h>
#include <winglk.h>

#include "win32_glk.h"

char *retro_filename = NULL;

void do_startup(void)
{
	//winglk_set_gui(IDI_ICON1);
}

/* parse command line into argc/argv?? */
int winglk_startup_code(const char* cmdline)
{
	char *p;
	do_startup();

	if(cmdline != NULL)
	{
		p = cmdline;
		while(*p == 0x20)
		{
			p++;
		}
	}

	if(cmdline == NULL || *p == 0)
	{
		char *q;
		int z;

		p = (char*)winglk_get_initial_filename(cmdline,"Retro Adventure Interpreter","RAI Game Files (*.rai)|*.rai|All Files (*.*)|*.*||");
		if(p == NULL)
			glk_exit();

		retro_filename = strdup(p);		
	}
	else
	{
		retro_filename = strdup(cmdline);
	}

	p = strchr(retro_filename, 0x0);
	p--;
	while(p > retro_filename && *p == 0x20)
	{
		*p = 0;
		p--;
	}

	while(retro_filename[0] == 0x20)
	{
		memmove(retro_filename, retro_filename+1, strlen(retro_filename+1)+1);
	}
	
	return 1;
}
