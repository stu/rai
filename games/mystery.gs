game "Mysterious Island"
author "Mountain Valley Software"

release					"r1"
ver						"%version%/%release%"

# require interpreter version to be of v1 standard
requires_interpreter		1
max_carry				6

##light_time 15

# setup our basic counters

counter tr_Zero
counter tr_Moves
counter tr_Pistol
counter tr_SprayCan

counter tr_Safe
counter tr_Safe1
counter tr_Safe2
counter tr_Safe3

counter c_prompt_q

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
verb light, ignite
verb unlight, extinguish
verb open
verb sleep, laydown
verb feed

verb call
verb dig
verb kill
verb say
verb turn, rotate, twist
verb whistle, blow


verb one, 1
verb two, 2
verb three, 3
verb four, 4
verb five, 5
verb six, 6
verb seven, 7
verb eight, 8
verb nine, 9
verb zero, nought, 0


flag fl_Safe				# rusted dial

flag fl_Safe1
flag fl_Safe2
flag fl_Safe3
flag fl_SafeCrack

pregame
{
	"Welcome to %game% by %author%"
	crlf
	"Expanded RAI Port version %ver% by Stu George."
	crlf
	crlf
	"Based on the C64 version created by %author% circa somewhere in the mid 80's."
	crlf
	crlf

	# 3 bullets
	setcounter tr_Pistol, 3

	# spray can
	Setcounter tr_SprayCan, 3

	"The goal is to find and store all ten treasures"
	"hidden on the island by the nasty long dead pirate Barnabous Redbeard!"

	SetLightOn
	goto rm_Shore
	exit true
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
	"I do not understand."
}

prompt
{
	try
	{
		in rm_Waterfall

		IsPresent itm_SilverDish
		IsPresent itm_IvoryBowl
		IsPresent itm_Emerald
		IsPresent itm_Ruby
		IsPresent itm_Pearl
		IsPresent itm_Diamond
		IsPresent itm_Lamp
		IsPresent itm_Necklace
		IsPresent itm_Sapphire
		IsPresent itm_GoldBar

		#success
		crlf
		crlf

		"*** CONGRATULATIONS ***"
		crlf
		crlf
		"I've been here in %moves% moves."
		crlf

		"And stored all the treasures."
		crlf
		crlf
		"I've solved it all!"

		crlf
		crlf

		wingame
	}

	try
	{
		IsFlagTrue fl_SafeCrack

		try
		{
			CounterEQ c_prompt_q, 4
			SetCounter c_prompt_q, 0

			try
			{
				try
				{
					IsFlagFalse fl_Safe
					"I try and turn the dial but it is rusted."
					crlf
					"> "
					exit true
				}

				try
				{
					IsFlagTrue fl_Safe

					try
					{
						# test safe opening
						CounterEq tr_safe1, 4
						CounterEq tr_safe2, 5
						CounterEq tr_safe3, 9
						"The safe opens with a click!"
						swap itm_safe, itm_SafeOpen
						"> "
						exit true
					}

					"I try the combination but it does not work."
					SetFlagFalse fl_Safe
					SetCounter tr_safe1, 0
					SetCounter tr_safe2, 0
					SetCounter tr_safe3, 0
					"> "
					exit true
				}
			}

			try
			{
				IsItemHere itm_safe
				"You spin the dial... and nothing"
				crlf
				"> "
				exit true
			}
		}
		try
		{
			CounterEQ c_prompt_q, 3
			"Enter Third Number > "
			SetCounter c_prompt_q, 4
			exit true
		}
		try
		{
			CounterEQ c_prompt_q, 2
			"Enter Second Number > "
			SetCounter c_prompt_q, 3
			exit true
		}
		try
		{
			CounterEQ c_prompt_q, 1
			"Enter First Number > "
			SetCounter c_prompt_q, 2
			exit true
		}
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

in_dark
{
	"It is too dark to see."
}

room rm_Shore "Shore"
{
	look
	{
		"I am on the shore. There are exits to the south, west and north."
	}

	on s
	{
		try
		{
			IsItemHere itm_Crab
			"The crab does not let me pass."
			exit true
		}

		goto rm_Peninsula
		exit true
	}

	on w
	{
		goto rm_Rocks
		exit true
	}

	on n
	{
		goto rm_PlainPath
		exit true
	}
}

room rm_Peninsula "Peninsula"
{
	look
	{
		"I am on a sandy beach, the shoreline extends to the north."
	}

	on n
	{
		goto rm_Shore
		exit true
	}
}

room rm_Rocks "Rocks"
{
	look
	{
		"I am by a steep cliff, the shoreline is to the east."
	}

	on e
	{
		goto rm_Shore
		exit true
	}
}

room rm_Shipwreck "Shipwreck"
{
	look
	{
		"I am on the deck of an old dilapidated ship. I can get back to the rocks by going north."
		exit true
	}

	on n
	{
		goto rm_Rocks
		exit true
	}
}

room rm_Cabin "Cabin"
{
	look
	{
		"I am in the captains cabin. What was left of the door is to the east."
	}

	on e
	{
		goto rm_Shipwreck
		exit true
	}
}

room rm_Bed "Bed"
{
	look
	{
		"I am in a cozy and warm bed."
	}

	on u
	{
		goto rm_Cabin
		exit true
	}

	on d
	{
		goto rm_Cabin
		exit true
	}

	on s
	{
		goto rm_Cabin
		exit true
	}
}

room rm_PlainPath "Plain Path"
{
	look
	{
		"I am on a path leading inland toward the center of the island. The path continues to the north and south. A faint path veers off to the west."
	}

	on n
	{
		goto rm_FootOfCliff
		exit true
	}

	on w
	{
		goto rm_NextToHut
		exit true
	}

	on s
	{
		goto rm_Shore
		exit true
	}
}

room rm_NextToHut "Outside Hut"
{
	look
	{
		"I am on the banks of a river next to a small hut."
	}

	on e
	{
		goto rm_PlainPath
		exit true
	}
}

room rm_FootOfCliff "Foot Of Cliff"
{
	look
	{
		try
		{
			"Besides the path to the south, I can see a tunnel in the face of the cliff."
			IsItemInRoom itm_Tunnel, rm_FootOfCliff
			exit true
		}

		"The path stops at the foot of a sheer wall of a cliff."
		"The only exit is to the south."
	}

	on s
	{
		goto rm_PlainPath
		exit true
	}
}

room rm_Plateau "Plateau"
{
	look
	{
		"I am on a plateau. From up here you can see a lighthouse, waterfall, river and the tunnel you came out of."
	}
}

room rm_Waterfall "Waterfall"
{
	look
	{
		"I am in a secret cave behind the waterfall."
	}

	on w
	{
		goto rm_Plateau
		exit true
	}
}

room rm_Lighthouse "Lighthouse"
{
	look
	{
		"I am in the lighthouse."
	}

	on e
	{
		goto rm_Plateau
		exit true
	}
}


room rm_Hut "Hut"
{
	look
	{
		"I am inside a wooden hut, the door to the east leads back outside."
	}

	on e
	{
		goto rm_NextToHut
		exit true
	}
}

room rm_River1 "River"
{
	look
	{
		"I am in a shallow river. There is a high plateau to the south."
	}

	on s
	{
		goto rm_Plateau
		exit true
	}
}

room rm_River2 "River"
{
	look
	{
		"I am on raft on a turbulent river."
	}
}

room rm_Jungle "Jungle"
{
	look
	{
		"I am in a jungle clearing."
	}
}

room rm_Celler "Cellar"
{
	look
	{
		"I am in a small damp cellar, the only exit is up."
	}

	on u
	{
		goto rm_Lighthouse
		exit true
	}
}

room rm_TopLighthouse "Top of the Lighthouse"
{
	look
	{
		"I am at the top of the lighthouse, down is the only way to go."
	}

	on d
	{
		goto rm_Lighthouse
		exit true
	}
}

# death room........
room rm_RedRoom "Death!"
{
	look
	{
		"Death!"
	}
}


item itm_Planks [planks, wood, plank] "Planks", "Some wooden planks" in void has take
item itm_Tiger [tiger] "Hungry Tiger", "A very hungry looking tiger!" in rm_Hut
item itm_Cupboard [cupboard] "Cupboard", "It is a cupboard and it is closed." in rm_Hut
item itm_CupboardOpen [cupboard] "Cupboard", "Your average cupboard and it is open" in void
item itm_Waterfall [waterfall] "Waterfall", "A very cold looking waterfall" in rm_Plateau
item itm_Lighthouse [lighthouse] "Lighthouse", "The old and weatherworn lighthouse could use a new coat of paint" in rm_Plateau
item itm_Pistol[pistol, gun] "Pistol", "A small weathered pistol" in rm_Waterfall has take
item itm_IronRing [ring] "Iron Ring", "A rusted iron ring" in rm_Lighthouse
item itm_IronRing2 [ring] "Iron Ring", "A greasy iron ring" in void
item itm_TrapDoor [trapdoor, trap] "Trapdoor", "A trapdoor in the floor" in void
item itm_Chest [chest] "Chest", "The chest is locked" in rm_Celler
item itm_ChestUnlocked [chest] "Chest", "The chest is unlocked" in void
item itm_Stairs [stairs] "Stairs", "Some rotten wooden stairs" in rm_Lighthouse has scenery
item itm_StairsFixed [stairs] "Stairs", "Some fixed wooden stairs" in void has scenery
item itm_CrabDead [crab] "Dead Crab", "A dead giant crab" in void
item itm_Crab [crab] "Giant Crab", "An angry looking giant crab" in rm_Shore
item itm_CrabMeat [meat, crabmeat] "Crab Meat", "You see some juicy white crab meat" in void has take
item itm_CrabShell [crab, shell] "Crab Shell", "You see nothing but crab shell pieces" in void has take
item itm_RottenTree [tree] "Tree", "Its old and rotten.\nThere is writing on the trunk." in rm_Shore has scenery
item itm_Ocean [ocean, sea] "Ocean", "The open ocean extends to the horizon" in rm_Peninsula has scenery
item itm_Sand [sand] "Sand", "Plain old sand." in rm_Peninsula
item itm_Boulder1 [boulder] "Boulder", "You see a very large boulder " in rm_Peninsula
item itm_Boulder [boulder] "Boulder", "You see a very large boulder" in rm_FootOfCliff
item itm_Raft [raft] "Raft", "You see a deflated rubber raft" in void has take
item itm_RaftInflated [raft] "Raft", "You see a inflated rubber raft" in void has take
item itm_Rocks [rocks] "Rocks", "Some plain old granite rocks, a few have some straffing marks" in rm_Rocks
item itm_Ship [ship] "Wrecked Ship", "A wrecked ship" in rm_Rocks
item itm_Cabin [cabin] "Cabin", "A dilapidated cabin on the deck" in rm_Shipwreck
item itm_ClosedCabinDoor [door] "Closed Door", "The cabin door is closed" in rm_Shipwreck
item itm_CutlassBlunt [cutlass, sword] "Cutlass (blunt)", "The cutlass is blunt" in rm_Shipwreck has take
item itm_CutlassSharp [cutlass, sword] "Cutlass (sharp)", "The cutlass is nice and sharp" in void has take
item itm_Rope [rope] "Rope", "You see some rope" in rm_Shipwreck has take
item itm_Bed [bed] "Bed", "You see a comfy looking bed" in rm_Cabin
item itm_Table [table] "Table", "You see a table" in rm_Cabin
item itm_SilverDish [dish] "*Silver Dish*", "You see an expensive looking dish" in void has take
item itm_Key [key] "Key", "It is a plain key" in void has take
item itm_IvoryBowl [bowl] "*Ivory Bowl*", "You see a small ivory bowl" in void has take
item itm_Mattress [mattress] "Mattress", "You see a lumpy mattress" in rm_Bed
item itm_MattressCut [mattress] "Mattress", "You see a cut up mattress" in void
item itm_Emerald [emerald] "*Emerald*", "It is a beautiful sea green emerald" in void has take
item itm_Paper [paper] "Paper", "You see some old paper, it has writing on it" in void has take
item itm_Sign [sign] "Sign", "You see a rotten wooden sign" in rm_PlainPath
item itm_Sign2 [sign] "Sign", "You see a rotten wooden sign" in rm_Waterfall
item itm_Shovel[shovel] "Shovel", "You see a rust pitted shovel" in rm_River1 has take
item itm_Ruby [ruby] "*Ruby*", "You see a small deep red ruby" in rm_River1 has take
item itm_River [river] "River", "You see a river" in rm_NextToHut
item itm_River1 [river] "River", "You see a river" in rm_Plateau
item itm_WoodenHut [hut] "Wooden Hut", "You see a small wooden hut in the jungle" in rm_NextToHut
item itm_Tunnel [tunnel] "Tunnel", "It is a damp tunnel" in void
item itm_Nails [nails] "Nails", "You see some iron nails" in void has take
item itm_Pump [pump] "Pump", "It is a small hand pump, look like its good for inflating things" in void has take
item itm_Pearl [pearl] "*Pearl*", "You see a small white pearl" in void has take
item itm_Diamond [diamond] "*Diamond*", "You see a pinkish diamond" in void has take
item itm_Rapids [rapids] "Rapids", "Those are some raging rapids" in rm_River2
item itm_Clearing [clearing] "Clearing", "An open clearing" in rm_River2
item itm_Croc [crocodile] "Crocodile", "You see a large hungry looking crocodile" in rm_Jungle
item itm_Rock [rock] "Rock", "You see a large rock" in rm_Jungle
item itm_Lamp [lamp] "*Gold Lamp*", "It is a beautiful golden lamp" in void has take
item itm_Spraycan [spraycan, can, spray] "Spray Can", "A can of penetrating oil spray" in void has take
item itm_Hammer [hammer] "Hammer", "An old carpenters hammer" in void has take
item itm_Safe [safe] "Safe", "You see a safe that is currently locked" in rm_TopLighthouse
item itm_SafeOpen [safe] "Safe", "You see a safe, and it is open!" in void
item itm_Necklace [necklace] "*Necklace*", "You see a sterling silver necklace" in void has take
item itm_Sapphire [sapphire] "*Sapphire*", "You see a large brilliant sapphire" in void has take
item itm_GoldBar [gold, goldbar, bar, ingot] "*Gold Bar*", "You see a small rectangular shaped golden bar" in void has take

noun sesame
verb sharpen


###############################################################################
# rm_Shore, crab + tree
#

action move tree
{
	IsItemHere itm_RottenTree
	"The rotten tree comes down on top of me!"
	LoseGame
}

action climb tree
{
	IsItemHere itm_RottenTree
	"My weight is too much for the rotten tree and it snaps."
	LoseGame
}


#action look tree
#{
#	IsItemHere itm_RottenTree
#	"Its old and rotten."
#	crlf
#	"There is writing on the trunk."
#	exit true
#}

noun writing
action read writing
{
	IsItemHere itm_RottenTree
	"It says..."
	crlf
	"'2 - 1 = 3'"
	crlf
	exit true
}


action look crab
{
	IsItemHere itm_Crab
	"Its green and mean and very large and its waving its big claws about."
	exit true
}

action get crab
{
	IsItemHere itm_Crab
	"I grab the giant crab, which grabs me!"
	crlf
	"Its pincers crush the life out of me."
	LoseGame
}

action kill crab
{
	IsItemHere itm_Crab

	try
	{
		has itm_Pistol
		continue true
	}

	try
	{
		has itm_CutlassBlunt
		"I swing at the crab with the cutlass, but I am too close!"
		crlf
		"Its pincers crush the life out of me."
		LoseGame
	}

	try
	{
		has itm_CutlassSharp
		"I swing at the crab with the cutlass, but I am too close!"
		crlf
		"Its pincers crush the life out of me."
		LoseGame
	}
}

action kill crab
{
	IsItemHere itm_Crab
	has itm_Pistol

	try
	{
		CounterEquals tr_Pistol, 0
		"*CLICK*.. You are out of bullets."
		crlf
		"The crab crushes you between its pincers."
		LoseGame
	}

	try
	{
		CounterGT tr_Pistol, 0
		SubCounter tr_Pistol, 1
		"Bang!"
		crlf
		"I got him right between the eyes."
		swap itm_CrabDead, itm_Crab
		exit true
	}
}

action shoot crab
{
	IsItemHere itm_Crab
	has itm_Pistol

	try
	{
		Counterequals tr_Pistol, 0
		"*CLICK*.. You are out of bullets."
		crlf
		"The crab crushes you between its pincers."
		LoseGame
	}

	try
	{
		CounterGT tr_Pistol, 0
		SubCounter tr_Pistol, 1
		"Bang!"
		crlf
		"I got him right between the eyes."
		swap itm_CrabDead, itm_Crab
		exit true
	}
}

action get crab
{
	IsItemHere itm_CrabDead
	"The crab carcass is too big to move"
	exit true
}

action cut crab
{
	IsItemHere itm_CrabDead

	try
	{
		has itm_CutlassBlunt
		swap itm_CrabDead, itm_CrabShell
		"You hack and destroy the crab with the blunt sword"
		exit true
	}

	try
	{
		has itm_CutlassSharp
		swap itm_CrabDead, itm_CrabMeat
		"The sharp sword makes easy work of the crab"
		exit true
	}
}



###############################################################################
# beach
#

action move boulder
{
	IsItemHere itm_Boulder1

	try
	{
		IsItemInRoom itm_Raft, void
		drop itm_Raft
		"I found : a rubber raft."
		exit true
	}
}

action look sand
{
	in rm_Peninsula
	"There's writing in the sand."
	crlf
	"It says..."
	crlf
	"'4 = 1'"
	exit true
}

action look ocean
{
	IsItemHere itm_Ocean
	"I see sharks."
	exit true
}

action dig any
{
	has itm_Shovel

	try
	{
		in rm_Peninsula

		IsItemInRoom itm_Pearl, void
		drop itm_Pearl
		"I found : a *Pearl*"
		exit true
	}

	"I found nothing."
	exit true
}

action go ocean
{
	IsItemHere itm_Ocean
	"Sharks in the ocean tear me to pieces"
	LoseGame
}

###############################################################################
# By Cliff
#

action look ship
{
	IsItemHere itm_Ship
	"It's an old sailing ship."
	crlf
	"It's stranded high and dry."
	exit true
}

action move rocks
{
	in rm_Rocks

	try
	{
		IsItemInRoom itm_Planks, void
		drop itm_Planks
		"I found : some wooden planks."
		exit true
	}
}

action go ship
{
	in rm_Rocks
	"OK."
	goto rm_Shipwreck
	exit true
}


action look rocks
{
	IsItemHere itm_Rocks
	"They are plain old granite rocks, a few have some straffing marks."

	exit true
}

action sharpen cutlass
{
	try
	{
		has itm_CutlassSharp
		"Why? It is sharp enough."
		exit true
	}

	try
	{
		IsItemHere itm_Rocks
		has itm_CutlassBlunt
		swap itm_CutlassBlunt, itm_CutlassSharp
		"You sharpen the sword on the rocks"
		exit true
	}

	"I'm not sure you know what you are doing."

	exit true
}

action look boulder
{
	IsItemHere itm_Boulder
	"There is some writing scratched on it."
	exit true
}


###############################################################################
# on Ship
#

action break door
{
	in rm_Shipwreck

	try
	{
		IsItemHere itm_ClosedCabinDoor
		destroy itm_ClosedCabinDoor

		"I give the door a swift kick!"
		crlf
		"I hurt my foot, but the door is open."
		exit true
	}

	"The door is already open."
	exit true
}

action look door
{
	in rm_Shipwreck

	try
	{
		IsItemHere itm_ClosedCabinDoor
		"Its wooden.. and locked."
		exit true
	}

	"There is no door anymore."
	exit true
}

action go cabin
{
	in rm_Shipwreck
	"OK."
	goto rm_Cabin
	exit true
}

###############################################################################
# in cabin
#

action look table
{
	in rm_Cabin

	try
	{
		IsItemInRoom itm_SilverDish, void
		drop itm_SilverDish
		"I found : a *Silver Dish*"
		exit true
	}

	try
	{
		IsItemInRoom itm_Key, void
		drop itm_Key
		"I found : a Key."
		exit true
	}
}

action look bed
{
	in rm_Cabin

	try
	{
		IsItemHere itm_Mattress
		"It looks like a warm place to sleep."
		exit true
	}

	try
	{
		IsItemHere itm_MattressCut
		"It looks pretty tattered but still a warm place to sleep"
		exit true
	}
}

action move bed
{
	in rm_Cabin

	try
	{
		IsItemInRoom itm_IvoryBowl, void
		drop itm_IvoryBowl
		"I found : an *Ivory Bowl*"
		exit true
	}
}

action go bed
{
	in rm_Cabin
	"OK."
	goto rm_Bed
	exit true
}

###############################################################################
# in bed
#

action look mattress
{
	try
	{
		#IsItemInRoom itm_Paper, void
		IsItemHere itm_Mattress
		"It looks a little lumpy."
		exit true
	}

	IsItemHere itm_MattressCut
	"It has a cut in it.."
	exit true
}

action sleep any
{
	in rm_Bed
	"Goodnight!"
	crlf
	pause 5
	"Good Morning!"

	try
	{
		IsItemHere itm_Mattress

		crlf
		"During the night I had a dream. A voice said"
		crlf
		"'I am sleeping on magic...'"
	}

	exit true
}

action cut mattress
{
	in rm_Bed

	try
	{
		IsItemHere itm_MattressCut
		"I already have."
		exit true
	}

	try
	{
		IsItemHere itm_Mattress

		try
		{
			has itm_CutlassSharp
			swap itm_Mattress, itm_MattressCut
			drop itm_Paper
			"I found : some paper"
			exit true
		}

		try
		{
			has itm_CutlassBlunt
			"The cutlass is not sharp enough"
			exit true
		}
	}

	"I can't do that... Yet."
	exit true
}

action move mattress
{
	in rm_Bed

	try
	{
		IsItemInRoom itm_Emerald, void
		drop itm_Emerald
		"I found : *Emerald*"
		exit true
	}

	"I found nothing."
	exit true
}

action read paper
{
	IsPresent itm_Paper
	"It says..."
	crlf
	"The magic word is 'sesame'"
	exit true
}

action look paper
{
	IsPresent itm_Paper
	"There is writing on it."
	exit true
}


###############################################################################
# on path
#

action look sign
{
	IsItemHere itm_Sign
	"There is writing on it."
	exit true
}

action read sign
{
	IsItemHere itm_Sign
	"It says..."
	crlf
	"'Beware of the CAT.'"
	exit true
}

###############################################################################
# next to hut
#

action go hut
{
	in rm_NextToHut
	"OK."
	goto rm_Hut
	exit true
}

action look river
{
	in rm_NextToHut
	"It's very deep and turbulent."
	exit true
}

action dig any
{
	in rm_NextToHut
	has itm_Shovel

	try
	{
		IsItemInRoom itm_Diamond, void
		drop itm_Diamond
		"I found : a *Diamond*"
		exit true
	}

	"I found nothing."
	exit true
}

action go river
{
	in rm_NextToHut

	try
	{
		IsPresent itm_RaftInflated
		goto rm_River2
		exit true
	}

	try
	{
		IsPresent itm_Raft
		"The raft is not inflated!"
		crlf
	}

	"The river is swift and deep. I sink to the bottom and drown."
	LoseGame
}

###############################################################################
# foot of sheer cliff
#

noun cliff
action look cliff
{
	in rm_FootOfCliff
	"There is some writing on the cliff."
	exit true
}

action read writing
{
	in rm_FootOfCliff
	"It says...'Say the Magic Word'."
	exit true
}

action say sesame
{
	in rm_FootOfCliff
	IsItemInRoom itm_Tunnel, void
	"'SESAME'"
	crlf
	"Part of the cliff moves aside revealing a tunnel."
	move itm_Tunnel, rm_FootOfCliff
	exit true
}

action open sesame
{
	in rm_FootOfCliff
	IsItemInRoom itm_Tunnel, void
	"'SESAME'"
	crlf
	"Part of the cliff moves aside revealing a tunnel."
	move itm_Tunnel, rm_FootOfCliff
	exit true
}

###############################################################################
# in river near plateau
#

action go waterfall
{
	in rm_River1
	"OK."
	goto rm_Waterfall
	exit true
}

###############################################################################
# on plateau
#

action go waterfall
{
	in rm_Plateau
	"OK."
	goto rm_Waterfall
	exit true
}

action go river
{
	in rm_Plateau
	"OK."
	goto rm_River1
	exit true
}

action look river
{
	in rm_Plateau
	"Its very shallow."
	exit true
}

action go lighthouse
{
	in rm_Plateau
	"OK."
	goto rm_Lighthouse
	exit true
}

action go tunnel
{
	IsPresent itm_Tunnel

	try
	{
		in rm_FootOfCliff
		goto rm_Plateau
		move itm_Tunnel, rm_Plateau
		exit true
	}

	in rm_Plateau
	goto rm_FootOfCliff
	move itm_Tunnel, rm_FootOfCliff
	exit true
}

###############################################################################
# in waterfall
#

action read sign
{
	IsPresent itm_Sign2
	"It says..."
	crlf
	"'Drop Treasures Here' and say score."
	exit true
}

###############################################################################
# in lighthouse
#

action look stairs
{
	try
	{
		IsPresent itm_StairsFixed
		"The stairs are repaired and look safe."
		exit true
	}

	try
	{
		IsPresent itm_Stairs
		"They are made of wooden planks which are completely rotten."
		exit true
	}
}


action turn ring
{
	in rm_Lighthouse

	try
	{
		IsPresent itm_IronRing
		"It's rusted solid."
		exit true
	}

	IsPresent itm_IronRing2

	try
	{
		IsItemInRoom itm_TrapDoor, void
		"A trapdoor opens in the floor...."
		move itm_TrapDoor, rm_Lighthouse
		exit true
	}

	"I already have."
	exit true
}

action go trapdoor
{
	IsPresent itm_TrapDoor
	"OK."
	goto rm_Celler
	exit true
}

action fix stairs
{
	in rm_Lighthouse

	try
	{
		IsPresent itm_StairsFixed
		"They are already fixed."
		exit true
	}

	try
	{
		has itm_Hammer
		has itm_Nails
		has itm_Planks

		destroy itm_Planks
		destroy itm_Nails

		swap itm_Stairs, itm_StairsFixed

		"The stairs are repaired."
		exit true
	}

	"I can't do that... Yet."
	exit true
}

action go stairs
{
	in rm_Lighthouse

	try
	{
		IsPresent itm_StairsFixed
		"OK."
		goto rm_TopLighthouse
		exit true
	}

	"The planks in the stairs are rotten. I fall through and break my neck!"
	LoseGame
}

action spray ring
{
	has itm_Spraycan
	in rm_Lighthouse

	try
	{
		CounterEQ tr_SprayCan, 0
		"The spray can is empty"
		exit true
	}


	try
	{
		CounterEQ tr_SprayCan, 3
		"SSSSSSSSSS."
	}

	try
	{
		CounterEQ tr_SprayCan, 2
		"SSSSS."
	}

	try
	{
		CounterEQ tr_SprayCan, 1
		"SS."
	}

	SubCounter tr_SprayCan, 1

	try
	{
		IsPresent itm_IronRing
		swap itm_IronRing, itm_IronRing2
	}

	"The ring is soaked."
	exit true
}

###############################################################################
# celler
#

action get chest
{
	IsPresent itm_Chest
	"Its too big to move."
	exit true
}

action get chest
{
	IsPresent itm_ChestUnlocked
	"Its too big to move."
	exit true
}

action unlock chest
{
	IsPresent itm_ChestUnlocked
	"It is already unlocked."
	exit true
}

action unlock chest
{
	IsPresent itm_Chest

	try
	{
		has itm_Key
		swap itm_Chest, itm_ChestUnlocked

		"The key turns with force... and unlocks the chest"
		exit true
	}

	"I can't do that... Yet."
	exit true
}

action look chest
{
	in rm_Celler
	"Its made of solid oak."
	crlf
	try
	{
		IsPresent itm_Chest
		"Its locked"
		exit true
	}

	try
	{
		IsItemInRoom itm_Necklace, void
		drop itm_Necklace
		"I found : a *Necklace*"
		exit true
	}

	"I found nothing."
	exit true
}

action move chest
{
	in rm_Celler

	try
	{
		IsItemInRoom itm_Hammer, void
		drop itm_Hammer
		"I found : a Hammer"
		exit true
	}

	"I found nothing."
	exit true
}

###############################################################################
# top of lighthouse
#

action get safe
{
	IsPresent itm_Safe
	"Its too heavy to lift"
	exit true
}

action get safe
{
	IsPresent itm_SafeOpen
	"Its too heavy to lift"
	exit true
}

action look safe
{
	IsPresent itm_Safe
	"It's got a combination dial."
	exit true
}

action look safe
{
	IsPresent itm_SafeOpen

	try
	{
		IsItemInRoom itm_GoldBar, void
		drop itm_GoldBar
		"I found : *Gold Bar*"
		exit true
	}

	"I found nothing."
	exit true
}

action move safe
{
	in rm_TopLighthouse

	try
	{
		IsItemInRoom itm_Sapphire, void
		drop itm_Sapphire
		"I found : a *Sapphire*"
		exit true
	}

	"I found nothing."
	exit true
}

action spray safe
{
	has itm_Spraycan
	in rm_TopLighthouse

	try
	{
		CounterEQ tr_SprayCan, 0
		"The spray can is empty"
		exit true
	}

	try
	{
		CounterEQ tr_SprayCan, 3
		"SSSSSSSSSS."
	}

	try
	{
		CounterEQ tr_SprayCan, 2
		"SSSSS."
	}

	try
	{
		CounterEQ tr_SprayCan, 1
		"SS."
	}

	SubCounter tr_SprayCan, 1

	setflagtrue fl_Safe
	"The dial is soaked."
	exit true
}

action open safe
{
	try
	{
		IsPresent itm_SafeOpen
		"It is already open."
		exit true
	}

	SetFlagTrue fl_SafeCrack
	SetCounter c_prompt_q, 1
	exit true
}

action one any
{
	try
	{
		IsFlagTrue fl_SafeCrack
		try
		{
			CounterEQ c_prompt_q, 1
			SetCounter tr_safe1, 1
			exit true
		}
		try
		{
			CounterEQ c_prompt_q, 2
			SetCounter tr_safe2, 1
			exit true
		}
		try
		{
			CounterEQ c_prompt_q, 3
			SetCounter tr_safe3, 1
			exit true
		}
	}
}

action two any
{
	try
	{
		IsFlagTrue fl_SafeCrack
		try
		{
			CounterEQ c_prompt_q, 1
			SetCounter tr_safe1, 2
			exit true
		}
		try
		{
			CounterEQ c_prompt_q, 2
			SetCounter tr_safe2, 2
			exit true
		}
		try
		{
			CounterEQ c_prompt_q, 3
			SetCounter tr_safe3, 2
			exit true
		}
	}
}

action three any
{
	try
	{
		IsFlagTrue fl_SafeCrack
		try
		{
			CounterEQ c_prompt_q, 1
			SetCounter tr_safe1, 3
			exit true
		}
		try
		{
			CounterEQ c_prompt_q, 2
			SetCounter tr_safe2, 3
			exit true
		}
		try
		{
			CounterEQ c_prompt_q, 3
			SetCounter tr_safe3, 3
			exit true
		}
	}
}

action four any
{
	try
	{
		IsFlagTrue fl_SafeCrack
		try
		{
			CounterEQ c_prompt_q, 1
			SetCounter tr_safe1, 4
			exit true
		}
		try
		{
			CounterEQ c_prompt_q, 2
			SetCounter tr_safe2, 4
			exit true
		}
		try
		{
			CounterEQ c_prompt_q, 3
			SetCounter tr_safe3, 4
			exit true
		}
	}
}

action five any
{
	try
	{
		IsFlagTrue fl_SafeCrack
		try
		{
			CounterEQ c_prompt_q, 1
			SetCounter tr_safe1, 5
			exit true
		}
		try
		{
			CounterEQ c_prompt_q, 2
			SetCounter tr_safe2, 5
			exit true
		}
		try
		{
			CounterEQ c_prompt_q, 3
			SetCounter tr_safe3, 5
			exit true
		}
	}
}

action six any
{
	try
	{
		IsFlagTrue fl_SafeCrack
		try
		{
			CounterEQ c_prompt_q, 1
			SetCounter tr_safe1, 6
			exit true
		}
		try
		{
			CounterEQ c_prompt_q, 2
			SetCounter tr_safe2, 6
			exit true
		}
		try
		{
			CounterEQ c_prompt_q, 3
			SetCounter tr_safe3, 6
			exit true
		}
	}
}

action seven any
{
	try
	{
		IsFlagTrue fl_SafeCrack
		try
		{
			CounterEQ c_prompt_q, 1
			SetCounter tr_safe1, 7
			exit true
		}
		try
		{
			CounterEQ c_prompt_q, 2
			SetCounter tr_safe2, 7
			exit true
		}
		try
		{
			CounterEQ c_prompt_q, 3
			SetCounter tr_safe3, 7
			exit true
		}
	}
}

action eight any
{
	try
	{
		IsFlagTrue fl_SafeCrack
		try
		{
			CounterEQ c_prompt_q, 1
			SetCounter tr_safe1, 8
			exit true
		}
		try
		{
			CounterEQ c_prompt_q, 2
			SetCounter tr_safe2, 8
			exit true
		}
		try
		{
			CounterEQ c_prompt_q, 3
			SetCounter tr_safe3, 8
			exit true
		}
	}
}

action nine any
{
	try
	{
		IsFlagTrue fl_SafeCrack
		try
		{
			CounterEQ c_prompt_q, 1
			SetCounter tr_safe1, 9
			exit true
		}
		try
		{
			CounterEQ c_prompt_q, 2
			SetCounter tr_safe2, 9
			exit true
		}
		try
		{
			CounterEQ c_prompt_q, 3
			SetCounter tr_safe3, 9
			exit true
		}
	}
}

action zero any
{
	try
	{
		IsFlagTrue fl_SafeCrack
		try
		{
			CounterEQ c_prompt_q, 1
			SetCounter tr_safe1, 0
			exit true
		}
		try
		{
			CounterEQ c_prompt_q, 2
			SetCounter tr_safe2, 0
			exit true
		}
		try
		{
			CounterEQ c_prompt_q, 3
			SetCounter tr_safe3, 0
			exit true
		}
	}
}

###############################################################################
# in hut
#

action get tiger
{
	IsPresent itm_Tiger
	"The tiger tears you to shreds!"
	LoseGame
}

action open cupboard
{
	IsPresent itm_Tiger
	IsPresent itm_Cupboard
	"It might be too dangerouse to get close to the cupboard yet"
	exit true
}

action look cupboard
{
	IsPresent itm_Cupboard
	"Its locked."
	exit true
}

action look tiger
{
	IsPresent itm_Tiger
	"Cute little fellow, those gleaming teeth and all"
	exit true
}

action drop meat
{
	IsPresent itm_Tiger
	has itm_CrabMeat

	destroy itm_CrabMeat
	destroy itm_Tiger

	"The tiger grabs the crab meat and bounds out of the hut."
	exit true
}

action feed tiger
{
	IsPresent itm_Tiger
	has itm_CrabMeat

	destroy itm_CrabMeat
	destroy itm_Tiger

	"The tiger grabs the crab meat and bounds out of the hut."
	exit true
}


action unlock cupboard
{
	IsPresent itm_Cupboard
	has itm_Key

	swap itm_Cupboard, itm_CupboardOpen
	"The cupboard is now open..."
	exit true
}

action unlock cupboard
{
	IsPresent itm_CupboardOpen
	"It is already unlocked."
	exit true
}

action look cupboard
{
	IsPresent itm_CupboardOpen

	try
	{
		IsItemInRoom itm_Nails, void
		drop itm_Nails
		"I found : Nails."
		exit true
	}

	try
	{
		IsItemInRoom itm_Pump, void
		drop itm_Pump
		"I found : Pump."
		exit true
	}

	"I found nothing."
	exit true
}

###############################################################################
# on river
#

action go rapids
{
	IsPresent itm_Rapids
	"The raft is smashed to pieces in the rapids."
	LoseGame
}

action look rapids
{
	IsPresent itm_Rapids
	"UH OH!"
	exit true
}

action look clearing
{
	IsPresent itm_Clearing
	"Looks like a safe place to go."
	exit true
}

action go clearing
{
	IsPresent itm_Clearing
	# move to hidden room so it still exists
	destroy itm_Raft
	"The raft is carried away by the river. I am stranded here now."
	goto rm_Jungle
	exit true
}

###############################################################################
# clearing
#

action get rock
{
	IsPresent itm_Rock
	"Its too big"
	exit true
}

action get crocodile
{
	IsPresent itm_Croc
	"The crocodile attacks and makes a meal of me"
	LoseGame
}

action shoot crocodile
{
	IsPresent itm_Croc

	try
	{
		has itm_Pistol

		try
		{
			CounterEQ tr_Pistol, 0
			"Click! The pistol is empty."
		}

		try
		{
			CounterGT tr_Pistol, 0
			"Bang!"
		}

		crlf
		"The crocodile wakes up and attacks."
		LoseGame
	}

	"What with?"
	exit true
}


action dig any
{
	in rm_Jungle
	has itm_Shovel
	IsItemInRoom itm_Lamp, void
	move itm_Lamp, rm_Jungle
	"I found : *Gold Lamp*"
	exit true
}

action move rock
{
	in rm_Jungle

	try
	{
		IsItemInRoom itm_Spraycan, void
		drop itm_Spraycan
		"I found : Spray Can"
		exit true
	}

	"I found nothing."
	exit true
}

action look crocodile
{
	in rm_Jungle
	"The crocodile is asleep."
	exit true
}

action look rock
{
	IsPresent itm_Rock
	"There is writing on the rock."
	exit true
}

action read writing
{
	in rm_Jungle
	"It says..."
	crlf
	"'5 + 1 = 2'"
	exit true
}

action look spraycan
{
	has itm_Spraycan
	try
	{
		CounterEQ tr_SprayCan, 3
		"It feels full"
	}

	try
	{
		CounterEQ tr_SprayCan, 2
		"It feels a bit light"
	}

	try
	{
		CounterEQ tr_SprayCan, 1
		"It feels close to empty"
	}

	try
	{
		CounterEQ tr_SprayCan, 0
		"It feels empty"
	}

	crlf
	"There is a label on the side."
	exit true
}

noun label
action read label
{
	IsPresent itm_Spraycan
	"It reads 'Rust Remover'."
	exit true
}

action spray spraycan
{
	has itm_Spraycan

	try
	{
		CounterEQ tr_SprayCan, 0
		"The spray can is empty"
		exit true
	}

	try
	{
		CounterEQ tr_SprayCan, 3
		"SSSSSSSSSS."
	}

	try
	{
		CounterEQ tr_SprayCan, 2
		"SSSSS."
	}

	try
	{
		CounterEQ tr_SprayCan, 1
		"SS."
	}

	SubCounter tr_SprayCan, 1
	exit true
}


###############################################################################
# raft
#

action pump raft
{
	has itm_Pump

	try
	{
		has itm_RaftInflated
		"The raft is already inflated."
		exit true
	}

	try
	{
		has itm_Raft
		swap itm_Raft, itm_RaftInflated
		"The raft is inflated."
		exit true
	}

	"You dont have a raft"
	exit true
}

action look raft
{
	try
	{
		has itm_Raft
		"The raft is not inflated."
		exit true
	}

	has itm_RaftInflated
	"The raft is inflated."
	exit true
}

###############################################################################
# lamp
#

action rub lamp
{
	IsPresent itm_Lamp
	"You rub the lamp and everything spins....."
	pause 5

	try
	{
		in rm_Jungle
		goto rm_NextToHut
		exit true
	}

	goto rm_Jungle
	exit true
}


###############################################################################
# gun commands
#
action shoot any
{
	try
	{
		has itm_Pistol

		try
		{
			CounterEQ tr_Pistol, 0
			"Click! The pistol is empty."
			exit true
		}

		try
		{
			SubCounter tr_Pistol, 1
			"Bang!"
			exit true
		}
	}

	"Shoot with what?"
	exit true
}

action look pistol
{
	has itm_Pistol

	try
	{
		CounterEQ tr_Pistol, 1
		"It's loaded with %tr_pistol% bullet."
		exit true
	}

	try
	{
		CounterEQ tr_Pistol, 0
		"It's out of bullets"
		exit true
	}
	try
	{
		"It's loaded with %tr_pistol% bullets"
		exit true
	}
}


###############################################################################
# generics
#

action dig any
{
	try
	{
		has itm_Shovel
		# gets here if we have the shovel
		"Digging here is useless."
		exit true
	}

	"I dont have anything to dig with."
	exit true
}

action quit any
{
	QuitRestartGame "Do you want to Quit or Restart. Q/R?"
}


action inventory any
{
	try
	{
		CanPlayerSee
		SubCounter moves, 1
		ShowInventory
		exit true
	}

	"It is too dark to see."
	exit true
}

action move any
{
	"I found nothing."
	exit true
}

action climb any
{
	"I can't climb that."
	exit true
}


action sleep any
{
	try
	{
		in rm_Bed
		exit false
	}

	"The nights are cold on the Island and I freeze to death."
	LoseGame
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

action open any
{
	"open what?"
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
	"'%game%' version %ver% by %author%"
	# issuing score does not count as a move
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
	"The %noun%? You don't have one."
	exit true
}

action light any
{
	"You have no matches"
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

action panic any
{
	"Aiieeeeeee!"
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

action rub any
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

action spray any
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

action fix any
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

action feed any
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

action whistle any
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

action sharpen any
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
