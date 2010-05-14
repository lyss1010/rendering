#include "Assignment0.h"

#include <math.h>
#include "Miro.h"
#include "Scene.h"
#include "Camera.h"
#include "Image.h"
#include "Console.h"

#include "PointLight.h"
#include "Sphere.h"
#include "Spirograph.h"
#include "TriangleMesh.h"
#include "Triangle.h"
#include "Vector3.h"
#include "Lambert.h"
#include "MiroWindow.h"
#include "DebugMem.h"

Assignment0::Assignment0()
{
}

Assignment0::~Assignment0()
{
}

void
Assignment0::makeSpiralScene()
{
    g_camera = new Camera;
    g_scene = new Scene;
    g_image = new Image;

    g_image->resize(512, 512);
    
    // set up the camera
    g_camera->setBGColor(Vector3(1.0f, 1.0f, 1.0f));
    g_camera->setEye(Vector3(-5, 2, 3));
    g_camera->setLookAt(Vector3(0, 0, 0));
    g_camera->setUp(Vector3(0, 1, 0));
    g_camera->setFOV(45);

    // create and place a point light source
    PointLight * light = new PointLight;
    light->setPosition(Vector3(-3, 15, 3));
    light->setColor(Vector3(1, 1, 1));
    light->setWattage(1000);
    g_scene->addLight(light);

    // create a spiral of spheres
    const int maxI = 200;
    const float a = 0.15f;
    for (int i = 1; i < maxI; ++i)
    {
        float t = i/float(maxI);
        float theta = 4*PI*t;
        float r = a*theta;
        float x = r*cos(theta);
        float y = r*sin(theta);
        float z = 2*(2*PI*a - r);

		float red = 0.5 * (1 + sin(theta));
		float green = 0.5 * (1 + cos(theta));
		float blue = 1 - (0.5 * (1 + sin(theta)));
		Material* mat = new Lambert(Vector3(red, green, blue));

        Sphere * sphere = new Sphere;
        sphere->setCenter(Vector3(x,y,z));
        sphere->setRadius(r/10);
        sphere->setMaterial(mat);
        g_scene->addObject(sphere);
    }
    
    // let objects do pre-calculations if needed
    g_scene->preCalc();
}

void
Assignment0::makeSpirographScene() {
	g_camera = new Camera;
    g_scene = new Scene;
    g_image = new Image;

    g_image->resize(512, 512);
    
    // set up the camera
    g_camera->setBGColor(Vector3(1.0f, 1.0f, 1.0f));
    g_camera->setEye(Vector3(-5, 2, 3));
    g_camera->setLookAt(Vector3(0, 0, 0));
    g_camera->setUp(Vector3(0, 1, 0));
    g_camera->setFOV(45);

    // create and place a point light source
    PointLight * light = new PointLight;
    light->setPosition(Vector3(-3, 15, 3));
    light->setColor(Vector3(1, 1, 1));
    light->setWattage(1000);
    g_scene->addLight(light);

	int numSpheres = 1500;
	Spirograph * spirograph = new Spirograph( 4.2, 18, 3.14, 6, 1.5, 10, numSpheres);	

    // create a spiral of spheres
    for (int i = 0; i < numSpheres; ++i)
    {
        float r = 1.0f;

		float t = i/float(numSpheres);
        float theta = 4*PI*t;
		float red = 0.5 * (1 + sin(theta));
		float green = 0.5 * (1 + cos(theta));
		float blue = 1 - (0.5 * (1 + sin(theta)));
		Material* mat = new Lambert(Vector3(red, green, blue));

        Sphere * sphere = new Sphere;
		sphere->setCenter(spirograph->getSphereLocation(i));
        sphere->setRadius(r/10);
        sphere->setMaterial(mat);
        g_scene->addObject(sphere);
    }
    
    // let objects do pre-calculations if needed
    g_scene->preCalc();

}