#include "simple_logger.h"



#include "battle.h"

//As a result.  The battle is going to need access to the Player's info.  And the Monster's info
//#include "player.h"
//#include "monster.h"   //I JUST PEEPED I DON'T EVEN NEED THESE
#include "ui.h"

void battle_start(Entity* player, Entity* monster) {
	
	slog("BATTLE START");
	
	GFC_Vector2D newposition = gfc_vector2d(monster->bounds.w*2, 0); // Move the player  2* the monster frame size over to the right
	gfc_vector2d_scale(newposition, newposition, -1);
	gfc_vector2d_add(player->position, player->position, newposition);


}

void battle_end() {
	//player->think = player_think;
}