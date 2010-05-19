#include "SpecularRefractor.h"
#include "Ray.h"
#include "Scene.h"
#include "DebugMem.h"
#include "EnvironmentMap.h"

#include <assert.h>

SpecularRefractor::SpecularRefractor( const float & refractiveIndex, const Vector3 & kd ) :
Lambert(kd)
{
	m_refractive_index = refractiveIndex;
	m_type = Material::SPECULAR_REFRACTOR;
}

SpecularRefractor::~SpecularRefractor()
{
}

float 
SpecularRefractor::getRefractiveIndex( RefractiveMaterial material )
{
	// these indeces come from http://www.robinwood.com/Catalog/Technical/Gen3DTuts/Gen3DPages/RefractionIndexList.html
	float index = 1.0f;

	switch( material )
	{
	case WATER_100_C:
		index = 1.31766;
		break;
	case WATER_0_C:
		index = 1.33346;
		break;
	case WATER_20_C:
		index = 1.33283;
		break;
	case DIAMOND:
		index = 2.417;
		break;
	case MILK:
		index = 1.35;
		break;
	case ICE:
		index = 1.309;
		break;
	case GLASS_COMMON:
		index = 1.52;
		break;
	case GLASS_PYREX:
		index = 1.474;
		break;
	}
	
	return index;
}

Vector3 
SpecularRefractor::shade(const Ray& ray, const HitInfo& hit,const Scene& scene) const
{
	static int numRecursiveCalls = 0;
	// we've maxed out our recursion
	if( numRecursiveCalls == Material::SPECULAR_RECURSION_DEPTH )
	{
		return Vector3(0,0,0);
	}

	numRecursiveCalls++;

	Vector3 viewDir = -ray.d; // d is a unit vector
	// set up refracted ray here
	float nDotViewDir = dot( viewDir, hit.N );

	float refractedRayIndex;
	float n1;
	float n2;
	Vector3 normal;

	bool flipped = false;
	// we're entering the refracted material
	if( nDotViewDir > 0 )
	{
		n1 = ray.refractiveIndex;
		n2 = hit.material->getRefractiveIndex();
		normal = hit.N;
		refractedRayIndex = hit.material->getRefractiveIndex();
	}
	// we're exiting the refractive material
	else
	{
		n1 = hit.material->getRefractiveIndex();
		n2 = 1.0f; // assume we're entering air
		normal = -hit.N;
		refractedRayIndex = 1.0f;
		nDotViewDir *= -1;
		flipped = true;
	}

	float refractiveIndexRatio = n1 / n2;
	float radicand = 1 - ( refractiveIndexRatio * refractiveIndexRatio ) * ( 1 - ( nDotViewDir * nDotViewDir ) );

	// direction to last "eye" point reflected across hit surface normal
	Vector3 reflectDir = -2 * dot(ray.d, hit.N) * hit.N + ray.d;
	reflectDir.normalize();

	Vector3 L;

	// total internal refraction: just use reflection instead
	if( radicand < 0 )
	{
		// if we've flipped the normal, we're INSIDE the surface. we don't need to shade it in that case.
		if( flipped )
		{
			numRecursiveCalls--;
			return Vector3(0,0,0);
		}

		Ray reflectedRay;
		reflectedRay.o = hit.P;
		reflectedRay.d = reflectDir;
		reflectedRay.refractiveIndex = ray.refractiveIndex;

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
				L = m_kd * Vector3(0.5f);
			}
		}
	}
	// use refraction
	else {
		Ray refractedRay;
		Vector3 refractDir = -refractiveIndexRatio * ( viewDir - nDotViewDir*normal ) - sqrt(radicand)*normal;
		refractDir.normalize();

		refractedRay.d = refractDir;
		refractedRay.o = hit.P;
		refractedRay.refractiveIndex = refractedRayIndex;

		HitInfo recursiveHit;
		// trace the refracted ray
		if( scene.trace( recursiveHit, refractedRay, epsilon, MIRO_TMAX ) )
			L = m_kd * recursiveHit.material->shade( refractedRay, recursiveHit, scene );
		else
		{
			if( USE_ENVIRONMENT_MAP && scene.environmentMap() )
			{
				L = EnvironmentMap::lookUp( refractedRay.d, scene.environmentMap(), scene.mapWidth(), scene.mapHeight() );
			}
			else
			{
				L = m_kd * Vector3(0.5f);
			}
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

			Vector3 lightReflectDir = 2 * dot( l, hit.N ) * hit.N - l; // direction to light reflected across normal
			float viewDirDotReflectDir = dot( viewDir, lightReflectDir );
			if( viewDirDotReflectDir > 0 )
				L += std::max(0.0f, pow(viewDirDotReflectDir, m_phong_exp)) * pLight->color();
		}
	}

	L += m_ka;
	
	numRecursiveCalls--;
	return L;
}