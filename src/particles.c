#include "simple_logger.h"

#include "gfc_config.h"
#include "gfc_config_def.h"
#include "gf2d_draw.h"


#include "particles.h"

typedef struct {
	Particle* particleList;
	Uint32 particleCount;

}ParticleManager;

static ParticleManager particleManager = { 0 };   //static so that it's effectively private (only accessible) within this file

void particle_system_close();

void particle_system_init(Uint32 maxParticles) {
	if (!maxParticles) {
		slog("Cannot initialize particle system for zero particles");
		return;
	}

	if (particleManager.particleList) {
		slog("particle manager already initialized");
		return;
	}
	particleManager.particleList = gfc_allocate_array(sizeof(Particle), maxParticles);    //We're using an ARRAY instead of a LIST  to be more efficient.   All the particles are right there.  AND we have memory locality (We shouldn't have to page swap often when iterating through this list.  Matter of fact it should only take up 1 page in memory)
	particleManager.particleCount = maxParticles;

	atexit(particle_system_close);

}

void particle_system_close() {
	//right now we have no extra cleanup.  (If we add sprites,  we will then ahve extra cleanup . )

	if (particleManager.particleList)  free(particleManager.particleList);
	particleManager.particleList = NULL;
	particleManager.particleCount = 0;
}

Particle* particle_new() {
	int i;
	for (i = 0; i < particleManager.particleCount; i++) {
		if (!particleManager.particleList[i]._inuse) {		//if this space in List is NOT in use.  Give us this space.
			memset(&particleManager.particleList[i], 0, sizeof(Particle));  //since it's not in use.  Clear it
			particleManager.particleList[i]._inuse = 1;		//set it to NOW be in use
			return &particleManager.particleList[i];		//return that spot  so we may, in fact, use it :)
		}

	}
	return NULL;
}

void particle_free(Particle* particle) {
	if (!particle) return;
	particle->_inuse = 0;
}

void particle_draw(Particle* particle) {
	if (!particle) return;
	particle->ttl--;
	if (particle->ttl <= 0) {
		particle_free(particle);  //Once its ttl is up.   Free it, and return.  THIS is out we get rid of our particles.
		return;
	}

	gfc_vector2d_add(particle->position, particle->position, particle->velocity);
	gfc_vector2d_add(particle->velocity, particle->velocity, particle->acceleration);
	gfc_color_add(&particle->drawColor, particle->drawColor, particle->colorVector); //

	gf2d_draw_pixel(particle->position, particle->drawColor);  //This funciton takes a 2D !! vector.  and a color



}

void particle_system_draw() {
	int i;
	for (i = 0; i < particleManager.particleCount; i++) {
		if (!particleManager.particleList[i]._inuse) continue;

		particle_draw(&particleManager.particleList[i]);
	}
}



void particles_from_def(const char* filename, Uint32 count, GFC_Vector2D position, GFC_Vector2D direction, GFC_Vector2D acceleration) {
	SJson* json;
	int i;
	json = gfc_config_def_get_by_name("particles", filename);
	if (!json) {
		return;
	}

	for (i = 0; i < count; i++) {
		particle_from_json(json, position, direction, acceleration);	//you can change these parameters as you need	
	}
	sj_free(json);

}

Particle* particle_from_json(SJson* json, GFC_Vector2D position, GFC_Vector2D direction, GFC_Vector2D acceleration) {	//you can change these parameters as you need
	float speed = 0;   //we'll take speed as a parameter for the particle
	float speedVariance = 0;		//We can vary what the speed is. 
	float angleVariance = 0;
	Uint32 ttlVariance = 0;
	GFC_Color colorVariance = { 0 };
	Particle* p;
	if (!json) return NULL;
	p = particle_new();
	if (!p) return NULL;

	gfc_vector2d_copy(p->position, position);
	gfc_vector3d_normalize(&direction);

	sj_object_get_float(json, "angleVariance", &angleVariance);
	angleVariance *= GFC_DEGTORAD;		//THE MATH IS IN RADIANS.   but if a Human writes it, might has well accept Degrees as input.  SO  here's this conversion step
	angleVariance *= gfc_crandom();			//now, if I want. I can take angle variances.  So if I'm getting Direction as my parameter.  IF I want variation in the angle that it flies: in my json file, I could input a nonzero AngleVariance
	direction = gfc_vector2d_rotate(direction, angleVariance);
	gfc_vector2d_copy(p->velocity, direction);
	


	sj_object_get_float(json, "speed", &speed);
	sj_object_get_float(json, "speedVariance", &speedVariance);
	speed += gfc_crandom() * speedVariance;  //where Crandom, we got from Quak,e  which can possibly give us a negative value
	gfc_vector2d_scale(p->velocity, p->velocity, speed);
	gfc_vector2d_copy(p->acceleration, acceleration);

	sj_object_get_color_value(json, "drawColor", &p-> drawColor);
	sj_object_get_color_value(json, "colorVector", &p->colorVector);
	if (sj_object_get_color_value(json, "drawColor", &p->drawColor) ) {
		colorVariance.r *= gfc_crandom();
		colorVariance.g *= gfc_crandom();
		colorVariance.b *= gfc_crandom();
		colorVariance.a *= gfc_crandom();
		
		gfc_color_add(&p->drawColor,  p->drawColor, colorVariance);		//destination and source are the same.  I'm adding the Variance to it
	}

	sj_object_get_uint32(json, "ttl", &p->ttl);
	if (sj_object_get_float(json, "ttlVariance", &ttlVariance)) {
		p->ttl += gfc_crandom() * ttlVariance;  //where Crandom, we got from Quak,e  which can possibly give us a negative value
	}


}
