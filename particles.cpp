#ifndef _particles_cpp
#define _particles_cpp

#include <sb7.h>
#include <vmath.h>
#include <cstdlib>

// CPU representation of a particle
struct Particle {
	vmath::vec3 pos, speed;
	vmath::vec4 color; // Color
	float size, angle, weight;
	float life; // Remaining life of the particle. if <0 : dead and unused.
	float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

	bool operator<(const Particle& that) const {
		// Sort in reverse order : far particles drawn first.
		return this->cameradistance > that.cameradistance;
	}
};

#endif