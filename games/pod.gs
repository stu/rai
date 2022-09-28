#
# Port of Planet of Death from its z80 code base actions
#
#

requires_interpreter	1
max_carry				6
author 					"Artic Computing"
version 				"1"
game 					"Planet Of Death"


verb wear, puton
verb remove, takeoff
verb get, take, grab, steal
verb drop
verb quit, q
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
verb go, enter
verb inventory, inv, i
verb version, ver, credits, author
verb score
verb kick, break

item itmBoots [boots, shoes] "boots", "Boots" in rmStrangeHouse has take
item itmBootsWorn [boots, shoes] "boots (worn)"," boots" in void
item itmStarterMotor [starter, motor] "starter motor", "starter motor" in rmTallLift has take
item itmKey [key] "key", "key" in rmComputer has take
item itmLaserGun [laser,gun] "laser gun", "laser gun" in rmOldShed has take
item itmMetalBar [bar] "metal bar", "metal bar" in void has take
item itmBaredWindow [window] "barred window", "barred window" in rmPrison
item itmHoleInWall [hole] "hole in wall", "hole in wall" in void
item itmGoldCoin [coin, gold] "gold coin", "gold coin" in void
item itmMirror [mirror] "mirror", "mirror" in void
item itmGreenManOnMirror [man, greenman] "sleeping green man", "The green man is sleeping on the mirror" in rmQuietCavern has take
item itmGreenMan [man, greenman] "green man", "The green man is sleeping" in void has take
item itmBrokenGlass [glass] "broken glass", "broken glass" in void has take
item itmGloves [gloves] "gloves", "gloves" in rmWindTunnel has take
item itmGlovesWorn [gloves] "gloves (worn)", "gloves (worn)" in void
item itmRope [rope] "rope", "rope" in void has take
item itmBoard [board] "floor board", "floor board" in rmStrangeHouse has take
item itmBrokenBoard [board] "broken floor board", "broken floor board" in void has take
item itmStalactites [stalactites, stalactite] "stalactites", "stalactites" in void has take
item itmIce [ice, iceblock, block] "block of ice", "block of ice" in rmIceCavern has take
item itmFlint [flint] "flint", "flint" in rmMountainPlateau has take
item itmStones [stones, stone] "stones", "stones" in rmDeepPit
item itmPool [pool, water] "pool of water", "pool of water" in void
item itmGuard [guard] "sleeping guard", "The security guard is asleep." in rmHanger
#item itmForceField [forcefield,field] "forcefield", "forcefield" in rmPassage

noun forcefield, field
noun spaceship, ship

counter cntIceMelts
counter cntGreenManWakesUp
counter cntGuardWakesUp
counter cntGreenManDeathMoves
counter cntForceField

counter cnt_9
counter cnt_10

counter cnt_12

flag flRope
flag flTouchedSlimyGreenMan
flag flTiedRopeDownPit

# triggers on each valid move
on_success
{
	AddCounter moves, 1
	#"OK"
}

on_fail
{
	AddCounter moves, 1
	"I do not understand."
}


on_gamewin
{
	SetCounter score, 100

	crlf
	crlf
	"Congratulations!"

	crlf
	crlf

	"The lift has taken me up to a plateau."
	"You have managed to complete this adventure alive."

	crlf
	crlf

	"Now try Adventure B: Inca Treasure"

	crlf
	crlf
	"Your score is %score% in %moves% moves."

	QuitRestartGame "Do you want to Play Again. Q/R?"
}

on_gamefail
{
	crlf
	crlf
	"You failed this time!"
	crlf
	crlf
	"Your score is %score% in %moves% moves."

	# takes always a Y/N
	QuitRestartGame "Do you want to Play Again. Q/R?"
}

pregame
{
	SetCounter score, 0
	SetCounter moves, 0

	SwitchPlayer player

	# set default light on
	SetLightOn

	"WELCOME TO ADVENTURE A"
	crlf
	crlf

	"(C)1981 ARTIC COMPUTING"
	crlf

	"You can have an adventure in your own home. You command me with short sentences."
	crlf
	crlf

	"Some useful words are;"
	crlf
	"inventory - tells you what you have"
	crlf
	"look - redescribe the location"
	crlf
	"quit - To restart the game"
	crlf
	crlf
	"also get, put, use and many more."
	crlf

	SetFlagFalse flTiedRopeDownPit
	SetFlagFalse flRope
	SetFlagFalse flTouchedSlimyGreenMan

	SetCounter cntGreenManDeathMoves, 0
	SetCounter cntForceField, 0
	SetCounter cntGreenManWakesUp, 0
	SetCounter cntGuardWakesUp, 0
	SetCounter cntIceMelts, 0

	SetCounter cnt_9, 0
	SetCounter cnt_10, 0
	SetCounter cnt_12, 0

	goto rmMountainPlateau
}

prompt
{
	# green man death
	try
	{
		IsFlagTrue flTouchedSlimyGreenMan
		SubCounter cntGreenManDeathMoves, 1
	}

	try
	{
		CounterGT cntIceMelts, 0

		# dont count down if ice block is in the ice cavern
		try
		{
			not IsItemInRoom itmIce, rmIceCavern
			SubCounter cntIceMelts, 1
		}
	}

	try
	{
		CounterGT cntGreenManWakesUp, 0
		SubCounter cntGreenManWakesUp, 1
	}

	try
	{
		in rmWindTunnel
		random 80
		goto rmQuietCavern
		crlf
		"The wind tunnel turned on."
		crlf
	}

	try
	{
		CounterEQ cntIceMelts, 1
		IsPresent itmIce
		#swap itmIce, itmPool
		destroy itmIce
		here itmPool
		crlf
		"The ice has melted into a puddle of water."
		crlf
	}

	try
	{
		CounterEQ cntGuardWakesUp, 1
		IsPresent itmGuard
		"The guard woke up and shot me!"
		LoseGame
		exit true
	}

	try
	{
		CounterEQ cntGreenManWakesUp, 1
		IsPresent itmGreenMan
		"The green man woke up and throttled me!"
		LoseGame
		exit true
	}

	try
	{
		CounterEQ cntGreenManWakesUp, 1
		IsPresent itmGreenManOnMirror
		"The green man woke up and throttled me!"
		LoseGame
		exit true
	}

	try
	{
		CounterEQ cntGreenManDeathMoves, 1
		IsFlagTrue flTouchedSlimyGreenMan
		"I have turned green and dropped down dead!"
		LoseGame
		exit true
	}

	"> "
}

action wear gloves
{
	has itmGloves
	swap itmGloves, itmGlovesWorn
	"You wear the gloves."
	exit true
}

action drop gloves
{
	has itmGlovesWorn
	"You will have to remove them first."
	exit true
}

action remove gloves
{
	has itmGlovesWorn
	swap itmGlovesWorn, itmGloves
	"You take the gloves off."
	exit true
}

action wear boots
{
	has itmBoots
	swap itmBoots, itmBootsWorn
	"You put the boots on."
	exit true
}

action drop boots
{
	has itmBootsWorn
	"You will have to remove them first."
	exit true
}

action remove boots
{
	has itmBootsWorn
	swap itmBootsWorn, itmBoots
	"You take the boots off your feet."
	exit true
}

action go spaceship
{
	in rmHanger
	goto rmSpaceShip
	exit true
}

## mirror does not have built in take
action get mirror
{
	try
	{
		CanPlayerSee

		try
		{
			IsItemhere itmMirror

			try
			{
				IsItemHere itmGreenManOnMirror
				"The green man is sleeping on the mirror!"
				exit true
			}

			take itmMirror
			"You %verb% the %noun%"
			exit true
		}

		continue true
	}

	"It is too dark to see."
	exit true
}

action look mirror
{
	"Its a mirror, nice and reflective!"
	exit true
}

action drop mirror
{
	has itmMirror
	drop itmMirror
	"You %verb% the %noun%"
	exit true
}

action get stones
{
	IsItemHere itmStones
	"You dont want to carry a bunch of stones around."
	exit true
}

## ice does not have built in take
action get ice
{
	try
	{
		CanPlayerSee

		try
		{
			IsItemHere itmIce
			take itmIce
			"You %verb% the %noun%"

			SetCounter cntIceMelts, 9

			exit true
		}

		continue true
	}

	"It is too dark to see."
	exit true
}

action drop ice
{
	has itmIce
	drop itmIce
	"You %verb% the %noun%"
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

		try
		{
			in rmIceCavern
			"I can see a steep slope."
			crlf
			exit true
		}

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

		"%verb% what? I do not see that here."
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


noun drawing
action look drawing
{
	in rmDampLimeStoneCave
	"It shows a man climbing down into a pit using a rope."
	exit true
}

action kick stalactites
{
	in rmDampLimeStoneCave
	"How can I reach them?"
	exit true
}

action climb any
{
	"%verb% what?"
	exit true
}

## NOTE: makes no sense?
action use ice
{
	in rmDampLimeStoneCave
	has itmIce
	here itmStalactites
	"A stalactite has fallen to the floor."
	exit true
}

## NOTE: Fix this somehow?
action use rope
{
	in rmDeepPit
	has itmRope

	SetFlagTrue flTiedRopeDownPit
	drop itmRope
	goto rmQuietCavern
	exit true
}

action use rope
{
	in rmQuietCavern
	IsFlagTrue flTiedRopeDownPit
	goto rmDeepPit
	exit true
}

action look up
{
	in rmQuietCavern
	IsFlagTrue flTiedRopeDownPit
	"I can see a rope hanging down the chimney."
	exit true
}

action cut rope
{
	in rmDenseForest
	IsFlagFalse flRope

	try
	{
		not has itmFlint
		"You will need something to cut it with."
		exit true
	}

	"You cut the rope with the flint and it falls to the ground."
	here itmRope
	SetFlagTrue flRope

	exit true
}

action wear boots
{
	has itmBoots
	swap itmBoots, itmBootsWorn
	"You put on the boots."
	exit true
}

verb jump, vault
noun ravine
action jump ravine
{
	in rmBesideLake
	"Your best jump falls short, and you plunge to your death breaking your neck at the bottom."
	losegame
	exit true
}

action jump ravine
{
	in rmOldShed
	"Your best jump falls short, and you plunge to your death breaking your neck at the bottom."
	losegame
	exit true
}

action jump any
{
	in rmDeepPit
	"Not likely. Its too deep!"
	exit true
}


verb use
action use board
{
	in rmBesideLake
	has itmBoard

	"You use the floor board to cross the ravine, picking it back up once you make it to the other side."

	goto rmOldShed
	exit true
}

action use board
{
	in rmOldShed
	has itmBoard

	"You use the floor board to cross the ravine, once you make it to the other side IT BREAKS!"

	swap itmBoard, itmBrokenBoard
	goto rmBesideLake
	drop itmBrokenBoard
	exit true
}


## NOTE: eh???
noun block
action use block
{
	in rmIceCavern
	has itmIce
	"You use the block of ice to slide down the slope!"

	SetCounter cntIceMelts, 0
	goto rmQuietCavern

	destroy itmIce
	here itmPool

	crlf
	"The ice melted! into a pool of water."

	exit true
}

noun ice
action use ice
{
	in rmIceCavern
	has itmIce
	"You use the block of ice to slide down the slope!"

	SetCounter cntIceMelts, 0
	goto rmQuietCavern

	destroy itmIce
	here itmPool

	crlf
	"The ice melted! into a pool of water."

	exit true
}


action get man
{

	try
	{
		CanPlayerSee

		try
		{
			IsItemHere itmGreenManOnMirror

			swap itmGreenManOnMirror, itmMirror
			take itmGreenMan

			"You gently pick up the small green man."

			try
			{
				not has itmGlovesWorn
				"Eeeeew he is all slimy!"

				SetCounter cntGreenManDeathMoves, 10
				SetFlagTrue flTouchedSlimyGreenMan

				exit true
			}
			exit true
		}
		exit false
	}

	"Its too dark to see."
	exit true
}

verb kill
action kill man
{
	has itmLaserGun

	try
	{
		IsItemHere itmGreenManOnMirror
		"He vanishes in a puff of smoke and you break the mirror in the process."
		destroy itmGreenManOnMirror
		here itmBrokenGlass
		exit true
	}

	try
	{
		IsItemHere itmGreenMan
		"He vanishes in a puff of smoke."
		destroy itmGreenMan
		exit true
	}

	"You cant quite do that yet."
	exit true
}

verb type
noun help

action type help
{
	in rmComputer
	"It says 2 west, 2 south for fun."
	exit true
}

verb smash
action smash forcefield
{
	in rmPassage
	has itmLaserGun

	try
	{
		CounterLT cntForceField, 2
		"You have weakened the forcefield."

		AddCounter cntForceField, 1
		exit true
	}

	try
	{
		CounterEQ cntForceField, 2
		"It has no affect."
		exit true
	}
}

verb shoot
action shoot forcefield
{
	in rmPassage
	has itmLaserGun

	try
	{
		CounterLT cntForceField, 2
		"You have weakened the forcefield."

		AddCounter cntForceField, 1
		exit true
	}

	try
	{
		CounterEQ cntForceField, 2
		"It has no affect."
		exit true
	}
}


verb dance
action dance any
{
	in rmPassage
	has itmMirror

	CounterEQ cntForceField, 2

	"You shimmy and shake your way through as the mirror deflecting the weakened force field"

	# moves until guard wakes up?
	SetCounter cntGuardWakesUp, 9

	goto rmHanger
	exit true
}

action dance any
{
	in rmPassage

	"The force field knocks you out."

	goto rmPrison
	exit true
}


verb unlock
noun door
action unlock door
{
	in rmHanger
	has itmKey
	"You unlock the door and leave."
	goto rmLiftControl
	exit true
}

action kill guard
{
	IsItemHere itmGuard
	has itmLaserGun
	"Pzzzzzzzt. The guard distintergates into nothing."
	destroy itmGuard
	exit true
}

noun up
action look up
{
	in rmPrison
	"The bars look loose."
	exit true
}

action look window
{
	in rmPrison
	"The bars look loose."
	exit true
}

noun bars
action kick bars
{
	in rmPrison
	IsItemInRoom itmHoleInWall, void
	swap itmBaredWindow, itmHoleInWall
	drop itmMetalBar
	"A mighty kick knocks one of the bars loose and it falls out leaving a hole."
	exit true
}

verb bribe
action bribe guard
{
	in rmPrison
	IsPresent itmGuard
	has itmGoldCoin

	"The guard seems to wake at the sight of the gold coin and snatches it from you."

	destroy itmGoldCoin

	crlf
	"You hurry out past him while you can."

	goto rmQuietCavern
	exit true
}

noun lake
action look lake
{
	in rmBesideLake

	"The lake looks very very cold!"

	try
	{
		IsItemInRoom itmGoldCoin, void

		crlf
		"I see a gold coin down there."
	}

	exit true
}

action get coin
{
	try
	{
		CanPlayerSee

		try
		{
			in rmBesideLake
			IsItemInRoom itmGoldCoin, void

			try
			{
				has itmBootsWorn
				"I walk into the freezing water with my boots on and grab the coin."
				take itmGoldCoin
				exit true
			}

			"I dont want to get your feet wet! That water is freezing."
			exit true
		}

		continue true
	}

	"It is too dark to see."
	exit true
}

action drop coin
{
	has itmGoldCoin
	drop itmGoldCoin
	"You %verb% the %noun%"
	exit true
}

action open door
{
	in rmWindTunnel
	goto rmComputer
	exit true
}

noun four, 4
noun three, 3
noun two, 2
noun one, 1
noun main
noun aux

verb push, press

action push main
{
	in rmSpaceShip
	IsPresent itmStarterMotor
	"It blew up and killed me."
	losegame
}

action push aux
{
	in rmSpaceShip
	IsPresent itmStarterMotor
	SetCounter cnt_12, 1

	"The space ship has flow into the large lift and is hovering there."
	"There are four buttons outside the window marked 1, 2, 3 and 4"

	exit true
}

action push four
{
	in rmSpaceShip
	CounterGT cnt_12, 0
	CounterEQ cnt_9, 3

	wingame
}

action push three
{
	in rmSpaceShip
	CounterGT cnt_12, 0
	"An alarm sounds. The security guard shoots me."
	losegame
}

action push two
{
	in rmSpaceShip
	CounterGT cnt_12, 0
	"An alarm sounds. The security guard shoots me."
	losegame
}

action push one
{
	in rmSpaceShip
	CounterGT cnt_12, 0
	CounterEQ cnt_9, 3

	"The lift has become electrified."

	try
	{
		has itmBootsWorn
		"Its a good job I was wearing my rubber boots."
		wingame
		exit true
	}

	"I have been electrocuted."
	losegame
}

action push three
{
	in rmLiftControl

	try
	{
		CounterEQ cnt_9, 0
		CounterEQ cnt_10, 0

		SetCounter cnt_9, 1
		"You push the button... It beeps."
		exit true
	}

	try
	{
		CounterEQ cnt_10, 0
		"You push the button... a fuse has just blown!"
		AddCounter cnt_10, 1
		SetCounter cnt_9, 0
		exit true
	}

	"Nothing happens."
	exit true
}

action push two
{
	in rmLiftControl

	try
	{
		CounterEQ cnt_9, 1
		CounterEQ cnt_10, 0

		SetCounter cnt_9, 2
		"You push the button... It beeps."
		exit true
	}

	try
	{
		CounterEQ cnt_10, 0
		"You push the button... a fuse has just blown!"
		AddCounter cnt_10, 1
		SetCounter cnt_9, 0
		exit true
	}

	"Nothing happens."
	exit true
}

action push one
{
	in rmLiftControl

	try
	{
		CounterEQ cnt_9, 2
		CounterEQ cnt_10, 0
		"The lift has been activated."

		SetCounter cnt_9, 3
		exit true
	}

	try
	{
		CounterEQ cnt_10, 0
		"You push the button... a fuse has just blown!"
		AddCounter cnt_10, 1
		SetCounter cnt_9, 0
		exit true
	}

	"Nothing happens."
	exit true
}


verb fix, repair, mend
noun fuse
action fix fuse
{
	in rmLiftControl
	CounterEQ cnt_10, 1

	try
	{
		not has itmMetalBar
		"You have nothing to fix the fuse with."
		exit true
	}

	SetCounter cnt_10, 0
	"You replace the broken fuse with the metal bar (its a big fuse!)"
	destroy itmMetalBar

	exit true
}

verb help
action help any
{
	in rmLiftControl
	"Keep off the middle men. One may be shocking."
	exit true
}

action help any
{
	in rmPassage
	"Vanity Waltz?"
	exit true
}

action help any
{
	in rmComputer
	"Try help."
	exit true
}

action help any
{
	in rmMaze7
	"Points of compass..."
	exit true
}

action help any
{
	in rmMaze8
	"Points of compass..."
	exit true
}

action help any
{
	in rmMaze9
	"Points of compass..."
	exit true
}

action help any
{
	in rmMaze10
	"Points of compass..."
	exit true
}

action help any
{
	in rmSpaceShip
	CounterGT cnt_12, 0
	"Keep off the middle men. One may be shocking."
	exit true
}

action help any
{
	"Try looking around."
	exit true
}


room rmDampLimeStoneCave "Damp Limestone Cave"
{
	look
	{
		"I am in a damp limestone cave. There are some stalactites. A passage leads north and I can also go west."
	}

	on n
	{
		goto rmMaze7
		exit true
	}

	on w
	{
		goto rmMountainPlateau
		exit true
	}

}

room rmDeepPit "Deep Pit"
{
	look
	{
		"I am at the edge of a deep pit. Obvious exits are east."
	}

	on d
	{
		"How?"
		exit true
	}

	on e
	{
		goto rmMountainPlateau
		exit true
	}
}

room rmMountainPlateau "Mountain Plateau"
{
	look
	{
		"I am on a mountain plateau, to the north is a cliff. There are exits down, east and west."
	}

	on d
	{
		goto rmDenseForest
		exit true
	}

	on e
	{
		goto rmDampLimeStoneCave
		exit true
	}

	on w
	{
		goto rmDeepPit
		exit true
	}
}

room rmDenseForest "Dense Forest"
{
	look
	{
		"I am in a dense forest. "
		try
		{
			IsFlagFalse flRope
			"There is a rope tied up in one of the trees. "
		}

		"There are exits to the south and west."
	}

	on s
	{
		goto rmBesideLake
		exit true
	}

	on w
	{
		goto rmMountainPlateau
		exit true
	}
}

room rmBesideLake "Beside a Lake"
{
	look
	{
		"I am beside a calm lake. There are exits to the east and north, to the west is a ravine."
	}

	on n
	{
		goto rmDenseForest
		exit true
	}

	on e
	{
		goto rmStrangeHouse
		exit true
	}
}

room rmStrangeHouse "Strange House"
{
	look
	{
		"I am in a strange house. There is a door to the north."
	}

	on n
	{
		goto rmBesideLake
		exit true
	}
}

room rmOldShed "Old Shed"
{
	look
	{
		"I am inside an old shed. To the east is a ravine."
	}
}

room rmMaze7 "Maze"
{
	look
	{
		"I am in a maze."
	}

	on d
	{
		goto rmMaze7
		exit true
	}

	on s
	{
		goto rmMaze7
		exit true
	}

	on e
	{
		goto rmMaze7
		exit true
	}

	on w
	{
		goto rmMaze7
		exit true
	}

	on n
	{
		goto rmMaze8
		exit true
	}
}

room rmMaze8 "Maze"
{
	look
	{
		"I am in a maze."
	}

	on d
	{
		goto rmMaze7
		exit true
	}

	on n
	{
		goto rmMaze7
		exit true
	}

	on e
	{
		goto rmMaze7
		exit true
	}

	on w
	{
		goto rmMaze7
		exit true
	}

	on s
	{
		goto rmMaze9
		exit true
	}
}

room rmMaze9 "Maze"
{
	look
	{
		"I am in a maze."
	}

	on d
	{
		goto rmMaze7
		exit true
	}

	on n
	{
		goto rmMaze7
		exit true
	}

	on s
	{
		goto rmMaze7
		exit true
	}

	on w
	{
		goto rmMaze7
		exit true
	}

	on e
	{
		goto rmMaze10
		exit true
	}
}

room rmMaze10 "Maze"
{
	look
	{
		"I am in a maze."
	}

	on d
	{
		goto rmMaze7
		exit true
	}

	on n
	{
		goto rmDampLimeStoneCave
		exit true
	}

	on s
	{
		goto rmMaze7
		exit true
	}

	on e
	{
		goto rmMaze7
		exit true
	}

	on w
	{
		goto rmIceCavern
		exit true
	}

}

room rmIceCavern "Ice Cavern"
{
	look
	{
		"I am in an ice cavern. There is an exit to the east, and what looks like a deep pit to the south."
	}

	on e
	{
		goto rmMaze7
		exit true
	}
}

room rmQuietCavern "Quiet Cavern"
{
	enter
	{
		try
		{
			CounterEQ cntGreenManWakesUp, 0
			SetCounter cntGreenManWakesUp, 10
		}
	}

	look
	{
		"I am in a quiet cavern. There are exits east, west and south."
	}

	on s
	{
		goto rmPassage
		exit true
	}

	on e
	{
		goto rmWindTunnel
		exit true
	}

	on w
	{
		goto rmPrison
		exit true
	}
}


room rmComputer "Computer Room"
{
	look
	{
		"I am in a room with a computer, it has a keyboard attached to it. There is an exit west."
	}

	on w
	{
		goto rmWindTunnel
		exit true
	}
}

room rmPassage "Passage"
{
	look
	{
		"I am in a passage. A forcefield is to the south. There are exits north, east and west."
	}

	on n
	{
		goto rmQuietCavern
		SetCounter cntGuardWakesUp, 7
		exit true
	}

	on e
	{
		goto rmPrison
		exit true
	}

	on w
	{
		goto rmPrison
		exit true
	}
}

room rmPrison "Prison Cell"
{
	look
	{
		"I am in a prison cell."
	}

	on u
	{
		try
		{
			IsPresent itmHoleInWall
			"You shimy up between the bars and out."
			goto rmQuietCavern
			exit true
		}

		"The bars block my exit."

		## NOTE: The original put the bars back after you escape.... ODD! infinite metal bars then....
		exit true
	}

}

room rmHanger "Large Hanger"
{
	look
	{
		"I am in a large hanger. A locked door is to the west. There are exits north, south and east."
	}

	on n
	{
		goto rmPassage
		exit true
	}

	on s
	{
		goto rmPrison
		exit true
	}

	on e
	{
		goto rmTallLift
		exit true
	}
}

room rmLiftControl "Lift Control"
{
	look
	{
		"I am in the lift control room. There are three switches. A sign reads 5, 4, dusty bin. There is an exit east."
	}

	on e
	{
		goto rmHanger
		exit true
	}

}

room rmTallLift "Tall Lift"
{
	look
	{
		"I am in a tall lift. The buttons are very high. There is an exit to the West."
	}

	on w
	{
		goto rmHanger
		exit true
	}
}

room rmWindTunnel "Wind Tunnel"
{
	look
	{
		"I am in a wind tunnel. A close door is at the end. There is an exit to the west."
	}

	on w
	{
		goto rmQuietCavern
		exit true
	}
}

room rmSpaceShip "Space Ship"
{
	look
	{
		"I am in a space ship. I can see no way out. There is a small window in the side and some buttons, one marked MAIN and the other AUX"
	}
}

