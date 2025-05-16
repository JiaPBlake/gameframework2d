#include "simple_logger.h"
#include "gfc_input.h"
#include "gfc_audio.h"

#include "gf2d_graphics.h"

#include "npc.h"
#include "world.h"
#include "spawn.h"

#include "dialogue.h"

extern int keySelectTimer; //10 frames
extern GFC_Sound* test_sound;



typedef enum {
	NPCT_None = 0,
	NPCT_Item = 1,
	NPCT_Move = 2,
	NPCT_Lore = 4,
	NPCT_Max = 7
}NPCType; //dk if they should be bit flags tho

//Note to self.  My Items come in 2 forms.  They are Entities if and only if they

typedef struct {
	NPCType		type;		//so whether it should have an Item or not

	GFC_TextLine	itemName;
	Entity			*item;  //so I can keep track of the item I've spawned in.   Once the player picks it up, the player_think function subsequenty free's the item for us. But just in case
	Uint8			itemSpawnedFlag;

	Move			*moveToTeach;

	Dialogue		*dialogue; //Pointer to a Dialogue object.  Which will be configured with all its appropriate lines

}NPCEntityData;


Entity* npc_new_entity(GFC_Vector2D position, const char* defFile) //Now that I'm configuring the spawn.h file, I'm going to iterate through my list of Spawnable entities
{															// and call this function providing the position to spawn it AND the DefFile to be used in Configuring this entity
	Entity* self;
	self = entity_new();  //create the new entity [spot in our total Entity List]. Where-in its _inuse flag will be set.
	if (!self) {
		slog("failed to spawn a new NPC entity");
		return NULL;
	}


	//Feb 24:   Then,  you would need to allocate the NPCEntityData here   ANDDD CLEAN IT UP!!   Especially that GFC_List
	NPCEntityData* data;
	
	self->layer = ECL_Entity;
	self->team = ETT_NPC;
	self->think = npc_think;  //Set my entity's Think function   TO the function we make in this file!
	self->update = npc_update;

//Def file section
	entity_configure_from_file(self, defFile);  //INSTEAD OF this Sprite loading bock underneath
	//slog("Entity configured (NPC)");

	//position override from the parameters:
	if (position.x >= 0) {//if position is a negative vector, don't override, just use the one from the def file
		//slog("Position override for NPC %s",self->name);
		gfc_vector2d_copy(self->position, position);
		self->position.y += self->bounds.y;
	} 


	self->firstCombat = 1;	//but.. y'know instead of combat it'll be used for interaction


//===================================================		NPC specific data
	self->data_free = npc_data_free;   //set the data free function   (y'know.. once I make one)

	data = gfc_allocate_array(sizeof(NPCEntityData), 1);
	if (data) {
		//slog("Calling NPC Data_configure function");
		self->data = data;
		npc_data_configure(self, defFile);
		if (data->type != 0) slog("NPC's NPC type was properly configured.");
		//data->item = NULL;		done IN function
	}

	self->data = data;

	//slog("Entity Spawned");

	return self;
} 

void npc_data_free(Entity* self) {
	if ((!self) || (!self->data)) return;
	slog("In the NPC's Data Free function");

	NPCEntityData* data; //create a pointer
	data = self->data;   //make it point.  Especially because "data", in THIS file  is specific to NPCs.  whereas self->data is declared to be a Void pointer

	//other cleanup goes here
	//gf2d_sprite_Free(data->profilePicture);  //for example.  if I had a sprite for profilePicture
	if (data->item) {
		slog("NPC has an item, so I'm just gonna free the entity prematurely");
		//this... happens even though I SEE IT being cleaned up by the Entity manager..  whatever. at least I have null checks all throughout the entity manager
		entity_free(data->item);
		data->item = NULL;
	}
	


	free(data);	//remember: don't wanna just pass in self->data here.  Because that'd be a void pointer. and that no worky
	self->data = NULL;						// ^^^ technically I COULD have...  but!  if we have other things to be cleaned up,  like a Sprite !!  making a PlayerEntityData pointer is the best (only...?) way to access it

}

int npc_return_type_from_string(Entity* self, const char* string) {
	if (gfc_strlcmp(string, "item") == 0) {
		//slog("Item giving npc found: %s", self->name);
		return NPCT_Item;
	}
	else if (gfc_strlcmp(string, "move") == 0) {
		//slog("Lore dumping npc found: %s", self->name);
		return NPCT_Move;
	}
	else if (gfc_strlcmp(string, "lore") == 0) {
		//slog("Lore dumping npc found: %s", self->name);
		return NPCT_Lore;
	}
	else if (gfc_strlcmp(string, "all") == 0) {
		//slog("Npc both gives and item AND lore dumps: %s", self->name);
		return NPCT_Max;
	}
	else {
		slog("Extracted NPC Type string, but didn't find a match");
		return NPCT_None;
	}
}


void npc_data_configure(Entity* self, const char* defFile) {
	if (!self) { slog("What the .  no NPC entity to configure"); return; }
	
	SJson* json;
	json = sj_load(defFile);
	if (!json) { slog("Could not load JSon from defFile");  return; }

	NPCEntityData* data; //create a pointer
	data = self->data;   //make it point.  Especially because "data", in THIS file  is specific to NPCs.  whereas self->data is declared to be a Void pointer
	data->item = NULL;
	data->itemSpawnedFlag = 0;

	NPCType type = 0;
	SJson* typeList = { 0 };	//in the event an NPC has more than 1 type
	const char* string = NULL;

	SJson* dialogueList;
	int i, c = 0;

	
//======================================================================================================	NPC Type
	typeList = sj_object_get_value(json, "npc_type");	//this will NOT return null.  the key IS there !!  What I'm concered with: is what form it takes
	if (sj_is_array(typeList)) {
		c = sj_array_get_count(typeList);
	}
	else {
		c = 1;

	}
	
	for (i = 0; i < c; i++) {
			//if c > 1,  then get the value(s) from the array.	ELSE !! typeList isn't even an array to begin with. it's just a SJson object that is a string. Get that string
		string = (c>1) ? sj_get_string_value(sj_array_get_nth(typeList, i)) : sj_get_string_value(typeList);
		
		if (string) {
			//slog("NPC type is string: %s", string);
			type = npc_return_type_from_string(self, string);
			data->type |= type;
		}
		else {
			slog("Could not extract what Type of NPC this is");
			data->type = NPCT_None;
		}
	}
	slog("Checkpoint 0");
//=======================================================================================================	ITEM carrying
	if (data->type & NPCT_Item) {
		string = sj_object_get_string(json, "item");
		if (string) {
			gfc_line_cpy(data->itemName, string);
			//slog("Extracted NPC's item name: %s",data->itemName);  //Jlog
		}
		else {
			slog("Could not extract the item the NPC should be holding");
			
		}
	}

//=======================================================================================================	MOVE giving
	if (data->type & NPCT_Move) { 
		string = sj_object_get_string(json, "move"); 
		if (string) {
			slog("NPC has move: %s to give to player.", string);
			data->moveToTeach = get_move_by_name(string);

		}
		else {
			slog("Could not extract the name of the item that the NPC should be holding");
		}
	}

//=======================================================================================================	Lore dumP
	if (data->type & NPCT_Lore) {
		slog("Lore NPC branch.");
		//dialogueList = sj_object_get_value(json, "dialogue");
		//This  SJson variable  is a JSon array.
		//dialogue_configure(self, dialogueList);
		//slog("The amount of dialogue lines this NPC '%s' has is: '%i'",self->name, c);
	}


	sj_free(json);
}


void npc_spawn_item(Entity* self, Entity* player) {
	if (!self || !player) {
		slog("No self (NPC) OR no player. Not spawning item");
		return;
	}
	NPCEntityData *data;
	data = self->data;

	if (!test_sound) {
		slog("Test sound not loaded :(((");
	}
	else {
		slog("Test sound loaded !!!");
	}

	//wait yeah duh.   no need to GIVE it to the player,  just spawn it right next to them,, LMAO
	GFC_Vector3D item_pos = { 0 };
	item_pos.x = self->position.x + 10;
	item_pos.y = self->position.y - self->bounds.y;
	item_pos.z = 0;
	//I can safely say that an NPC will never be spawned THAT close to a tile of a higher elevation. So just take the NPC'd position, and offset it by like 20 pixels to the right.
	
	data->item = spawn_entity(data->itemName, item_pos, NULL);
		//honestly !! this SHOULD add it to my masterlist in the Entity maanger right???   so I don't ned a separate draw function for this

}

void npc_teach_move(Entity* self, Entity* player) {
	if (!self || !player) {
		slog("No self (NPC) OR no player. Cannot teach move");
		return;
	}
	NPCEntityData* data;
	data = self->data;
	slog("Teaching Player the move now");
	gfc_list_append(player->move_list, data->moveToTeach);
	
}

void npc_lore_drop(Entity* self, Entity* player) {
	// hm.  This is going to have to include dialogue.h ,,, once I make that..
}

void npc_perform_action(Entity *self, Entity *player) {
	if (!self || !player) {
		slog("NPC or Player pointer is NULL.  This NPC will not perform an action");
		return;
	}
	
	NPCEntityData* data; //create a pointer
	data = self->data;


	if (data->type & NPCT_Item && !data->itemSpawnedFlag) {
		//give the player the item
		slog("spawning item");
		npc_spawn_item(self, player);
		data->itemSpawnedFlag = 1;
	}
	if (data->type & NPCT_Move && !data->itemSpawnedFlag) {
		//give the player the item
		slog("spawning item");
		npc_teach_move(self, player);
		data->itemSpawnedFlag = 1;
	}
	if (data->type & NPCT_Lore) {   //... :|  for some reason this always equates to 1..  can't remember why but I think it was smth smth the order. Fierce came first.  Maybe I never properly declared types..? don't remember actually
		slog("The type of %s is: %i", self->name, data->type);
	}

	slog("NPC interacted with");
}



void npc_think(Entity *self) {
	if (!self) return;

	//lmfao if I'm the teacher, I should walk at a really slow place back and forth...




	//check if player is colliding with me
	Entity* other;
	GFC_List* others;
	others = entity_collide_all(self);  //BE SURE TO DELETE THIS LIST ONCE WE'RE DONE WITH IT	
	if (others) { //if I'm colliding with ANYTHING  [namely the player]:

		other = gfc_list_get_nth(others, 0); //in my game, the player will really only be colliding with 1 thing at a time
		if (self->firstCombat == 1 && other->team & ETT_player) {
			if (gfc_input_command_down("enter")) {  //IF I press Enter, to interact
				if (keySelectTimer <= 0) { //Less than or equal to 0  just in case we miss 0

					npc_perform_action(self, other);
					
					keySelectTimer = 10;
				}
				if (keySelectTimer > 0) {  //ONLY decrement if it's positive.   Just in case
					keySelectTimer--;					//just realized this decrementation should be happening regardless of if I'm pressing a key lmfao.  so OUTSIDE the  gfc_input_command  blocks... but it still feels great as it is now
				}
			}
		}
	}

	gfc_list_delete(others);



}

void npc_update(Entity* self) {

}


/*end of file*/