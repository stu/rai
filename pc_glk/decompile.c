
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>

#include "game_structs.h"

uint32_t game_length;
uGame *game_hdr;

uint8_t *data;

char strLogBuffer3[1024];

static void print_msg(int idx);

void gprintf(char *strX, ...)
{
	va_list args;

	va_start(args, strX);
	vsprintf(strLogBuffer3, strX, args);

	fprintf(stdout, "%s", strLogBuffer3);

	va_end(args);
}

void error(char *msg, ...)
{
	char strLogBuffer[128];

	va_list args;

	va_start(args, msg);
	vsprintf(strLogBuffer, msg, args);
	fprintf(stdout, "%s", strLogBuffer);
	va_end(args);

	exit(1);
}


static uint16_t swap_bytes(uint8_t *ptr)
{
	return(ptr[0] << 8) + ptr[1];
}


static uint16_t get_pointer(uint8_t *ptr)
{
	return(ptr[0] << 8) + ptr[1];
}

static void discombobulate_header(uGame *g)
{
	g->string_count = swap_bytes((void*)&g->string_count);
	g->offs_string_table = swap_bytes((void*)&g->offs_string_table);
	g->offs_verb_table = swap_bytes((void*)&g->offs_verb_table);
	g->offs_room_data = swap_bytes((void*)&g->offs_room_data);
	g->offs_item_data = swap_bytes((void*)&g->offs_item_data);
	g->offs_noun_table = swap_bytes((void*)&g->offs_noun_table);
	g->offs_global_codeblocks = swap_bytes((void*)&g->offs_global_codeblocks);
	g->offs_action_codeblocks = swap_bytes((void*)&g->offs_action_codeblocks);
}

static void load_game(FILE *fp)
{
	fseek(fp, 0x0, SEEK_END);
	game_length = ftell(fp);
	fseek(fp, 0x0, SEEK_SET);

	data = malloc(game_length + 32);

	if (fread(data, 1, game_length, fp) != game_length)
	{
		error("bad data file");
	}

	game_hdr = (void*)data;
	discombobulate_header(game_hdr);

	if (memcmp(game_hdr->magic, "XADV", 4) != 0)
		error("bad magic in file header");

	/* bounds checking */
	if (game_hdr->offs_string_table > game_length) error("offset outside of file");
	if (game_hdr->offs_verb_table > game_length)	error("offset outside of file");
	if (game_hdr->offs_room_data > game_length)	error("offset outside of file");
	if (game_hdr->offs_item_data > game_length)	error("offset outside of file");
	if (game_hdr->offs_noun_table > game_length) error("offset outside of file");
	if (game_hdr->offs_global_codeblocks > game_length)	error("offset outside of file");
	if (game_hdr->offs_action_codeblocks > game_length)	error("offset outside of file");

}


static void print_player(int i)
{
	if (i==0)
		fprintf(stdout, "player");
	else
		fprintf(stdout, "player_%02i", i);
}

static void print_item(int i)
{
	if (i==0)
		fprintf(stdout, "any");
	else
		fprintf(stdout, "itm%02i", i);
}

static void print_noun(int i)
{
	uint8_t *ptr;
	int verb;

	ptr = data + game_hdr->offs_noun_table;
	verb = *ptr++;

	while (*ptr != 0x0)
	{
		do
		{
			if (verb == i)
			{
				fprintf(stdout, "%s", ptr);
				return;
			}

			while (*ptr != 0) ptr++;
			ptr++;
		}while (*ptr != 0);
		ptr++;
		verb = *ptr++;
	}
}

static void print_room(int i)
{
	if (i==0)
		fprintf(stdout, "VOID");
	else if (i==1)
		fprintf(stdout, "DESTROYED");
	else if (i==2)
		fprintf(stdout, "INVENTORY");
	else
		fprintf(stdout, "rm%02i", i);
}

static void decomp_verbs(void)
{
	uint8_t *ptr;
	int verb;

	ptr = data + game_hdr->offs_verb_table;

	fprintf(stdout, "\n###############################################################\n");
	fprintf(stdout, "## Verbs\n");

	verb = *ptr++;

	while (*ptr != 0x0)
	{
		if (verb == 0)
			fprintf(stdout, "#verb ");
		else
			fprintf(stdout, "verb ");

		do
		{
			if (verb == 0)
				fprintf(stdout, "# %s", ptr);
			else
				fprintf(stdout, "%s", ptr);
			while (*ptr != 0) ptr++;
			ptr++;
			if (*ptr != 0x0)
			{
				fprintf(stdout, ", ");
			}
		}while (*ptr != 0);
		ptr++;
		if (verb == 0)
			fprintf(stdout, " ## (predefined by compiler)\n");
		else
			fprintf(stdout, "\n");

		verb = *ptr++;
	}

	fprintf(stdout, "\n");
}

static void decomp_nouns(void)
{
	uint8_t *ptr;
	int noun;

	ptr = data + game_hdr->offs_noun_table;

	fprintf(stdout, "\n###############################################################\n");
	fprintf(stdout, "## Nouns\n");

	noun = *ptr++;

	while (*ptr != 0x0)
	{
		if (noun == 0)
			fprintf(stdout, "#noun ");
		else
			fprintf(stdout, "noun ");

		do
		{
			fprintf(stdout, "%s", ptr);

			while (*ptr != 0) ptr++;
			ptr++;
			if (*ptr != 0x0)
			{
				fprintf(stdout, ", ");
			}
		}while (*ptr != 0);

		ptr++;

		if (noun == 0)
			fprintf(stdout, " ## (predefined by compiler)\n");
		else
			fprintf(stdout, "\n");

		noun = *ptr++;
	}

	fprintf(stdout, "\n");
}


static char* get_string(int idx)
{
	uint8_t *ptr;

	ptr = data + game_hdr->offs_string_table + (idx*2);
	ptr = data + get_pointer(ptr);

	return(ptr);
}

static void decomp_items(void)
{
	int i;
	uint8_t item;
	uint8_t flags;
	uint8_t room;
	uint8_t noun;
	uint8_t *name;

	uint8_t *ptr = data + game_hdr->offs_item_data;

	fprintf(stdout, "\n###############################################################\n");
	fprintf(stdout, "## Items\n");

	for (i=0; i < game_hdr->item_count + game_hdr->player_count; i++)
	{
		uint16_t idx;
		item = *ptr++;
		room = *ptr++;
		flags = *ptr++;
		noun = *ptr++;


		if (i == game_hdr->item_count)
		{
			fprintf(stdout, "\n\n###############################################################\n");
			fprintf(stdout, "## Players\n");
		}

		idx = (ptr[0]<<8) + ptr[1];
		//name = get_string((ptr[0]<<8) + ptr[1]);
		ptr += 2;

		//item itmLitTorch [torch] "Torch (on)" in VOID has LIGHT
		//if (i >= 0 && i != game_hdr->item_count)
		{
			if (i < game_hdr->item_count)
			{
				if (i==0)
					fprintf(stdout, "#item ");
				else
					fprintf(stdout, "item ");
				print_item(item);
			}
			else if (i >= game_hdr->item_count)
			{
				if (i==game_hdr->item_count)
					fprintf(stdout, "#player ");
				else
					fprintf(stdout, "player ");
				print_player(item);
			}

			fprintf(stdout, " [");
			print_noun(noun);
			fprintf(stdout, "] ");

			print_msg(idx);

			fprintf(stdout, " in ");
			print_room(room);


			if ((flags&ITEM_LIGHT) == ITEM_LIGHT)
			{
				fprintf(stdout, " has LIGHT");
			}
			if ((flags&ITEM_SCENERY) == ITEM_SCENERY)
			{
				fprintf(stdout, " has SCENERY");
			}

			if (i == 0 || i == game_hdr->item_count)
				fprintf(stdout, " ## (compiler defined)");

			fprintf(stdout, "\n");
		}
	}
}

static char* get_verb(int idx)
{
	int v;
	uint8_t *ptr = data + game_hdr->offs_verb_table;

	v = *ptr++;
	while (*ptr != 0x0 && v != idx)
	{
		do
		{
			while (*ptr != 0) ptr++;
			ptr++;
		}while (*ptr != 0);
		ptr++;

		v = *ptr++;
	}

	return(ptr);
}

static char* get_noun(int idx)
{
	int i;
	uint8_t *ptr = data + game_hdr->offs_item_data;

	for (i=0; i<game_hdr->item_count; i++)
	{
		uint8_t item;

		item = *ptr++;
		ptr += 3;

		if (item == idx)
			return(ptr);

		while (*ptr != 0x0)
		{
			while (*ptr != 0x0)	ptr++;
			ptr++;
		}

		ptr++;
	}

	return(data + game_hdr->offs_item_data);
}

static void tabout(int i)
{
	while (i>=0)
	{
		fprintf(stdout, "\t");
		i -= 1;
	}
}


static uint8_t* get_stringX(int idx)
{
	uint8_t *ptr, *x, *xx;
	int offs;
	int len;
	uint16_t z;

	x = get_string(idx);
	ptr = x;
	len = 0;

	uint16_t s1, s2, s3;

	s1 = get_pointer(data + game_hdr->offs_string_table + (idx * 2) + 0);
	s2 = get_pointer(data + game_hdr->offs_string_table + (idx * 2) + 2);

	s3 = s2 - s1;
	while (s3>0)
	{
		z = get_pointer(x);
		x += 2;
		s3 -= 2;
		len += z >> 12;
		if (len == 15)
		{
			z = get_pointer(x);
			x += 2;
			s3 -= 2;
			len += z >> 12;
		}

		len += 1;
	}

	x = ptr;
	xx = calloc(1, len+4);
	ptr = xx;

	s3 = s2 - s1;
	while (s3>0)
	{
		z = get_pointer(x);
		x += 2;
		s3 -= 2;

		offs = z & 0xFFF;
		len = z >> 12;

		memmove(ptr, data + offs, len);
		ptr += len;

		if (len == 15)
		{
			z = get_pointer(x);
			x += 2;
			s3 -= 2;

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
		*ptr=0;
	}

	return(xx);
}

static void print_msg(int idx)
{
	uint8_t *ptr;
	uint8_t *x;
	char *xx;

	xx = get_stringX(idx);
	ptr = xx;

	while (*ptr == 0x04)
	{
		fprintf(stdout, "\\n");
		ptr++;
	}

	if (*ptr != 0x0)
	{
		fprintf(stdout, "\"");

		while (*ptr != 0)
		{
			if (*ptr == 0x01)
			{
				fprintf(stdout, "%%cnt%02i%%", ptr[1]);
				ptr += 2;
			}
			else if (*ptr == 0x02)
			{
				fprintf(stdout, "%%verb%%");
				ptr++;
			}
			else if (*ptr == 0x03)
			{
				fprintf(stdout, "%%noun%%");
				ptr++;
			}
			else if (*ptr == 0x04)
			{
				fprintf(stdout, "\\n");
				ptr++;
			}
			else if (*ptr == 0x0d || *ptr == 0x0D || *ptr == 0x0A)
			{
				fprintf(stdout, "\\n");
				ptr++;
			}
			else
			{
				fputc(*ptr, stdout);
				ptr++;
			}
		}

		fprintf(stdout,"\"");
	}

	free(xx);
}

static uint8_t* dump_codeblock(uint8_t *ptr, int tab_depth, int comment)
{
	int try_depth = tab_depth;
	int do_not = 0;

	while (*ptr != X_ENDOPCODES)
	{
		if(*ptr == X_NOT)
		{
			ptr++;
			if (comment == 1) fprintf(stdout, "# ");
			tabout(try_depth);
			fprintf(stdout, "not ");
			do_not = 1;
		}

		switch (*ptr)
		{
			case X_TRY:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "try\n");
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "{\n");
				try_depth += 1;
				ptr += 2;	// skip size
				break;

			case X_IN:
				ptr++;
				if (do_not == 0 && comment == 1) fprintf(stdout, "# ");
				if (do_not == 0) tabout(try_depth);
				fprintf(stdout, "In ");
				print_room(ptr[0]);
				fprintf(stdout, "\n");
				ptr++;
				break;

			case X_NOTIN:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "*** NotIn ");
				print_room(ptr[0]);
				fprintf(stdout, "\n");
				ptr++;
				break;

			case X_RANDOM:
				ptr++;
				if (do_not == 0 && comment == 1) fprintf(stdout, "# ");
				if (do_not == 0) tabout(try_depth);
				fprintf(stdout, "Random %i", ptr[0]);
				fprintf(stdout, "\n");
				ptr++;
				break;

			case X_ISPRESENT:
				ptr++;
				if (do_not == 0 && comment == 1) fprintf(stdout, "# ");
				if (do_not == 0) tabout(try_depth);
				fprintf(stdout, "IsPresent ");
				print_item(ptr[0]);
				fprintf(stdout, "\n");
				ptr++;
				break;

			case X_ISNOTPRESENT:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "*** IsNotPresent ");
				print_item(ptr[0]);
				fprintf(stdout, "\n");
				ptr++;
				break;

			case X_MSG:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				print_msg(get_pointer(ptr));
				fprintf(stdout, "\n");
				ptr += 2;
				break;

			case X_EXIT:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "exit %s\n", *ptr == 1 ? "true":"false");
				ptr++;
				break;

			case X_CONTINUE:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "exit %s\n", *ptr == 1 ? "true":"false");
				ptr++;
				break;

			case X_LOOK:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "look\n");
				break;

			case X_ENDTRY:
				ptr++;
				try_depth -= 1;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "}\n\n");
				break;

			case X_SWAP:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "swap ");
				print_item(ptr[0]);
				fprintf(stdout, ", ");
				print_item(ptr[1]);
				fprintf(stdout, "\n");
				ptr += 2;
				break;

			case X_NOUNIS:
				ptr++;
				if (do_not == 0 && comment == 1) fprintf(stdout, "# ");
				if (do_not == 0) tabout(try_depth);
				fprintf(stdout, "NounIs ");
				print_item(ptr[0]);
				fprintf(stdout, "\n");
				ptr++;
				break;

			case X_SHOWINVENTORY:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "ShowInventory\n");
				break;

			case X_CANCARRY:
				ptr++;
				if (do_not == 0 && comment == 1) fprintf(stdout, "# ");
				if (do_not == 0) tabout(try_depth);
				fprintf(stdout, "CanCarry\n");
				break;

			case X_MOVE:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "Move ");
				print_item(ptr[0]);
				fprintf(stdout, ", ");
				print_room(ptr[1]);
				fprintf(stdout, "\n");
				ptr+=2;
				break;

			case X_TAKE:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "take ");
				print_item(ptr[0]);
				fprintf(stdout, "\n");
				ptr+=1;
				break;

			case X_TRANSPORT:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "Transport ");
				print_player(ptr[0]);
				fprintf(stdout, ", ");
				print_room(ptr[1]);
				fprintf(stdout, "\n");
				ptr+=2;
				break;

			case X_NPCHERE:
				ptr++;
				if (do_not == 0 && comment == 1) fprintf(stdout, "# ");
				if (do_not == 0) tabout(try_depth);
				fprintf(stdout, "IsNPCHere ");
				print_player(ptr[0]);
				fprintf(stdout, "\n");
				ptr++;
				break;

			case X_SWITCH:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "SwitchPlayer ");
				print_player(ptr[0]);
				fprintf(stdout, "\n");
				ptr++;
				break;

			case X_SETPLAYERINVENTORY:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "SetPlayerInventory ");
				print_player(ptr[0]);
				fprintf(stdout, ", ");
				print_room(ptr[1]);
				fprintf(stdout, "\n");
				ptr+=2;
				break;

			case X_HAS:
				ptr++;
				if (do_not == 0 && comment == 1) fprintf(stdout, "# ");
				if (do_not == 0) tabout(try_depth);
				fprintf(stdout, "Has ");
				print_item(ptr[0]);
				fprintf(stdout, "\n");
				ptr++;
				break;

			case X_HASNOT:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "*** HasNot ");
				print_item(ptr[0]);
				fprintf(stdout, "\n");
				ptr++;
				break;

			case X_HERE:
				ptr++;
				if (do_not == 0 && comment == 1) fprintf(stdout, "# ");
				if (do_not == 0) tabout(try_depth);
				fprintf(stdout, "Here ");
				print_item(ptr[0]);
				fprintf(stdout, "\n");
				ptr++;
				break;

			case X_SETCOUNTER:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "SetCounter cnt%02i, %i\n", ptr[0], (ptr[1]<<8) + ptr[2]);
				ptr += 3;
				break;

			case X_SETLIGHT:
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				ptr++;
				if (*ptr == 1)
					fprintf(stdout, "SetLightOn\n");
				else
					fprintf(stdout, "SetLightOff\n");

				ptr++;
				break;

			case X_COUNTEREQUALS:
				ptr++;
				if (do_not == 0 && comment == 1) fprintf(stdout, "# ");
				if (do_not == 0) tabout(try_depth);
				fprintf(stdout, "CounterEquals cnt%02i, %i\n", ptr[0], (ptr[1]<<8) + ptr[2]);
				ptr += 3;
				break;

			case X_COUNTERNOTEQUALS:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "*** CounterNotEquals cnt%02i, %i\n", ptr[0], (ptr[1]<<8) + ptr[2]);
				ptr += 3;
				break;

			case X_ADDCOUNTER:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "AddCounter cnt%02i, %i\n", ptr[0], (ptr[1]<<8) + ptr[2]);
				ptr += 3;
				break;

			case X_SUBCOUNTER:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "SubCounter cnt%02i, %i\n", ptr[0], (ptr[1]<<8) + ptr[2]);
				ptr += 3;
				break;


			case X_QUITRESTARTGAME:
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				ptr++;
				fprintf(stdout, "QuitRestartGame ");
				print_msg((ptr[0] << 8) + (ptr[1]));
				fprintf(stdout, "\n");
				ptr += 2;
				break;

			case X_ISIMTEINROOM:
				ptr++;
				if (do_not == 0 && comment == 1) fprintf(stdout, "# ");
				if (do_not == 0) tabout(try_depth);
				fprintf(stdout, "IsItemInRoom ");
				print_item(ptr[0]);
				fprintf(stdout, ", ");
				print_room(ptr[1]);
				fprintf(stdout, "\n");
				ptr+=2;
				break;

			case X_WINLOOSEGAME:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				if (ptr[1]==1)
					fprintf(stdout, "WinGame\n");
				else
					fprintf(stdout, "LooseGame\n");
				ptr++;
				break;

			case X_GOTO:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "goto ");
				print_room(ptr[0]);
				fprintf(stdout, "\n");
				ptr++;
				break;

			case X_CANPLAYERSEE:
				ptr++;
				if (do_not == 0 && comment == 1) fprintf(stdout, "# ");
				if (do_not == 0) tabout(try_depth);
				fprintf(stdout, "CanPlayerSee\n");
				break;

			case X_COUNTERGT:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "CounterGT cnt%02i, %i\n", ptr[0], (ptr[1]<<8) + ptr[2]);
				ptr += 3;
				break;

			case X_COUNTERLT:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				fprintf(stdout, "CounterLT cnt%02i, %i\n", ptr[0], (ptr[1]<<8) + ptr[2]);
				ptr += 3;
				break;

			case X_ISIMTEMHERE:
				ptr++;
				if (do_not == 0 && comment == 1) fprintf(stdout, "# ");
				if (do_not == 0) tabout(try_depth);
				fprintf(stdout, "IsItemHere ");
				print_item(ptr[0]);
				fprintf(stdout, "\n");
				ptr+=1;
				break;

			case X_SETFLAG:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				tabout(try_depth);
				if (ptr[1] == 1)
					fprintf(stdout, "SetFlagTrue f%02i\n", ptr[0]);
				else
					fprintf(stdout, "SetFlagFalse f%02i\n", ptr[0]);
				ptr+=2;
				break;

			case X_ISFLAG:
				ptr++;
				if (do_not == 0 && comment == 1) fprintf(stdout, "# ");
				if (do_not == 0) tabout(try_depth);
				if (ptr[1] == 1)
					fprintf(stdout, "IsFlagTrue f%02i\n", ptr[0]);
				else
					fprintf(stdout, "IsFlagFalse f%02i\n", ptr[0]);
				ptr+=2;
				break;

			case X_PAUSE:
				ptr++;
				if (comment == 1) fprintf(stdout, "# ");
				fprintf(stdout, "pause %i\n", ptr[0]);
				ptr++;
				break;

			case X_BYTECODE_GOTO:
				dump_codeblock(data + get_pointer(1 + ptr), try_depth, comment);
				return ptr + 3;
				break;

			default:
				fprintf(stdout, "undef opcode %i\n", *ptr);
				exit(1);
				break;
		}

		do_not = 0;
	}

	ptr++;

	return(ptr);
}

static void decomp_actions(void)
{
	uint8_t *ptr = data + game_hdr->offs_action_codeblocks;
	int verb;
	int noun;

	fprintf(stdout, "\n###############################################################\n");
	fprintf(stdout, "## Actions\n");


	while (*ptr != 0x0)
	{
		verb = *ptr++;
		noun = *ptr++;

		ptr += 2;

		fprintf(stdout, "action %s ", get_verb(verb));
		print_noun(noun);
		fprintf(stdout, "\n{\n");
		ptr = dump_codeblock(ptr, 0, 0);
		fprintf(stdout, "}\n\n");
	}
}

static void decomp_global_codeblocks(void)
{
	uint8_t *ptr;
	int offs;
	int i;
	char *scodes[]={ "reset", "pregame", "on_gamefail", "on_gamewin","prompt","on_fail","on_success", NULL};

	fprintf(stdout, "\n###############################################################\n");
	fprintf(stdout, "## Global CodeBlocks \n");

	ptr = data + game_hdr->offs_global_codeblocks;

	// dont output reset codeblock
	for (i=0; scodes[i] != NULL; i++)
	{
		offs = (ptr[0]<<8) + ptr[1]; ptr += 2;
		if (offs != 0)
		{
			if (i == 0)
			{
				fprintf(stdout, "\n#\n# This is generated by the compiler and is here for reference.\n#\n");
				fprintf(stdout, "# %s\n#{\n", scodes[i]);
				dump_codeblock(data + offs, 0, 1);
				fprintf(stdout, "#}\n\n");
			}
			else
			{
				fprintf(stdout, "%s\n{\n", scodes[i]);
				dump_codeblock(data + offs, 0, 0);
				fprintf(stdout, "}\n\n");
			}
		}
	}
}

static void decomp_rooms(void)
{
	int i;
	uint8_t *ptr = data + game_hdr->offs_room_data;
	int tab_depth = 0;
	char *scodes[] = { "enter", "look", "n", "s", "e", "w", "ne", "nw", "se", "sw", "u", "d", NULL};
	int j;

	uint8_t *rptr;

	fprintf(stdout, "\n###############################################################\n");
	fprintf(stdout, "## Rooms\n\n");

	for (i=0; i < game_hdr->room_count; i++)
	{
		ptr = data + game_hdr->offs_room_data;

		ptr += (i * 2);
		rptr = data + ((ptr[0]<<8) + (ptr[1]));

		if (i < 3)
			fprintf(stdout, "# Compiler Defined\n");

		fprintf(stdout, "room ");
		print_room(*rptr++);
		fprintf(stdout, " ");
		print_msg((rptr[0] << 8) + (rptr[1]));
		fprintf(stdout, "\n{\n");
		rptr += 2;

		for (j = 0; scodes[j] != NULL; j++)
		{
			int offs;

			offs = (rptr[0]<<8) + rptr[1];
			rptr += 2;

			if (offs != 0)
			{
				tabout(tab_depth);
				fprintf(stdout, "%s\n", scodes[j]);
				tabout(tab_depth);
				fprintf(stdout, "{\n");
				dump_codeblock(data + offs+1, tab_depth+1, 0);
				tabout(tab_depth);
				fprintf(stdout, "}\n\n");
			}
		}

		fprintf(stdout, "}\n\n");
	}
}

int main(int argc, char *argv[])
{
	FILE *file;

	int i;

	if (argc < 2 && argv[1] == NULL)
	{
		error("Missing game filename");
	}


	file = fopen(argv[1], "rb");
	load_game(file);
	fclose(file);

	fprintf(stdout, "#\n# Decompiled source\n#\n\n");
	fprintf(stdout, "game\t\"%s\"\n", game_hdr->game);
	fprintf(stdout, "author\t\"%s\"\n", game_hdr->author);
	fprintf(stdout, "version\t\"%s\"\n", game_hdr->version);

	fprintf(stdout, "\nrequires_interpreter\t%i\n", game_hdr->interp_required_version);
	fprintf(stdout, "\nmax_carry\t%i\n\n", game_hdr->max_carry);

	// skip counter 0
	for (i=1; i<game_hdr->counter_count; i++)
	{
		fprintf(stdout, "counter cnt%02i\n", i);
	}

	if (game_hdr->counter_count>1)
	{
		fprintf(stdout, "\n");
	}

	for (i=0; i<game_hdr->flag_count; i++)
	{
		fprintf(stdout, "flag f%02i\n", i);
	}

	if (game_hdr->flag_count>0)
	{
		fprintf(stdout, "\n");
	}

	decomp_verbs();
	decomp_global_codeblocks();
	decomp_rooms();
	decomp_actions();
	decomp_nouns();
	decomp_items();

	fprintf(stdout, "\n\n");

	return(0);
}


