#ifndef __MOVES_H__
#define __MOVES_H__

#include "gfc_config.h"
#include "gfc_text.h"
#include "gfc_vector.h"
#include "gfc_shape.h"
#include "gf2d_sprite.h"	//don't think I'm gonna need sprites,,,  OR shape ??  or even Vector-


//YEAH DO NOT INCLUDE ANY  ENTITIES IN HERE LMAO  Move --> Ent --> Player, Monster, NPC

//Attack Type enumerations

typedef struct {
	
	int						attackPower;



	Sprite					*sprite;		/**<graphical representation of the entity*/   //Pointer TO the sprite data managed by the Sprite Manager. As opposed to a copy of the picture
	GFC_Vector2D			sprite_size;	/**<Size of the sprite. Can be divided in half for the center to pass to the Sprite draw argument*/
	float					frame;			/**<for drawing the sprite*/
	Uint32					framesPerLine;
	GFC_Vector2D			position;		/**<where to draw it*/
	GFC_Rect				bounds;		//x,y  Top left corner
	GFC_Vector2D			center;
	float					rotation;
	GFC_Vector2D			flip;			//To flip the entity sprite - either x or y axis

}Attack_Move;  //shit .  Do I want to do Attack AND Converse ??  honestly yeah


typedef struct {
	const char* name;
	int			numOflines;		//??
	int			lineNum;		//to know which line we're on.

}Converse_Move;

//==================================================================


typedef enum
{
	MOVET_NONE = 0,
	MOVET_ATTACK = 1,
	MOVET_CONVERSE = 2,
	MOVET_TAME = 4,
	MOVET_MAX = 7
}MoveTypes;

typedef struct
{
	Uint8				_inuse;		//For cleanup,  just like my UI system. Becauseee I just wanna be sure that I'm cleaning up everything (every Sprite, really) that I end up creating and allocating
			//wait.. I have the entire list of all my Moves.. why don't I just configure all of them..?

	GFC_TextLine		name;
	MoveTypes			type;
	//Each  Move will only be one of each
	union
	{
		Attack_Move			attack;			//And Labels will just overlyyyy use my text.c functions
		Converse_Move		converse;
	}m;

	//Free function can be assigned BASED ON  a given object's->type  data member
		//yeah no I did not do this ^   I don't PLAN on doing this.  I did all 3 possible cases in 1 Free function
	void				(*data_free)(struct Entity_S* self); /**<function to call to free any Sub-class specific Entity data (e.g. Player or Caves/Exits)*/
	void				(*elem_draw)(struct UI_Element* self/*, int selected*/);		//J note: I have since made use of the _selected flag for buttons lmao

}Move;





void move_masterlist_initialize(const char* filename);

/**
 * @brief	Search for a specific Move by iterating through all the individual Json objects in the global variable made to hold the masterlist JSon object of Moves
 * @param	name - name of the move
 * @return NULL if the definition file for the Move was not found, otherwise: pointer to the SJson object
*/
SJson* moves_get_def_by_name(const char* name);

/**
 * @brief	Since the total number of moves in the game is within 30. I just configure the entire list as soon as the game starts. This function does that
*/
void configure_all_moves();

Move* get_move_by_name(const char* move_name);

//===================  RESOURCE MANAGER SUBSYSTEM
/**
 * @brief initializes sub system for Moves												MAKE USE OF THE BIG LIST -- LIKE ITEMS
 * @param maxEnts upper limit for how many entitise ca exist at once
 */
void move_system_init(Uint32 maxMoves);

/**
 * @brief free all Move objects in the manager;
 */
void moves_free_all();


/**
 * @brief Create a Move  (allocate a space for it in the manager)
 * @return NULL if not enough space; else pointer to the ui element's position in the list
 * @note Does not initialize the data members. Just allocates the space
 */
Move* move_create();


/**
 * @brief Free a given move
 * @param self - the Move to clean up
 */
void move_free(Move* self);


/**
 * @brief [To be called in entity.c's entity_configure() function]  Using the list of names,  configure Moves and append them to the entity's->move_list
 * @param listOfMoveNames - [LIST OF STRINGS] A pointer to the list of Names corresponding to the moves an entity should have.
 * @param entMoveList - [LIST OF MOVES] A pointer to the Entity structure's move-list data member, which is to be filled with corresponding Move structures/objects
 */
void configure_moves_for_ent(GFC_List* listOfMoveNames, GFC_List* entMoveList);




#endif