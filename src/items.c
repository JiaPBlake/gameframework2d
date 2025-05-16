#include "simple_logger.h"


#include "items.h"


static SJson* _itemJson = NULL; //Global variable to store the entire JSon file 'items.def'
static SJson* _itemDefs = NULL; //Global variable to store the JSon list of 'items':
static int numItems;

void items_close();

void items_initialize(const char* filename) {
	if (!filename) {
		slog("no filename provided for item initialization");
		return;
	}
	_itemJson = sj_load(filename);  //Making a JSON object out of the entire file
	if(!_itemJson) {
		slog("Failed to load the json for item definition");
		return;
	}
	//SPECIFICALLY extracting the masterlist of Items:
	_itemDefs = sj_object_get_value(_itemJson, "items");	//A JSon object/list containing our masterlist of Item Objects
	if (!_itemDefs) {
		slog("Item Def file '%s' does not contain a list of items", filename);
		sj_free(_itemJson);
		_itemJson = NULL;
		return;
	}

	numItems = sj_array_get_count(_itemDefs);

	slog("Items initialized");
	atexit(items_close);
}
// * NOTE abuot BOTH of these ^ \|/,  dynamically allocated Items are cleaned up BY inventory.   I don't have a system in this file to do it. So any Item I create through means of an Entity should EXPLCITLY be cleaned up by Entity
void items_close() {
	if (_itemJson) {
		sj_free(_itemJson);
	}
	_itemJson = NULL;
	_itemDefs = NULL; //Defs is only a POINTER to a JSON object I sj_load'ed.  And thus, it needn't be sj_free'd
	slog("Item Lists successfully closed");
}


SJson* items_get_def_by_name(const char* name) {
	if (!name) return NULL;
	if (!_itemDefs) {	//since this is our static variable,  once this is intialized TO BEGIN WITH. We will continuously refer back to it
		slog("no item definitions loaded");
		return NULL;
	}

	int i, c;
	SJson* item;
	const char* itemName = NULL;

	c = sj_array_get_count(_itemDefs);
	for (i = 0; i < c; i++) { //iterate through the list until we find what we're searching for
		item = sj_array_get_nth(_itemDefs,i);
		if (!item) continue;

		itemName = sj_object_get_value_as_string(item, "name"); //get the Name of each Item object
		if (!itemName) continue; //if no name, continue
		if ( gfc_strlcmp(name,itemName) == 0 ) { //Length compare to forgo matching ONLY on a substring
			//slog("Found the JSON object for item '%s' on iteration %i", name, i);	//Jlog
			return item; //found it
		}

	}
	slog("No item of name %s found in the list",name);
	return NULL;
}


//@brief return the configured Item object using the Definition object stored in the master list
Item* item_new(const char* name) {
	if (!name) { slog("No item name provided to make new item"); }
	Item* item;
	SJson* itemDef;

	itemDef = items_get_def_by_name(name);
	if (!itemDef) return NULL;

	item = gfc_allocate_array(sizeof(Item), 1);  //be sure to free this in the free function   [Gets cleaned up in inventory_cleanup()  in inventory.c  Since the only funciton that calls this one is inventory_add_item() ]
	if (!item) { slog("Unable to allocate array for new Item"); return NULL; }

	gfc_line_cpy(item->name, name);
	//then fill out the rest of the json fields

	const char* filename = NULL;
	filename = sj_object_get_string(itemDef, "sprite");
	if (filename) { //if it has a sprite
		GFC_Vector2D sp_sz = { 0 };
		sj_object_get_vector2d(itemDef, "sprite_size", &sp_sz);  //which we grabbed from gfc_config.h
		//slog("Sprite Size vector fetched from json object's x: %f", sp_sz.x);		//Jlog
		//For some reason this slog prints 0... but the saving from the sj obviously worked bc it loaded the sprite properly...
		Uint32 framesPerLine;
		sj_object_get_int32(itemDef, "spriteFPL", &framesPerLine);
		item->sprite = gf2d_sprite_load_all(
			filename,
			(Uint32)sp_sz.x,
			(Uint32)sp_sz.y,
			framesPerLine,
			0
		);
		//if (!self->sprite->surface) { slog("Entity has no sprite surface"); }  //for Window comparison... these don't have surfaces
		item->sprite_size = sp_sz;
		//item->center = gfc_vector2d(sp_sz.x * 0.5, sp_sz.y * 0.5);
		item->framesPerLine = framesPerLine;
		item->frame = 0; //Since frame 0 will be the default for every entity,  just et the first frame here in the configure function

		GFC_Vector4D bounds; //Use this as a 4D vector to save the numbers from the json file. THEN uset he get Rect from 4D vector function to save it into the entity's bounds
		sj_object_get_vector4d(itemDef, "bounds", &bounds);
		item->bounds = gfc_rect_from_vector4(bounds);

		//This would, ideally be its position in the Inventory UI

		GFC_Vector2D pos = { 0 };
		sj_object_get_vector2d(itemDef, "inven_Position", &pos);
		item->position = pos;
	}

	else { slog("Item does not have a sprite"); }
	item->count = 1;

	return item;
}

void item_free(Item* item) {
	if (!item) { slog("No item provided to free!!");  return; }

	//other cleanup for Items specifically goes here  (sprite)
	if (item->sprite) { //if the entity has a sprite.   THIS check is especially important since I'm calling this function from 2 different places. One of which would have cleaned up the sprite FOR me
		gf2d_sprite_free(item->sprite); //frees the spot IN the masterlist of Sprites.   gf2d_sprite.h takes care of actually deleting things.
	}

	free(item); //free the dynamically allocated Item object variable 'item'
	slog("Item freed");
}


//implement this after inventory-specific Items
					//where the defFile parameter instead serves as the name of the item as it's stored in items.def
Entity* item_new_entity(GFC_Vector2D position, const char* defFile) { //matching the parameters in the definition JUST so that it can work with the spawn funciton..
	Entity* self;		//This 'item' (I'll call it self to be consistent)  is an Entity now. It's being:
	SJson* itemDef;

	//slog("In my item new entity function trying to spawn Item name: '%s'",defFile);		//Jlog

	self = entity_new();			//CREATED BY  the entity manager.  STORED in the entity manager.   and will be cleaned up by the entity manager
	if (!self) {
		slog("failed to spawn a new monster entity");
		return NULL;
	}

	itemDef = items_get_def_by_name(defFile);
	entity_configure(self, itemDef);
	if (position.x >= 0) { gfc_vector2d_copy(self->position, position); } //position override from the parameters

	self->team = ETT_item; //set it to team Item
	//slog("Item Entity configured to have the following type (should be 4): %i",self->team);


	self->data_free = NULL;
	//self->data_free = item_free; //Set this so that any Items created as Entities can clean up their  Item-specific related things
	//wh-  wait a minute.  I highkey don't even need this. My purpose right new is to make an entity using a specific Def file.  which is what ALL entities have been doing !!
	//this function RETURNS an Entity !!  That's all it is. It's not an Item at all. It has no Item-specific data to free!!!  The entity system can take care of its sprite

	//if I ever NEED a thing on the groun to have data then... I mean sure. Yeah I'll need that but !! Not right now :triumph:

	return self;
}

////Just so that I don't have to override anything about my entity spawn system.  I'll change ITEM to fit IT.
//Entity* item_new_spawn(GFC_Vector2D position, const char* defFile) {
//
//}


