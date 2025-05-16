#include "simple_logger.h"


#include "moves.h"
	//  *Battle will be in charge of drawing any move (or dialogue,,>?  most likely by calling text_draw on each line)


//YEAH DO NOT INCLUDE ANY  ENTITIES IN HERE LMAO  Move --> Ent --> Player, Monster, NPC

//oh christ.

static SJson* _moveJson = NULL; //Global variable to store the entire JSon file 'moves.def'
static SJson* _moveDefs = NULL; //Global variable to store the JSon list of 'moves':
static int numMoves;

void moves_close();

void move_masterlist_initialize(const char* filename) {
	if (!filename) {
		slog("no filename provided for move list initialization");
		return;
	}
	_moveJson = sj_load(filename);  //Making a JSON object out of the entire file
	if (!_moveJson) {
		slog("Failed to load the json for move list");
		return;
	}
	//SPECIFICALLY extracting the masterlist of Moves:
	_moveDefs = sj_object_get_value(_moveJson, "moves");	//A JSon object/list containing our masterlist of Move Objects
	if (!_moveDefs) {
		slog("Move Def file '%s' does not contain a list of moves", filename);
		sj_free(_moveJson);
		_moveJson = NULL;
		return;
	}

	numMoves = sj_array_get_count(_moveDefs);

	slog("Masterlist of moves initialized");
	atexit(moves_close);
}

void moves_close() {
	
	//do NOT have move_system_close() here.  I have atexit functions to take care of it
	
	if (_moveJson) {
		sj_free(_moveJson);
	}
	_moveJson = NULL;
	_moveDefs = NULL; //Defs is only a POINTER to a JSON object I sj_load'ed.  And thus, it needn't be sj_free'd
	slog("Moves List successfully closed");
}

//===========================================================

//	Sub system  and  MasterList (like my items)  will be separate.  the cleanup function of one should NOT call the other,  it's fine

//===========================================================
typedef struct {  //our singleton system/manager
	Uint32			move_system_max;
	Move			*move_system_list;		//this will be a POINTER  to a list of all the moves we plan to create (and thus, of course, configure)

}MoveSystem;

static MoveSystem move_sub_system = { 0 };

void move_system_close();
void move_free(Move* self);

// Calls _free_all()   and free()'s the .move_system_list.  Done.
void move_system_close() {
	if (move_sub_system.move_system_list != NULL)  //if our system still has elements in it  (if the pointer does not point to NULL)
	{ //free all the entities
		moves_free_all(); //J SPECIFIC. When I want to clear absolutely everything ('cause I'm shutting down the game), pass NULL to the ignore
		free(move_sub_system.move_system_list); //free the dynamically allocated list itself
		slog("All Moves in the Move Sub System's List Freed");
	}
	move_sub_system.move_system_list = NULL; //reset the entity list pointer
	memset(&move_sub_system, 0, sizeof(MoveSystem)); //completely unecessary.  This doesn't free nor clean up anything. Just sets the data there to be 0
	slog("Move system closed");
}

//Done.   Just allocates .move_system_list ;   atexit(move_system_close)
void move_system_init(Uint32 maxMoves) {
	if (move_sub_system.move_system_list) {
		slog("Cannot initialize more than one Move System/List. One is already active");
		return;
	}
	if (!maxMoves) {
		slog("Cannot initialize move system for zero elements");
		return;
	}

	move_sub_system.move_system_list = gfc_allocate_array(sizeof(Move), maxMoves);	//highkey double check the video for this honestly  just 'cause I wanna be sure I'm giving it 
	if (!move_sub_system.move_system_list) {
		slog("Failed to allocate a move_system_list");
	}
	move_sub_system.move_system_max = maxMoves;
	atexit(move_system_close);
	slog("Move sub-system initialized");
}


void moves_free_all() {  //JUST iterates and calls  move_free()
	int i;
	for (i = 0; i < move_sub_system.move_system_max; i++) {
		if (move_sub_system.move_system_list[i]._inuse) {
			move_free(&move_sub_system.move_system_list[i]); //free it so long as it's NOT the ignore pionter, && if it's _inuse
		}
	}
}


void move_free(Move* self) {
	if (!self || !self->_inuse) return;


	//any cleanup that's common to all  Move objects would go here...  if ever I add smth like that

	//don't have to worry about freeing names because they are GFC_TextLines  :D	bless up the humble character array



	//Based on the type of the Move:
	if (self->type & MOVET_ATTACK) {
		//slog("Freeing Attacking Move's sprite");
		//Free the sprite
		if (self->m.attack.sprite) {
			//slog("Actually freed sprite :)");
			gf2d_sprite_free(self->m.attack.sprite);
		}
		
	}
	else { slog("uhhh heuston, don't panic  but the Move we're freeing isn't an attack,,,"); }

	//Add  Converse here later  (freeing text,,)

	//slog("Move Freed");
	//Just to set the rest of the memory that used to be in use   to 0.  
	memset(self, 0, sizeof(Move));
	self->_inuse = 0;
}


//Done. Does not initialize the data members. Just returns pointer to the space in our list
Move* move_create() {
	int i;
	for (i = 0; i < move_sub_system.move_system_max; i++) {
		if (move_sub_system.move_system_list[i]._inuse)  //If this entity (from the list our pointer points to)  IS in use (_inuse flag is == 1)
			continue;
		memset(&move_sub_system.move_system_list[i], 0, sizeof(Move));   //Set the memory allocated at this spot to 0

		move_sub_system.move_system_list[i]._inuse = 1;					//Set the inuse flag to 1.
		return &move_sub_system.move_system_list[i];
	}
	slog("Failed to allocate new Move: list full");
	return NULL; //return NULL outside the for loop
}



//====================================			CONFIGURATION
void attack_configure(Attack_Move* self, SJson* json) {
	if ((!self) || (!json)) { slog("No JSON object (OR BUTTON) provided to create the image"); return; }

	int atkPow;

	sj_object_get_int(json, "attackPower", &atkPow);
	self->attackPower = atkPow;
	//well. this is easy. it's just sprite at a very specific location within our window
	const char* sprite = NULL;
	sprite = sj_object_get_string(json, "sprite");
	if (sprite) { //if the Window itself has a sprite
		//slog("This images's Sprite is '%s'", sprite);
		GFC_Vector2D i_sp_sz = { 0 };
		sj_object_get_vector2d(json, "sprite_size", &i_sp_sz);  //function from gfc_config.h
		//slog("Sprite Size vector fetched from json object's x: %i", (Uint32)i_sp_sz.x);

		Uint32 framesPerLine;
		sj_object_get_int32(json, "spriteFPL", &framesPerLine);
		//load the sprite into the object's sprite data member
		self->sprite = gf2d_sprite_load_all(
			sprite,
			(Uint32)i_sp_sz.x,
			(Uint32)i_sp_sz.y,
			framesPerLine,
			1);
		if (!self->sprite->surface) { slog("Attack's sprite does not have a sprite SURFACE"); }
		//slog("Button Sprite's Frame Height is: %i", self->sprite->frame_h);
		self->sprite_size = i_sp_sz;
		self->center = gfc_vector2d(i_sp_sz.x * 0.5, i_sp_sz.y * 0.5);
		self->framesPerLine = framesPerLine;
		self->frame = 0;

		GFC_Vector4D bounds;
		sj_object_get_vector4d(json, "bounds", &bounds);
		self->bounds = gfc_rect_from_vector4(bounds);
	}
	else { slog("Attack Sprite couldn't be found"); }
}


void move_configure(Move* self, SJson* json) {
	if ((!self) || (!json)) return;

	char* move_name = sj_object_get_string(json, "name");
	if (move_name) {
		gfc_line_cpy(self->name, move_name);  //something something  Copy A into B  and make sure it's not longer than the length of a Line.
		//slog("Name copied: %s",move_name);
	}

	//Get the TYPE  first.  then depending on the type,  call either  Label_configure  or button configure

	const char* move_type = sj_object_get_string(json, "type");
	if (move_type) {
		if (gfc_strlcmp(move_type, "attack") == 0) {
			//slog("Fierce entity found: %s", self->name);
			self->type = MOVET_ATTACK;
		}
		else if (gfc_strlcmp(move_type, "converse") == 0) {
			self->type = MOVET_CONVERSE;

		}
	}
	else {
		self->type = MOVET_NONE;
	}

	if (self->type == MOVET_NONE) {
		slog("Tried to get the Move's type, but none matched. Not configuring.");
		return;
	}

	if (self->type & MOVET_ATTACK) {
		//slog("This Move is an Attack move");
		attack_configure(&self->m.attack, json);
		//self->elem_draw = label_draw;				//think function ??
	}

	if (self->type & MOVET_CONVERSE) {
		slog("This Move is a Conversation move");
	}

	if (self->type & MOVET_TAME) {
		slog("lmfao what am I even doing with a Tame move...");

	}

}


//Just call it in game.c  for testing.    You can figure out the order of operations to call IT  AND the subsystem  in move_list_init  later..
void configure_all_moves() {
	slog("The number of moves as per what was loaded from the '_moveDefs' array is: %i", numMoves);
	int i;
	Move* move;
	SJson* moveDef;

	for (i = 0; i < numMoves; i++) {
		move = move_create();
		moveDef = sj_array_get_nth(_moveDefs, i);  //for EVERY Object in the masterlist array:

		move_configure(move, moveDef);  //Sooo   my masterlist which is move_sub_system's move_system_list,  is getting allocated by move_Create,  and al lthose allocated spots are being filled in with relevant data

		//if (move->type == MOVET_NONE) slog("Move '%s' was not properly configured. It has no type",move->name);

		//if(move->type != MOVET_NONE) slog("Just configured the %i'th move. The name of this move's attack is: %s", i, move->name);
	}
	slog("Done configuring all moves");

}


Move* get_move_by_name(const char* move_name) {
	Move* move;
	int i;

	for (i = 0; i < move_sub_system.move_system_max; i++) {
		move = &move_sub_system.move_system_list[i];  //use a shorter variable name,, my goodness
		if (!move->_inuse) continue;  //if not in use, continue

		if (gfc_strlcmp(move_name, move->name) == 0) { //Length compare to forgo matching ONLY on a substring
			if (move->type == MOVET_NONE) {
				slog("The move '%s' has not been configured properly.. not appending it to the list");
				continue;
			}
			//slog("Found the JSON object for item '%s' on iteration %i", name, i);	//Jlog
			return move; //found it
		}

	}
	slog("Move by name %s could not be found in the currently allocated list.", move_name);
	return NULL;
}


//Now that all the Move objects have been previously configured.  Just search through the list USING the Names  and append that Move object to the MoveList
void configure_moves_for_ent(GFC_List* listOfMoveNames, GFC_List* entMoveList) {
	if (!listOfMoveNames || listOfMoveNames->count < 1) { slog("The provided moveList does not exist, or has a count less than 1"); return; }
	int i, c;
	Move* move;		//Pointer to a place in memory - where a new move is allocated every iteration in the for loop down there
	SJson* moveDef;
	const char* nameOfMove;

	c = gfc_list_count(listOfMoveNames);
	for (i = 0; i < c; i++) {
		nameOfMove = gfc_list_get_nth(listOfMoveNames, i);
		//slog("The thing extracted from the list in String form: %s\nThe thing extracted from the list in Pointer form: %p", nameOfMove, nameOfMove);

		//Get the move  according to the name
		move = get_move_by_name(nameOfMove);
		//append it to the MoveList
		gfc_list_append(entMoveList, move);  //apending the  Address of that corresponding Move object as it exists in our list
		//slog("Move of name '%s' appended to the entity's MoveList", move->name);	//Jlog
	}
}




//==========================================================================

	//	I don't need to do it like this.   I have a PRE-CONFIGURED masterlist 
//this one is just good to have.. ig
SJson* moves_get_def_by_name(const char* name) {
	if (!name) return NULL;
	if (!_moveDefs) {	//since this is our static variable,  once this is intialized TO BEGIN WITH. We will continuously refer back to it
		slog("no move definitions loaded");
		return NULL;
	}

	int i, c;
	SJson* move;
	const char* moveName = NULL;

	c = sj_array_get_count(_moveDefs);
	slog("By the way,  there are %i Moves in the loaded Definition file", c);

	for (i = 0; i < c; i++) { //iterate through the list until we find what we're searching for
		move = sj_array_get_nth(_moveDefs, i);
		if (!move) continue;

		moveName = sj_object_get_value_as_string(move, "name"); //get the Name of each Move object
		if (!moveName) continue; //if no name, continue
		if (gfc_strlcmp(name, moveName) == 0) { //Length compare to forgo matching ONLY on a substring
			//slog("Found the JSON object for move '%s' on iteration %i", name, i);	//Jlog
			return move; //found it
		}

	}
	slog("No move of name %s found in the list", name);
	return NULL;
}

//So that I can pass the player's (or monster's)->moveList into this function,  and have this file configure it
	//AND BY CONFIGURE.  I mean,  my moveList will consist of strings:  the NAMES of the attacks.   In this file, I have to MAKE the Move Object  per that list of strings..
void configure_moves_for_ent_old(GFC_List* listOfMoveNames, GFC_List* entMoveList) {
	if (!listOfMoveNames || listOfMoveNames->count < 1) { slog("The provided moveList does not exist, or has a count less than 1"); return; }
	int i, c;
	Move* move;		//Pointer to a place in memory - where a new move is allocated every iteration in the for loop down there
	SJson* moveDef;
	const char* nameOfMove;

	c = gfc_list_count(listOfMoveNames);
	for (i = 0; i < c; i++) {
		move = move_create();

		nameOfMove = gfc_list_get_nth(listOfMoveNames, i);
		slog("The thing extracted from the list in String form: %s\nThe thing extracted from the list in Pointer form: %p", nameOfMove, nameOfMove);
	//IT WORKS !!  IT WORKS FLAWLESSLY  THANK YOU PROFESSOR DJ KEHOE  AND THE CONSISTENCY OF C STRINGS!!


		//Search for the move by its name in the bigass List (similar to that of items)
		moveDef = moves_get_def_by_name(nameOfMove);
		//configure it
		move_configure(move, moveDef); //Configure the newly created move with the matching Definiton (JSon object)
	
		//append it to the MoveList
		gfc_list_append(entMoveList, move);
	}
}


//=============================	USING IN BATTLE
   //o h.  well,, this would just be what the Battle.  files are for, no?  Yes. the answer to that question is yes

