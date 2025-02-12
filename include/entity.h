//J START  -- creating the Entity class
#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "gfc_config.h"
#include "gfc_text.h"
#include "gfc_vector.h"
#include "gfc_shape.h"
#include "gf2d_sprite.h"

typedef struct Entity_S{		//Using Entity_S  up here is a sort of  "Forward naming"  which allows us to Refer to this structure within the structure itself!
	Uint8				_inuse;		/**<memory management flag*/  //The underscore at the start is a convention in C,  to say this is a Private variable, and probably shouldn't be touched
	GFC_TextLine		name;			/**<name of the entity*/	//in order to access this  we need to access the gfc_text.h file
	GFC_Rect			bounds;
	Sprite				*sprite;		/**<graphical representation of the entity*/   //Pointer TO the sprite data managed by the Sprite Manager. As opposed to a copy of the picture
	float				frame;			/**<for drawing the sprite*/
	GFC_Vector2D		position;		/**<where to draw it*/
	GFC_Vector2D		velocity;		/**<how we are moving*/
	GFC_Vector2D		acceleration;	/**Whether we should change our speed ? */
	float				rotation;		
	
	float				speedMax;   //Just so that entity.c can compile.  I don't think I'll be needing a speedmax, but it'd be incredibly nice to learn about and have

	//Time to set up the think function baby oh boy
	void				(*think)(struct Entity_S* self);   /**<function to call to make decisions based on the world state*/  //The think function will take a pointer to an Entity
	void				(*update)(struct Entity_S* self);  /**<function to call to execute those decisions*/ //Making another function  to update,  such that Think can SPECIFICALLY occur before updating

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
 * @param pointer to the [position of the] entity created,  pointer to the json object. (Created through use of the  sj_load(filename) function )
 */
void entity_configure(Entity* self, SJson* json);

/**
 * @brief configures an entity Using the filename as a parameter. Will create the json object for you, then call entity_configure()
 * @param pointer to the [position of the] entity created, the name of the file as a string
 */
void entity_configure_from_file(Entity* self, const char* filename);


#endif