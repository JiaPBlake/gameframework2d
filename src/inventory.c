#include "simple_logger.h"

#include "inventory.h"
#include "gf2d_graphics.h"

//initialized upon creating a new Player
void inventory_init(Inventory* inventory) {
	if (!inventory) return;

	inventory->sprite = gf2d_sprite_load_all(
		"images/ui/inventory.png",
		540,
		300,
		1,
		0
	);
	if (!inventory->sprite) { slog("Inventory sprite wasn't able to load for some reason"); return; }
	inventory->position = gfc_vector2d(20, 20);

	inventory->itemsList = gfc_list_new();
	if (!inventory->itemsList) { slog("Inventory unable to create a new itemsList"); return; }
	
	slog("Inventory successfully initialized (it's empty rn)");

	//inventory_layer_build("");
	//slog("Inventory Sprite Surface successfully created");
}


void inventory_cleanup(Inventory* inventory) {

	if (!inventory) return;		//I could just copy this gfc_list_foreach method into world.. make an Item List instead of tryna 

	//clean up the sprite
	if (inventory->sprite) {
		gf2d_sprite_free(inventory->sprite); //frees the spot IN the masterlist of Sprites.   gf2d_sprite.h takes care of actually deleting things.
		slog("Inventory's Sprite freed");
	}
	//Free the layer made from the Sprite and all the items		nvm doesn't have a layer anymore
	//if (inventory->layer) gf2d_sprite_free(inventory->layer);
	//slog("Inventory's Layer freed");

	//this'll take care of the Item's individual sprites
	gfc_list_foreach( inventory->itemsList, (gfc_work_func*)item_free ); //each thing in my list will be called with item_free
	gfc_list_delete(inventory->itemsList);
	inventory->itemsList = NULL;
	//all clean :)
	slog("Inventory's itemsList gfc_list deleted");
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
	slog("Could not find item '%s' in Inventory", name);
	return NULL; //defualt final case
}

void inventory_add_item(Inventory* inventory, const char* name) {
	Item* item;
	if ((!inventory) || (!name)) return;

	item = inventory_get_item_by_name(inventory, name);
	if (item) {
		slog("Item already exists in inventory");
		item->count++;
		return;
	}

	else { slog("Item does not currently exist in inventory. Creating it and adding it now"); }
	item = item_new(name);
	if (!item) return;
	
	gfc_list_append(inventory->itemsList, item);
	slog("Item '%s' added to inventory",item->name);

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
	//slog("and by now, I would have drawn the sprite");
	if (!item) { slog("Item cannot be displayed"); return; }

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

void inventory_draw(Inventory* inventory) {
	//just straight up draw every item in our inventory lol
	if (!inventory) { slog("No valid inventory provided to draw"); return; }

	if (!inventory->sprite) { slog("Inventory does not have sprite. Cannot draw"); return; }

	//slog("Drawing inventory sprite");
	gf2d_sprite_draw(inventory->sprite,
		inventory->position,	//position      without offset we use self's position.  WITH the camera's offset, we use the position vector created above 
		NULL,			//scale
		NULL,		//center which is a 2D vector
		NULL,	//rotation
		NULL,		//flip
		NULL,		//colorShift
		0);

	//Draw items:
	int i, c;
	Item* item;
	int frame;
	if (inventory->itemsList) {
		//slog("Drawing items now");
		c = gfc_list_count(inventory->itemsList);
		for (i = 0; i < c; i++) {
			item = gfc_list_nth(inventory->itemsList, i);
			//if(ent->name) slog("ent grabbed: %s",ent->name); //Jlog
			if (!item) continue;
			frame = 0;

			gf2d_sprite_draw(item->sprite,
				item->position,	//position      without offset we use self's position.  WITH the camera's offset, we use the position vector created above 
				NULL,			//scale
				NULL,		//center which is a 2D vector
				NULL,	//rotation
				NULL,		//flip
				NULL,		//colorShift
				frame);
		}
	}
}


