game "Lost Dutchman's Mine"
author "Cleveland M. Blakemore"
#version "20110402"

# require interpreter version to be of v1 standard
requires_interpreter	1

max_carry		6

# setup our basic counters
counter cntScore
counter cntMoves
counter cntWolf
counter cntGhost
counter cntRiver
counter cntReleasedWater
counter cntBullets


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
verb say, shout, yell
verb move, push, pull
verb shoot
verb version, ver, credits, author
verb score
verb light, ignite
verb unlight, extinguish

verb call
verb dig
verb kill
verb say
verb turn, rotate, twist
verb whistle, blow


noun gozer
noun shelf


pregame
{
	SetCounter cntScore, 0
	SetCounter cntMoves, 0

	SetCounter cntGhost, 0
	SetCounter cntWolf, 0
	SetCounter cntRiver, 3
	SetCounter cntReleasedWater, 0
	SetCounter cntBullets, 3

	# set default light on
	SetLightOn

	"%game% was written by %author%. Written in 1988 for Ahoy! magazine"
	"ported to RAI by Stu George."

	goto rm_GasStation
}


# triggers on each valid move
on_success
{
	AddCounter cntMoves, 1
	#"OK"
}

on_fail
{
	AddCounter cntMoves, 1
	"I do not understand."
}

prompt
{
	try
	{
		in rm_StoneStaircase
		try
		{
			IsPresent itm_Wolf
			AddCounter cntWolf, 1

			"The timber wolf edges closer..."
			crlf
			crlf

			try
			{
				CounterEquals cntWolf, 3
				"The timber wolf tears you to shreds."
				crlf
				LoseGame
			}
		}
	}


	try
	{
		in rm_RiverBed
		CounterEquals cntReleasedWater, 1

		try
		{
			not CounterEquals cntRiver, 0
			SubCounter cntRiver, 1

			try
			{
				CounterEquals cntRiver, 1
				"Bubbles are coming out of your clenched lips under water."
				crlf
				crlf
			}

			try
			{
				CounterEquals cntRiver, 0
				"You drown in the river"
				LoseGame
			}
		}
	}

	try
	{
		in rm_GasStation
		setcounter cntscore, 0

		try
		{
			IsItemHere itm_Diamond
			addcounter cntscore, 10
		}
		try
		{
			IsItemHere itm_Nugget
			addcounter cntscore, 10
		}
		try
		{
			IsItemHere itm_Ruby
			addcounter cntscore, 10
		}

		WinGame
	}

	"> "
}


on_gamewin
{
	crlf
	crlf
	"Congratulations!"
	crlf
	crlf
	"Your score is %cntscore% in %cntmoves% moves."

	QuitRestartGame "Do you want to Play Again. Q/R?"
}

on_gamefail
{
	crlf
	crlf
	"You failed this time!"
	crlf
	crlf
	"Your score is %cntscore% in %cntmoves% moves."

	# takes always a Y/N
	QuitRestartGame "Do you want to Play Again. Q/R?"
}

in_dark
{
	"It is too dark to see."
}


# we have default rooms of VOID, INVENTORY and DESTROYED
noun player, me



item itm_Ruby [ruby] "ruby", "a lustrous red ruby" in void has take
item itm_Nugget [nugget] "nugget", "the dutchman's nugget" in void has take


item itm_Bottle [bottle] "bottle", "empty 7-Up bottle" in rm_GasStation has take
item itm_Plaque [plaque] "plaque", "a metal plaque above a glass shelf" in rm_GasStation
item itm_Matchbook [matchbook, matches] "matchbook", "matchbook" in rm_GasStation has take
item itm_Revolver [revolver, gun, pistol] "revolver", "a perl handled revolver" in rm_GasStation has take

room rm_GasStation "Gas Station"
{
	## this is our treasure room!
	## return here to win.
	enter
	{

	}

	look
	{
		"You are in a deserted last chance gas station."
	}
}

action read matchbook
{
	has itm_Matchbook
	"Gozer Travel inc. 'Need to travel?? Call Gozer!!'"
	exit true
}

noun gozer
action call gozer
{
	try
	{
		in rm_GasStation
		"Gozer's Taxi service pulls up outside and drives you out to an old Adobe House."
		goto rm_AdobeHouse
		exit true
	}

	try
	{
		in rm_AdobeHouse
		"Gozer's Taxi service drops you back off at the gas station."
		goto rm_GasStation
		exit true
	}

	"You are not sure how to do that from where you are."
	exit true
}

item itm_UnlightTorch [torch] "torch","a wooden torch" in rm_AdobeHouse has take
item itm_Torch [torch] "torch (lit)","a lit torch" in void has LIGHT has take
item itm_Skull [skull] "skull","a skull mounted on a spear" in rm_AdobeHouse has take

room rm_AdobeHouse "Adobe House"
{
	look
	{
		"You are in a dusty adobe house with a sunbeam coming through an east window. You can leave the house to the north."
	}

	on n
	{
		goto rm_MineEntrance
		exit true
	}
}

item itm_Skeleton [skeleton, bones] "bones","a skeleton draped with cobwebs" in rm_EW_MineShaft
item itm_Bullets [bullets] "bullets","a handfull of rusty bullets" in rm_EW_MineShaft has take

action look skeleton
{
	IsItemHere itm_Skeleton
	"His bony claw points to the east..."
	exit true
}

action get skeleton
{
	IsItemHere itm_Skeleton
	"Out of respect, you think it better to leave them here."
	exit true
}

room rm_EW_MineShaft "Mine Shaft"
{
	enter
	{
		SetLightOn
	}

	look
	{
		"You are in a long e-w running mineshaft, curving upward is a tunnel."
	}

	on u
	{
		goto rm_GraniteTunnel
		exit true
	}

	on w
	{
		goto rm_StoneStaircase
		exit true
	}

	on e
	{
		goto rm_CoolCavern
		exit true
	}
}

room rm_GraniteTunnel "Granite Tunnel"
{
	enter
	{
		SetLightOff
	}

	look
	{
		try
		{
			# do we have light
			#IsPresent itm_Torch
			CanPlayerSee
			IsItemInRoom itm_Ruby, void
			here itm_Ruby
		}

		"You are in a sloping granite tunnel that continues downward and upward."
	}

	on d
	{
		goto rm_EW_MineShaft
		exit true
	}

	on u
	{
		goto rm_MineEntrance
		exit true
	}
}


item itm_Spirit [spirit, ghost, wraith] "evil spirit", "an angry whistling evil spirit" in rm_MineEntrance has take

room rm_MineEntrance "Mine Entrance"
{
	enter
	{
		SetLightOn
	}

	look
	{
		"You are in front of an old boarded up mineshaft entrance. The Adobe House is"
		"to the south and a rough track off to the west."
	}

	on n
	{
		try
		{
			IsPresent itm_Spirit
			"The spirit scares you back."
			exit true
		}

		goto rm_GraniteTunnel
		exit true
	}

	on s
	{
		goto rm_AdobeHouse
		exit true
	}

	on w
	{
		goto rm_Ravine
		exit true
	}
}

noun crack
action go crack
{
	try
	{
		in rm_CoolCavern

		try
		{
			CounterLT inventory_count, 2
			goto rm_RiverBed
			exit true
		}

		"You are carrying too much to fit through the crack."
		exit true
	}

	try
	{
		in rm_RiverBed

		try
		{
			CounterLT inventory_count, 2
			goto rm_CoolCavern
			exit true
		}

		"You are carrying too much to fit through the crack."
		exit true
	}
}

action look calendar
{
	IsItemHere itm_Calendar
	"You wonder what a stone sun calendar is doing all the way down here..."
	exit true
}

item itm_Calendar [calendar, stone] "stone calendar","a colossal stone sun calendar" in rm_CoolCavern

room rm_CoolCavern "Cool Cavern"
{
	look
	{
		"You are inside a cool cavern with a small crack leading down to the north, and the mineshaft"
		"off to the west"
	}

	on n
	{
		try
		{
			CounterLT inventory_count, 2
			goto rm_RiverBed
			exit true
		}

		"You are carrying too much to fit through the crack."
		exit true
	}

	on d
	{
		try
		{
			CounterLT inventory_count, 2
			goto rm_RiverBed
			exit true
		}

		"You are carrying too much to fit through the crack."
		exit true
	}

	on w
	{
		goto rm_EW_MineShaft
		exit true
	}
}

item itm_Wolf [wolf] "ravenous wolf", "a ravenous snarling timber wolf!" in rm_StoneStaircase
item itm_DeadWolf [wolf] "dead wolf", "a dead timber wolf" in void

action get wolf
{
	try
	{
		IsItemHere itm_Wolf
		"You have second thoughts about grabbing a vicious wolf"
		exit true
	}

	try
	{
		IsItemHere itm_DeadWolf
		"It is too heavy to drag around"
		exit true
	}
}

room rm_StoneStaircase "Stone Staircase"
{
	enter
	{
		IsItemInRoom itm_DeadWolf, void
		SetCounter cntWolf, 0
	}

	look
	{
		"You are on a spiral stone staircase that continues downward. To the east is the mineshaft."
	}

	on e
	{
		goto rm_EW_MineShaft
		exit true
	}

	on d
	{
		try
		{
			IsPresent itm_Wolf
			"The wolf wont let me...."
			exit true
		}

		try
		{
			CounterEquals cntReleasedWater, 0
			"You quickly come back up, its flooded down there."
			exit true
		}

		goto rm_BurialGround
		exit true
	}
}

item itm_Wheel [wheel] "wheel", "a circular wheel set in the middle of an iron door to the east" in rm_RiverBed
item itm_Diamond [diamond] "diamond", "a glowing diamond" in rm_RiverBed has take

room rm_RiverBed "River Bed"
{
	look
	{
		"You are in a subterranean river bed running east & west. The crack is to your south."
	}

	on s
	{
		goto rm_CoolCavern
		exit true
	}

	on u
	{
		goto rm_CoolCavern
		exit true
	}
}

item itm_Mound [mound] "burial mound", "a large burial mound" in rm_BurialGround

room rm_BurialGround "Burial Ground"
{
	look
	{
		"You are in an ancient indian burial ground. The stone stairs go up from here."
	}

	on u
	{
		goto rm_StoneStaircase
		exit true
	}
}

item itm_Shovel [shovel] "rusty shovel", "a rusty shovel" in rm_Ravine has take
room rm_Ravine "Ravine"
{
	look
	{
		"You are in a deep ravine. The track you followed in goes off to the east."
	}

	on e
	{
		goto rm_MineEntrance
		exit true
	}
}


action read plaque
{
	IsPresent itm_Plaque
	"Put all the treasure on this shelf."
	exit true
}

verb help, hint
action help any
{
	try
	{
		in rm_AdobeHouse
		"Try reading the matchbook."
		exit true
	}

	"Try to find the gold and escape!"
	exit true
}


action whistle bottle
{
	has itm_Bottle
	IsPresent itm_Spirit

	"The spirit writhes and vanishes in a cloud of smoke."
	destroy itm_Spirit
	exit true
}

action whistle any
{
 	"Tweet Tweet Tweet..."

 	try
 	{
		IsPresent itm_Spirit
		AddCounter cntGhost, 1

		try
		{
			CounterEquals cntGhost, 1
			crlf
			crlf
			"The spirit trembles and wavers a little."
			#exit true
		}

		try
		{
			CounterEquals cntGhost, 2
			crlf
			crlf
			"The spirit looks real angry!"
			#exit true
		}

		try
		{
			CounterEquals cntGhost, 5
			crlf
			crlf
			"The evil spirit sucked the breath out of you!"
			LoseGame
		}
	}
	exit true
}


action light torch
{
	try
	{
		IsPresent itm_Torch
		"It's already burning."
		exit true
	}


	try
	{
		IsPresent itm_UnlightTorch

		try
		{
			IsItemInRoom itm_Matchbook, INVENTORY
			"You light the torch."
			swap itm_UnlightTorch, itm_Torch
			exit true
		}

		"You have nothing to light the torch with."
		exit true
	}

	"There is no torch around here"
	exit true
}

action look bottle
{
	IsPresent itm_Bottle
	"Its an old 7-Up bottle"
	exit true
}

action look bullets
{
	IsPresent itm_Bullets
	"There are %cntbullets% old bullets"
}

action look torch
{
	try
	{
		IsPresent itm_Torch
		"The torch is lit."
		exit true
	}

	try
	{
		IsPresent itm_UnlightTorch
		"The torch is not currently lit."
		exit true
	}
}

action look skeleton
{
	IsPresent itm_Skeleton
	"His bony claw points to the east."
	exit true
}


action shoot wolf
{
	IsPresent itm_Wolf
	has itm_Revolver

	try
	{
		has itm_Bullets
		"The revolver thunders fire and the wolf falls over dead in mid-leap."
		swap itm_DeadWolf, itm_Wolf
		SubCounter cntBullets, 1

		try
		{
			CounterEquals cntBullets, 0
			destroy itm_Bullets
		}

		exit true
	}

	"I have no bullets!"
	exit true
}

action shoot spirit
{
	IsPresent itm_Spirit
	has itm_Revolver

	try
	{
		has itm_Bullets
		"The revolver fires with a loud report"
		SubCounter cntBullets, 1

		try
		{
			CounterEquals cntBullets, 0
			destroy itm_Bullets
		}

		exit true
	}

	"I have no bullets!"
	exit true
}

action shoot any
{
	"The gun makes a loud report."

	SubCounter cntBullets, 1

	try
	{
		CounterEquals cntBullets, 0
		destroy itm_Bullets
	}

	exit true
}

action turn wheel
{
	in rm_RiverBed

	CounterEquals cntReleasedWater, 0
	SetCounter cntReleasedWater, 1

	"The door blasts open with a tidal wave of water!!!"
	crlf
	crlf
	"The river bed quickly fills up and you are drenched."
	crlf
	crlf
	"You'd better leave. It's almost neck deep in here!"

	SetCounter cntRiver, 3

	exit true
}


action dig mound
{
	try
	{
		has itm_Shovel

		try
		{
			in rm_BurialGround

			try
			{
				IsItemInRoom itm_Nugget, void
				"okay, you have dug a deep hole... hey it looks like there is a nugget down there"
				here itm_Nugget
				exit true
			}

			"The mound has already been excavated."
			exit true
		}

		"You dig and nothing much happens."
		exit true
	}

	"You'll need something to dig with first."
	exit true
}

action dig any
{
	try
	{
		has itm_Shovel

		"Try it somewhere else"
		exit true
	}

	"You'll need something to dig with first."
	exit true
}

action look wolf
{
	try
	{
		IsPresent itm_DeadWolf
		"The wolf is dead."
		exit true
	}

	try
	{
		IsPresent itm_Wolf
		"The wolf is fierce looking, and kinda rabid. You dont want to get too close!"
		exit true
	}
}

action quit any
{
	"Your score is %cntscore% in %cntmoves% moves."

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
	"Your score is %cntscore% in %cntmoves% moves."
	exit true
}

action version any
{
	"'%game%' version %version%"
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
	"The %noun%? You don't have one."
	exit true
}

action light any
{
	try
	{
		IsPresent itm_Matchbook
		"You are not a pyromaniac"
		exit true
	}

	"You have no matches"
	exit true
}

action go any
{
	"What?."
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

action throw any
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

action call any
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

action kill any
{
	try
	{
		nounis any
		"Umm.. what?"
		exit true
	}

	try
	{
		nounis wolf
		here itm_Wolf
		"You need a better plan of action."
		exit true
	}

	try
	{
		nounis wolf
		here itm_DeadWolf
		"Its already dead."
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


action say any
{
	try
	{
		nounis any
		"..."
		exit true
	}

	crlf
	crlf
	"''%noun%'"
	crlf

	exit true
}
