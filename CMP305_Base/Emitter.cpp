#include "Emitter.h"

Emitter::Emitter(XMFLOAT3 position)
	: position_(position)
{
	emitter_behaviour_ = EmitterBehaviour::kDefault;
}

Emitter::~Emitter()
{
}

Particle Emitter::dropParticle()
{
	Particle particle;

	if (emitter_behaviour_ == EmitterBehaviour::kDefault)
	{
		particle.position = position_;
	}

	return particle;
}
