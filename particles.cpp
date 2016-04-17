#ifndef _particles_cpp
#define _particles_cpp

#include <sb7.h>
#include <vmath.h>

// CPU representation of a particle
struct Particle {
	vmath::vec3 pos, speed;
	unsigned char r, g, b, a; // Color
	float size, angle, weight;
	float life; // Remaining life of the particle. if < 0 : dead and unused.
};

// Finds a Particle in ParticlesContainer which isn't used yet.
// (i.e. life < 0);
int static FindUnusedParticle(int * LastUsedParticle, Particle * ParticlesContainer, int * MaxParticles) {

	for (int i = *LastUsedParticle; i<*MaxParticles; i++) {
		if (ParticlesContainer[i].life < 0) {
			LastUsedParticle = &i;
			return i;
		}
	}

	for (int i = 0; i<*LastUsedParticle; i++) {
		if (ParticlesContainer[i].life < 0) {
			LastUsedParticle = &i;
			return i;
		}
	}

	return 0; // All particles are taken, override the first one
}

void static MainSimulationLoop(int * LastUsedParticle, Particle * ParticlesContainer, int * MaxParticles, float delta) {
	//// Simulate all particles
	//int ParticlesCount = 0;
	//for (int i = 0; i<MaxParticles; i++) {

	//	Particle& p = ParticlesContainer[i]; // shortcut

	//	if (p.life > 0.0f) {

	//		// Decrease life
	//		p.life -= delta;
	//		if (p.life > 0.0f) {

	//			// Simulate simple physics : gravity only, no collisions
	//			p.speed += vmath::vec3(0.0f, -9.81f, 0.0f) * (float)delta * 0.5f;
	//			p.pos += p.speed * (float)delta;
	//			p.cameradistance = glm::length2(p.pos - CameraPosition);
	//			//ParticlesContainer[i].pos += glm::vec3(0.0f,10.0f, 0.0f) * (float)delta;

	//			// Fill the GPU buffer
	//			g_particule_position_size_data[4 * ParticlesCount + 0] = p.pos.x;
	//			g_particule_position_size_data[4 * ParticlesCount + 1] = p.pos.y;
	//			g_particule_position_size_data[4 * ParticlesCount + 2] = p.pos.z;

	//			g_particule_position_size_data[4 * ParticlesCount + 3] = p.size;

	//			g_particule_color_data[4 * ParticlesCount + 0] = p.r;
	//			g_particule_color_data[4 * ParticlesCount + 1] = p.g;
	//			g_particule_color_data[4 * ParticlesCount + 2] = p.b;
	//			g_particule_color_data[4 * ParticlesCount + 3] = p.a;

	//		}
	//		else {
	//			// Particles that just died will be put at the end of the buffer in SortParticles();
	//			p.cameradistance = -1.0f;
	//		}

	//		ParticlesCount++;

	//	}
	//}
}

#endif