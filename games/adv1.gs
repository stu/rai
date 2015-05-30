#
# some declarations
#

game "Test Adventure"
author "Stu George"
## version left null becomes 20110313 date etc
release					"r1"
version "20110921/%release%"

# require interpreter version to be of v1 standard
requires_interpreter	1

max_carry		5

# setup our basic counters
# moves + score setup by compiler

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
verb panic
verb quit, q
verb climb, shinny
verb throw, chuck, pitch, lob, toss
verb look, l, examine, ex, x
verb break, kick, smash, bash
verb unlock
verb pump, inflate
verb say, shout, yell
verb cut, saw
verb move, push, pull
verb rub, polish, shine
verb spray, squirt
verb fix, repair, mend, build
verb shoot
verb version, ver, credits, author
verb score
verb light
verb unlight, extinguish
verb switch
verb oil, grease

verb fill
verb empty
verb open

pregame
{
	SetCounter score, 0
	SetCounter moves, 0

	# set default light on
	SetLightOn

	"An aimless sunday meander has found you in a meadow peering down into a small dark crack into the unknown."

	goto rmMeadow
}


# triggers on each valid move
on_success
{
	AddCounter moves, 1
	#"OK"
}

on_fail
{
	AddCounter moves, 1
	"I do not understand"
}

prompt
{
	"> "
}

on_gamewin
{
	crlf
	crlf
	"Congratulations you came out of the cave with a gold bar!"
	crlf
	crlf
	"Your score is %score% in %moves% moves."

	QuitRestartGame "Do you want to Play Again. Q/R?"
}

on_gamefail
{
	crlf
	crlf
	"You failed this time."
	crlf
	crlf
	"Your score is %score% in %moves% moves."

	# takes always a Y/N
	QuitRestartGame "Do you want to Play Again. Q/R?"
}

in_dark
{
	"\nIt is too dark to see."
}

item itmLamp [lamp] "Lamp", "An old adventurers lamp, it is off." in rmMeadow has take
item itmLampLit [lamp] "Lamp (lit)", "An old adventurers lamp, it is on." in void has light has take

item itmVase [vase] "Vase", "A beautiful vase." in rmCavern5 has take
item itmVaseOil [vase] "Vase full of oil", "A beautiful vase filled with oil" in void has take

item itmPoolOil [oil] "Pool of Oil", "A shiny black pool of oil." in rmCavern3

item itmRustyDoor [door] "Rusty door", "An old rusted door." in rmCavern4
item itmOiledDoor [door] "Rusty door", "An old rusted door that has been oiled." in void

item itmGold [gold] "Gold", "A shiny bar of gold" in rmCavern6 has take


room rmMeadow "Meadow"
{
	enter
	{
		SetLightOn

		try
		{
			has itmGold
			WinGame
			exit true
		}
	}

	look
	{
		"You stand in a meadow, there is a crack delving to darkness here."
	}

	on d
	{
		"You squeeze your body through the very narrow crack in the ground."
		goto rmCavern2
		exit true
	}
}

noun crack
action go crack
{
	try
	{
		in rmMeadow
		"You squeeze your body through the very narrow crack in the ground."
		goto rmCavern2
		exit true
	}
}


room rmCavern2 "Cave"
{
	enter
	{
		SetLightOff
	}

	look
	{
		"You are in a cavern. You can see exists to the east, south and west, and one dimly filtering the sunlight above you."
	}

	on u
	{
		goto rmMeadow
		exit true
	}

	on w
	{
		goto rmCavern5
		exit true
	}

	on e
	{
		goto rmCavern3
		exit true
	}

	on s
	{
		goto rmCavern4
		exit true
	}
}


room rmCavern5 "Cave"
{
	enter
	{

	}

	look
	{
		"You are in a cavern, it has a small opening to the east"
	}

	on e
	{
		goto rmCavern2
		exit true
	}
}


room rmCavern3 "Cave"
{
	enter
	{

	}

	look
	{
		"You are in a cavern, it has a small opening to the west."
	}

	on w
	{
		goto rmCavern2
		exit true
	}
}


room rmCavern4 "Cave"
{
	enter
	{
		try
		{
			IsItemInRoom itmRustyDoor, void
			here itmOiledDoor
		}
	}

	look
	{
		"You are in a cavern, a jumble of shadows reveal a door to the south and the gaping maw of an opening to the north."
	}

	on n
	{
		goto rmCavern2
		exit true
	}

	on s
	{
		try
		{
			IsPresent itmRustyDoor
			"The rusty door holds fast."
			exit true
		}

		"With some hard work you push aside the heavy door... you then hear it grind closed behind you."

		goto rmCavern6
		exit true
	}
}



room rmCavern6 "Cave"
{
	enter
	{
		here itmOiledDoor
	}

	look
	{
		"You are in a cavern, a heavy looking door is to your north."
	}

	on n
	{
		"With your shoulder to the door you manage to squeeze past the door which then closes behind you."

		goto rmCavern4
		exit true
	}
}

##############################################################################
##############################################################################
##############################################################################
##############################################################################

action light lamp
{
	try
	{
		IsPresent itmLampLit
		"The %noun% is already lit."
		exit true
	}

	try
	{
		IsPresent itmLamp
		swap itmLamp, itmLampLit
		"You turn the %noun% on."

		exit true
	}

	"You do not have the %noun% to %verb%."
	exit true
}

action unlight lamp
{
	try
	{
		IsPresent itmLampLit
		swap itmLamp, itmLampLit
		"You turn the %noun% off."
		exit true
	}

	try
	{
		IsPresent itmLamp
		"The %noun% is not lit."
		exit true
	}

	"You do not have the %noun% to %verb%."
	exit true
}

action oil door
{
	try
	{
		IsPresent itmOiledDoor
		"You have alredy oiled it."
		exit true
	}

	try
	{
		IsPresent itmRustyDoor
		try
		{
			IsPresent itmVaseOil
			"You %verb% the door."

			swap itmVaseOil, itmVase
			swap itmRustyDoor, itmOiledDoor

			AddCounter score, 5
			exit true
		}

		"You have nothing to %verb% it with."
		exit true
	}

	"%verb% the what?"
	exit true
}

action get oil
{
	try
	{
		IsPresent itmPoolOil
		try
		{
			IsPresent itmVaseOil
			"You already have some."
			exit true
		}

		try
		{
			IsPresent itmVase
			"You fill the vase with oil."
			swap itmvase, itmVaseOil
			exit true
		}

		"You have nothing to carry it in."
		exit true
	}

	"You can't see any %noun%"
	exit true
}

action fill vase
{
	IsPresent itmVase

	try
	{
		IsPresent itmPoolOil
		try
		{
			IsPresent itmVaseOil
			"You already have some."
			exit true
		}

		try
		{
			IsPresent itmVase
			"You fill the vase with oil."
			swap itmvase, itmVaseOil
			exit true
		}
	}

	"You can't see any %noun%"
	exit true
}

action drop oil
{
	IsPresent itmVaseOil

	try
	{
		IsPresent itmPoolOil
		"You pour the oil back into the pool."
		swap itmVase, itmVaseOil
		exit true
	}

	"You empty the vase of oil on the ground."
	swap itmVase, itmVaseOil
	exit true
}


action empty vase
{
	IsPresent itmVaseOil

	try
	{
		IsPresent itmPoolOil
		"You pour the oil back into the pool."
		swap itmVase, itmVaseOil
		exit true
	}

	"You empty the vase of oil on the ground."
	swap itmVase, itmVaseOil
	exit true
}

action open door
{
	try
	{
		IsPresent itmRustyDoor
		"The door is stuck fast."
		exit true
	}

	try
	{
		IsPresent itmOiledDoor
		"You flex some flaccid muscle opon the door and it moves some and closes again."
		exit true
	}
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

action get gold
{
	isitemhere itmGold
	AddCounter score, 10
	take itmGold
	"You pickup the %noun%."
	exit true
}

action drop gold
{
	has itmGold
	SubCounter score, 10
	drop itmGold
	"You drop the %noun%."
	exit true
}

