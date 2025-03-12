//J START  -- creating the Entity class
#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "gfc_config.h"
#include "gfc_text.h"
#include "gfc_vector.h"
#include "gfc_shape.h"
#include "gf2d_sprite.h"


typedef enum {
	ENT_none = 0,
	ENT_fierce = 1,
	ENT_docile = 2,
	ENT_cunning = 4,  //I really don't need a team for items.. lol  but I supposed I could implement it elsewhere
	ENT_treasure = 8,
	ENT_MAX = 15
}EntityType;

typedef enum {
	ETT_none = 0,
	ETT_player = 1,
	ETT_monsters = 2,
	ETT_item = 4,  //I really don't need a team for items.. lol  but I supposed I could implement it elsewhere
	ETT_cave = 8,
	ETT_MAX = 15
}EntityTeamType;

//I actually don't really need the type of collision to this level of detail..  Most I'd need is to know if it's tha player, which I'm using TeamType for
typedef enum {
	ECT_none = 0,
	ECT_player = 1,		//he also said Add them all up,,,,,,
	ECT_monsters = 2,
	ECT_item = 4,
	ECT_MAX = 8 //hehe  MAX is the only name I caught.  I guess I can call these whatever
}EntityCollisionType;

typedef enum {
	ECL_none = 0,
	ECL_World = 1,
	ECL_Entity = 2,
	ECL_Item = 4,
	ECL_ALL = 15
}EntityCollisionLayers; //A given entity should have these layers enabled if it is meant to collide with the things in that layer.
	// Ex: The player is meant to collide with everything. So it should have ALL. but a Dragon (even though it's not moving) should not collide with the World nor Items

/*For example:
* Layer 1 = 1							00001
* Layer 2 = 2							00010
* Layer 3 = 4							00100
* Layer 4 = 8	which correspondes to:	01000  respectively.
*/

//But I don't remember if he did that^^  through:  #define WORLD_LAYER 1
//or.. if he used the enum typedef.   'cause I think layers are supposed to be Uint8's.


typedef struct Entity_S{		//Using Entity_S  up here is a sort of  "Forward naming"  which allows us to Refer to this structure within the structure itself!
	Uint8					_inuse;		/**<memory management flag*/  //The underscore at the start is a convention in C,  to say this is a Private variable, and probably shouldn't be touched
	GFC_TextLine			name;			/**<name of the entity*/	//in order to access this  we need to access the gfc_text.h file
	EntityTeamType			team;
	EntityCollisionLayers	layer;
	EntityType				type;

	Sprite					*sprite;		/**<graphical representation of the entity*/   //Pointer TO the sprite data managed by the Sprite Manager. As opposed to a copy of the picture
	GFC_Vector2D			sprite_size;	/**<Size of the sprite. Can be divided in half for the center to pass to the Sprite draw argument*/
	float					frame;			/**<for drawing the sprite*/
	Uint32					framesPerLine;
	GFC_Vector2D			position;		/**<where to draw it*/
	GFC_Vector2D			velocity;		/**<how we are moving*/
	GFC_Vector2D			acceleration;	/**Whether we should change our speed ? */
	
	GFC_Rect				bounds;		//x,y  Top left corner
	GFC_Vector2D			center;
	float					rotation;		
	

	float					speedMax;
	//used to have float speedmax here, but since only my Player will be using it, I decided to make it a member of PlayerEntityData

	//Time to set up the think function baby oh boy
	void					(*think)(struct Entity_S* self);   /**<function to call to make decisions based on the world state*/  //The think function will take a pointer to an Entity
	void					(*update)(struct Entity_S* self);  /**<function to call to execute those decisions*/ //Making another function  to update,  such that Think can SPECIFICALLY occur before updating
	
	//void					(*damage)
	//int					(*collide)(struct Entity_S* self, struct Entity_S* other, EntityCollisionType type);



	//Feb 24   "Let's talk about the void":
	void					*data;		/**<ad hoc data per instance */ //uhhh  smth smth  in Quake,  EVERY ENTITY had these  *monster  *item, etc pointers. which were not required,
			//but kinda serves as a means to tell you WHAT that Entity was. (I think he said)... Us calling it "data" allows us to be generic

	//Feb 24:
	void					(*data_free)(struct Entity_S* self); /**<function to call to free any Sub-class specific Entity data (e.g. Player or Caves/Exits)*/


//---------------- MOVES
			//or.. maybe I implement this with the Data^^ thing..?

	//I'm gonna want a GFC_List moveList here.  bc both the Player and Monsters will have :)
		// List of Attack objects

	//I think I'm also going to want a Flag that can be used PER ENTITY.  as opposed to a global _INBATTLE flag,  to help me know when I should be Drawing the Sprites that I plan to make a part of my Attack.c objects



}Entity;

/**		Description for what the function below me does.  "@brief" describes what the function does @ parameter <parameter>
 * @brief initializes entity sub system
 * @param maxEnts upper limit for how many entitise ca exist at noce	
 */
void entity_system_init(Uint32 maxEnts);

/**
 * @brief free all entities in the manager;
 */
void entity_system_free_all(Entity *ignore); //J SPECIFIC:  adding an entity to ignore when cleaning up

 /**
 * @brief draw all inuse entities, if they have a sprite
 */
void entity_system_draw_all();

/**  (INSTEAD of draw all)
 * @brief draw all entities in the provided list
 * @param entities a GFC_List of entity pointers
 */
//void entity_system_draw_list(GFC_List *entities);


/*
 *@bried run think functions for all entities that have them 
*/
void entity_system_think_all();	

/*
 *@bried run think functions for all entities that have them
*/
void entity_system_update_all();


/**
 * @brief get a new empty entity to work with
 * @return NULL if out of entities/room for entities,  or a blank entity otherwise
 */
Entity* entity_new();

/**
 * @brief free a previously created entity
 * #param Pointer to the entity to free
 */
void entity_free(Entity *);

//void entity_draw(Entity *);   //lmao he no longer has an entity draw ??  maybe I hallucinated this function
void entity_update_position(Entity* self);


//After delving into definition files:
/**
 * @brief configures an entity - meaning sets all of its data members/parameters according to the def (json) file
 * @param self - pointer to the [position of the] entity created, json - pointer to the json object. (Created through use of the  sj_load(filename) function )
 */
void entity_configure(Entity* self, SJson* json);

/**
 * @brief configures an entity Using the filename as a parameter. Will create the json object for you, then call entity_configure()
 * @param self - pointer to the [position of the] entity created, filename - the name of the file as a string
 */
void entity_configure_from_file(Entity* self, const char* filename);


int entity_collision_check(Entity* self, Entity* other);

/**
 * @brief get a list of all colliding entities, honoring team and layers
 * @param self - the entity to check with
 * @return NULL if there are no collisions,  a list of entity poniters otherwise
 * @note any list returned MUST BE FREED with gfc_list_delete()
*/
GFC_List *entity_collide_all(Entity * self);

#endif