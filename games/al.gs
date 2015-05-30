
#
# Port Adventureland from TI99
#

requires_interpreter	2
max_carry				6
author 					"Scott Adams"
version 				"1"
game 					"Adventureland"


#first one is primary, rest as synonyms
verb look, l, examine, ex, x
verb north, n
verb northwest, nw
verb northeast, ne
verb south, s
verb southwest, sw
verb southeast, se
verb east, e
verb west, w
verb up, u
verb down, d
verb go

verb drop, putdown, put, place, leave
verb get, take, pickup, grab, steal, grift, lift, snatch
verb read, see, glance, peruse
verb inventory, inv, i
verb quit, q
verb climb, shinny
verb throw, chuck, pitch, lob, toss
verb look, l, examine, ex, x
verb break, kick, smash, bash
verb unlock
verb lock
verb pump, inflate
verb say
verb cut, saw
verb move, push, pull
verb rub, polish, shine
verb version, ver, credits, author
verb score
verb light
verb unlight, extinguish
verb open
verb close
verb push
verb pull
verb turn, twist, spin
verb use
verb exit, leave

verb switch
verb fill
verb free, release
verb remove, takeoff
verb wear, puton

verb eat
verb drink
verb scream, yell, shout
verb pour, empty

verb jump
verb at
verb chop
verb swim
verb wake
verb attack
verb wave
verb make
verb dam
verb cross
verb scratch
verb help
verb find


flag flag_01_MIRROR_HINT1
flag flag_02_MIRROR_HINT2
flag flag_03_AT_VERB

flag flag_08_LAMP_DROP1
flag flag_09_LAMP_DROP2
flag flag_10_LAMP_DESTROY1
flag flag_11_LAMP_DESTROY2

flag flag_13_GAME_START
flag flag_14_BEES_FROM_BOTTLE

room rm_001 "Dismal Swamp"
{
	enter
	{
		setlighton
	}

	look
	{
		"I am in a dismal swamp. There are exists to the north, east and west"
	}

    on n
	{
		goto  rm_023
		exit true
	}

    on e
	{
		goto  rm_029
		exit true
	}

    on w
	{
		goto  rm_025
		exit true
	}
}

room rm_002 "Top of Tree"
{
	look
	{
		"I am in a top of a tall cypress tree."
	}

    on d
	{
		goto  rm_001
		exit true
	}
}

room rm_003 "Hollow Stump"
{
	look
	{
		"I am in a damp hollow stump in the swamp. There are exists up and down"
	}

    on u
	{
		goto  rm_001
		exit true
	}

    on d
	{
		goto  rm_004
		exit true
	}
}

room rm_004 "Root Champer"
{
	look
	{
		"I am in a root chamber under the stump."
	}

    on u
	{
		goto  rm_003
		exit true
	}
}

room rm_005 "Semi-Dark Hole"
{
	enter
	{
		setlighton
	}

	look
	{
		"I am in a semi-dark hole by the root chamber."
	}

    on u
	{
		goto  rm_004
		exit true
	}
}

room rm_006 "Sloping Hall"
{
	enter
	{
		setlightoff
	}

	look
	{
		"I am in a long down sloping hall. You can go up or down."
	}

    on u
	{
		goto  rm_005
		exit true
	}

    on d
	{
		goto  rm_007
		exit true
	}
}

room rm_007 "Large Cavern"
{
	look
	{
		"I am in a large cavern. There are exists to the north, south, west, up and down."
	}

    on n
	{
		goto  rm_031
		exit true
	}

    on s
	{
		goto  rm_009
		exit true
	}

    on w
	{
		goto  rm_027
		exit true
	}

    on u
	{
		goto  rm_006
		exit true
	}

    on d
	{
		goto  rm_012
		exit true
	}
}

room rm_008 "Eight Sided Room"
{
	look
	{
		"I am in a large 8 sided room. You can leave by going south"
	}

    on s
	{
		goto  rm_031
		exit true
	}
}

room rm_009 "Royal Anteroom"
{
	look
	{
		"I am in a royal anteroom. There are exits to the north and upwards"
	}

    on n
	{
		goto  rm_007
		exit true
	}

    on u
	{
		goto  rm_020
		exit true
	}
}

room rm_010 "Lake Shore"
{
	look
	{
		"I'm on the shore of a lake. You can go north, south or west."
	}

    on n
	{
		goto  rm_026
		exit true
	}

    on s
	{
		goto  rm_029
		exit true
	}

    on w
	{
		goto  rm_023
		exit true
	}
}

room rm_011 "Forest"
{
	look
	{
		"I am in a forest. There are exists to the north, south, east and west."
	}

    on n
	{
		goto  rm_011
		exit true
	}

    on s
	{
		goto  rm_011
		exit true
	}

    on e
	{
		goto  rm_023
		exit true
	}

    on w
	{
		goto  rm_011
		exit true
	}
}

room rm_012 "Maze of pits"
{
	look
	{
		"I am in a maze of pits. There are exits to the north, south, east and downward."
	}

    on n
	{
		goto  rm_013
		exit true
	}

    on s
	{
		goto  rm_015
		exit true
	}

    on e
	{
		goto  rm_015
		exit true
	}

    on d
	{
		goto  rm_013
		exit true
	}
}

room rm_013 "Maze of pits"
{
	look
	{
		"I am in a maze of pits. There are exits to the west and up"
	}

    on w
	{
		goto  rm_014
		exit true
	}

    on u
	{
		goto  rm_012
		exit true
	}
}

room rm_014 "Maze of puts"
{
	look
	{
		"I am in a maze of pits. There are exits to the north, south, east, west, upward and downward."
	}

    on n
	{
		goto  rm_017
		exit true
	}

    on s
	{
		goto  rm_012
		exit true
	}

    on e
	{
		goto  rm_013
		exit true
	}

    on w
	{
		goto  rm_016
		exit true
	}

    on u
	{
		goto  rm_016
		exit true
	}

    on d
	{
		goto  rm_017
		exit true
	}
}

room rm_015 "Maze of pits"
{
	look
	{
		"I am in a maze of pits. There are exits to the north, east, west and upward."
	}

    on n
	{
		goto  rm_012
		exit true
	}

    on e
	{
		goto  rm_013
		exit true
	}

    on w
	{
		goto  rm_012
		exit true
	}

    on u
	{
		goto  rm_013
		exit true
	}
}


room rm_016 "Maze of pits"
{
	look
	{
		"I am in a maze of pits. There are exits to the south, upward and downward"
	}

    on s
	{
		goto  rm_017
		exit true
	}

    on u
	{
		goto  rm_014
		exit true
	}

    on d
	{
		goto  rm_017
		exit true
	}
}

room rm_017 "Maze of pits"
{
	look
	{
		"I am in a maze of pits. There are exits to the north, south, east, west, upward and downard."
	}

	enter
	{
		setlightoff
	}

    on n
	{
		goto  rm_017
		exit true
	}

    on s
	{
		goto  rm_012
		exit true
	}

    on e
	{
		goto  rm_012
		exit true
	}

    on w
	{
		goto  rm_015
		exit true
	}

    on u
	{
		goto  rm_014
		exit true
	}

    on d
	{
		goto  rm_018
		exit true
	}
}

room rm_018 "Deep Chasm"
{
	enter
	{
		setlighton
	}

	look
	{
		"I'm at the bottom of a very deep chasm. High above me is a pair of ledges."
		"One has a bricked up window across its face. The other faces a Throne room. You can leave by going up."
	}

    on u
	{
		goto  rm_017
		exit true
	}
}

room rm_019 "Narrow Ledge"
{
	look
	{
		"I'm on a narrow ledge by a chasm. Across the chasm is the throne room. There is an exit to the west"
	}

    on w
	{
		goto  rm_020
		exit true
	}
}

room rm_020 "Royal Chamber"
{
	look
	{
		"I am in a royal chamber. You can leave by going down."
	}

    on d
	{
		goto  rm_009
		exit true
	}
}

room rm_021 "Narrow Ledge"
{
	look
	{
		"I'm on a narrow ledge by a Throne-room. Across the chasm is another ledge."
	}

}

room rm_022 "Throne Room"
{
	look
	{
		"I am in a throne room. There is an exit to the west"
	}

    on w
	{
		goto  rm_021
		exit true
	}
}

room rm_023 "Sunny Meadow"
{
	enter
	{
		setlighton
	}

	look
	{
		"I am in a sunny meadow. There are exits to the south, east and west."
	}

    on s
	{
		goto  rm_001
		exit true
	}

    on e
	{
		goto  rm_010
		exit true
	}

    on w
	{
		goto  rm_011
		exit true
	}
}


room rm_024 "Hell"
{
	look
	{
		"I think I'm in real trouble now. There's a fellow here with a pitchfork and pointed tail. ...Oh Hell!"
	}
}

room rm_025 "Hidden Grove"
{
	look
	{
		"I am in a hidden grove. There are exits to the north and east."
	}

    on n
	{
		goto  rm_011
		exit true
	}

    on e
	{
		goto  rm_001
		exit true
	}
}

room rm_026 "Bog"
{
	look
	{
		"I am in a quick-sand bog."
	}
}

room rm_027 "TI99"
{
	look
	{
		"I am in a Memory chip of a TI 99/4! I took a wrong turn! You can leave by going east"
	}

    on e
	{
		goto  rm_007
		exit true
	}
}

room rm_028 "Top of Oak"
{
	look
	{
		"I'm in a branch on the top of an old oak tree. To the east I see a meadow beyond a lake."
	}

    on d
	{
		goto  rm_011
		exit true
	}
}


room rm_029 "Bottomless Hole"
{
	look
	{
		"I'm at the edge of a BOTTOMLESS hole. There are exits to the north and west."
	}

    on n
	{
		goto  rm_010
		exit true
	}

    on w
	{
		goto  rm_001
		exit true
	}
}


room rm_030 "Ledge"
{
	look
	{
		"I'm on a ledge just below the rim of the BOTTOMLESS hole. I don't think I want to go down. There are exits going upward and downward"
	}

    on u
	{
		goto  rm_029
		exit true
	}

    on d
	{
		goto  rm_024
		exit true
	}
}

room rm_031 "Long Tunnel"
{
    look
    {
		"I am in a long tunnel. Exits lead north and south"
	}

    on n
	{
		goto  rm_008
		exit true
	}

    on s
	{
		goto  rm_007
		exit true
	}
}

room rm_032 "Endless Corridor"
{
	look
	{
		"I'm in an endless corridor. There are exits in all directions."
	}

    on n
	{
		goto  rm_032
		exit true
	}

    on s
	{
		goto  rm_033
		exit true
	}

    on e
	{
		goto  rm_032
		exit true
	}

    on w
	{
		goto  rm_032
		exit true
	}

    on u
	{
		goto  rm_032
		exit true
	}

    on d
	{
		goto  rm_032
		exit true
	}
}

room rm_033 "Misty Room"
{
	look
	{
		"I am in a large misty room with strange unreadable letters over all the exits. There are exits in all directions."
	}

	on n
	{
		goto  rm_032
		exit true
	}

    on s
	{
		goto  rm_024
		exit true
	}

    on e
	{
		goto  rm_011
		exit true
	}

    on w
	{
		goto  rm_024
		exit true
	}

    on u
	{
		goto  rm_028
		exit true
	}

    on d
	{
		goto  rm_024
		exit true
	}

}


#treasure=rm_003;

######################################################################################################################
######################################################################################################################
######################################################################################################################
######################################################################################################################
######################################################################################################################


counter cntLightTime

on_success
{
	AddCounter moves, 1
	#"OK"
}

on_fail
{
	AddCounter moves, 1
	"I do not understand.\n"
}

in_dark
{
	"It's too dark to see.\n"
}

prompt
{
	try
	{
		not IsItemInRoom itm_009, void
		SubCounter cntLightTime, 1

		try
		{
			IsPresent itm_009
			CounterEQ cntLightTime, 20
			"The lamp flickers\n\n"
		}

		try
		{
			IsPresent itm_009
			CounterEQ cntLightTime, 10
			"The lamp flickers wildly\n\n"
		}

		try
		{
			IsPresent itm_009
			CounterEQ cntLightTime, 5
			"The lamp flickers and pops\n\n"
		}

		try
		{
			CounterEQ cntLightTime, 0
			swap itm_009, itm_060

			IsPresent itm_060
			"The lamp sputters and goes out.\n\n"
		}
	}

	try
	{
		in rm_003
		setcounter score, 0
		try
		{
			IsItemHere itm_002
			addcounter score, 10
		}
		try
		{
			IsItemHere itm_008
			addcounter score, 10
		}
		try
		{
			IsItemHere itm_019
			addcounter score, 10
		}
		try
		{
			IsItemHere itm_023
			addcounter score, 10
		}
		try
		{
			IsItemHere itm_029
			addcounter score, 10
		}
		try
		{
			IsItemHere itm_037
			addcounter score, 10
		}
		try
		{
			IsItemHere itm_038
			addcounter score, 10
		}
		try
		{
			IsItemHere itm_044
			addcounter score, 10
		}
		try
		{
			IsItemHere itm_046
			addcounter score, 10
		}
		try
		{
			IsItemHere itm_047
			addcounter score, 10
		}
		try
		{
			IsItemHere itm_048
			addcounter score, 10
		}
		try
		{
			IsItemHere itm_049
			addcounter score, 10
		}
		try
		{
			IsItemHere itm_056
			addcounter score, 10
		}
	}

	try
	{
		countereq score, 130
		wingame
		exit true
	}

	try
	{
		try
		{
			random 100
			In rm_026
			has itm_029        # *Thick PERSIAN RUG*
			swap itm_029, itm_061     # *Thick PERSIAN RUG*        # Muddy worthless old rug
		}

		try
		{
			random 100
			IsFlagTrue flag_01_MIRROR_HINT1
			IsFlagTrue flag_02_MIRROR_HINT2
			"'Don't waste honey, get mad instead! Dam lava!?'\n\n"
			SetFlagFalse flag_01_MIRROR_HINT1
			SetFlagFalse flag_02_MIRROR_HINT2
		}

		try
		{
			random 100
			IsFlagFalse flag_13_GAME_START
			"A voice BOOOOMS out:"
			SetFlagTrue flag_13_GAME_START
			"Welcome to Adventure number 1: 'ADVENTURELAND'. In this Adventure you're to find *TREASURES* & store them away. To see how well you're doing, say: 'SCORE'."
			"Remember, you can always say 'HELP'.\n\n"
		}

		try
		{
			random 100
			IsFlagTrue flag_14_BEES_FROM_BOTTLE
			#take itm_013     # Empty bottle
			move itm_013, inventory

			SetFlagFalse flag_14_BEES_FROM_BOTTLE
			losegame
			exit true
		}

		try
		{
			random 100
			IsFlagTrue flag_01_MIRROR_HINT1
			IsFlagFalse flag_02_MIRROR_HINT2
			"'*DRAGON STING*' and fades. I don't get it, I hope you do.\n\n"
			SetFlagFalse flag_01_MIRROR_HINT1
			SetFlagTrue flag_02_MIRROR_HINT2
		}

		try
		{
			random 50
			IsPresent itm_027        # Large sleeping dragon
			has itm_007        # Evil smelling mud
			"The Dragon smells something, awakens, and attacks me!\n\n"
			losegame
			exit true
		}

		try
		{
			random 30
			has itm_042        # Chiggers
			not ispresent itm_021        # Infected chigger bites
			not ispresent itm_020        # Chigger bites
			not has itm_007        # Evil smelling mud
			#take itm_020     # Chigger bites
			move itm_020, inventory
			"I'm bitten by chiggers.\n\n"
		}

		try
		{
			random 50
			has itm_008        # *GOLDEN FISH*
			not has itm_012        # Water in bottle
			"It's too dry, the fish died.\n\n"
			destroy itm_008     # *GOLDEN FISH*
			take itm_055     # Dead fish
		}

		try
		{
			random 75
			has itm_008        # *GOLDEN FISH*
			not ispresent itm_019        # *GOLDEN NET*
			"The fish have escaped back to the lake.\n\n"
			move itm_008, rm_010     # *GOLDEN FISH*
		}

		try
		{
			random 10
			has itm_021        # Infected chigger bites
			not in rm_033
			"My bites have rotted my whole body!\n\n"
			losegame
			exit true
		}

		try
		{
			random 10
			has itm_020        # Chigger bites
			not ispresent itm_007        # Evil smelling mud
			"My chigger bites are now INFECTED!\n\n"
			destroy itm_020     	# Chigger bites
			take itm_021     		# Infected chigger bites
		}

		## this is nasty, so lets remove!
		#try
		#{
		#	random 8
		#	IsPresent itm_026        # Bees in a bottle
		#	IsFlagFalse flag_17
		#	"The bees all suffocated and disappeared."
		#	swap itm_026, itm_013     # Bees in a bottle        # Empty bottle
		#}

		try
		{
			random 100
			In rm_024
			"You lost *ALL* treasures.\n\n"
			losegame
			exit true
		}

		try
		{
			random 100
			In rm_031

			not ispresent itm_026        # Bees in a bottle
			# test instead for bees in original room
			IsItemInRoom itm_024, rm_008
			"I hear buzzing ahead!\n\n"
		}

		try
		{
			random 5
			has itm_007        # Evil smelling mud
			not ispresent itm_012        # Water in bottle
			"The mud dried up and fell off.\n\n"
			move itm_007, rm_001     # Evil smelling mud
		}

		try
		{
			random 8
			not ispresent itm_020        # Chigger bites
			not ispresent itm_021        # Infected chigger bites
			IsPresent itm_042        	# Chiggers
			not has itm_007        # Evil smelling mud
			move itm_020, inventory     # Chigger bites
			"I'm bitten by chiggers.\n\n"
		}

		try
		{
			random 8
			IsPresent itm_024        # Large African bees
			not ispresent itm_007        # Evil smelling mud
			"The bees sting me!\n\n"
			losegame
			exit true
		}
	}

	"> "
}

on_gamefail
{
	crlf
	crlf
	"Your score is %score% in %moves% moves."

	# takes always a Y/N
	QuitRestartGame "Do you want to Play Again. Q/R?"
}

on_gamewin
{
	crlf
	crlf
	"Congratulations!"
	crlf
	crlf
	"Your score is %score% in %moves% moves."

	QuitRestartGame "Do you want to Play Again. Q/R?"
}

pregame
{
	SetCounter score, 0
	SetCounter moves, 0
	SetCounter cntLightTime, 125

	goto rm_011
}

noun ledge
action go ledge
{
    In rm_020
    IsPresent itm_035        # Bricked up window with a hole in it
    goto rm_019
	exit true
}

noun throne
action go throne
{
    In rm_019
    "How?"
	exit true
}

action go throne
{
    In rm_021
    IsPresent itm_025        # Very thin black bear
    "The bear won't let me."
	exit true
}

action go throne
{
    In rm_021
	not ispresent itm_025        # Very thin black bear
    goto rm_022
	exit true
}


action go lava
{
    In rm_018
    "No, it's too hot."
	exit true
}

action climb tree
{
    IsPresent itm_005        # Cypress tree
    goto rm_002
	exit true
}

action go ledge
{
    In rm_018
    "Not here."
	exit true
}



action go hole
{
    IsPresent itm_035        # Bricked up window with a hole in it
    goto rm_019
	exit true
}

action go stump
{
    IsPresent itm_004        # -HOLLOW- stump and remains of a felled tree
    goto rm_003
	exit true
}

action go stump
{
    In rm_011
    goto rm_028
	exit true
}

noun hallway, hall
action go hallway
{
    IsPresent itm_017        # Open door with a hallway beyond
    goto rm_006
    #SetFlagTrue flag_15_ROOM_IS_DARK
	exit true
}

action go hole
{
    IsPresent itm_052        # Smoking hole, Pieces of dragon and gore
    goto rm_024
	exit true
}

#noun hollow
action go hole
{
    In rm_004
    goto rm_005
	exit true
}

action go hole
{
    In rm_029
    goto rm_030
	exit true
}

action jump any
{
    In rm_019
	not ispresent itm_036        # Loose fire bricks

    goto rm_021
	exit true
}

action jump any
{
    In rm_021
    goto rm_019
	exit true
}

action jump any
{
    In rm_019
    IsPresent itm_036        # Loose fire bricks
    "Something's too heavy. I'm falling!"
    losegame
	exit true
}

action jump any
{
    "Not here."
	exit true
}


action at bear
{
    IsFlagTrue flag_03_AT_VERB
    not ispresent itm_038        # *MAGIC MIRROR*
    "OK, I threw it."
    "A voice BOOOOMS out:"
    "Please leave it alone."
    SetFlagFalse flag_03_AT_VERB
	exit true
}

action at dragon
{
    IsFlagTrue flag_03_AT_VERB
    IsPresent itm_027        # Large sleeping dragon
    "OK, I threw it."
    "It doesn't seem to bother him at all!"
    SetFlagFalse flag_03_AT_VERB
	exit true
}

action at window
{
    IsFlagTrue flag_03_AT_VERB
    SetFlagFalse flag_03_AT_VERB
    "Nothing happens."
    "A voice BOOOOMS out:"
    "Remember, you can always say 'HELP'."
	exit true
}

action at bear
{
    IsFlagTrue flag_03_AT_VERB
    IsPresent itm_038        # *MAGIC MIRROR*
    destroy itm_038     # *MAGIC MIRROR*
    destroy itm_038     # *MAGIC MIRROR*
    move itm_041, rm_021     # Broken glass
    "OH NO... The bear dodges... CRASH!"
    SetFlagFalse flag_03_AT_VERB
	exit true
}

action at door
{
    IsPresent itm_016        # Locked door
    IsFlagTrue flag_03_AT_VERB
    destroy itm_016     # Locked door
    drop itm_017     # Open door with a hallway beyond
    "The lock shatters."
    SetFlagFalse flag_03_AT_VERB
	exit true
}

noun shore
action at shore
{
    IsFlagTrue flag_03_AT_VERB
    In rm_026
    SetFlagFalse flag_03_AT_VERB
    move itm_011, rm_010     # Rusty axe (Magic word 'BUNYON' on it.)
    "OK."
	exit true
}

action at any
{
    IsFlagTrue flag_03_AT_VERB
    SetFlagFalse flag_03_AT_VERB
    "OK, I threw it."
    "Nothing happens."
	exit true
}

action at any
{
    "What?"
	exit true
}


action chop tree
{
    IsPresent itm_005        # Cypress tree

	#!moved (itm_014        # Ring of skeleton keys
	IsItemInRoom itm_014, rm_002
    IsPresent itm_011        # Rusty axe (Magic word 'BUNYON' on it.)
    destroy itm_005     # Cypress tree
    drop itm_004     # -HOLLOW- stump and remains of a felled tree
    "TIMBER. Something fell from the tree top & vanished in the swamp."
	exit true
}

action chop tree
{
    IsPresent itm_005        # Cypress tree
    IsPresent itm_011        # Rusty axe (Magic word 'BUNYON' on it.)
    destroy itm_005     # Cypress tree
    drop itm_004     # -HOLLOW- stump and remains of a felled tree
    "TIMBER!"
	exit true
}

action chop any
{
	not ispresent itm_011        # Rusty axe (Magic word 'BUNYON' on it.)
    "I'm not carrying the axe, take inventory!"
	exit true
}

action chop any
{
    "Nothing happens."
    "Maybe if I threw something?..."
	exit true
}

action get chiggers
{
	IsPresent itm_042        	# Chiggers

	try
	{
		ispresent itm_020        # Chigger bites
		"You already have chigger bites!"
		exit true
	}

	try
	{
		ispresent itm_021        # Infected chigger bites
		"You already have infected chigger bites!"
		exit true
	}

	try
	{
		has itm_007        # Evil smelling mud
		"The evil smelling mud keeps them just out of arms reach."
		exit true
	}

	move itm_020, inventory     # Chigger bites
	"I'm bitten by chiggers."
	exit true
}

action get mud
{
    IsPresent itm_007        # Evil smelling mud
    IsPresent itm_021        # Infected chigger bites
    destroy itm_021     # Infected chigger bites
    take itm_007     # Evil smelling mud
    "OK."
    "BOY, that really hit the spot!"
	exit true
}

action get honey
{
    IsPresent itm_023        # *ROYAL HONEY*
	not ispresent itm_007     # Evil smelling mud
    IsPresent itm_024        # Large African bees
    "The bees sting me!"
    losegame
	exit true
}


action get mud
{
    IsPresent itm_007        # Evil smelling mud
    IsPresent itm_020        # Chigger bites
    destroy itm_020     # Chigger bites
    take itm_007     # Evil smelling mud
    "OK."
    "BOY, that really hit the spot!"
	exit true
}

action get bees
{
    IsPresent itm_024        # Large African bees
    not ispresent itm_007
    "The bees sting me!"
    losegame
	exit true
}


action get bees
{
    IsPresent itm_024        # Large African bees
    IsPresent itm_007        # Evil smelling mud
    not ispresent itm_013        # Empty bottle

    "First, I need an empty container."
	exit true
}

action get bees
{
    IsPresent itm_024        # Large African bees
    IsPresent itm_007        # Evil smelling mud
    IsPresent itm_013        # Empty bottle
    swap itm_013, itm_026     # Empty bottle        # Bees in a bottle
    "OK."
	exit true
}

action get gas
{
    In rm_001
    not ispresent itm_040
    "First, I need an empty container."
	exit true
}

action get gas
{
    In rm_001
    IsPresent itm_040        # Empty wine bladder
    swap itm_040, itm_031     # Empty wine bladder        # Distended gas bladder
    "OK."
	exit true
}

action get mirror
{
    IsPresent itm_038        # *MAGIC MIRROR*
 	not ispresent itm_025
    take itm_038     # *MAGIC MIRROR*
    "OK."
	exit true
}


action get water
{
    IsPresent itm_006        # Water
    IsPresent itm_013        # Empty bottle
    destroy itm_013     # Empty bottle
    take itm_012     # Water in bottle
    "OK."
	exit true
}

action get water
{
    IsPresent itm_006        # Water
    not ispresent itm_013        # Empty bottle

    "First, I need an empty container."
	exit true
}

action get bricks
{
    IsPresent itm_036        # Loose fire bricks
    take itm_036     # Loose fire bricks
    "OK."
    "They're heavy!"
	exit true
}


action get mirror
{
    IsPresent itm_038        # *MAGIC MIRROR*
    IsPresent itm_025        # Very thin black bear
    "The bear won't let me."
	exit true
}

action get lava
{
    IsPresent itm_034        # Stream of lava
    "No, it's too hot."
	exit true
}

action get mud
{
    IsPresent itm_007        # Evil smelling mud
    take itm_007     # Evil smelling mud
    "OK."
	exit true
}


action get web
{
    IsPresent itm_003        # Spider web with writing on it
    "I'm bitten by a spider."
    losegame
	exit true
}

action get sign
{
    "A voice BOOOOMS out:"
    "Please leave it alone."
	exit true
}

action get honey
{
    IsPresent itm_023        # *ROYAL HONEY*
    take itm_023     		 # *ROYAL HONEY*
    "OK."
	exit true
}

action get firestone
{
    IsPresent itm_000        # Glowing *FIRESTONE*
    "No, it's too hot."
	exit true
}

action get firestone
{
    IsPresent itm_056        # *FIRESTONE* (cold now)
    "OK."
    take itm_056     # *FIRESTONE* (cold now)
	exit true
}

action light any
{
    not ispresent itm_028        # Flint & steel

    "I have nothing to light it with."
    exit true
}

action light gas
{
    has itm_031        # Distended gas bladder
    has itm_028        # Flint & steel
    "The gas bladder blew up in my hands!"
    losegame
    destroy itm_031     # Distended gas bladder

    exit true
}

action light gas
{
    IsItemHere itm_031        # Distended gas bladder
    has itm_028        # Flint & steel

    destroy itm_031     # Distended gas bladder
    "BANG! The gas bladder explodes!"

    try
    {
        IsPresent itm_032        # Bricked up window
        drop itm_036     # Loose fire bricks
        destroy itm_032     # Bricked up window
        drop itm_035     # Bricked up window with a hole in it
        exit true
    }

    try
    {
        IsPresent itm_027        # Large sleeping dragon
        drop itm_052     		# Smoking hole, Pieces of dragon and gore
        destroy itm_027     	# Large sleeping dragon
        exit true
    }

	exit true
}


action light gas
{
    IsItemHere itm_018        # Swamp gas
    has itm_028        # Flint & steel
    "Gas needs to be contained before it will burn."
	exit true
}

action light lamp
{
    IsPresent itm_009        # Lit brass lamp
    "The lamp burns with a cold, flameless blue glow."
	exit true
}

action light lamp
{
    IsPresent itm_010        # Old fashioned brass lamp
    swap itm_010, itm_009     # Old fashioned brass lamp        # Lit brass lamp
    "The lamp burns with a cold, flameless blue glow."
	exit true
}

action light any
{
    IsPresent itm_028        # Flint & steel
    not ispresent itm_018        # Swamp gas
    "That won't ignite."
	exit true
}


action drop honey
{
    IsPresent itm_023        # *ROYAL HONEY*
    IsPresent itm_025        # Very thin black bear
    destroy itm_023     # *ROYAL HONEY*
    "The bear eats the honey and falls asleep."
    drop itm_039     # Sleeping bear
    destroy itm_025     # Very thin black bear
	exit true
}

action drop honey
{
    IsPresent itm_023        # *ROYAL HONEY*
    drop itm_023     # *ROYAL HONEY*
	exit true
}

action drop gas
{
    IsPresent itm_031        # Distended gas bladder
    swap itm_031, itm_040     # Distended gas bladder        # Empty wine bladder

	"The gas dissipates. (I think you blew it.)"
	exit true
}

action drop mirror
{
    IsPresent itm_038        # *MAGIC MIRROR*
    IsPresent itm_029        # *Thick PERSIAN RUG*
    drop itm_038     # *MAGIC MIRROR*
    "The mirror lands softly on the rug, lights up and says:"
    SetFlagTrue flag_01_MIRROR_HINT1
	exit true
}

action drop bees
{
    IsPresent itm_026        # Bees in a bottle
    IsPresent itm_025        # Very thin black bear
    "The bees madden the bear, the bear then attacks me!"
    destroy itm_026     # Bees in a bottle
    drop itm_024     # Large African bees
    SetFlagTrue flag_14_BEES_FROM_BOTTLE

	exit true
}

action drop water
{
    IsPresent itm_012        # Water in bottle

    not In rm_018
    swap itm_012, itm_013     # Water in bottle        # Empty bottle

    "It soaks into the ground."
    exit true
}

action drop mirror
{
    IsPresent itm_038        # *MAGIC MIRROR*
	not ispresent itm_029        # *Thick PERSIAN RUG*

    "The mirror hits the floor and shatters into a MILLION pieces!"
    drop itm_041     # Broken glass
    destroy itm_038     # *MAGIC MIRROR*
	exit true
}

action drop bees
{
    IsPresent itm_026        # Bees in a bottle
    IsPresent itm_027        # Large sleeping dragon
    drop itm_024     # Large African bees
    drop itm_044     # *DRAGON EGGS* (very rare)
    destroy itm_027     # Large sleeping dragon
    swap itm_026, itm_013     # Bees in a bottle        # Empty bottle
    "The bees attack the dragon, which gets so annoyed it gets up and flies away..."
	exit true
}

action drop bees
{
    IsPresent itm_026        # Bees in a bottle
    drop itm_024     # Large African bees
    swap itm_026, itm_013     # Bees in a bottle        # Empty bottle
    "You drop the bees"

	exit true
}

## pour water

action pour water
{
	has itm_012        # Water in bottle
    In rm_018
    "Sizzle..."
    swap itm_012, itm_013     # Water in bottle        # Empty bottle

    try
    {
        IsPresent itm_000        # Glowing *FIRESTONE*
        swap itm_056, itm_000     # *FIRESTONE* (cold now)        # Glowing *FIRESTONE*
    }

	exit true
}

action drop water
{
    has itm_012        # Water in bottle
    In rm_018
    "Sizzle..."
    swap itm_012, itm_013     # Water in bottle        # Empty bottle

    try
    {
        IsPresent itm_000        # Glowing *FIRESTONE*
        swap itm_056, itm_000     # *FIRESTONE* (cold now)        # Glowing *FIRESTONE*
    }

	exit true
}


## TODO : Test the 'AT TREE' thing
action throw axe
{
    IsPresent itm_011        # Rusty axe (Magic word 'BUNYON' on it.)
    "In 2 words tell me at what...like: AT TREE."
    SetFlagTrue flag_03_AT_VERB
    #drop itm_011     # Rusty axe (Magic word 'BUNYON' on it.)
	exit true
}

action throw axe
{
	not ispresent itm_011        # Rusty axe (Magic word 'BUNYON' on it.)

    "I'm not carrying the axe, look in your inventory!"
	exit true
}

action throw any
{
    "Sorry, I can only throw the axe."
	exit true
}


action rub lamp
{
    IsPresent itm_009        # Lit brass lamp
    "No, it's too hot."
	exit true
}

action rub lamp
{
    has itm_010        # Old fashioned brass lamp
    IsFlagFalse flag_08_LAMP_DROP1
    "A glowing Genie appears, drops something, and then vanishes."
    here itm_048     # *DIAMOND RING*
    SetFlagTrue flag_08_LAMP_DROP1
	exit true
}

action rub lamp
{
    has itm_010        # Old fashioned brass lamp
    IsFlagTrue flag_11_LAMP_DESTROY2
	"Nothing happens."
	exit true
}

action rub lamp
{
    has itm_010        # Old fashioned brass lamp
    IsFlagTrue flag_10_LAMP_DESTROY1
    "A glowing Genie appears, says 'Boy, you're selfish', takes something and then makes 'ME' vanish!"

    SetFlagTrue flag_11_LAMP_DESTROY2

    losegame

    destroy itm_048     # *DIAMOND RING*
	exit true
}

action rub lamp
{
    has itm_010        # Old fashioned brass lamp
    IsFlagTrue flag_09_LAMP_DROP2

    "A glowing Genie appears, says 'Boy, you're selfish', takes something and then makes 'ME' vanish!"

    SetFlagTrue flag_10_LAMP_DESTROY1
    losegame

    destroy itm_049     # *DIAMOND BRACELET*
    exit true
}

action rub lamp
{
    has itm_010        # Old fashioned brass lamp
    IsFlagTrue flag_08_LAMP_DROP1

    "A glowing Genie appears, drops something, and then vanishes."

    drop itm_049     # *DIAMOND BRACELET*

    SetFlagTrue flag_09_LAMP_DROP2
	exit true
}

action rub lamp
{
	IsPresent itm_010
	"You dont have the %noun% to %verb% it."
	exit true
}

action rub any
{
    "Nothing happens."
	exit true
}

action look lava
{
    IsPresent itm_034        # Stream of lava
    "There's something there all right! Maybe I should go there?"
    exit true
}

action look tree
{
    IsPresent itm_004        # -HOLLOW- stump and remains of a felled tree
    "There's something there all right! Maybe I should go there?"
	exit true
}

action look hole
{
    "There's something there all right! Maybe I should go there?"
    exit true
}

action look any
{
	try
	{
		CanPlayerSee
		look
		exit true
	}

	"It is too dark to see."
	exit true
}


action swim any
{
    In rm_026
    #something;

    CounterGT inventory_count, 0
    "Something's too heavy. I'm sinking."

	exit true
}

action swim any
{
    In rm_026

    #nothing
    CounterEQ inventory_count, 0

    goto rm_010
    exit true
}

action swim any
{
	not In rm_026

    "Not here you cant."
    exit true
}


action wake any
{
    "Nothing happens."
    "Maybe if I threw something?..."
    exit true
}


action unlock door
{
    IsPresent itm_016        	# Locked door

    try
    {
		IsPresent itm_014        	# Ring of skeleton keys
		here itm_017     			# Open door with a hallway beyond
		destroy itm_016     		# Locked door
		"You unlock the door with the skeleton keys."
		exit true
	}

	"I can't, I dont have the keys"

	exit true
}

action unlight lamp
{
    IsPresent itm_009        	# Lit brass lamp
    swap itm_009, itm_010     	# Lit brass lamp        # Old fashioned brass lamp

    "The lamp is off."
    exit true
}

action read web
{
    IsPresent itm_003        # Spider web with writing on it
    "Chop 'er down!"
    exit true
}

action read advert
{
    IsPresent itm_062        # Large outdoor Advertisement

    "Check with your favorite computer dealer for the next Adventure program:"
    " PIRATE ADVENTURE. If they don't carry 'ADVENTURE' have them"
    " call: 1-305-862-6917 today!"
	exit true
}


action attack bear
{
    IsPresent itm_025        # Very thin black bear
    "The bear won't let me."
    "Maybe if I threw something?..."
	exit true
}

action attack dragon
{
    IsPresent itm_027        # Large sleeping dragon
    "It doesn't seem to bother him at all!"
    "Maybe if I threw something?..."
	exit true
}

noun spider
action attack spider
{
    "I don't know where it is."
	exit true
}

action attack any
{
   "How?"
   exit true
}

noun away
action say away
{
    has itm_029        # *Thick PERSIAN RUG*
    In rm_017
    "Something I'm holding vibrates and..."
    #SetFlagFalse flag_15_ROOM_IS_DARK
	goto rm_023
	exit true
}

action say away
{
    has itm_029        # *Thick PERSIAN RUG*

	not In rm_017
    not In rm_033
    not In rm_026

    "Something I'm holding vibrates and..."

    #SetFlagTrue flag_15_ROOM_IS_DARK
    goto rm_017
    exit true
}



action open door
{
	IsItemHere itm_016        	# Locked door

	try
	{
		has itm_014				# ring of skeleton keys

		destroy itm_016			# Locked door
		here itm_017			# Open door with a hallway beyond

		"You unlock the door with the skeleton keys."
		exit true
	}

	"I can't, it's locked."
	exit true
}

action wave any
{
	"Nothing happens."
	exit true
}

action make lava
{
	IsPresent itm_036		# Loose fire bricks
	IsPresent itm_034		# Stream of lava
    swap itm_000, itm_034	# Glowing *FIRESTONE*, Stream of lava
    here itm_045			# Lava stream with brick dam

	## BUG : does not destroy the bricks...
	## so we fix that.
	destroy itm_036

    "The fire bricks dam up the lava stream."
    exit true
}

action dam lava
{
	IsPresent itm_036		# Loose fire bricks
    IsPresent itm_034       # Stream of lava
    swap itm_000, itm_034   # Glowing *FIRESTONE*, Stream of lava
    here itm_045			# Lava stream with brick dam
    destroy itm_036			# Loose fire bricks

    "The fire bricks dam up the lava stream."
    exit true
}


action cross lava
{
    in rm_018
	"No, it's too hot."
	exit true
}

action cross any
{
    "How?"
    exit true
}

action fill lamp
{
    IsPresent itm_022		# Patches of 'OILY' slime
    has itm_060	        	# Empty lamp
    destroy itm_022	        # Patches of 'OILY' slime

    ## BUG : Destroys empty lamp but did not bring in lit lamp explicitly

    #destroy itm_060	        # Empty lamp
    swap itm_009, itm_060

    "The lamp is now full & lit."

    SetCounter cntLightTime, 125
    exit true
}


action scream any
{
    IsPresent itm_025        	# Very thin black bear
    "The bear is so startled that he FELL off the ledge!"
    move itm_043, rm_018     	# Slightly woozy bear
    destroy itm_025        		# Very thin black bear
	exit true
}

action scratch any
{
    IsPresent itm_020        	# Chigger bites
    "BOY, that really hit the spot! My chigger bites are now INFECTED!"
    swap itm_020, itm_021    	# Chigger bites        # Infected chigger bites
	exit true
}

action scratch any
{
	IsPresent itm_021		# Infected chigger bites
    "BOY, that really hit the spot! My bites have rotted my whole body!"

	LoseGame
	exit true
}

action scratch any
{
    "Nothing happens."
	exit true
}

noun bunyon
action say bunyon
{
    IsPresent itm_047        	# *Small statue of a BLUE OX*
    has itm_011        			# Rusty axe (Magic word 'BUNYON' on it.)
    crlf
    crlf
    "'%noun%'"
    crlf
    crlf
    move itm_011, rm_025        # Rusty axe (Magic word 'BUNYON' on it.)
    move itm_047, rm_025        # *Small statue of a BLUE OX*

    "Something I'm holding vibrates and..."
    exit true
}

action say bunyon
{
    has itm_011					# Rusty axe (Magic word 'BUNYON' on it.
    not in rm_026

	crlf
	crlf
    "'%noun%'"
    crlf
    crlf

    move itm_011, rm_025		# Rusty axe (Magic word 'BUNYON' on it.)

    "Something I'm holding vibrates and..."
    exit true
}

action say bunyon
{
    IsPresent itm_011        	# Rusty axe (Magic word 'BUNYON' on it.)

    crlf
	crlf
    "'%noun%'"
    crlf
    crlf

    "The ax vibrated!"
    exit true
}

action say bunyon
{
    crlf
    crlf
    "'%noun%'"
    crlf
    crlf
    "Nothing happens."
    exit true
}


action help any
{
    has itm_020        # Chigger bites

    "A voice BOOOOMS out:"
    "Medicine is good for bites."
    "Try --> 'LOOK, JUMP, SWIM, CLIMB, FIND, TAKE, SCORE, DROP' and any other verbs you can think of..."
    exit true
}

action help any
{
    has itm_021        # Infected chigger bites

    "A voice BOOOOMS out:"
    "Medicine is good for bites."
    "Try --> 'LOOK, JUMP, SWIM, CLIMB, FIND, TAKE, SCORE, DROP' and any other verbs you can think of..."
	exit true
}

action help any
{
    in rm_026
    "A voice BOOOOMS out:"
    "Try --> 'LOOK, JUMP, SWIM, CLIMB, FIND, TAKE, SCORE, DROP' and any other verbs you can think of..."
    "You may need to say magic words here."
	exit true
}

action help any
{
    in rm_011
    "A voice BOOOOMS out:"
    "Try --> 'LOOK, JUMP, SWIM, CLIMB, FIND, TAKE, SCORE, DROP' and any other verbs you can think of..."
    exit true
}

action help any
{
    in rm_019
    "A voice BOOOOMS out:"
    "Try --> 'LOOK, JUMP, SWIM, CLIMB, FIND, TAKE, SCORE, DROP' and any other verbs you can think of..."
    exit true
}

action help any
{
    in rm_023
    "A voice BOOOOMS out:"
    "There are only 3 ways to wake the Dragon!"
    exit true
}

action help any
{
    in rm_013
    "A voice BOOOOMS out:"
    "You may need to say magic words here."
    exit true
}

action help any
{
    in rm_017
    "A voice BOOOOMS out:"
    "You may need to say magic words here."
    exit true
}

action help any
{
    in rm_015
    "A voice BOOOOMS out:"
    "You may need to say magic words here."
    exit true
}

action help any
{
    in rm_021
    "A voice BOOOOMS out:"
    "Try --> 'LOOK, JUMP, SWIM, CLIMB, FIND, TAKE, SCORE, DROP' and any other verbs you can think of..."
    exit true
}

action help any
{
    in rm_008
    "A voice BOOOOMS out:"
    "Read the sign in the meadow!"
	exit true
}

action help any
{
    in rm_001
    "A voice BOOOOMS out:"
    "Try --> 'LOOK, JUMP, SWIM, CLIMB, FIND, TAKE, SCORE, DROP' and any other verbs you can think of..."
    exit true
}

action help any
{
    in rm_020
    "A voice BOOOOMS out:"
    "Blow it up!"
    "Try the swamp."
    exit true
}

action help any
{
    "Nothing happens."
    "You might try examining things..."
    exit true
}


action say any
{
	try
	{
		nounis any
		"Nothing happens."
		exit true
	}

    crlf
    crlf
    "'%noun%'"
    crlf
    crlf
    "Nothing happens."
    exit true
}

action eat fruit
{
    IsPresent itm_046        # *JEWELED FRUIT*
    "BOY, that really hit the spot!"
    destroy itm_046     	# *JEWELED FRUIT*
	exit true
}

action drink water
{
    IsPresent itm_012        # Water in bottle
    "BOY, that really hit the spot!"
    destroy itm_012        	# Water in bottle
    take itm_013        	# Empty bottle
	exit true
}

action drink water
{
    IsPresent itm_006        # Water
    "BOY, that really hit the spot!"
	exit true
}

action drink honey
{
    IsPresent itm_023        # *ROYAL HONEY*
    "BOY, that really hit the spot!"
    destroy itm_023        	# *ROYAL HONEY*
    exit true
}

action drink any
{
    "Huh? I don't think so!"
    exit true
}

noun swamp
action find axe
{
    "A voice BOOOOMS out: I don't know where it is."
    exit true
}

action find tree
{
    "Try the swamp."
    exit true
}

action find key
{
    "Try the swamp."
	exit true
}

action find mud
{
    "Try the swamp."
    exit true
}

action find any
{
    "I don't know where it is."
    exit true
}


action quit any
{
	"Your score is %score% in %moves% moves."

	QuitRestartGame "Do you want to Quit or Restart. Q/R?"
}


action inventory any
{
	try
	{
		CanPlayerSee
		ShowInventory
		exit true
	}

	"It is too dark to see."
	exit true
}


action score any
{
	"Your score is %score% in %moves% moves."

	# issuing score does not count as a move
	SubCounter moves, 1

	exit true
}

action version any
{
	"%game%\nRAI Version %version%\nOriginally written by %author%. Ported to RAI by Stu George."
	# issuing vers does not count as a move
	SubCounter moves, 1

	exit true
}

action look any
{
	try
	{
		CanPlayerSee
		look
		exit true
	}

	"It is too dark to see."
	exit true
}

action get any
{
	try
	{
		CanPlayerSee

		"%verb% what?"
		exit true
	}

	"It is too dark to see."
	exit true
}

action drop any
{
	"The %noun%? You don't have it."
	exit true
}

action read any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action go any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"%verb% where?"
	exit true
}

action dam any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action make any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action pour any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action eat any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action wear any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action remove any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action free any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action fill any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action switch any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action exit any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action use any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action turn any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action push any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action pull any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action open any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action close any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action unlight any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action pump any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action cut any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action move any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action lock any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action unlock any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action break any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}

action climb any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	"You can't %verb% the %noun%?"
	exit true
}


item itm_000 [firestone] "Glowing *FIRESTONE*","Glowing *FIRESTONE*" in void
item itm_001 [hole] "Dark hole","Dark hole" in rm_004
item itm_002 [rubies] "*Pot of RUBIES*","*Pot of RUBIES*" in rm_004 has take
item itm_003 [web] "Spider web with writing on it","Spider web with writing on it" in rm_002
item itm_004 [stump, treestump] "-HOLLOW- stump and remains of a felled tree","-HOLLOW- stump and remains of a felled tree" in void
item itm_005 [tree,trees] "Cypress tree","Cypress tree" in rm_001
item itm_006 [water] "Water","Water" in rm_010
item itm_007 [mud] "Evil smelling mud","Evil smelling mud" in rm_001 has take
item itm_008 [fish] "*GOLDEN FISH*","*GOLDEN FISH*" in rm_010 has take
item itm_009 [lamp] "Lit brass lamp","Lit brass lamp" in void has light has take
item itm_010 [lamp] "Old fashioned brass lamp","Old fashioned brass lamp" in rm_003 has take
item itm_011 [axe] "Rusty axe (Magic word 'BUNYON' on it.)","Rusty axe (Magic word 'BUNYON' on it.)" in rm_010 has take
item itm_012 [bottle] "Water in bottle","Water in bottle" in rm_003 has take
item itm_013 [bottle] "Empty bottle","Empty bottle" in void has take
item itm_014 [key,keys] "Ring of skeleton keys","Ring of skeleton keys" in rm_002 has take
item itm_015 [sign] "Sign 'Leave TREASURES here, then say: SCORE'","Sign 'Leave TREASURES here, then say: SCORE'" in rm_003
item itm_016 [door] "Locked door","Locked door" in rm_005
item itm_017 [door] "Open door with a hallway beyond","Open door with a hallway beyond" in void
item itm_018 [gas] "Swamp gas","Swamp gas" in rm_001
item itm_019 [net] "*GOLDEN NET*","*GOLDEN NET*" in rm_018 has take
item itm_020 [chigger,bites] "Chigger bites","Chigger bites" in void
item itm_021 [chigger,bites] "Infected chigger bites","Infected chigger bites" in void
item itm_022 [slime] "Patches of 'OILY' slime","Patches of 'OILY' slime" in rm_001 has take
item itm_023 [honey] "*ROYAL HONEY*","*ROYAL HONEY*" in rm_008
item itm_024 [bees] "Large African bees","Large African bees" in rm_008
item itm_025 [bear] "Very thin black bear","Very thin black bear" in rm_021
item itm_026 [bottle] "Bees in a bottle","Bees in a bottle" in void has take
item itm_027 [dragon] "Large sleeping dragon","Large sleeping dragon" in rm_023
item itm_028 [flint] "Flint & steel","Flint & steel" in rm_030 has take
item itm_029 [rug] "*Thick PERSIAN RUG*","*Thick PERSIAN RUG*" in rm_017 has take
item itm_030 [sign] "Sign: 'magic word's AWAY! Look la...' (Rest of sign is missing!)","Sign: 'magic word's AWAY! Look la...' (Rest of sign is missing!)" in rm_018
item itm_031 [bladder] "Distended gas bladder","Distended gas bladder" in void has take
item itm_032 [window] "Bricked up window","Bricked up window" in rm_020
item itm_033 [sign] "Sign here says 'In many cases mud is good. In others...'","Sign here says 'In many cases mud is good. In others...'" in rm_023
item itm_034 [lava] "Stream of lava","Stream of lava" in rm_018
item itm_035 [window] "Bricked up window with a hole in it","Bricked up window with a hole in it" in void
item itm_036 [bricks, brick] "Loose fire bricks","Loose fire bricks" in void has take
item itm_037 [crown] "*GOLD CROWN*","*GOLD CROWN*" in rm_022 has take
item itm_038 [mirror] "*MAGIC MIRROR*","*MAGIC MIRROR*" in rm_021 has take
item itm_039 [bear] "Sleeping bear","Sleeping bear" in void
item itm_040 [bladder] "Empty wine bladder","Empty wine bladder" in rm_009 has take
item itm_041 [glass] "Broken glass","Broken glass" in void
item itm_042 [chiggers] "Chiggers","Chiggers" in rm_001
item itm_043 [bear] "Slightly woozy bear","Slightly woozy bear" in void
item itm_044 [egg, eggs] "*DRAGON EGGS* (very rare)","*DRAGON EGGS* (very rare)" in void has take
item itm_045 [lava] "Lava stream with brick dam","Lava stream with brick dam" in void
item itm_046 [fruit] "*JEWELED FRUIT*","*JEWELED FRUIT*" in rm_025 has take
item itm_047 [ox] "*Small statue of a BLUE OX*","*Small statue of a BLUE OX*" in rm_026 has take
item itm_048 [ring] "*DIAMOND RING*","*DIAMOND RING*" in void has take
item itm_049 [bracelet] "*DIAMOND BRACELET*","*DIAMOND BRACELET*" in void has take
item itm_050 [scratchings] "Strange scratchings on rock says: 'ALADDIN was here'","Strange scratchings on rock says: 'ALADDIN was here'" in rm_014
item itm_051 [sign] "Sign says 'LIMBO. Find right exit and live again!'","Sign says 'LIMBO. Find right exit and live again!'" in rm_033
item itm_052 [hole] "Smoking hole, Pieces of dragon and gore","Smoking hole, Pieces of dragon and gore" in void
item itm_053 [sign] "Sign says 'No swimming allowed here.'","Sign says 'No swimming allowed here.'" in rm_010
item itm_054 [arrow] "Arrow pointing down","Arrow pointing down" in rm_017
item itm_055 [fish] "Dead fish","Dead fish" in void
item itm_056 [firestone] "*FIRESTONE* (cold now)","*FIRESTONE* (cold now)" in void has take
item itm_057 [sign] "Sign says 'Paul's place.'","Sign says 'Paul's place.'" in rm_025
item itm_058 [trees] "Trees","Trees" in rm_011
item itm_059 [sign] "Sign here says 'Opposite of LIGHT is UNLIGHT.'","Sign here says 'Opposite of LIGHT is UNLIGHT.'" in rm_012
item itm_060 [lamp] "Empty lamp","Empty lamp" in void has take
item itm_061 [rug] "Muddy worthless old rug","Muddy worthless old rug" in void has take
item itm_062 [advert, ad, advertisement] "Large outdoor Advertisement","Large outdoor Advertisement" in rm_029
item itm_063 [hole] "Hole","Hole" in rm_029
