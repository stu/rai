#!/usr/bin/env ruby

#
# game version 1, packed dictionary of length/offset 0xF000 len, 0xFFF offset. 4kb dictionary
# game version 3, unpacked dictionary of offset 0x7FFF, 32kb dictionary
#
#

$pointer_round		= 0
$input_file		= ""
$verbose_flag		= 0
$enable_action_any = 0
$force_game_version = 0
$create_getdrop_all = 0

$template_any =
<<eos
action @verb any
{
	try
	{
		nounis any
		"Do what now?"
		exit true
	}

	"%verb% what? I don't know how to do that."
	exit true
}
eos

$template_get =
<<eos
action get @item_noun
{
	try
	{
		CanPlayerSee

		try
		{
			#NounIs @item_noun
			IsItemhere @item
			take @item
			"You %verb% the %noun%"
			exit true
		}

		continue true
	}

	"It is too dark to see."
	exit true
}
eos

$template_drop =
<<eos
action drop @item_noun
{
	#NounIs @item_noun
	has @item
	drop @item
	"You %verb% the %noun%"
	exit true
}
eos

$template_look =
<<eos
action look @item_noun
{
	try
	{
		CanPlayerSee

		try
		{
			#NounIs @item_noun
			IsPresent @item
			@item_description
			exit true
		}

		continue false
	}

	"It is too dark to see."
	exit true
}
eos


if RUBY_VERSION.split('.')[0] <= "1" && RUBY_VERSION.split('.')[1] < "9"
	puts "Requires ruby 1.9.2 or better"
	exit
end

###########################################################
# tokeniser routines

class TokeniseDelimiters
	def initialize
		@aDelims = Array.new
	end

	def clear
		@aDelims.clear
	end

	def addDelim(*arg)
		arg.each do |a|
			@aDelims << a
		end
	end

	def include?(arg)
		return @aDelims.include?(arg)
	end
end

class Tokenise
	attr_reader :glue, :breakers, :grabbers, :quotes, :linecomments

	def initialize
		@glue = TokeniseDelimiters.new
		@breakers = TokeniseDelimiters.new
		@grabbers = TokeniseDelimiters.new
		@quotes = TokeniseDelimiters.new
		@linecomments = TokeniseDelimiters.new
	end


	def tokenise(line)
		ll = line

		lx = Array.new
		index = 0
		start = 0

		ll.strip

		while index < ll.length
			if @breakers.include?(ll[index].chr) == true
				if index - start > 0
					lx <<  ll.slice(start, index - start)
				end

				index += 1
				start = index

				# grabbers.
			elsif @grabbers.include?(ll[index].chr) == true
				# take what exists.
				if index - start > 0
					lx <<  ll.slice(start, index - start)
					start = index
				end

				# now take this
				lx << ll.slice(start, index - start + 1)
				index += 1
				start = index

			elsif @linecomments.include?(ll[index].chr) == true
				#index = ll.length
				ll[index] = "\n"
				ll=ll.slice(0, index+1)
				index = ll.length


			elsif @quotes.include?(ll[index].chr) == true
				# skip over opening quote.
				index += 1

				line_endded = false
				catch :breakout do
					while index < ll.length

						if ll[index].chr == "\\"
							index += 2
						end

						if @quotes.include?(ll[index].chr) == true
							#index += 1
							throw :breakout
							line_endded = true
						else
							index += 1
						end
					end
				end


				lx <<  ll.slice(start, 1 + index - start)
				index += 1
				start = index
			else
				index += 1
			end
		end

		if start != index
			lx << ll.slice(start, index - start)
		end

		# glue
		i = 0

		gcount = 1
		while gcount > 0
			gcount = 0
			1..lx.length.times do |l|
				if @glue.include?(lx[l-1].to_s + lx[l].to_s) == true
					lx[l-1] = lx[l-1].to_s + lx[l].to_s
					lx[l] = nil
					gcount += 1
				end
			end

			lx.compact!
		end

		return lx
	end
end

require "zlib"

class XStr
	@@idx = 0

	attr_accessor :id, :str, :offs, :strx, :nstring, :nlength

	def initialize(s)
		@id = @@idx
		@@idx += 1
		@str = s
		@strx = Array.new
		@offs = 0
	end
end

class GlobalCodeBlockID
	XGC_RESET = 1
	XGC_PREGAME = 2
	XGC_ON_SUCCESS = 3
	XGC_ON_FAIL = 4
	XGC_PROMPT = 5
	XGC_ON_GAMEWIN = 6
	XGC_ON_GAMEFAIL = 7
end

class Opcode
	X_ADDCOUNTER = 1
	X_SUBCOUNTER = 2
	X_SETCOUNTER = 3
	X_SETLIGHT = 4
	X_MSG = 5
	X_QUITRESTARTGAME = 6
	X_TRY = 7
	X_ISIMTEINROOM = 8
	X_WINLOOSEGAME = 9
	X_CONTINUE = 10
	X_EXIT = 11
	X_MOVE = 12
	X_SWAP = 13
	X_CANPLAYERSEE = 14
	X_ISPRESENT = 15
	X_GOTO = 16
	X_NOUNIS = 17
	X_SHOWINVENTORY = 18
	X_CANCARRY = 19
	X_HAS = 20
	X_HERE = 21
	X_ENDTRY = 22

	X_LOOK = 24
	X_IN = 25
	X_TRANSPORT = 26
	X_SWITCH = 27
	X_EQUALSCOUNTER = 28

	X_TAKE = 30
	X_COUNTERGT = 31
	X_COUNTERLT = 32
	X_ISIMTEMHERE = 33
	X_ISFLAG = 34
	X_SETFLAG = 35
	X_SETPLAYERINVENTORY = 36
	X_NPCHERE = 37
	X_PAUSE = 38

	X_RANDOM = 40

	X_NOT = 42

	X_BYTECODE_GOTO = 43

	X_ENDOPCODES = 255
end

class Game
	attr_accessor :game, :version, :author, :interp_version, :max_carry
	attr_accessor :counters, :verbs, :nouns, :items, :rooms, :actions
	attr_accessor :strings, :codeblocks, :string_id, :players, :flags
	attr_accessor :filename

	def initialize
		@version = "" + Time.new.strftime("%Y%m%d")
		@author = "RAI Compiler"
		@game = "RAI GAME"
		@counters = Hash.new
		@rooms = Hash.new
		@verbs = Hash.new
		@nouns = Hash.new
		@items = Hash.new
		@actions = Hash.new
		@strings = Hash.new
		@string_id = Hash.new
		@codeblocks = Hash.new
		@players = Hash.new
		@flags = Hash.new
	end
end

class Flag
	@@idz = 0
	attr_accessor :id, :idx
	def initialize(idx)
		@id = idx
		@idx = @@idz
		@@idz += 1
	end
end

class Counter
	@@idz = 0
	attr_accessor :id, :idx
	def initialize(idx)
		@id = idx
		@idx = @@idz
		@@idz += 1
	end
end

class Player
	@@idz = 0

	attr_accessor :id, :inventory, :location, :idx, :noun, :name, :flags

	def initialize(idx)
		@id = idx.to_s.downcase
		@flags = Hash.new
		@location = "VOID".to_s.downcase
		@inventory = "VOID".to_s.downcase
		@idx = @@idz
		@@idz += 1
		@flags["player"] = true

		@noun = ""
		@name = ""
	end
end

class Verb
	@@idz = 0

	attr_accessor :id, :synonyms, :idx

	def initialize(idx)
		@id = idx.to_s.downcase
		@synonyms = Hash.new
		@idx = @@idz
		@@idz += 1
	end
end

class Noun
	@@idz = 0

	attr_accessor :id, :synonyms, :idx

	def initialize(idx)
		@id = idx.to_s.downcase
		@synonyms = Hash.new
		@idx = @@idz
		@@idz += 1
	end
end

class Item
	@@idz = 0

	attr_accessor :id, :location, :flags, :idx, :noun, :name, :descr

	def initialize(idx)
		@id = idx.to_s.downcase
		@flags = Hash.new
		@location = "VOID".to_s.downcase
		@descr = ""
		@idx = @@idz
		@@idz += 1

		@noun = ""
		@name = ""
		@flags["light"] = false
		@flags["scenery"] = false
		@flags["player"] = false
		@flags["take"] = false
	end
end

class Room
	@@idz = 0

	attr_accessor :id, :title, :idx
	attr_accessor :n, :s, :e, :w, :ne, :nw, :se, :sw, :u, :d
	attr_accessor :codeblocks

	def initialize(name)
		@id = name.to_s.downcase
		@idx = @@idz
		@@idz += 1
		@codeblocks = Hash.new

		if $verbose_flag != 0
			puts "-> Room #{name.to_s.downcase}"
		end

		@n = "VOID".to_s.downcase
		@s = "VOID".to_s.downcase
		@e = "VOID".to_s.downcase
		@w = "VOID".to_s.downcase
		@u = "VOID".to_s.downcase
		@d = "VOID".to_s.downcase
		@ne = "VOID".to_s.downcase
		@nw = "VOID".to_s.downcase
		@se = "VOID".to_s.downcase
		@sw = "VOID".to_s.downcase
	end
end

class Action
	@@idz = 0

	attr_accessor :id, :verb, :noun, :unique_id
	attr_accessor :codeblock

	def initialize(name)
		@id = name
		@@idz += 1
		@unique_id = @@idz
	end

end

class CB_SetPlayerInventory
	attr_accessor :player, :room, :opcode

	def compile(g)
		if g.players[@player] == nil
			puts "***** Player #{@player} does not exist."
			exit
		end
		if g.rooms[@room] == nil
			puts "***** Room #{@room} does not exist."
			exit
		end

		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (g.players[@player].idx.to_i & 0xFF)
		@bytes << (g.rooms[@room].idx.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_SETPLAYERINVENTORY
	end
end

class CB_IsFlag
	attr_accessor :flag, :val, :opcode

	def compile(g)
		if g.flags[@flag] == nil
			puts "***** Flag #{@flag} does not exist."
			exit
		end
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (g.flags[@flag].idx.to_i & 0xFF)
		if @val == true
			@bytes << (1.to_i & 0xFF)
		else
			@bytes << (0.to_i & 0xff)
		end
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_ISFLAG
	end
end


class CB_SetFlag
	attr_accessor :flag, :val, :opcode

	def compile(g)
		if g.flags[@flag] == nil
			puts "***** Flag #{@flag} does not exist."
			exit
		end
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (g.flags[@flag].idx.to_i & 0xFF)
		if @val == true
			@bytes << (1.to_i & 0xFF)
		else
			@bytes << (0.to_i & 0xff)
		end
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_SETFLAG
	end
end


class CB_EqualsCounter
	attr_accessor :cnt, :val, :opcode

	def compile(g)
		if g.counters[@cnt] == nil
			puts "***** Counter #{@cnt} does not exist."
			exit
		end
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (g.counters[@cnt].idx.to_i & 0xFF)
		@bytes << ((@val.to_i>>8) & 0xFF)
		@bytes << (@val.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_EQUALSCOUNTER
	end
end


class CB_CounterGT
	attr_accessor :cnt, :val, :opcode

	def compile(g)
		if g.counters[@cnt] == nil
			puts "***** Counter #{@cnt} does not exist."
			exit
		end
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (g.counters[@cnt].idx.to_i & 0xFF)
		@bytes << ((@val.to_i>>8) & 0xFF)
		@bytes << (@val.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_COUNTERGT
	end
end

class CB_CounterLT
	attr_accessor :cnt, :val, :opcode

	def compile(g)
		if g.counters[@cnt] == nil
			puts "***** Counter #{@cnt} does not exist."
			exit
		end
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (g.counters[@cnt].idx.to_i & 0xFF)
		@bytes << ((@val.to_i>>8) & 0xFF)
		@bytes << (@val.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_COUNTERLT
	end
end

class CB_AddCounter
	attr_accessor :cnt, :val, :opcode

	def compile(g)
		if g.counters[@cnt] == nil
			puts "***** Counter #{@cnt} does not exist."
			exit
		end
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (g.counters[@cnt].idx.to_i & 0xFF)
		@bytes << ((@val.to_i>>8) & 0xFF)
		@bytes << (@val.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_ADDCOUNTER
	end
end

class CB_SubCounter
	attr_accessor :cnt, :val, :opcode

	def compile(g)
		if g.counters[@cnt] == nil
			puts "***** Counter #{@cnt} does not exist."
			exit
		end
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (g.counters[@cnt].idx.to_i & 0xFF)
		@bytes << ((@val.to_i>>8) & 0xFF)
		@bytes << (@val.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_SUBCOUNTER
	end
end

class CB_SetCounter
	attr_accessor :cnt, :val, :opcode

	def compile(g)
		if g.counters[@cnt] == nil
			puts "***** Counter #{@cnt} does not exist."
			exit
		end
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (g.counters[@cnt].idx.to_i & 0xFF)
		@bytes << ((@val.to_i>>8) & 0xFF)
		@bytes << (@val.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_SETCOUNTER
	end

end

class CB_SetLight
	attr_accessor :light_flag, :opcode, :opcode

	def compile(g)
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		if @light_flag == true
			@bytes << (1.to_i & 0xFF)
		else
			@bytes << (0.to_i & 0xFF)
		end
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_SETLIGHT
	end
end

class CB_msg
	attr_accessor :msg, :id, :opcode

	def compile(g)
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << ((g.strings[@msg.to_s].id.to_i>>8) & 0xFF)
		@bytes << (g.strings[@msg.to_s].id.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_MSG
	end
end

class CB_QuitRestartGame
	attr_accessor :msg, :opcode

	def compile(g)
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << ((g.strings[@msg].id.to_i>>8) & 0xFF)
		@bytes << (g.strings[@msg].id.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_QUITRESTARTGAME
	end
end

class CB_Try
	attr_accessor :codeblock, :opcode

	def compile(g)
		@bytes =  Array.new

		# walk + compile sub blocks

		@bytes << @codeblock.compile(g)
		@bytes = @bytes.flatten

		foo = Array.new

		foo << (@opcode.to_i & 0xFF)
		foo << ((@bytes.length.to_i >> 8) & 0xFF)
		foo << (@bytes.length.to_i & 0xFF)

		@bytes = (foo << @bytes).flatten

		@bytes = @bytes << Opcode::X_ENDTRY
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_TRY
	end
end

class CB_IsItemInRoom
	attr_accessor :item, :room, :opcode

	def compile(g)
		if g.rooms[@room] == nil
			puts "***** Room #{@room} does not exist."
			exit
		end
		if g.items[@item] == nil
			puts "***** Item #{@item} does not exist."
			exitNOUNIS
		end
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (g.items[@item].idx.to_i & 0xFF)
		@bytes << (g.rooms[@room].idx.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_ISIMTEINROOM
	end
end

class CB_IsItemHere
	attr_accessor :item, :room, :opcode

	def compile(g)
		if g.items[@item] == nil
			puts "***** Item #{@item} does not exist."
			exit
		end
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (g.items[@item].idx.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_ISIMTEMHERE
	end
end


class CB_In
	attr_accessor :room, :opcode

	def compile(g)
		if g.rooms[@room] == nil
			puts "***** Room #{@room} does not exist."
			exit
		end
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (g.rooms[@room].idx.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_IN
	end
end

class CB_Not
	attr_accessor :opcode

	def compile(g)
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_NOT
	end
end

class CB_Random
	attr_accessor :rnd

	def compile(g)
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (@rnd.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_RANDOM
	end
end

class CB_WinLoseGame
	attr_accessor :winloose_flag, :opcode

	def compile(g)
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		if @winloose_flag == true
			@bytes << (1.to_i & 0xFF)
		else
			@bytes << (0.to_i & 0xff)
		end
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_WINLOOSEGAME
	end
end

class CB_Continue
	attr_accessor :continue_flag, :opcode

	def compile(g)
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		if @continue_flag == true
			@bytes << (1.to_i & 0xFF)
		else
			@bytes << (0.to_i & 0xff)
		end
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_CONTINUE
	end
end

class CB_Exit
	attr_accessor :exit_flag, :opcode

	def compile(g)
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		if @exit_flag == true
			@bytes << (1.to_i & 0xFF)
		else
			@bytes << (0.to_i & 0xff)
		end
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_EXIT
	end
end

class CB_Transport
	attr_accessor :player, :room, :opcode

	def compile(g)
		if g.rooms[@room] == nil
			puts "***** Room #{@room} does not exist."
			exit
		end
		if g.players[@player] == nil
			puts "***** Player #{@player} does not exist."
			exit
		end
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (g.players[@player].idx.to_i & 0xFF)
		@bytes << (g.rooms[@room].idx.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_TRANSPORT
	end
end

class CB_NPCHere
	attr_accessor :npc, :opcode

	def compile(g)
		if g.players[@npc] == nil
			puts "***** Player #{@npc} does not exist."
			exit
		end
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (g.players[@npc].idx.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_NPCHERE
	end
end

class CB_Switch
	attr_accessor :player, :opcode

	def compile(g)
		if g.players[@player] == nil
			puts "***** Player #{@player} does not exist."
			exit
		end
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (g.players[@player].idx.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_SWITCH
	end
end


class CB_Move
	attr_accessor :item, :room, :opcode

	def compile(g)
		if g.rooms[@room] == nil
			puts "***** Room #{@room} does not exist."
			exit
		end
		if g.items[@item] == nil
			puts "***** Item #{@item} does not exist."
			exit
		end

		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (g.items[@item].idx.to_i & 0xFF)
		@bytes << (g.rooms[@room].idx.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_MOVE
	end
end


class CB_Take
	attr_accessor :item, :opcode

	def compile(g)
		if g.items[@item] == nil
			puts "***** Item #{@item} does not exist."
			exit
		end

		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (g.items[@item].idx.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_TAKE
	end
end

class CB_Pause
	attr_accessor :time, :opcode

	def compile(g)
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (@time.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_PAUSE
	end
end



class CB_Swap
	attr_accessor :item1, :item2, :opcode

	def compile(g)
		if g.items[@item1] == nil
			puts "***** Item #{@item1} does not exist."
			exit
		end
		if g.items[@item2] == nil
			puts "***** Item #{@item2} does not exist."
			exit
		end
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (g.items[@item1].idx.to_i & 0xFF)
		@bytes << (g.items[@item2].idx.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_SWAP
	end
end

class CB_EndOpcodes
	attr_accessor :opcode

	def compile(g)
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_ENDOPCODES
	end
end

class CB_CanPlayerSee
	attr_accessor :opcode

	def compile(g)
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_CANPLAYERSEE
	end
end

class CB_IsPresent
	attr_accessor :item, :opcode

	def compile(g)
		if g.items[@item] == nil
			puts "***** Item #{@item} does not exist."
			exit
		end
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (g.items[@item].idx.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_ISPRESENT
	end
end

class CB_Goto
	attr_accessor :room, :opcode

	def compile(g)
		if g.rooms[@room] == nil
			puts "***** Room #{@room} does not exist."
			exit
		end

		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (g.rooms[@room].idx.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_GOTO
	end
end

class CB_NounIs
	attr_accessor :item, :opcode

	def compile(g)
		if g.nouns[@item] == nil
			puts "***** Item #{@item} does not exist."
			exit
		end
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		#@bytes << (g.items[@item].idx.to_i & 0xFF)
		@bytes << (g.nouns[@item].idx.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_NOUNIS
	end
end

class CB_ShowInventory
	attr_accessor :opcode

	def compile(g)
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_SHOWINVENTORY
	end
end

class CB_CanCarry
	attr_accessor :opcode

	def compile(g)
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_CANCARRY
	end
end


class CB_Look
	attr_accessor :opcode

	def compile(g)
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_LOOK
	end
end

class CB_Has
	attr_accessor :item, :opcode

	def compile(g)
		if g.items[@item] == nil
			puts "***** Item #{@item} does not exist."
			exit
		end
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (g.items[@item].idx.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_HAS
	end
end

class CB_Here
	attr_accessor :item, :opcode

	def compile(g)
		if g.items[@item] == nil
			puts "***** Item #{@item} does not exist."
			exit
		end
		@bytes =  Array.new
		@bytes << (@opcode.to_i & 0xFF)
		@bytes << (g.items[@item].idx.to_i & 0xFF)
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize
		@opcode = Opcode::X_HERE
	end
end

class CodeBlock
	attr_accessor :id, :codes

	def compile(g)
		@bytes =  Array.new

		# walk + compile sub blocks

		@codes.each do |cb|
			@bytes << (cb.compile(g))
			@bytes = @bytes.flatten
		end

		#foo = Array.new
		#foo << ((@bytes.length.to_i >> 8) & 0xFF)
		#foo << (@bytes.length.to_i & 0xFF)
		#@bytes = (foo << @bytes).flatten

		@bytes = @bytes.flatten
	end

	def bytecode_length
		return @bytes.length
	end

	def bytecode
		return @bytes
	end

	def initialize(name)
		@id = name
		@codes = Array.new
	end

	def print_code
		i = 0
		@bytes.each do |bb|
			print  ("00" + bb.to_s(16))[-2..-1] + " "
			i += 1
			if i == 32
				print "\n"
				i = 0
			end
		end
		print "\n"
	end
end

def tok_assert(a, b, c)
	if a != b
		puts "***** " + c.to_s
		exit
	end
end

def tok_not_assert(a, b, c)
	if a == b
		puts "***** " + c.to_s
		exit
	end
end

def destring(s)
	if s.slice(0, 1) == "\""
		return s.slice(1, s.length-2)
	else
		return s
	end
end

def parse_code_block(game, toks, idx, name)
	state = 1
	block = CodeBlock.new(name)
	brace_depth = 1

	tok_assert(toks[idx], "{", "Bad code block at \"" + name + "\"")

	# skip brace + EOC
	idx += 2

	last_opcode_was_not = 0

	while state == 1  && idx < toks.length
		if toks[idx].downcase == "not"
			cb = CB_Not.new
			idx += 1
			block.codes << cb

			last_opcode_was_not = 1
		end

		if toks[idx].downcase == "setcounter"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_SetCounter.new
			cb.cnt = toks[idx + 1].downcase
			tok_assert(toks[idx+2], ",", "Comma required at \"" + cb.cnt.to_s + "\"")
			cb.val = toks[idx + 3]
			idx += 4
			block.codes << cb

		elsif toks[idx].downcase == "counterequals" or toks[idx].downcase == "countereq"
			last_opcode_was_not = 0
			cb = CB_EqualsCounter.new
			cb.cnt = toks[idx + 1].downcase
			tok_assert(toks[idx+2], ",", "Comma required at \"" + cb.cnt + "\"")
			cb.val = toks[idx + 3].to_s
			idx += 4
			block.codes << cb

		elsif toks[idx].downcase == "countergt"
			last_opcode_was_not = 0
			cb = CB_CounterGT.new
			cb.cnt = toks[idx + 1].downcase
			tok_assert(toks[idx+2], ",", "Comma required at \"" + cb.cnt + "\"")
			cb.val = toks[idx + 3].to_s
			idx += 4
			block.codes << cb

		elsif toks[idx].downcase == "counterlt"
			last_opcode_was_not = 0
			cb = CB_CounterLT.new
			cb.cnt = toks[idx + 1].downcase
			tok_assert(toks[idx+2], ",", "Comma required at \"" + cb.cnt + "\"")
			cb.val = toks[idx + 3].to_s
			idx += 4
			block.codes << cb

		elsif toks[idx].downcase == "addcounter"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_AddCounter.new
			cb.cnt = toks[idx + 1].downcase
			tok_assert(toks[idx+2], ",", "Comma required at \"" + cb.cnt + "\"")
			cb.val = toks[idx + 3].to_s
			idx += 4
			block.codes << cb

		elsif toks[idx].downcase == "subcounter"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_SubCounter.new
			cb.cnt = toks[idx + 1].downcase
			tok_assert(toks[idx+2], ",", "Comma required at \"" + cb.cnt + "\"")
			cb.val = toks[idx + 3]
			idx += 4
			block.codes << cb

		elsif toks[idx].downcase == "setlighton"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_SetLight.new
			cb.light_flag = true
			idx += 1
			block.codes << cb

		elsif toks[idx].downcase == "setlightoff"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_SetLight.new
			cb.light_flag = false
			idx += 1
			block.codes << cb

		elsif toks[idx].downcase == "wingame"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_WinLoseGame.new
			cb.winloose_flag = true
			idx += 1
			block.codes << cb

		elsif toks[idx].downcase == "losegame"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_WinLoseGame.new
			cb.winloose_flag = false
			idx += 1
			block.codes << cb

		elsif toks[idx].downcase == "quitrestartgame"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_QuitRestartGame.new
			cb.msg = destring(toks[idx + 1])
			game.strings[cb.msg.to_s] = cb.msg.to_s
			idx += 2
			block.codes << cb

		elsif toks[idx].downcase == "try"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_Try.new

			#skip try enc
			idx += 1

			tok_assert(toks[idx], "<<EOC>>", "Missing eoc on try line")
			idx += 1

			bb, idx = parse_code_block(game, toks, idx, "try")
			cb.codeblock = bb
			block.codes << cb

		elsif toks[idx].downcase == "isiteminroom"
			last_opcode_was_not = 0
			cb = CB_IsItemInRoom.new
			cb.item = toks[idx + 1].to_s.downcase
			tok_assert(toks[idx+2], ",", "Comma required at \"" + cb.item + "\"")
			cb.room = toks[idx + 3].to_s.downcase
			idx += 4
			block.codes << cb

		elsif toks[idx].downcase == "isitemhere"
			last_opcode_was_not = 0
			cb = CB_IsItemHere.new
			cb.item = toks[idx + 1].to_s.downcase
			idx += 2
			block.codes << cb

		elsif toks[idx].downcase == "in"
			last_opcode_was_not = 0
			cb = CB_In.new
			cb.room = toks[idx + 1].to_s.downcase
			idx += 2
			block.codes << cb

		elsif toks[idx].downcase == "random"
			last_opcode_was_not = 0
			cb = CB_Random.new
			cb.rnd = toks[idx + 1].to_s
			idx += 2
			block.codes << cb

		elsif toks[idx].downcase == "ispresent"
			last_opcode_was_not = 0
			cb = CB_IsPresent.new
			cb.item = toks[idx + 1].to_s.downcase
			idx += 2
			block.codes << cb

		elsif toks[idx].downcase == "goto"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_Goto.new
			cb.room = toks[idx + 1].to_s.downcase
			idx += 2
			block.codes << cb

		elsif toks[idx].downcase == "transport"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_Transport.new
			cb.player = toks[idx + 1].to_s.downcase
			tok_assert(toks[idx+2], ",", "Comma required at \"" + cb.player + "\"")
			cb.room = toks[idx + 3].to_s.downcase
			idx += 4
			block.codes << cb

		elsif toks[idx].downcase == "switchplayer"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_Switch.new
			cb.player = toks[idx + 1].to_s.downcase
			idx += 2
			block.codes << cb

		elsif toks[idx][0] == "\""
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_msg.new
			cb.msg = destring(toks[idx + 0])
			idx += 1
			tok_assert(toks[idx], "<<EOC>>", "Missing EOC on string.. " + toks[idx])

			while toks[idx] == "<<EOC>>" and toks[idx+1][0] == "\""
				if cb.msg.slice(-1,1) != " " then
					cb.msg += " "
				end
				cb.msg += destring(toks[idx+1])
				idx += 2
			end

			game.strings[cb.msg.to_s] = cb.msg.to_s
			block.codes << cb

		elsif toks[idx].downcase == "crlf"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_msg.new
			cb.msg = "\\n"
			idx += 1
			game.strings[cb.msg.to_s] = cb.msg.to_s
			block.codes << cb

		elsif toks[idx].downcase == "continue"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_Continue.new
			if toks[idx+1] == "true" then
				cb.continue_flag = true
			else
				cb.continue_flag = false
			end

			idx += 2
			block.codes << cb

		elsif toks[idx].downcase == "exit"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_Exit.new
			if toks[idx+1] == "true" then
				cb.exit_flag = true
			elsif toks[idx+1] == "false" then
				cb.exit_flag = false
			else
				puts "Missing exit command around #{toks[idx+1]}\n"
			end

			idx += 2
			block.codes << cb

		elsif toks[idx].downcase == "setflagtrue"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_SetFlag.new
			cb.flag = toks[idx+1].to_s.downcase
			cb.val = true
			idx += 2
			block.codes << cb

		elsif toks[idx].downcase == "setflagfalse"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_SetFlag.new
			cb.flag = toks[idx+1].to_s.downcase
			cb.val = false
			idx += 2
			block.codes << cb

		elsif toks[idx].downcase == "isflagtrue"
			last_opcode_was_not = 0
			cb = CB_IsFlag.new
			cb.flag = toks[idx+1].to_s.downcase
			cb.val = true
			idx += 2
			block.codes << cb

		elsif toks[idx].downcase == "isflagfalse"
			last_opcode_was_not = 0
			cb = CB_IsFlag.new
			cb.flag = toks[idx+1].to_s.downcase
			cb.val = false
			idx += 2
			block.codes << cb

		elsif toks[idx].downcase == "canplayersee"
			last_opcode_was_not = 0
			cb = CB_CanPlayerSee.new
			idx += 1
			block.codes << cb

		elsif toks[idx].downcase == "setplayerinventory"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_SetPlayerInventory.new
			cb.player = toks[idx + 1].to_s.downcase
			tok_assert(toks[idx+2], ",", "Comma required at \"" + cb.player + "\" in setplayerinventory")
			cb.room = toks[idx + 3].to_s.downcase
			idx += 4
			blocks.codes << cb

		elsif toks[idx].downcase == "destroy"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_Move.new
			cb.item = toks[idx + 1].to_s.downcase
			cb.room = "void".to_s.downcase
			idx += 2
			block.codes << cb

		elsif toks[idx].downcase == "move"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_Move.new
			cb.item = toks[idx + 1].to_s.downcase
			tok_assert(toks[idx+2], ",", "Comma required at \"" + cb.item + "\" in move")
			cb.room = toks[idx + 3].to_s.downcase
			idx += 4
			block.codes << cb

		elsif toks[idx].downcase == "pause"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_Pause.new
			cb.time = toks[idx + 1].to_s.downcase
			idx += 2
			block.codes << cb

		elsif toks[idx].downcase == "take"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_Take.new
			cb.item = toks[idx + 1].to_s.downcase
			idx += 2
			block.codes << cb

		elsif toks[idx].downcase == "swap"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_Swap.new
			cb.item1 = toks[idx + 1].to_s.downcase
			tok_assert(toks[idx+2], ",", "Comma required at \"" + cb.item1 + "\"")
			cb.item2 = toks[idx + 3].to_s.downcase
			idx += 4
			block.codes << cb

		elsif toks[idx].downcase == "nounis"
			last_opcode_was_not = 0
			cb = CB_NounIs.new
			cb.item = toks[idx + 1].to_s.downcase
			idx += 2
			block.codes << cb

		elsif toks[idx].downcase == "showinventory"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_ShowInventory.new
			idx += 1
			block.codes << cb

		elsif toks[idx].downcase == "cancarry"
			last_opcode_was_not = 0
			cb = CB_CanCarry.new
			idx += 1
			block.codes << cb

		elsif toks[idx].downcase == "look"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_Look.new
			idx += 1
			block.codes << cb

		elsif toks[idx].downcase == "has"
			last_opcode_was_not = 0
			cb = CB_Has.new
			cb.item = toks[idx + 1].to_s.downcase
			idx += 2
			block.codes << cb

		elsif toks[idx].downcase == "here" or toks[idx].downcase == "drop"
			if last_opcode_was_not == 1
				puts "Can't apply 'NOT' to a non-test condition #{toks[idx].downcase}"
				exit
			end

			cb = CB_Here.new
			cb.item = toks[idx + 1].to_s.downcase
			idx += 2
			block.codes << cb

		elsif toks[idx].downcase == "isnpchere"
			last_opcode_was_not = 0
			cb = CB_NPCHere.new
			cb.npc = toks[idx + 1].to_s.downcase
			idx += 2
			block.codes << cb

		elsif toks[idx].downcase == "}"
			break

		else
			puts "Unknown command \"" + toks[idx] + "\""
			exit
		end

		tok_assert(toks[idx], "<<EOC>>", "Expected <<EOC>>, got [#{toks[idx]}]\nSomething off in code block around " + toks[idx-6] + ", "  + toks[idx-5] + ", "  + toks[idx-4] + ", "  + toks[idx-3] + ", "  + toks[idx-2] + ", "  + toks[idx-1] + ", " + toks[idx] )
		idx += 1
	end

	tok_assert(toks[idx], "}", "Bad code block at \"" + name + "\"")
	idx += 1

	return block, idx
end

def parse_room(g, toks, idx)
	state = 3

	tok_assert(toks[idx], "room", "Missing room ID at " + toks[idx])
	idx += 1
	r = Room.new(toks[idx].to_s)
	idx += 1

	tok_not_assert(toks[idx], "<<EOC>>", "Missing EOC at " + toks[idx])

	r.title = destring(toks[idx])
	g.strings[r.title.to_s] = r.title.to_s
	idx += 1

	tok_assert(toks[idx], "<<EOC>>", "Missing EOC at " + toks[idx])
	idx += 1

	tok_assert(toks[idx], "{", "Missing EOC at " + toks[idx])
	idx += 1

	tok_assert(toks[idx], "<<EOC>>", "Missing EOC at " + toks[idx])
	idx += 1

	while state == 3
		if toks[idx].downcase == "n" then
			r.n = toks[idx+1].to_s.downcase
			idx += 2

		elsif toks[idx].downcase == "s" then
			r.s = toks[idx+1].to_s.downcase
			idx += 2

		elsif toks[idx].downcase == "e" then
			r.e = toks[idx+1].to_s.downcase
			idx += 2

		elsif toks[idx].downcase == "w" then
			r.w = toks[idx+1].to_s.downcase
			idx += 2

		elsif toks[idx].downcase == "ne" then
			r.ne = toks[idx+1].to_s.downcase
			idx += 2

		elsif toks[idx].downcase == "nw" then
			r.nw = toks[idx+1].to_s.downcase
			idx += 2

		elsif toks[idx].downcase == "se" then
			r.se = toks[idx+1].to_s.downcase
			idx += 2

		elsif toks[idx].downcase == "sw" then
			r.sw = toks[idx+1].to_s.downcase
			idx += 2

		elsif toks[idx].downcase == "u" then
			r.u = toks[idx+1].to_s.downcase
			idx += 2

		elsif toks[idx].downcase == "d" then
			r.d = toks[idx+1].to_s.downcase
			idx += 2

		elsif toks[idx].downcase == "enter" then
			idx += 2
			block, idx = parse_code_block(g, toks, idx, "enter")
			block.codes << CB_EndOpcodes.new
			r.codeblocks[block.id.to_s] = block

		elsif toks[idx].downcase == "look" then
			idx += 2
			block, idx = parse_code_block(g, toks, idx, "look")
			block.codes << CB_EndOpcodes.new
			r.codeblocks[block.id.to_s] = block

		elsif toks[idx].downcase == "on" then
			idx += 1
			block, idx = parse_code_block(g, toks, idx+2, toks[idx])
			block.codes << CB_EndOpcodes.new
			r.codeblocks[block.id] = block

		elsif toks[idx] == "}"
			break

		else
			puts "Unknown command \"" + toks[idx] + "\", last was " + toks[idx-1] + ", " + toks[idx-2]
			exit
		end

		tok_assert(toks[idx], "<<EOC>>", "Something off in code block around " + toks[idx])
		idx += 1
	end

	tok_assert(toks[idx], "}", "Bad code block at \"" + r.id + "\"")
	idx += 1

	return r, idx
end


def parse_item(g, toks, idx)
	tok_assert(toks[idx], "item", "Missing item ID at " + toks[idx])
	idx += 1

	i = Item.new(toks[idx].to_s.downcase)
	idx += 1

	if toks[idx].to_s == "[" then
		idx += 1
		tok_not_assert(toks[idx], "]", "Missing noun in item def #{i.id}")
		i.noun = toks[idx].to_s.downcase
		idx += 1

		if g.nouns[i.noun.to_s] == nil
			g.nouns[i.noun.to_s] = Noun.new(i.noun)
		end

		while toks[idx].to_s == ","
			idx += 1
			tok_not_assert(toks[idx], "]", "Missing noun in item def #{i.id}")
			g.nouns[i.noun.to_s].synonyms[toks[idx].to_s.downcase] = toks[idx].to_s.downcase
			idx += 1
		end

		tok_assert(toks[idx], "]", "Missing closing noun in #{i.id}")
		idx += 1
	end

	i.name = destring(toks[idx].to_s)
	g.strings[i.name.to_s] = i.name.to_s
	idx +=1

	tok_assert(toks[idx], ",", "Missing comma in item def #{i.id}")
	idx += 1

	i.descr = destring(toks[idx].to_s)
	g.strings[i.descr.to_s] = i.descr.to_s
	idx +=1

	while toks[idx].downcase != "<<eoc>>" do
		if toks[idx].downcase == "in"
			idx += 1
			i.location = toks[idx].to_s.downcase
		elsif toks[idx].downcase == "has"
			idx += 1
			if toks[idx].downcase == "light"
				i.flags["light"] = true
			elsif toks[idx].downcase == "scenery"
				i.flags["scenery"] = true
			elsif toks[idx].downcase == "take"
				i.flags["take"] = true
			else
				puts "Unknown flag in item " + i.id
				exit
			end
		else
			puts "Unknown " + toks[idx] + " in item " + i.id
			exit
		end

		idx += 1
	end

	i.flags["player"] = false

	return i, idx
end

def parse_player(g, toks, idx)
#player pJohn ["john"] "John" in rmRiverBank

	tok_assert(toks[idx], "player", "Missing item ID at " + toks[idx])
	idx += 1

	i = Player.new(toks[idx].to_s.downcase)
	idx += 1

	i.inventory = "inventory#{("000" + g.players.count.to_s).slice(-3,3)}"
	g.rooms[i.inventory.to_s] = Room.new(i.inventory.to_s)
	g.rooms[i.inventory.to_s].title = i.inventory.to_s
	g.strings[i.inventory.to_s] = i.inventory.to_s

	if toks[idx].to_s == "[" then
		idx += 1
		tok_not_assert(toks[idx], "]", "Missing noun in item def #{i.id}")
		i.noun = toks[idx].to_s.downcase
		idx += 1

		if g.nouns[i.noun.to_s] == nil
			g.nouns[i.noun.to_s] = Noun.new(i.noun)
		end

		while toks[idx].to_s == ","
			idx += 1
			tok_not_assert(toks[idx], "]", "Missing noun in player def #{i.id}")
			g.nouns[i.noun.to_s].synonyms[toks[idx].to_s.downcase] = toks[idx].to_s.downcase
			idx += 1
		end

		tok_assert(toks[idx], "]", "Missing closing noun in #{i.id}")
		idx += 1
	end

	i.name = destring(toks[idx].to_s)
	g.strings[i.name] = i.name
	idx +=1

	while toks[idx].downcase != "<<eoc>>" do
		if toks[idx].downcase == "in"
			idx += 1
			i.location = toks[idx].to_s.downcase
		else
			puts "Unknown " + toks[idx] + " in player " + i.id
			exit
		end

		idx += 1
	end

	i.flags["player"] = true

	return i, idx
end


def parse_action(g, toks, idx)

	# action FOO BAR <<EOC>>
	tok_assert(toks[idx], "action", "Missing action ID at " + toks[idx])
	tok_not_assert(toks[idx+1], "<<EOC>>", "Missing EOC at " + toks[idx+1])
	tok_not_assert(toks[idx+2], "<<EOC>>", "Missing EOC at " + toks[idx+2])
	tok_assert(toks[idx+3], "<<EOC>>", "Missing EOC at " + toks[idx+3])

	a = Action.new(toks[idx+0].to_s)
	a.verb = toks[idx+1].to_s.downcase
	a.noun = toks[idx+2].to_s.downcase

	if g.verbs[a.verb.to_s] == nil
		g.verbs[a.verb.to_s] = Verb.new(a.verb.to_s)
		puts "******* Creating extra verb for #{a.verb}"
	end

	#if g.nouns[a.noun.to_s] == nil
	#	g.nouns[a.noun.to_s] = Noun.new(a.noun.to_s)
	#	puts "******* Creating extra noun for #{a.noun}"
	#end

	idx += 4

	block, idx = parse_code_block(g, toks, idx, a.verb)
	block.codes << CB_EndOpcodes.new
	a.codeblock = block

	return a, idx
end

def test_data(game)
	game.rooms.each do |k,r|
		#puts r.id
	end

	game.strings.each do |k,s|
		#puts s
	end

	do_getall = false
	do_dropall = false
	if $create_getdrop_all == 1
		do_getall = true
		do_dropall = true
	end

	game.verbs.each do |vk, vv|
		game.actions.each do |ka, kv|
			if vv.id == "get" and kv.noun == "all"
				do_getall = false
				puts "skip get all because #{vv.id} #{kv.noun}"
			end
		end
		game.actions.each do |ka, kv|
			if vv.id == "get" and kv.noun == "all"
				do_getall = false
				puts "skip get all because #{vv.id} #{kv.noun}"
			end
		end
	end

	if do_getall == true or do_dropall == true
		if game.nouns["all"] == nil
			puts "Adding 'all' to nouns'"
			game.nouns["all"] = Noun.new("all")
		end
	end

	if do_getall == true
		puts "Generating get all"
		game.flags["__temp_getall_dropall"] = Flag.new("__temp_getall_dropall")
		code = "action get all\n{\n"
		code += "SetFlagFalse __temp_getall_dropall\n"
		game.items.each do |keyitem,valueitem|
			if valueitem.flags["take"] == true
				code += "try\n{\n"
				code += "CanCarry\n"
				code += "IsItemHere #{valueitem.id}\n"
				code += "Take #{valueitem.id}\n"
				code += "\"You pick up the #{valueitem.noun}\\n\"\n"
				code += "SetFlagTrue __temp_getall_dropall\n"
				code += "}\n"
				toks_get = parse_lines(get_code_lines(code))
			end
		end
		code += "try\n"
		code += "{\n"
		code += "IsFlagTrue __temp_getall_dropall\n"
		code += "exit true\n"
		code += "}\n"
		code += "}\n";
		toks_any = parse_lines(get_code_lines(code))
		action, index = parse_action(game, toks_any, 0)
		game.actions["#{action.unique_id}.get.all"] = action
	end

	if do_dropall == true
		puts "Generating drop all"
		if do_getall == false
			game.flags["__temp_getall_dropall"] = Flag.new("__temp_getall_dropall")
		end
		code = "action drop all\n{\n"
		code += "SetFlagFalse __temp_getall_dropall\n"
		game.items.each do |keyitem,valueitem|
			if valueitem.flags["take"] == true
				code += "try\n{\n"
				code += "Has #{valueitem.id}\n"
				code += "Drop #{valueitem.id}\n"
				code += "\"You drop the #{valueitem.noun}\\n\"\n"
				code += "SetFlagTrue __temp_getall_dropall\n"
				code += "}\n"
				toks_get = parse_lines(get_code_lines(code))
			end
		end
		code += "try\n"
		code += "{\n"
		code += "IsFlagTrue __temp_getall_dropall\n"
		code += "exit true\n"
		code += "}\n"
		code += "}\n";
		toks_any = parse_lines(get_code_lines(code))
		action, index = parse_action(game, toks_any, 0)
		game.actions["#{action.unique_id}.drop.all"] = action
	end

	game.verbs.each do |vk, vv|
		vflag = false

		game.actions.each do |ka, kv|
			if kv.noun == "any" and kv.verb == vv.id
				vflag = true
			end
		end

		## 10 and below are directional verbs handled by the interpreter.
		if vflag == false and vv.idx > 10
			if $enable_action_any == 1
				puts "No ANY action found for #{vv.id}. Generating it."
				code = $template_any.to_s
				code = code.gsub("@verb", "#{vv.id}")
				toks_any = parse_lines(get_code_lines(code))
				action, index = parse_action(game, toks_any, 0)
				game.actions["#{action.unique_id}.#{vv.id}.any"] = action
			else
				puts "No ANY action found for #{vv.id}"
			end
		end
	end

end

def scan_verb(g, vvv)
	g.verbs.each do |k,v|
		if v.synonyms[vvv] != nil
			return v.idx
		end
	end
	return 255
end

def new_parser()
	t = Tokenise.new
	t.breakers.addDelim(" ", "\t")
	t.quotes.addDelim("\"")
	t.grabbers.addDelim(",", "{", "}", "\n", "\r", "[", "]")
	t.linecomments.addDelim("#", "//")

	return t
end

def parse_lines(lines)
	tokstream = Array.new
	t = new_parser()

	lines << "\n"
	lines.each do |l|
		toks = t.tokenise(l)
		toks.each do |tt|
			if tt == "\n" then
				if tokstream[tokstream.length-1] != "<<EOC>>"
					tokstream << "<<EOC>>"
				end
			elsif tt.to_s[0] == "#"
				# do nothing
			else
				tokstream << tt.to_s
			end
		end
	end

	return tokstream
end

def get_code_lines(code)
	x = Array.new()


	c = code.split("\n")
	c.each do |l|
		x << "" + l.to_s + "\n"
	end

	return x
end

def pointer_round(f)
	x = f.tell

	z = x + $pointer_round
	z = (z / ($pointer_round + 1)) * ($pointer_round + 1)

	while f.tell < z
		f.putc 0
	end
end

def fix_round(l)
	l += $pointer_round
	l = (l / ($pointer_round+1)) * ($pointer_round+1)

	return l
end

def out_cstring(xx, fp)
	xx.bytes.to_a.each do |x|
		fp.putc x & 0xFF
	end
	fp.putc 0x0
end

class Cached_Block
	attr_accessor :bytes, :offset

	def initialize(off, arr)
		@bytes = arr
		@offset = off
	end
end

# arr comes in as whole codeblock, so we know its on the front of an opcode
def test_cache(c, arr)

	max_block = nil
	max_len = arr.length

	if arr.length - 3 < 1
		return nil, nil
	end

	c.each do |x|
		if x.bytes.length >= arr.length
			if x.bytes == arr
				#puts "Comparing \n#{x.bytes}\n#{arr}"
				return x.offset, 0
			end

			if x.bytes[(x.bytes.length - arr.length)..] == arr
				#puts "#{x.bytes}\n#{x.bytes[(x.bytes.length - arr.length)..]}\n#{arr}"
				return x.offset + (x.bytes.length - arr.length), 0
			end


			#offset = 0
			#while offset < arr.length - 3
			#	if x.bytes[((x.bytes.length - arr.length) + offset)..] == arr[offset..]
			#		if offset < max_len
			#			max_block = x
			#			max_len = offset
			#			#break
			#		end
			#		offset += 1
			#	else
			#		offset += 1
			#	end
			#end
		end
	end

	#if max_block != nil
		#puts "Sub Comparing\n#{max_block.bytes}\n#{arr}\n#{max_block.bytes[((max_block.bytes.length - arr.length) + max_len)..]}\n#{arr[max_len..]}"
		#return max_block.offset + (max_block.bytes.length - max_len), max_len
	#end

	return nil, nil
end

def output_game(g)
	cache = Array.new
	saved = 0


	filename = g.filename
	filename = File.basename(filename, File.extname(filename)) + ".rai"

	puts "Building #{g.game}"

	## prebuild item actions
	g.items.each do |k,i|

		if i.flags["take"] == true
			# generate get/drop code.
			code = $template_get.to_s
			code = code.gsub("@item_noun", "#{i.noun}")
			code = code.gsub("@item_description", "#{i.descr}")
			code = code.gsub("@item", "#{i.id}")
			toks_get = parse_lines(get_code_lines(code))

			code = $template_drop.to_s
			code = code.gsub("@item_noun", "#{i.noun}")
			code = code.gsub("@item_description", "#{i.descr}")
			code = code.gsub("@item", "#{i.id}")
			toks_drop = parse_lines(get_code_lines(code))

			code = $template_look.to_s
			code = code.gsub("@item_noun", "#{i.noun}")
			code = code.gsub("@item_description", "\"#{i.descr}\"")
			code = code.gsub("@item", "#{i.id}")
			toks_look = parse_lines(get_code_lines(code))

			action, index = parse_action(g, toks_get, 0)
			g.actions["#{action.unique_id}.get.#{i.noun}"] = action
			action, index = parse_action(g, toks_drop, 0)
			g.actions["#{action.unique_id}.drop.#{i.noun}"] = action
			action, index = parse_action(g, toks_look, 0)
			g.actions["#{action.unique_id}.look.#{i.noun}"] = action
		elsif i.flags["scenery"] == true
			# generate get/drop code.
			code = $template_look.to_s
			code = code.gsub("@item_noun", "#{i.noun}")
			code = code.gsub("@item_description", "\"#{i.descr}\"")
			code = code.gsub("@item", "#{i.id}")
			toks_look = parse_lines(get_code_lines(code))

			action, index = parse_action(g, toks_look, 0)
			g.actions["#{action.unique_id}.look.#{i.noun}"] = action
		end

	end


	hh = Hash.new
	g.strings.each do |k, v|
		g.string_id.each do |kk, gg|
			v = v.gsub("%#{kk}%", "#{gg}")
		end

		# fixes aliases in aliases.. need to fix but this is quick and dirty
		#hack
		g.string_id.each do |kk, gg|
			v = v.gsub("%#{kk}%", "#{gg}")
		end
		#hack
		g.string_id.each do |kk, gg|
			v = v.gsub("%#{kk}%", "#{gg}")
		end

		g.counters.each do |kk,gg|
			v = v.gsub("%#{kk}%", ("\x1" << gg.idx))
		end

		v = v.gsub("%max_carry%", "#{g.max_carry}")

		v = v.gsub("\\n", "\x4")
		v = v.gsub("%verb%", "\x2")
		v = v.gsub("%noun%", "\x3")

		hh[k.to_s] = XStr.new(v)
	end
	g.strings = hh

	hh = Hash.new
	g.strings.each do |k,v|
		z = v.str.split(' ')

		0.upto(z.length-1) do |vv|

			## test for missing variable/counter
			if z[vv][0] == '%' then
				if z[vv].split('%').length > 1 then
					puts "Error : Variable %#{z[vv][z[vv].split('%')[1]]}% undefined."
					puts "#{v.str}"
					exit
				end
			end

			kk = "" + z[vv].to_s
			hh[kk.to_s] = XStr.new(kk.to_s)
		end
	end

	xh = Hash.new
	xhh = hh.sort {|k,v| k[1].str <=> v[1].str}
	xhh.each do |k,v|
		xh[k] = hh[k]
	end
	hh = xh

	qStrings = Array.new()
	zStrings = Hash.new()
	hh.each do |k,v|
		qStrings << v.str
		zStrings["" + v.str] = "" + v.str
	end

	qStrings = qStrings.sort #{|x,y| y <=> x }

	if g.interp_version < 3
		qStrings.each do |s|
			idx = qStrings.index(s)
			idx2 = idx
			idx3 = 0

			z = qStrings[idx2].to_s.slice(0, s.length)
			while z == s
				idx3 = idx2
				if idx2 < qStrings.length
					idx2 = idx2 + 1
					z = qStrings[idx2].to_s.slice(0, s.length)
				end
			end

			if idx3 > idx
				#puts "match on #{s} for #{qStrings[idx3]}"
				zStrings["#{s}"] = "#{qStrings[idx3]}"
			end
		end

		qStrings = Array.new
		hh.each do |k,v|
			if v.str != zStrings[v.str] then
				#puts "#{v.str} as #{zStrings[v.str]}"
			else
				qStrings << zStrings[v.str]
			end
			v.nstring = zStrings[v.str]
			v.nlength = v.str.length
		end

		qStrings.sort
	else
		qStrings = Array.new
		hh.each do |k,v|
			v.nstring = v.str
			v.nlength = v.str.length
		end

		qStrings.sort
	end

	#puts "Total words : #{("000" + hh.length.to_s(16)).slice(-3,3).upcase}"
	if $verbose_flag > 0
		puts "Total words : #{("000" + qStrings.length.to_s(16)).slice(-3,3).upcase} (#{qStrings.length.to_s})"
	end

	fp = File.new(filename,"wb")

	fp.putc 'X'
	fp.putc 'A'
	fp.putc 'D'
	fp.putc 'V'

	## version
	fp.putc g.max_carry.to_i & 0xFF

	## required interp version
	fp.putc g.interp_version.to_i & 0xFF

	# string count
	fp.putc ((g.strings.length >> 8)&0xFF).to_i & 0xFF
	fp.putc (g.strings.length & 0xFF).to_i & 0xFF
	if $verbose_flag > 0
		puts "Generated #{g.strings.length} strings"
	end

	# room count
	fp.putc (g.rooms.length & 0xFF).to_i & 0xFF

	if $verbose_flag > 0
		puts "Generated #{g.rooms.length} rooms"
	end

	#player count
	fp.putc g.players.count
	if $verbose_flag > 0
		puts "Generated #{g.players.count} players"
		puts "Generated #{g.verbs.length} verbs"
		puts "Generated #{g.nouns.length} nouns"
		puts "Generated #{g.counters.length} counters"
		puts "Generated #{g.flags.length} flags"
	end

	while fp.tell < 0x20 do
		fp.putc 0x0 & 0xFF
	end

	s = g.string_id["game"]
	g.string_id.each do |kk, gg|
		s = s.gsub("%#{kk}%", "#{gg}")
	end
	s.bytes.each do |ss|
		fp.putc ss
	end
	while fp.tell < 0x40 do
		fp.putc 0x0 & 0xFF
	end
	fp.seek 0x40, IO::SEEK_SET


	s = g.string_id["author"]
	g.string_id.each do |kk, gg|
		s = s.gsub("%#{kk}%", "#{gg}")
	end
	s.bytes.each do |ss|
		fp.putc ss
	end
	while fp.tell < 0x60 do
		fp.putc 0x0 & 0xFF
	end
	fp.seek 0x60, IO::SEEK_SET


	s = g.string_id["version"]
	g.string_id.each do |kk, gg|
		s = s.gsub("%#{kk}%", "#{gg}")
	end
	s.bytes.each do |ss|
		fp.putc ss
	end
	while fp.tell < 0x70 do
		fp.putc 0x0 & 0xFF
	end
	fp.seek 0x70, IO::SEEK_SET



	# 0 XADV
	# 4 max carry
	# 5 req interp
	# 6 string count
	# 8 room count
	# 0x0A - string table offset
	# 0x0C - verbs offset
	# 0x0E - room data
	# 0x10 - item data
	# 0x12 - codes

	# output string table offset
	fp.seek 0x0A, IO::SEEK_SET
	fp.putc 0x00
	fp.putc 0x70

	fp.seek 0x70, IO::SEEK_SET

	# dry run calculation of dictionary size
	start_offs = fp.tell
	end_offs = start_offs

	hh.each do |k, s|
		if s.str == s.nstring
			end_offs += s.str.bytes.to_a.length
			# zero term word
			if g.interp_version > 2
				end_offs += 1
			end
		end
	end

	max_dict_size = 0xFFF
	if g.interp_version > 2
		max_dict_size = 0x7FFF
	end

	if end_offs >= max_dict_size then
		puts "************ STRING TABLE TO BIG FOR OFFSET SIZE OF 0x#{max_dict_size.to_s(16).upcase}. Calculated at #{"0x" + ("0000" + end_offs.to_s(16).upcase).slice(-4,4)}"
		fp.close
		File.delete(filename)
		exit
	end


	hh.each do |k, s|
		s.offs = fp.tell

		if s.str == s.nstring
			s.str.bytes.to_a.each do |x|
				fp.putc x & 0xFF
			end
			# zero term word
			if g.interp_version > 2
				fp.putc 0x00
			end
		end
	end

	if $verbose_flag > 0
		puts "Last string offset ends at #{"0x" + ("0000" + fp.tell.to_s(16).upcase).slice(-4,4)}. String Freespace to 0x#{max_dict_size.to_s(16).upcase} is #{"0x" + ("0000" + (max_dict_size - fp.tell).to_s(16).upcase).slice(-4,4)} (#{(max_dict_size - fp.tell)} bytes)"
	end

	if fp.tell >= max_dict_size then
		puts "************ STRING TABLE TO BIG FOR OFFSET SIZE OF 0x#{max_dict_size.to_s(16).upcase}"
		fp.close
		File.delete(filename)
		exit
	end

	# output code offset, round to 32byte boundary.
	pointer_round(fp)
	offs = fp.tell

	fp.seek 0x0C, IO::SEEK_SET
	fp.putc ((offs >> 8)&0xFF).to_i & 0xFF
	fp.putc (offs & 0xFF).to_i & 0xFF
	fp.seek 0, IO::SEEK_END

	g.verbs.each do |k, v|
		fp.putc v.idx & 0xFF
		out_cstring(v.id, fp)

		v.synonyms.each do |kk, kv|
			out_cstring(kv, fp)
		end
		fp.putc 0x0
	end
	fp.putc 0x0
	fp.putc 0x0


	pointer_round(fp)
	offs = fp.tell

	fp.seek 0x12, IO::SEEK_SET
	fp.putc ((offs >> 8)&0xFF).to_i & 0xFF
	fp.putc (offs & 0xFF).to_i & 0xFF
	fp.seek 0, IO::SEEK_END

	g.nouns.each do |k, v|
		# ID
		fp.putc v.idx & 0xFF

		# main noun
		out_cstring(v.id, fp)

		# all synonyms
		v.synonyms.each do |kk, kv|
			out_cstring(kv, fp)
		end

		# end of noun maker
		fp.putc 0x0
	end
	#end of end of nouns.
	fp.putc 0x0
	fp.putc 0x0



	pointer_round(fp)
	offs = fp.tell

	fp.seek 0x0A, IO::SEEK_SET
	fp.putc (offs>>8) & 0xFF
	fp.putc (offs&0xFF)

	fp.seek 0x00, IO::SEEK_END

	# +1 covers the final marker for non-existant string, allows
	# us to work out total string block size.
	offs = fp.tell + ((g.strings.length+1) * 2)
	g.strings.each do |k, s|
		fp.putc ((offs >> 8)&0xFF).to_i & 0xFF
		fp.putc (offs & 0xFF).to_i & 0xFF

		z = s.str.split(' ')

		0.upto(z.length-1) do |vv|
			kk = "" + z[vv].to_s

			if g.interp_version > 2 then
				#kz = hh[kk].nstring
				#klen = hh[kk].nlength
				koffs = hh[kk].offs

				#if klen >= 16 then
				#	s.strx << 0xF000 + (koffs & 0x0F00) + (koffs & 0xFF)
				#	s.strx << (((koffs + 15) & 0x0F00) + ((klen-15) << 12)) + ((koffs + 15) & 0xFF)
				#else
					s.strx << (koffs & 0xFFFF)
				#end
			else
				kz = hh[kk].nstring
				klen = hh[kk].nlength
				koffs = hh[hh[kk].nstring].offs

				if klen >= 16 then
					s.strx << 0xF000 + (koffs & 0x0F00) + (koffs & 0xFF)
					s.strx << (((koffs + 15) & 0x0F00) + ((klen-15) << 12)) + ((koffs + 15) & 0xFF)
				else
					s.strx << ((koffs & 0x0F00) + (klen << 12)) + (koffs & 0xFF)
				end
			end
		end
		#s.strx << 0
		l = s.strx.length
		offs += l*2
	end

	# put last marker down so we know overall length.
	fp.putc ((offs >> 8)&0xFF).to_i & 0xFF
	fp.putc (offs & 0xFF).to_i & 0xFF

	g.strings.each do |k, s|
		s.strx.each do |x|
			fp.putc (x >> 8) & 0xFF
			fp.putc x & 0xFF
		end
	end

	pointer_round(fp)
	offs = fp.tell

	fp.seek 0x0E, IO::SEEK_SET
	fp.putc ((offs >> 8)&0xFF).to_i & 0xFF
	fp.putc (offs & 0xFF).to_i & 0xFF
	fp.seek 0, IO::SEEK_END

	room_offsx = fp.tell

	g.rooms.each do |k,r|
		fp.putc 0
		fp.putc 0
	end

	g.rooms.each do |k,r|
		if $verbose_flag > 0
			puts "Compiling room #{r.idx} #{k}"
		end

		fp.seek 0, IO::SEEK_END
		offs = fp.tell
		fp.seek room_offsx, IO::SEEK_SET
		fp.putc offs >> 8
		fp.putc offs & 0xFF
		fp.seek 0, IO::SEEK_END

		room_offsx += 2

		fp.putc r.idx & 0xFF

		fp.putc (g.strings[r.title].id >> 8) & 0xFF
		fp.putc g.strings[r.title].id & 0xFF

		offsx = fp.tell

		# 10 directions + look + enter
		0.upto 25 do |xx|
			fp.putc 0
		end

		r.codeblocks.each do |k,cb|
			#puts "Compiling room code blocks #{r.id}.#{k}"
			cb.compile(g)
			#cb.print_code()

			if k == "enter"
				offs = offsx + 0
			elsif k == "look"
				offs = offsx + 2
			elsif k == "n"
				offs = offsx + 4
			elsif k == "s"
				offs = offsx + 6
			elsif k == "e"
				offs = offsx + 8
			elsif k == "w"
				offs = offsx + 10
			elsif k == "ne"
				offs = offsx + 12
			elsif k == "nw"
				offs = offsx + 14
			elsif k == "se"
				offs = offsx + 16
			elsif k == "sw"
				offs = offsx + 18
			elsif k == "u"
				offs = offsx + 20
			elsif k == "d"
				offs = offsx + 22
			else
				puts "***** Unknown room codeblock! Only enter or directions allowed. Not #{k}"
				fp.close
				File.delete(filename)
				exit
			end

			fp.seek 0, IO::SEEK_END
			offsz = fp.tell

			fp.seek offs, IO::SEEK_SET
			fp.putc offsz >> 8
			fp.putc offsz & 0xFF

			fp.seek 0, IO::SEEK_END

			if k == "enter"
				fp.putc 0xFF
			else
				zx = scan_verb(g, k)
				fp.putc scan_verb(g,k)
			end

			q_index, q_offset = test_cache(cache, cb.bytecode)
			if q_index != nil

				if q_offset > 0
					for i in 1..q_offset
						fp.putc cb.bytecode[i-1] & 0xFF
					end
				end

				fp.putc Opcode::X_BYTECODE_GOTO
				fp.putc q_index >> 8
				fp.putc q_index & 0xFF
				saved += (cb.bytecode.length - q_offset) - 3
			else
				cache << Cached_Block.new(fp.tell, cb.bytecode)
				cb.bytecode.each do |bb|
					fp.putc bb.to_i & 0xFF
				end
			end
		end
	end

	pointer_round(fp)
	offs = fp.tell

	fp.seek 0x10, IO::SEEK_SET
	fp.putc ((offs >> 8)&0xFF).to_i & 0xFF
	fp.putc (offs & 0xFF).to_i & 0xFF
	fp.seek 0, IO::SEEK_END

	g.items.each do |k,i|
		if $verbose_flag > 0
			puts "Compiling item #{i.idx} #{k}"
		end

		fp.putc i.idx

		if g.rooms[ i.location ] == nil
			puts "***** Item #{i.id} points to non existant room #{i.location}"
			fp.close
			File.delete(filename)
			exit
		end

		# current and for where it goes on reset
		fp.putc g.rooms[ i.location ].idx

		flags = 0
		if i.flags["light"] == true
			flags |= 0x01
		end
		if i.flags["scenery"] == true
			flags |= 0x02
		end
		if i.flags["player"] == true
			flags |= 0x04
		end

		if i.flags["take"] == true
			# done...
		end

		fp.putc flags

		if g.nouns[i.noun] == nil
			puts "***** Item #{i.id} has no noun"
			fp.close
			File.delete(filename)
			exit
		end
		fp.putc g.nouns[ i.noun ].idx

		fp.putc g.strings[ i.name.to_s ].id >> 8
		fp.putc g.strings[ i.name.to_s ].id & 0xFF
	end

	g.players.each do |k,i|
		if $verbose_flag > 0
			puts "Compiling player #{i.idx} #{k}"
		end

		fp.putc i.idx

		#puts "***** #{i.name} Location #{i.location} #{g.rooms[ i.location ].idx}"
		#puts "***** #{i.name} Inventory #{i.inventory} #{g.rooms[ i.inventory ].idx}"

		if g.rooms[ i.location ] == nil
			puts "***** Player Inventory #{i.id} points to non existant room #{i.location}"
			fp.close
			File.delete(filename)
			exit
		end

		if g.rooms[ i.inventory ] == nil
			puts "***** Player Inventory #{i.id} points to non existant room #{i.inventory}"
			fp.close
			File.delete(filename)
			exit
		end

		# current and for where it goes on reset
		fp.putc g.rooms[ i.location ].idx

		fp.putc g.rooms[ i.inventory ].idx

		#if i.id != "player" && g.nouns[i.noun] == nil
		#	puts "***** Player #{i.id} has no noun"
		#	exit
		#end

		#if i.id != "player"
			fp.putc g.nouns[ i.noun ].idx
		#else
		#	fp.putc 0
		#end

		fp.putc g.strings[ i.name.to_s ].id >> 8
		fp.putc g.strings[ i.name.to_s ].id & 0xFF
	end


	pointer_round(fp)
	offs = fp.tell

	fp.seek 0x14, IO::SEEK_SET
	fp.putc ((offs >> 8)&0xFF).to_i & 0xFF
	fp.putc (offs & 0xFF).to_i & 0xFF
	fp.seek 0, IO::SEEK_END


	gc_offs = fp.tell

	16.times do
		fp.putc 0
	end

	#generate RESET codeblock
	block = CodeBlock.new("reset")
		g.flags.each do |k,c|
			x = CB_SetFlag.new
			x.flag = c.id
			x.val = false
			block.codes << x
		end

		g.counters.each do |k,c|
			x = CB_SetCounter.new
			x.cnt = c.id
			x.val = 0
			block.codes << x
		end

		g.players.each do |k,r|
			x = CB_SetPlayerInventory.new
			x.room = r.inventory
			x.player = r.id
			block.codes << x
		end

		g.players.each do |k,r|
			x = CB_Transport.new
			x.room = r.location
			x.player = r.id
			block.codes << x
		end

		g.items.each do |k,r|
			if r.id == "any"
				# dont touch system nouns
			else
				x = CB_Move.new
				x.room = r.location
				x.item = r.id
				block.codes << x
			end
		end

	block.codes << CB_EndOpcodes.new
	g.codeblocks["reset"] = block

	g.codeblocks.each do |k,gc|
		#puts "Compiling global codeblocks #{k}"
		if $verbose_flag > 0
			puts "Compiling global codeblocks #{k}"
		end

		fp.seek 0, IO::SEEK_END
		offs = fp.tell

		if k == "reset"
			gc_offsx = gc_offs + 0
		elsif k == "pregame"
			gc_offsx = gc_offs + 2
		elsif k == "on_success"
			gc_offsx = gc_offs + 4
		elsif k == "on_fail"
			gc_offsx = gc_offs + 6
		elsif k == "prompt"
			gc_offsx = gc_offs + 8
		elsif k == "on_gamewin"
			gc_offsx = gc_offs + 10
		elsif k == "on_gamefail"
			gc_offsx = gc_offs + 12
		elsif k == "in_dark"
			gc_offsx = gc_offs + 14
		else
			puts "***** Unknown key for global codeblock #{k}"
			fp.close
			File.delete(filename)
			exit
		end

		fp.seek gc_offsx, IO::SEEK_SET
		fp.putc (offs >> 8) & 0xFF
		fp.putc offs  & 0xFF
		fp.seek 0, IO::SEEK_END

		gc.codes.each do |kk|
			kk.compile(g)
			kk.bytecode.each do |bb|
				fp.putc bb.to_i & 0xFF
			end
		end
	end

	pointer_round(fp)
	offs = fp.tell

	fp.seek 0x16, IO::SEEK_SET
	fp.putc ((offs >> 8)&0xFF).to_i & 0xFF
	fp.putc (offs & 0xFF).to_i & 0xFF
	fp.seek 0, IO::SEEK_END

	if $verbose_flag > 0
		puts "Generated #{g.actions.length} actions"
	end

	glist = Array.new
	g.actions.each do |k,cb|
		cb.codeblock.compile(g)
		glist << cb.codeblock.bytecode.length
	end

	glist = glist.sort_by { |x| -x }
	glist = glist.uniq

	glist.each do |glist_size|
		g.actions.each do |k,cb|
			if cb.codeblock.bytecode.length == glist_size
				if cb.noun != "any" then
					if $verbose_flag > 0
						puts "Compiling action #{k}"
					end
					#cb.codeblock.compile(g)
					#cb.codeblock.print_code()

					if g.verbs[cb.verb] == nil then
						puts "***** Missing verb '#{cb.verb}'"
						fp.close
						File.delete(filename)
						exit
					end

					if g.nouns[cb.noun] == nil then
						puts "***** Missing item/noun '#{cb.noun}'"
						fp.close
						File.delete(filename)
						exit
					end

					fp.putc g.verbs[cb.verb].idx
					fp.putc g.nouns[cb.noun].idx

					q_index, q_offset = test_cache(cache, cb.codeblock.bytecode)
					if q_index != nil
						if q_offset > 0
							qqq = q_offset + 3
							fp.putc qqq >> 8
							fp.putc qqq & 0xFF

							for i in 1..q_offset
								fp.putc cb.codeblock.bytecode[i-1] & 0xFF
							end
						else
							qqq = 3
							fp.putc qqq >> 8
							fp.putc qqq & 0xFF
						end

						fp.putc Opcode::X_BYTECODE_GOTO
						fp.putc q_index >> 8
						fp.putc q_index & 0xFF
						saved += (cb.codeblock.bytecode.length - q_offset) - 3
					else
						fp.putc cb.codeblock.bytecode.length >> 8
						fp.putc cb.codeblock.bytecode.length & 0xFF

						cache << Cached_Block.new(fp.tell, cb.codeblock.bytecode)
						cb.codeblock.bytecode.each do |bb|
							fp.putc bb.to_i & 0xFF
						end
					end
				end
			end
		end
	end

	glist.each do |glist_size|
		g.actions.each do |k,cb|
			if (cb.codeblock.bytecode.length == glist_size)
				if cb.noun == "any" then
					if $verbose_flag > 0
						puts "Compiling action #{k}"
					end
					#cb.codeblock.compile(g)
					#cb.codeblock.print_code()

					if g.verbs[cb.verb] == nil then
						puts "***** Missing verb '#{cb.verb}'"
						fp.close
						File.delete(filename)
						exit
					end

					if g.nouns[cb.noun] == nil then
						puts "***** Missing item/noun '#{cb.noun}'"
						fp.close
						File.delete(filename)
						exit
					end

					fp.putc g.verbs[cb.verb].idx
					fp.putc g.nouns[cb.noun].idx

					q_index, q_offset = test_cache(cache, cb.codeblock.bytecode)
					if q_index != nil
						qqq = q_offset + 3#(cb.codeblock.bytecode.length - q_offset) + 3
						fp.putc qqq >> 8
						fp.putc qqq & 0xFF

						if q_offset > 0
							for i in 1..q_offset
								fp.putc cb.codeblock.bytecode[i-1] & 0xFF
							end
						end

						fp.putc Opcode::X_BYTECODE_GOTO
						fp.putc q_index >> 8
						fp.putc q_index & 0xFF
						saved += (cb.codeblock.bytecode.length - q_offset) - 3
					else
						fp.putc cb.codeblock.bytecode.length >> 8
						fp.putc cb.codeblock.bytecode.length & 0xFF

						cache << Cached_Block.new(fp.tell, cb.codeblock.bytecode)

						cb.codeblock.bytecode.each do |bb|
							fp.putc bb.to_i & 0xFF
						end
					end
				end
			end
		end
	end

	fp.putc 0x0

	fp.seek 0x18, IO::SEEK_SET
	fp.putc g.counters.length		#0x18
	fp.putc g.items.length			#0x19
	# round flags up into bytes
	fp.putc g.flags.length			#0x1A
	fp.putc 0						#0x1B

	fp.seek 0, IO::SEEK_END

	xtotal_length = fp.tell


	total_length = 1 + (xtotal_length / 1024);

	fp.seek 0x1C, IO::SEEK_SET
	fp.putc total_length >> 8
	fp.putc total_length & 0xFF

	puts "Cache blocking saved #{saved} bytes"
	if $verbose_flag > 0
		puts "Game file requires (#{xtotal_length} vs #{xtotal_length+saved}) #{total_length} kb (0x#{total_length.to_s(16).upcase})"
	end

	fp.close
end

def parse_game_file(filename, token_stream)
	g = Game.new

	g.filename = filename

	g.nouns["ANY".to_s.downcase] = Noun.new("ANY")

	g.rooms["void".to_s] = Room.new("void")
	g.rooms["void"].title = "Void"
	g.strings["Void"] = "Void"

	g.rooms["destroyed".to_s] = Room.new("destroyed")
	g.rooms["destroyed"].title = "Destroyed"
	g.strings["Destroyed"] = "Destroyed"

	g.rooms["inventory".to_s] = Room.new("inventory")
	g.rooms["inventory"].title = "Inventory"
	g.strings["Inventory"] = "Inventory"

	p = Player.new("player")
	p.inventory = "inventory"
	p.location = "void"
	p.name = "Player"
	p.noun = "player"

	g.players[p.id] = p
	g.nouns["player"] = Noun.new(p.noun)
	g.strings[p.name] = p.name

	g.items["ANY".to_s.downcase] = Item.new("ANY")
	g.items["ANY".to_s.downcase].noun = "ANY".to_s.downcase
	g.items["ANY".to_s.downcase].name = "ANY".to_s.downcase
	#g.items["ANY".to_s.downcase].synonyms["ANY".to_s.downcase] = "ANY".to_s.downcase
	g.strings["any"] = "any"

	g.verbs["__VERB__".to_s.downcase] = Verb.new("__VERB__")

	g.verbs["north".to_s] = Verb.new("north")
	g.verbs["south".to_s] = Verb.new("south")
	g.verbs["east".to_s] = Verb.new("east")
	g.verbs["west".to_s] = Verb.new("west")
	g.verbs["northeast".to_s] = Verb.new("northeast")
	g.verbs["northwest".to_s] = Verb.new("northwest")
	g.verbs["southeast".to_s] = Verb.new("southeast")
	g.verbs["southwest".to_s] = Verb.new("southwest")
	g.verbs["up".to_s] = Verb.new("up")
	g.verbs["down".to_s] = Verb.new("down")

	g.verbs["north".to_s].synonyms["n"] = "n"
	g.verbs["south".to_s].synonyms["s"] = "s"
	g.verbs["east".to_s].synonyms["e"] = "e"
	g.verbs["west".to_s].synonyms["w"] = "w"
	g.verbs["northeast".to_s].synonyms["ne"] = "ne"
	g.verbs["northwest".to_s].synonyms["nw"] = "nw"
	g.verbs["southeast".to_s].synonyms["se"] = "se"
	g.verbs["southwest".to_s].synonyms["sw"] = "sw"
	g.verbs["up".to_s].synonyms["u"] = "u"
	g.verbs["down".to_s].synonyms["d"] ="d"

	g.counters["inventory_count".to_s.downcase] = Counter.new("inventory_count".to_s.downcase)
	g.counters["score".to_s.downcase] = Counter.new("score".to_s.downcase)
	g.counters["moves".to_s.downcase] = Counter.new("moves".to_s.downcase)

	state = 0
	index = 0

	while state == 0 && index < token_stream.length
		if token_stream[index].downcase == "game"
			g.game = token_stream[index + 1]
			g.string_id["game"] = destring(token_stream[index + 1].to_s)
			g.strings[destring(token_stream[index + 1].to_s)] = destring(token_stream[index + 1].to_s)
			index += 2

		elsif token_stream[index].downcase == "author"
			g.author = token_stream[index + 1]
			g.string_id["author"] = destring(token_stream[index + 1].to_s)
			g.strings[destring(token_stream[index + 1].to_s)] = destring(token_stream[index + 1].to_s)
			index += 2

		elsif token_stream[index].downcase == "version"
			g.version = token_stream[index + 1]
			g.string_id["version"] = destring(token_stream[index + 1].to_s)
			g.strings[destring(token_stream[index + 1].to_s)] = destring(token_stream[index + 1].to_s)
			index += 2

		elsif token_stream[index].downcase == "requires_interpreter"
			g.interp_version = token_stream[index + 1].to_i
			index += 2

			if $force_game_version > 0 && $force_game_version > g.interp_version
				#puts "Force game version #{$force_game_version} greater than game file requirement #{g.interp_version}."
			end

			if $force_game_version > 0 && $force_game_version < g.interp_version
				puts "Force game version #{$force_game_version} less than game file requirement #{g.interp_version}."
			end

			if $force_game_version > 0 && $force_game_version > g.interp_version
				g.interp_version = $force_game_version
			end

		elsif token_stream[index].downcase == "max_carry"
			g.max_carry = token_stream[index + 1]
			index += 2

		elsif token_stream[index].downcase == "counter"
			if nil == g.counters[token_stream[index + 1].downcase]
				if $verbose_flag > 0
					puts "Adding counter #{token_stream[index + 1].to_s.downcase}"
				end
				g.counters[token_stream[index + 1].to_s.downcase ] = Counter.new(token_stream[index + 1].to_s.downcase)
			else
				puts "Counter \"" + token_stream[index + 1] + "\" already exists"
			end
			index += 2

		elsif token_stream[index].downcase == "flag"
			if nil == g.flags[token_stream[index + 1].downcase]
				g.flags[token_stream[index + 1].to_s.downcase ] = Flag.new(token_stream[index + 1].to_s.downcase)
			else
				puts "Flag \"" + token_stream[index + 1] + "\" already exists"
			end
			index += 2

		elsif token_stream[index].downcase == "verb"
			index += 1
			vname = token_stream[index].to_s

			if g.verbs[vname] == nil then
				g.verbs[vname] = Verb.new(vname)
			end

			index += 1

			while token_stream[index] != "<<EOC>>"
				if token_stream[index] != ","
					puts "Missing comma separator in verb definition \"" + vname + "\""
					exit
				end

				index += 1

				g.verbs[vname].synonyms[token_stream[index].to_s] = token_stream[index].to_s
				index += 1
			end

		elsif token_stream[index].downcase == "pregame"
			index += 2 # skip EOC
			block, index = parse_code_block(g, token_stream, index, "pregame")
			block.codes << CB_EndOpcodes.new
			g.codeblocks[block.id] = block

		elsif token_stream[index].downcase == "player"
			pl, index = parse_player(g, token_stream, index)
			g.players[pl.id] = pl

		elsif token_stream[index].downcase == "in_dark"
			index += 2 # skip EOC
			block, index = parse_code_block(g, token_stream, index, "in_dark")
			block.codes << CB_EndOpcodes.new
			g.codeblocks[block.id] = block

		elsif token_stream[index].downcase == "on_success"
			index += 2 # skip EOC
			block, index = parse_code_block(g, token_stream, index, "on_success")
			block.codes << CB_EndOpcodes.new
			g.codeblocks[block.id] = block

		elsif token_stream[index].downcase == "on_fail"
			index += 2 # skip EOC
			block, index = parse_code_block(g, token_stream, index, "on_fail")
			block.codes << CB_EndOpcodes.new
			g.codeblocks[block.id] = block

		elsif token_stream[index].downcase == "prompt"
			index += 2 # skip EOC
			block, index = parse_code_block(g, token_stream, index, "prompt")
			block.codes << CB_EndOpcodes.new
			g.codeblocks[block.id] = block

		elsif token_stream[index].downcase == "on_gamewin"
			index += 2 # skip EOC
			block, index = parse_code_block(g, token_stream, index, "on_gamewin")
			block.codes << CB_EndOpcodes.new
			g.codeblocks[block.id] = block

		elsif token_stream[index].downcase == "on_gamefail"
			index += 2 # skip EOC
			block, index = parse_code_block(g, token_stream, index, "on_gamefail")
			block.codes << CB_EndOpcodes.new
			g.codeblocks[block.id] = block

		elsif token_stream[index].downcase == "room"
			room, index = parse_room(g, token_stream, index)
			g.rooms[room.id] = room

		elsif token_stream[index].downcase == "action"
			action, index = parse_action(g, token_stream, index)
			g.actions["#{action.unique_id}.#{action.verb}.#{action.noun}"] = action

		elsif token_stream[index].downcase == "noun"
			index += 1
			vname = token_stream[index].to_s

			if g.nouns[vname] == nil then
				g.nouns[vname] = Noun.new(vname)
			end

			index += 1

			while token_stream[index] != "<<EOC>>"
				if token_stream[index] != ","
					puts "Missing comma separator in noun definition \"" + vname + "\""
					exit
				end

				index += 1

				g.nouns[vname].synonyms[token_stream[index].to_s] = token_stream[index].to_s
				index += 1
			end

		elsif token_stream[index].downcase == "item"
			item, index = parse_item(g, token_stream, index)
			g.items[item.id] = item

		elsif token_stream[index].downcase == "<<eoc>>"
			##skip
		else
			#puts "Main Unknown token " + token_stream[index] + "."
			#exit
			g.string_id[token_stream[index].to_s] = destring(token_stream[index + 1].to_s)
			g.strings[destring(token_stream[index + 1].to_s)] = destring(token_stream[index + 1].to_s)
			#puts "Creating alias for #{token_stream[index]} as #{token_stream[index+1]}"
			index += 2
		end

		if token_stream[index] == "<<EOC>>"
			index += 1
		else
			puts "Something out of whack around " + token_stream[index] + ", " + token_stream[index-1]
			exit
		end
	end

	g.string_id["version"] = destring(g.version)
	g.string_id["author"] = destring(g.author)
	g.string_id["game"] = destring(g.game)

	test_data(g)

	output_game(g)
end

def main(filename)
	# parse all lua files for kw_ keys

	lfiles = filename

	if $verbose_flag > 0
		puts "Running Compiler on #{lfiles}"
		if $force_game_version > 0
			puts "Forcing game version #{$force_game_version}"
		end
	end

	tokstream = parse_lines(IO.readlines(lfiles))

	parse_game_file(lfiles, tokstream)
end



if __FILE__ == $0
	puts "Retro Adventure game compiler"

	i = 0;
	while i < ARGV.length
	#ARGV.each do |x|
		x = ARGV[i].to_s

		if x == "-v"
			$verbose_flag = 1
		elsif x == "-x"
			$enable_action_any = 1
		elsif x == "-z"
			$create_getdrop_all = 1
		elsif x == "-f"
			i += 1
			x = ARGV[i].to_s
			$force_game_version = x.to_i
		elsif x == "-?" or x == "-h"
			puts "syntax; compiler.rg [-v] inputfile"
			puts "-x  Enable generation of action ANY commands when missing"
			puts "-z  Create a get all / drop all action"
			puts "-f# Force game file version (1, 2, 3)"
			puts "-v  Verbose"
			exit
		else
			$input_file = x
		end

		i += 1
	end

	if $input_file.length > 0
		if File.exist?($input_file)
			main($input_file)

			if $verbose_flag > 0
				puts ""
			end
		end
	else
		puts "syntax; compiler.rg [-v -x] inputfile"
	end
end
