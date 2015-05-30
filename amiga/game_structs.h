#ifndef GAME_STRUCTS_H
#define GAME_STRUCTS_H
#ifdef __cplusplus
extern "C"{
#endif

#define ROOM_VOID		0
#define ROOM_DESTROYED	1
#define ROOM_INVENTORY	2

#define ITEM_LIGHT		0x01
#define ITEM_SCENERY	0x02
#define ITEM_MASK		0x03

#define ITEM_PLAYER		0x04

#define COUNTER_INVENTORY	0

enum eGC
{
	XGC_RESET = 0,
	XGC_PREGAME = 1,
	XGC_ON_SUCCESS = 2,
	XGC_ON_FAIL = 3,
	XGC_PROMPT = 4,
	XGC_ON_GAMEWIN = 5,
	XGC_ON_GAMEFAIL = 6,
	XGC_IN_DARK = 7
};

enum eRM
{
	XRM_ENTER = 0,
	XRM_LOOK,
	XRM_N,
	XRM_S,
	XRM_E,
	XRM_W,
	XRM_NE,
	XRM_NW,
	XRM_SE,
	XRM_SW,
	XRM_U,
	XRM_D
};

enum eOpcode
{
	X_ADDCOUNTER = 1,
	X_SUBCOUNTER = 2,
	X_SETCOUNTER = 3,
	X_SETLIGHT = 4,
	X_MSG = 5,
	X_QUITRESTARTGAME = 6,
	X_TRY = 7,
	X_ISIMTEINROOM = 8,
	X_WINLOOSEGAME = 9,
	X_CONTINUE = 10,
	X_EXIT = 11,
	X_MOVE = 12,
	X_SWAP = 13,
	X_CANPLAYERSEE = 14,
	X_ISPRESENT = 15,
	X_GOTO = 16,
	X_NOUNIS = 17,
	X_SHOWINVENTORY = 18,
	X_CANCARRY = 19,
	X_HAS = 20,
	X_HERE = 21,
	X_ENDTRY = 22,
	X_ISNOTPRESENT = 23,
	X_LOOK = 24,
	X_IN = 25,
	X_TRANSPORT = 26,
	X_SWITCH = 27,
	X_COUNTEREQUALS = 28,
	X_COUNTERNOTEQUALS = 29,
	X_TAKE = 30,
	X_COUNTERGT = 31,
	X_COUNTERLT = 32,
	X_ISIMTEMHERE = 33,
	X_ISFLAG = 34,
	X_SETFLAG = 35,
	X_SETPLAYERINVENTORY = 36,
	X_NPCHERE = 37,
	X_PAUSE = 38,
	X_NOTIN = 39,
	X_RANDOM = 40,
	X_HASNOT = 41,
	X_NOT = 42,

	X_ENDOPCODES = 255
};

typedef struct udtGame
{
	uint8_t     magic[4];

	uint8_t     max_carry;
	uint8_t     interp_required_version;

	uint16_t    string_count;
	uint8_t    	room_count;
	uint8_t		player_count;

	uint16_t    offs_string_table;
	uint16_t    offs_verb_table;
	uint16_t    offs_room_data;
	uint16_t    offs_item_data;
	uint16_t    offs_noun_table;
	uint16_t    offs_global_codeblocks;
	uint16_t    offs_action_codeblocks;

	uint8_t     counter_count;
	uint8_t     item_count;
	uint8_t		flag_count;
	uint8_t		filler0;

	uint16_t	size_in_kb;

	uint8_t     filler[0x20 - 30];

	uint8_t     game[0x20];
	uint8_t     author[0x20];
	uint8_t     version[0x10];
} uGame;


#ifdef __cplusplus
};
#endif
#endif

