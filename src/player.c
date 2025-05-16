#include <SDL.h>

#include "simple_logger.h"
#include "gfc_input.h"
#include "gfc_audio.h"
#include "gf2d_graphics.h"

#include "player.h"
#include "camera.h"
#include "collision.h"
#include "world.h"
//#include "window.h" //to set selected AND for everything about Player input in the god damn think function..
#include "inventory.h"
#include "battle.h"

//extern Entity *otherEnt;  //artifact of old thinking (me trying to implement collision before we went over it in class
Uint8 _INBATTLE = 0;
//Uint8 _NEWENCOUNTER = 0;  //I can now take care of this fully within Object.c
extern Uint32 _MOUSEBUTTON;
Uint8 health_frame = 5;
int keySelectTimer = 0; //10 frames
int jumpKeyTimer = 0; //Can only hold for 300 frames
Uint8 _INVENTORY_FLAG = 0;
Uint8 rightKey = 0;
Uint8 leftKey = 0;

extern GFC_Sound* test_sound;


void player_damage(Entity* self, Entity* other, Entity* creit, float damage, Uint8 damageType);

//if you want to make the player a global entity for other files to access
Entity* thePlayer = NULL;

// "Rule of 3"s in programming: if ever we need a constant Constructor -  we'll need a Deconstructor  as well as a co-? Constructor

typedef struct {
	//float	speedMax; /**<Max speed of the player.So that when Player is in battle, we can change this value to 0 so that they don't move*/
	//int xp, neededxp;   //don't need these
	int		fierce_points;
	int		docile_points;
	int		cunning_points;															//WAIT I LIED I COULD TOTALLY MAKE PICK-UPABLE ITEMS ENTITIES !! THAT WOULD MAKE COLLISSION SO MUCH EASIER LMAOO
			//Something important to think about.  INVENTORY is something only a player will have.  Inventory has its OWN *itemsList.  and the World will have its own as well - for the purpose of drawing them for pickup
	Inventory inventory;  //   this.. this should be a pointer . 'cause our init function takes a pointer. (I mean yeah I could just pass &)  but like I feel like... a pointer here makes things easier..?

	int		attack_stat;		//deal higher damage:   Base_dmg of the attack  + attack_stat/2   Dragon's Health out of 100
	int		persuasion_stat;	//
	int		worthiness_stat;

	//List of Attacks  (mmm  for Changing ?)

	//mm !   What I could do is I could keep a list of the Attack's Names !  Every Ent will have a GFC_List of Move objects .   but for easy storage  and perhaps even JSon file SAVING: I can keep just the names here
			//(whereas in entity.c  I'm extracting those names into a list and deleting it in the same funciton as soon as I've configured the Move objects)


}PlayerEntityData;

Entity* player_get_the() {
	return thePlayer;
}

Entity *player_new_entity(GFC_Vector2D position, const char* defFile) //added def file const char *
{
	Entity *self;

	PlayerEntityData *data; //lmfao make sure this is a pointer

	if (thePlayer) {
		gfc_vector2d_copy(thePlayer->position, position);
		slog("The Player already exists");
		return thePlayer;
	}

	self = entity_new();  //create the new entity [spot in our total Entity List]. Where-in its _inuse flag will be set.
	//now, we have an empty section of memory to work with, to fill in all the data members of our Entity class
	if (!self) {
		slog("failed to spawn a new player entity");
		return NULL;
	}
	entity_configure_from_file(self, defFile);  //Configure the entity (Loading the sprite and setting spawn position)
	if (position.x >= 0) { gfc_vector2d_copy(self->position, position); } //position override from the parameters
	//if position is a negative vector, don't override, just use the one from the def file

	//Gravity is a constant.
	self->gravity.x = self->acceleration.x;  //X I will use to give the player that upward boost in initial velocity while they hold Up
	self->gravity.y = self->acceleration.y;  //Y is normal gravity. which should only start taking place after the key is released
	self->acceleration.x = 0;

	self->layer = ECL_Entity;
	self->layer |= ECL_ALL; //The Player should collide with all possible things. Other Entities, the World, AND Items
	self->team = ETT_player;
	self->think = player_think;  //Set my entity's Think function   TO the function we make in this file!
	self->update = player_update;
	//self->damage = player_damage;

	//Feb 24:
	self->data_free = player_data_free;
	//Feb 24:   Polymorphism --  our Player IS an Entity, but we're adding more to it
//And I don't have to worry about trying to implement this into entity_configure,  because all this upcoming info is SPECIFIC to the player
	data = gfc_allocate_array(sizeof(PlayerEntityData), 1);
	if (data) {
		data->fierce_points = 0;
		data->docile_points = 0;
		data->cunning_points = 0;
		data->attack_stat = 5;
		data->persuasion_stat = 5;
		data->worthiness_stat = 5;

		slog("Trying to initialize the player's inventory");
		inventory_init(&data->inventory); //Player Entity Data's inventory

		//inventory_add_item(&data->inventory, "item_resounding_artifact");  //Just did this for testing purposes
		self->data = data;   //"data" is the type-associated version.
	}
	

	//Set my global variable. So others can find me with the player_get_the() function
	thePlayer = self;
	test_sound = gfc_sound_load("audio/dink.mp3", 0.3, 1);
	if (!test_sound) {
		slog("What the!! :(  Player new  no loaded test sound");
	}
	else {
		slog("HEYOOO soundies work");
	}

	return self;
}

void player_data_free(Entity* self) {
	//slog("Made it to the player_data_free function even though it's a void pointer");
	if ((!self) || (!self->data)) return;
	
	PlayerEntityData* data;
	data = (PlayerEntityData *)self->data; //cast the Void pointer specifically to a PlayerEntityData* pointer
	
	//other cleanup goes here
	//gf2d_sprite_Free(data->profilePicture);  //for example.  if I had a sprite for profilePicture
	slog("In the Player data free function. About to free up the inventory");
	inventory_cleanup(&data->inventory);


//free the data once we're done with it
	free(data);	///BECAUSEE   my Entity, self.  ONLY knows "data"  as a void pointer.  It does not know that it's a PlayerEntityData
	self->data = NULL; //could have just done   free(self->data),  but!  if we have other things to be cleaned up,  like a Sprite !!  making a PlayerEntityData pointer is (the only...?) best way to access it

}

int get_player_points(EntityType type) {
	Entity* self;
	self = thePlayer;

	if (!self) {
		slog("something is seriously wrong,,,"); return -1;
	}

	else {
		PlayerEntityData* data;
		data = (PlayerEntityData*)self->data;
		int total_points = 0;
		//Alphe first since it will be all 3
		/*if (type & ENT_fierce & ENT_docile & ENT_docile) {			//I'm.... an idiot :sob:  I DON'T EVEN NEED THIS LMFAOOO  this shit is flat out wrong.  if I tame an alpha then I just INCREMENT all 3!!!
			slog("Returning ALL");
			total_points = data->fierce_points + data->docile_points + data->cunning_points;
			return data->fierce_points;
		}

		else*/
		if (type & ENT_fierce) {   //... :|  for some reason this always equates to 1..
			//slog("Returning FIERCE");
			return data->fierce_points;
		}
		else if (type & ENT_docile) {   //... :|  for some reason this always equates to 1..
			//slog("Returning DOCILE");
			return data->docile_points;
		}
		else if (type & ENT_cunning) {   //... :|  for some reason this always equates to 1..
			//slog("Returning CUNNING");
			return data->cunning_points;
		}
		
	}
	slog("uh oh. made it to the end of get points");
	return -1;
}

void inc_player_points(EntityType type) {
	Entity* self;
	self = thePlayer;

	if (!self) {
		slog("something is seriously wrong,,,"); return;
	}

	else {
		PlayerEntityData* data;
		data = (PlayerEntityData*)self->data;
		int total_points = 0;
		//Alphe first since it will be all 3
		/*if (type & ENT_fierce & ENT_docile & ENT_docile) {			//I'm.... an idiot :sob:  I DON'T EVEN NEED THIS LMFAOOO  this shit is flat out wrong.  if I tame an alpha then I just INCREMENT all 3!!!
			slog("Returning ALL");
			total_points = data->fierce_points + data->docile_points + data->cunning_points;
			return data->fierce_points;
		}

		else*/
		if (type & ENT_fierce) {   //... :|  for some reason this always equates to 1..
			//slog("Returning FIERCE");
			data->fierce_points++;
		}
		if (type & ENT_docile) {   //... :|  for some reason this always equates to 1..
			//slog("Returning DOCILE");
			data->docile_points++;
		}
		if (type & ENT_cunning) {   //... :|  for some reason this always equates to 1..
			//slog("Returning CUNNING");
			data->cunning_points++;
		}

	}
	slog("uh oh. made it to the end of get points");
	return;
}

void player_think(Entity* self) {
	
	if (!self) return;  //if I no am, then can not think!
	
	PlayerEntityData* data = {0};

//---------------------------------------------------------------------------  PLAYER INPUT MOVEMENT
	//gfc_input_update();  //I have since learned that gfc_input_init being in game.c WAS the correct place.  And the reason it didn't work off the bat was because there was a specific SDL update function call and not gfc_input_update().
			//This has since been moved.	
	self->acceleration.y = self->gravity.y;

	if (gfc_input_command_down("right")) {  //if I'm pressin'  right
		//self->velocity.x = 1.0;
		self->acceleration.x = 0.5;
		self->flip.x = 1;
	}
	else if (gfc_input_command_down("left")) {  //if I'm pressin'  left
		//self->velocity.x = -1.0;
		self->acceleration.x = -0.5;
		self->flip.x = 0;
	}
	else { self->velocity.x = 0; self->acceleration.x = 0; }

	if (gfc_input_command_down("down")) {  //if I'm pressin'  down
		//self->velocity.y = 1.0;
		self->acceleration.y = 2;
	}
	else if ( (gfc_input_command_down("up") || gfc_input_command_down("jump")) && jumpKeyTimer < 20 ) {  //if I'm pressin'  up
		//self->velocity.y = -5.0;
		self->acceleration.y = self->gravity.x; //go up!
		jumpKeyTimer++; //WHILE holding UP.  You can only jump for a max amount of time
	}
	else { self->velocity.y = 0; self->acceleration.y = self->gravity.y;  }	//stop movement if we're not holding down a button

	//if ( gfc_input_command_down("jump") && jumpKeyTimer < 300  ) {  //if I'm pressin'  down
	//	self->acceleration.y = self->gravity.x; //go up!
	//	jumpKeyTimer++; //WHILE holding UP.  You can only jump for a max amount of time
	//}
	//else { self->velocity.y = 0; self->acceleration.y = self->gravity.y; }

	/* This was for diagonal screen bouncing
	if (self->position.x <= 0) self->velocity.x = 1;
	if (self->position.y <= 0) self->velocity.y = 1;
	if (self->position.x > 800) self->velocity.x = -1;
	if (self->position.y > 650) self->velocity.y = -1;*/

	
//---------------------------------------------------------------------------  PLAYER INVENTORY

	if (gfc_input_command_down("inventory")) {
		_INVENTORY_FLAG = 1;
	}
	else {
		_INVENTORY_FLAG = 0;
	}
	//Feb 24:
	if (self->data) {  //for Point handling.  amongst other things ofc
		data = (PlayerEntityData*)self->data;
	}


//---------------------------------------------------------------------------  COLLISION

	//slog("Printing out something about the Other Entity in the Player_new function. to make sure we can see it: Position.x = %f", otherEnt->position.x);
	/*//this is the collision function I tried to make myself, but uhhhh.   Yeah that's not fleshed out much at all lol
	if (collision_check(self, otherEnt)) { 
		slog("collision in PLAYER.C!!!");
		gfc_vector2d_scale(self->velocity, self->velocity, -2);
	}*/
	//Collission test !!   Single collision grabbing one other entity through a global variable
	/*if (entity_collision_check(self, otherEnt)) {
		//slog("Collision using the function defined in entity!"); //WORKS
		_INBATTLE = 1;
	}
	else { _INBATTLE = 0; }*/

	
	//Collision in class:    (Works !)
	Entity* other;
	GFC_List* others;
	int i, c; //i for iterating.   c for getting the  gfc_list_count(others);  so that we can iterate  i < c

	others = entity_collide_all(self);  //BE SURE TO DELETE THIS LIST ONCE WE'RE DONE WITH IT
	
	if (others) { //if I'm colliding with ANYTHING:
		
		other = gfc_list_get_nth(others, 0); //in my game, the player will really only be colliding with 1 thing at a time
		if (other->team & ETT_monsters && other->firstCombat == 1) {
			slog("Colliding with %s",other->name);
			
			if (!_INBATTLE) {
				if (health_frame == 0) { health_frame = 5; slog("Health reset"); }
				else health_frame--;
					
			}
			
			//instead, I would like to just:   
			battle_start(self, other);
			
			
			_INBATTLE = 1;  //I only want health to decrease ONCE PER collision.  not while I'm colliding
			//self->velocity.x = 0;
			self->think = player_think_battle;
			reset_selected();

			other->firstCombat = 0;

		}
	
		/*if (other->team & ETT_cave) {		//Aha ! I don't need to take care of gfc_input  here in player.c ONLY. I can do it anywhere :)
			if (gfc_input_command_down("proceed") && !_NEWENCOUNTER) { //no idea why the fuck gfc_input_command_pressed isn't working .
				if(other->name) slog("Entering cave: %s",other->name);
				_NEWENCOUNTER = 1;  //figure out when to set this to 0. probably in the World funciton ONCE the world is loaded
			}
		}*/

		if (other->team & ETT_item) {
			if (keySelectTimer <= 0) { //Perform Action.  Then reset timer
				slog("Pick up item");
				if (test_sound) gfc_sound_play(test_sound, 0, 0.1, -1, -1);
				if(data) inventory_add_item(&data->inventory, other->name);//update the player's INVENTORY  and free the ENTITY
				entity_free(other); 
				keySelectTimer = 10;
			}
			else if (keySelectTimer > 0) {  //No aciton. Just wait
				keySelectTimer--;
			}

		}
	}
	else {
		_INBATTLE = 0;
	}
	
	gfc_list_delete(others);
	//slog("collision list deleted");
	
}

void player_update(Entity* self) {
	if (!self) return;
	//GFC_Vector2D screen; //instead of the screen = gf2d_graphics_get_resolution(); //should be 1200 by 720... I'm gonna make it the camera a.k.a world
	float speed = 0;
	GFC_Rect screen;
	GFC_Rect rect = { 0 };
	GFC_Rect* tile = { 0 };
	GFC_List* tiles;
	int i, c;
	GFC_Vector2D ground = { 0 };
	GFC_Vector4D direction = { 0 };
	float distInGround = 0;
	GFC_Vector2D center;
	Uint8 tileIndex;

//==========================================================================================  ANIMATION & MOVEMENT

	if(self->velocity.x != 0) {
		//Move forward animation
		self->frame += 0.5;
		//if I'm slowing down,,,  frame should be decreasing ?  (Perhaps From a set point ?)
		if (((self->velocity.x > 0) && (self->acceleration.x < 0)) || ((self->velocity.x < 0) && (self->acceleration.x > 0))) {
			self->frame -= 0.5;
			//or should I split the FPL in half to see if I'm closer to the End or Beginning of my anim cycle..?
		}
		if (self->frame >= self->framesPerLine) self->frame = 0;
	}

	if (self->velocity.y < 0) {
		self->frame = 0;
	}

	//Update position using the velocity that was determined through Think()ing  'cause what is velocity?? METERS per unit second [frame] :D
	//if (self->velocity.y != 0) slog("My y velocity is: %f",self->velocity.y);
	
	//What happens here is that acceleration (Y ONLY)   decreases my velocity in the Y.
	//slog("My acceleration is: %f , %f", self->acceleration.x, self->acceleration.y);
	//slog("My X velocity is: %f", self->velocity.x);
	//slog("My Y velocity is: %f", self->velocity.y);
	gfc_vector2d_add(self->velocity, self->velocity, self->acceleration);   //GRAVITY
	//slog("\nMy velocity is NOW: X: %f  Y: %f\n", self->velocity.x, self->velocity.y);

	//slog("Velocity is: %f", self->velocity.x);
	if (fabs(self->velocity.x) > self->speedMax.x) {
		
		if(self->velocity.x > 0) self->velocity.x = self->speedMax.x;
		
		if (self->velocity.x < 0) {
			self->velocity.x = 0 - self->speedMax.x;
		}
	}

	if (fabs(self->velocity.y) > self->speedMax.y) {

		if (self->velocity.y > 0) self->velocity.y = self->speedMax.y;

		if (self->velocity.y < 0) {
			self->velocity.y = 0 - self->speedMax.y;
		}
	}

	//speed = gfc_vector2d_magnitude(self->velocity);
	

	//  LMAO  HELP   IT DOESN'T DROP ME ONCE I SPAWN  BECAUSE WHEN I SPAWN MY X VEL == 0
	//speed = self->velocity.x;  //Because my speed on the GROUND   is the speed stored in my x component..   the addition of acceleration is gonna 'cause that horizontal component to FEEL less than it actually is.
	//	//so SAVE IT in this varaible !.  and scale the normalized vector so I can feel as fast as I normally am! :D
	//slog("My speed is: %f", speed);
	//gfc_vector2d_normalize(&self->velocity);  //takes a pointer 
	//gfc_vector2d_scale(self->velocity, self->velocity, fabs(speed));  //Scale the velocity

	//Before  !  I update my position,,  in the event I'm falling downward too fast..  run the world_Test_shape with a dummy shape to see if I WOULD be colliding into that tile
	gfc_vector2d_add(self->position, self->position, self->velocity);
	// ...  I actually wanna do this on different axises  explicitly
	//self->position.x += self->velocity.x


//==========================================================================================  SCREEN BOUNDS

	screen = camera_get_bounds();
	//WORLD Bounds.  So that when I'm updating,  I don't update to a position where I'm offscreen. instead I will not cross that border
	if (self->position.x + self->bounds.x < 0) self->position.x = 0 - self->bounds.x;
	if (self->position.y + self->bounds.y < 0) self->position.y = 0 - self->bounds.y;
	if (self->position.x - self->bounds.x > screen.w) self->position.x = screen.w + self->bounds.x;
	if (self->position.y - self->bounds.y > screen.h) self->position.y = screen.h + self->bounds.y;


//==========================================================================================  WORLD BOUNDS

	//Testing World bounds:
	gfc_rect_copy(rect, self->bounds); //copy our bounds rectangle into our new rect
	gfc_vector2d_add(rect, rect, self->position); //ALL OF MY ENTITIES.  have their position determined with the camera offset included, because that's how I'm. DRAWING them.
	//now, rect serves as the Player's moving Bounds rectangle
	
	direction.z = self->velocity.x;
	direction.w = 0;
	world_collision_handle(self, gfc_shape_from_rect(rect), &direction);
	if (direction.w) { jumpKeyTimer = 0; }
	/*tiles = world_get_collided(world_get_active(), gfc_shape_from_rect(rect));
	
	if (tiles) {
		c = gfc_list_count(tiles);
		//For every tile in the list of collisions.  Do the EXTRA WORK  of figuring out the direction  USING that tile.
		
		for (i = 0; i < c; i++) {
			/*distInGround = (rect.y + rect.h) - tile.y;
			slog("I am %f pixels into the ground", distInGround);  //At my current speed, this number doesn't seem to go over 3.0

			//idea.   IF I'm colliding.  Get the direction I collided FROM.
			  //Player collides on the Left of a Tile.   Conidering I'm already overlapping:  my Right edge JUST barely became > tile's Left edge
			if ((rect.x + rect.w) > (tile.x)) {
				slog(" (R) PLAYER RIGHT EDGE collision ON tile's Left");
				direction.x = 1;
			}
			else if ((rect.x) < (tile.x + tile.w)) {
				slog(" (L) PLAYER LEFT EDGE collision ON tile's Right");
				direction.x = -1;
			}


			direction.x = 0;
			direction.y = 0;
			direction.z = self->velocity.x;
			direction.w = self->velocity.y;  //this will always be down UNLESS I am jumping.

			tile = gfc_list_get_nth(tiles, i);
			
			if (tile) {
				tile_collide_check(tile, gfc_shape_from_rect(rect), &direction);
				slog("Tile's x position is: %f", tile->x);
				slog(" MY x position is: %f", self->position.x);

				if (direction.y != 0 || direction.x != 0) { //my y position (taking into account my bounds) should be 614 compared to ground's 640 
					slog("Checkpoint 2");
					//I have to handle collision here .   tile_collide_check tells me IF I am colliding.  AND the rect I'm colliding with. but only one at a time,,,

					slog("Player's X position is: %f", self->position.x);

					//I return BOTH x & y dimensions for direction.  I choose whether I want to act on them, tho.

							//dir.z let's me know that I'm moving Right on the ground.
					if (direction.x > 0 && direction.z != 0) {   //if I'm colliding right 
						self->position.x = tile->x + self->bounds.x;
						direction.y = 0;
						slog("You have been stopped .");
						//direction.z = self->velocity.x;
					}

					if (direction.y > 0 && direction.z == 0) {  //if I'm colliding top.  and top ONLY.  Don't let me go into the ground.
						//We turned z off.  Because we don't want GROUND telporting me Left.
						self->position.y = tile->y + self->bounds.y;
						slog("You have been raised");
					}

					slog("Colliding");

				}
			}
			else { slog("literally what the fuck are we doing."); }
		}

		gfc_list_delete(tiles);
	}*/
	//entity_update_position();  //he has this single function here.  I don't.. REALLY need to implement this since the dragons aren't doing any moving. 


//slog("MY y position = %f",self->position.y); 
	ground = world_get_ground();
	//slog("Ground's y value is: %f", ground.y);
	//slog("Player's y position is: %f", self->position.y);
	//slog("Ground y position = %f", ground.y);
	if (self->position.y > ground.y) { slog("what the FUCK WE'RE UNDERGROUND??? at PLAYER POS: %f   ground pos: %f",self->position.y, ground.y); self->position.y = ground.y - self->bounds.h; }



//==========================================================================================  CAMERA CENTER
	center = gfc_vector2d(self->position.x, self->position.y - 200);
	camera_center_on(/*self->position*/ center);


}

/*void player_think_battle(Entity* self) {
	//We should ONLY be in this function if the _INBATTLE flag is on.
	if (!_INBATTLE) {
		slog("Not in battle.  Exiting Battle Think function and setting player think back to normal");
		self->think = player_think;
	}

	self->position = gfc_vector2d(400, 620);
	
	
	//Select UI box
	if (gfc_input_command_down("right")) {  //if I'm pressin'  right
		if (keySelectTimer <= 0) { //Less than or equal to 0  just in case we miss 0
			//slog("Going right");
			inc_selected();
			keySelectTimer = 10;
		}
		if (keySelectTimer > 0) {  //ONLY decrement if it's positive.   Just in case
			keySelectTimer--;
		}
	}
	else if (gfc_input_command_down("left")) {  //if I'm pressin'  left
		if (keySelectTimer <= 0) { //Less than or equal to 0  just in case we miss 0
			dec_selected();
			keySelectTimer = 10;
		}
		if (keySelectTimer > 0) {  //ONLY decrement if it's positive.   Just in case
			keySelectTimer--;
		}
	}

	if (gfc_input_command_down("down")) {  //if I'm pressin'  right
		//self->velocity.x = 1.0;
		reset_selected();
		_INBATTLE = 0;
		self->think = player_think;
	}

	if (gfc_input_command_down("enter")) {  //if I'm pressin'  right
		
		if (keySelectTimer <= 0) { //Less than or equal to 0  just in case we miss 0
			
			slog("Performing action");
			button_perform_action(get_selected(), window_get_active());

			keySelectTimer = 10;
		}
		if (keySelectTimer > 0) {  //ONLY decrement if it's positive.   Just in case
			keySelectTimer--;
		}
	}
	
	if (gfc_input_command_down("back")) {
		
		if (keySelectTimer <= 0) { //Less than or equal to 0  just in case we miss 0

			slog("Trying to back out to last window");
			window_go_back();	

			keySelectTimer = 10;
		}
		if (keySelectTimer > 0) {  //ONLY decrement if it's positive.   Just in case
			keySelectTimer--;
		}
	}

	//I  want toooooooo   call  battle_start()   in my Think
			//and battle_end  in THIS think
	

}*/

//Using this to try and draw the sprite of any 1 item just to see what purpose I could give that
void player_show_inven(Entity* self) {	//This is essentially a draw() function
	PlayerEntityData* data;
	data = self->data;
	data->inventory;

	//I think I wanna make this an entire surface.  To make it look nice, y'know?   little boxes  nice and neat for things to fit in :)  and then whenever I pick it up be sure to draw it to the surface alongside adding it to the inventory.
	//Matter of fact,  maybe Inventory could have it's own sprite. So I draw Inven then Items on top.  like Window then Buttons on top.  (my ass will NOT be making the Inventory a UI_Window...   MUCH easier to keep the 2 separate).

	//slog("Trying to display Resounding Artifact from the player inventory");
	//display_item(&data->inventory, "item_resounding_artifact");
	inventory_draw(&data->inventory);  //not finished just yet but use this instead

}
