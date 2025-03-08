#ifndef __BATTLE_H__
#define __BATTLE_H__

//Upon collision with a Dragon (and maybe pressing W  ?)   A Battle will start with   battle_start()
	//As a result.  The battle is going to need access to the Player's info.  And the Monster's info

#include "entity.h" //entity here so that my funciton parameters can work

extern Uint8 _CHANCE1;  //wait shit.  move this to the C file. Don't really need this here.. I mean unless I pass it as a parameter

/** 
 * @brief Start the battle
 * @param player - pointer to the player, monster - pointer to the monster you're fighting
*/
void battle_start(Entity *player, Entity *monster);


/**
 * @brief End the battle
*/
void battle_end();

#endif