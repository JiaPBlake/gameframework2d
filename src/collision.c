#include <SDL.h>
#include "simple_logger.h"

#include "collision.h"

//yeahhh I'ma just use the one he made highkey..

										//My dumbass forgot the Bounds were an offset .  I was treating Position like it was BOUNDS's x,y a.k.a the corner this whole time . Position is the CENTER BRUHH
//Owner  a.k.a Player, P;  Other, O.
Bool collision_check(Entity* owner, Entity* other) {
	//Check for all the false cases, and return False if those cases are passed
	//slog("Owner Position.x = %f", owner->position.x + owner->bounds.w);

	//slog("Dragon's Position.x = %f", other->position.x);

//Owner's Right Edge.  Other's Left edge:   P|   |O
	if ((owner->position.x - owner->bounds.x) < (other->position.x + other->bounds.x)) { //Only the x-coord's matters
		return false;
	}
//Owner's Left Edge.  Other's Right edge:   O|   |P
	if ( (owner->position.x + owner->bounds.x) > (other->position.x - other->bounds.x)) { //Only the x-coord's matters
		return false;
	}

////Owner's Top Edge.  Other's Bottom edge:   P\   \O
//	if ( (owner->position.y + owner->bounds.y) < (other->position.y - other->bounds.y)) { //Only the y-coord's matters
//		return false;
//	}
////Owner's Bottom Edge.  Other's Top edge:   P/   /O
//	if ((owner->position.y - owner->bounds.y) > (other->position.y + other->bounds.y)) { //Only the y-coord's matters
//		return false;
//	}
	return true;
}

//It IS infinitely easier to just do what he did . LMAOO
