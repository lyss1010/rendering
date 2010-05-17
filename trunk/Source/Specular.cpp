#include "Specular.h"
#include "Ray.h"
#include "Scene.h"
#include "DebugMem.h"

#include <assert.h>

const int Specular::RECURSION_DEPTH = 20;

Specular::Specular( const Vector3 & kd ) :
m_kd(kd)
{
	m_type = Material::SPECULAR;
}

Specular::~Specular()
{
}

Vector3
Specular::shade( const Ray& ray, const HitInfo& hit, const Scene& scene ) const
{
	bool hitSomething;
	HitInfo recursiveHit = hit;
	Ray reflectedRay = ray;
	int numRecursiveCalls = 0;
	do
	{
		// direction to last "eye" point reflected across hit surface normal
		Vector3 reflectDir = -2 * dot(reflectedRay.d, recursiveHit.N) * recursiveHit.N + reflectedRay.d;
		reflectDir.normalize();
		
		reflectedRay.o = recursiveHit.P;
		reflectedRay.d = reflectDir;
		hitSomething = scene.trace( recursiveHit, reflectedRay, epsilon, MIRO_TMAX );

		numRecursiveCalls++;
	}
	while( hitSomething && recursiveHit.material->getType() == Material::SPECULAR && numRecursiveCalls < RECURSION_DEPTH );

	// we maxed out our recursion by hitting a specular surface or we simply didn't hit anything
	if( !hitSomething || recursiveHit.material->getType() == Material::SPECULAR )
	{
		return m_kd;
	}
	else
	{
		return m_kd * recursiveHit.material->shade( reflectedRay, recursiveHit, scene );
	}
}