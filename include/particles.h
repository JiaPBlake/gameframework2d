#ifndef __PARTICLES_H__
#define __PARTICLES_H__

#include "simple_json.h"
#include "gfc_vector.h"
#include "gfc_color.h"

typedef struct {
	
	Uint8			_inuse;
	GFC_Vector2D	position;		//We can make use of the 3D nature  in a way that's specific to our game.  FOr the sake of demonstration in class,  we will ignore the Z component
	GFC_Vector2D	velocity;
	GFC_Vector2D	acceleration;
	GFC_Color		drawColor;
	GFC_Color		colorVector;
	//GFC_Color		colorDestination;		//gets rid of this,  because he has a Color_add funciton, which we're using in particle_draw()
	Uint32			ttl;			//Time to live in frames

}Particle;
 

void particle_system_init(Uint32 maxParticles);

Particle* particle_new();

void particle_free(Particle* particle);

void particle_system_draw();

void particle_draw(Particle* particle);

void particles_from_def(const char* filename, Uint32 count, GFC_Vector2D position, GFC_Vector2D direction, GFC_Vector2D acceleration);

Particle* particle_from_json(SJson* json, GFC_Vector2D position, GFC_Vector2D direction, GFC_Vector2D acceleration);


#endif