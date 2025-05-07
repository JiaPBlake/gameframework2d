#include "simple_logger.h"



#include "battle.h"

extern Uint8 _INBATTLE;

//As a result.  The battle is going to need access to the Player's info.  And the Monster's info
//#include "player.h"
//#include "monster.h"   //I JUST PEEPED I DON'T EVEN NEED THESE

void battle_start(Entity* player, Entity* monster) {
	
	slog("BATTLE START");
	
	GFC_Vector2D newposition = gfc_vector2d(monster->bounds.w*2, 0); // Move the player  2* the monster frame size over to the right
	gfc_vector2d_scale(newposition, newposition, -1);
	gfc_vector2d_add(player->position, player->position, newposition);


	//Hmm  maybe I should split it into battle_think  and Draw ?  where Draw is essentially my update function  :o

}

void battle_end() {
	//player->think = player_think;
	slog("The value of the _INBATTLE flag is: %i", _INBATTLE);
	_INBATTLE = 0;
}