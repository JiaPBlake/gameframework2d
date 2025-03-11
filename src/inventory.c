#include "simple_logger.h"

#include "inventory.h"

//initialized upon creating a new Player
void inventory_init(Inventory* inventory) {
	if (!inventory) return;

	inventory->itemsList = gfc_list_new();
}

void inventory_cleanup(Inventory* inventory) {

	if (!inventory) return;

	gfc_list_foreach( inventory->itemsList, (gfc_work_func*)item_free ); //each thing in my list will be called with item_free
	gfc_list_delete(inventory->itemsList);
	inventory->itemsList = NULL;
	//all clean :)

}

Item* inventory_get_item_by_name(Inventory* inventory, const char* name) {
	if (!inventory) return NULL;

	Item* item;
	int i, c;
	c = gfc_list_get_count(inventory->itemsList);
	for (i = 0; i < c; i++) {
		item = gfc_list_get_nth(inventory->itemsList, i);
		if (!item) continue;

		if (gfc_strlcmp(item->name, name) == 0) { //Length compare to forgo matching ONLY on a substring
			return item; //found it
		}
	}
	slog("Could not find item '%s' in Inventory");
	return NULL; //defualt final case
}

void inventory_add_item(Inventory* inventory, const char* name) {
	Item* item;
	if ((!inventory) || (!name)) return;

	item = inventory_get_item_by_name(inventory, name);
	if (item) {
		item->count++;
		return;
	}

	item = item_new(name);
	if (!item) return;
	
	gfc_list_append(inventory->itemsList, item);
	slog("Item '%s' added to inventory");

}

void inventory_remove_item(Inventory* inventory, const char* name) {
	Item* item;
	if ((!inventory) || (!name)) return;

	item = inventory_get_item_by_name(inventory, name);
	if (item) {
		if (item->count > 0) {
			slog("Item count decreasing");
			item->count--;
		}
		else {
			slog("Trying to remove the item from the Inventory's list and Free the item");
			//check the order for this, but I'm pretty sure I can remove it from my list THEN free it
			gfc_list_delete_data(inventory->itemsList, item); //takes a void pointer,  so.. any kind of pointer tbh
			item_free(item);
		}
		return;
	}
	slog("Item '%s' not found in Inventory");

}

//To be used in player.c  which only has access to inventory
void display_item(Inventory* inventory, const char* name) {
	Item* item;
	if ((!inventory) || (!name)) return;

	item = inventory_get_item_by_name(inventory, name);

	if (item->sprite) {
		gf2d_sprite_draw(item->sprite,
			item->position,	//position      without offset we use self's position.  WITH the camera's offset, we use the position vector created above 
			NULL,			//scale
			NULL,		//center which is a 2D vector
			NULL,		//rotation
			NULL,		//flip
			NULL,		//colorShift
			0);
	}
	else { slog("Item %s has no sprite to display", name); }
}


