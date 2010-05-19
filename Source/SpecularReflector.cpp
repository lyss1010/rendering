#include "SpecularReflector.h"
#include "Ray.h"
#include "Scene.h"
#include "DebugMem.h"
#include "EnvironmentMap.h"

#include <assert.h>

SpecularReflector::SpecularReflector( const Vector3 & kd ) :
Lambert(kd)
{
	m_type = Material::SPECULAR_REFLECTOR;
}

SpecularReflector::~SpecularReflector()
{
}

Vector3
SpecularReflector::shade( const Ray& ray, const HitInfo& hit, const Scene& scene ) const
{
	static int numRecursiveCalls = 0;
	// we've maxed out our recursion
	if( numRecursiveCalls == Material::SPECULAR_RECURSION_DEPTH )
	{
		return Vector3(0,0,0);
	}

	numRecursiveCalls++;

	// direction to last "eye" point reflected across hit surface normal
	Vector3 reflectDir = -2 * dot(ray.d, hit.N) * hit.N + ray.d;
	reflectDir.normalize();
	
	Ray reflectedRay;
	reflectedRay.o = hit.P;
	reflectedRay.d = reflectDir;
	reflectedRay.refractiveIndex = ray.refractiveIndex;

	Vector3 L;
	HitInfo recursiveHit;
	if( scene.trace( recursiveHit, reflectedRay, epsilon, MIRO_TMAX ) )
		L = m_kd * recursiveHit.material->shade( reflectedRay, recursiveHit, scene );
	else
	{
		if( USE_ENVIRONMENT_MAP && scene.environmentMap() )
		{
			L = EnvironmentMap::lookUp( reflectedRay.d, scene.environmentMap(), scene.mapWidth(), scene.mapHeight() );
		}
		else
		{
			L = m_kd;
		}
	}

	// add in the phong highlights (if necessary)
	if( m_phong_exp != 0 )
	{
		const Lights *lightlist = scene.lights();
    
		// loop over all of the lights
		Lights::const_iterator lightIter;
		for (lightIter = lightlist->begin(); lightIter != lightlist->end(); lightIter++)
		{
			PointLight* pLight = *lightIter;

			Vector3 l = pLight->position() - hit.P;
			l.normalize();

			Vector3 viewDir = -ray.d;
			Vector3 lightReflectDir = 2 * dot( l, hit.N ) * hit.N - l; // direction to light reflected across normal
			float viewDirDotReflectDir = dot( viewDir, lightReflectDir );
			if( viewDirDotReflectDir > 0 )
				L += std::max(0.0f, pow(viewDirDotReflectDir, m_phong_exp)) * pLight->color();
		}
	}

	// add ambient color
	L += m_ka;

	// we're about to pop a (potentially) recursive call off the stack
	numRecursiveCalls--;

	return L;
}