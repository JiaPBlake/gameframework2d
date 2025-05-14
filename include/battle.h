#ifndef __BATTLE_H__
#define __BATTLE_H__

//I think how I'll do this is.   Entity -> Battle  & Player & Montser.   and then in Planyer and Monster I can include battle.  that shouldn't be circular.
//So this file and all its functions will be responsible for Attacks and retrieving / updating stats ???


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


void player_think_battle(Entity* self);



#endif