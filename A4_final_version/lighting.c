/*
 * deals with lights/shading functions
 *
 *	John Amanatides, Oct 2017
 */
/*
Name: Nisha Shlaymoon
Student Number: 213-415-864
Partner: Michael Founk
Student Number: 213641204
*/
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include "artInternal.h"

#define CHECKERBOARD    1
#define ZONE_PLATE      2


#define MAX_RECURSION	10

extern double	Normalize(Vector *);
extern Vector	ReflectRay(Vector, Vector);
extern int	IntersectScene(Ray *, double *, Vector *, Material *);
extern int	ShadowProbe(Ray *, double);
extern int TransmitRay(Vector, Vector, double, double, Vector *);
extern double frandom();

typedef struct LightNode {
	Point position;
	double intensity;
	double radius;
	struct LightNode *next;
} LightNode;

static LightNode *lights;
static Color	background;
static Material	currentMaterial;
static Color black= {0.0, 0.0, 0.0}, white= {1.0, 1.0, 1.0};


char *art_Light(double x, double y, double z, double intensity, double radius)
{
	LightNode *newLight;

	if(intensity <= 0.0 || radius < 0.0)
		return "art_Light: domain error";
	newLight= (LightNode *) malloc(sizeof(LightNode));
	newLight->position.v[0]= x;
	newLight->position.v[1]= y;
	newLight->position.v[2]= z;
	newLight->intensity= intensity;
	newLight->radius= radius;
	newLight->next= lights;
	lights= newLight;

	return NULL;
}


char *art_Material(Material material)
{
	currentMaterial= material; /* should really check for mistakes */
	return NULL;
}


Material GetCurrentMaterial(void)
{
	return currentMaterial;
}


char *art_Background(Color color)
{
	if(color.v[0] < 0.0 || color.v[0] > 1.0 || color.v[1] < 0.0 || color.v[1] > 1.0 || color.v[2] < 0.0 || color.v[2] > 1.0)
		return "art_Background: domain error";
	background= color;
	return NULL;
}

/* for A4 */
static Color Texture(Material *material, Point position)
{               
	int funnySum;
	double EPSILON= 0.0001;
	double contribution;
	Color result;

	switch(material->texture) {

	case CHECKERBOARD: 
		funnySum= floor(position.v[0]+EPSILON)
			+ floor(position.v[1]+EPSILON)
			+ floor(position.v[2]+EPSILON);
		if(funnySum % 2)
			return white;
		else    return material->col;
	case ZONE_PLATE:
		contribution= 0.5*cos(DOT(position, position))+0.5;
		TIMES(result, material->col, contribution);
		return result;  
	default:                
	return material->col;
	}       
}       

/*
 * a simple shader
 */
static Color ComputeRadiance(Ray *ray, double t, Vector normal, Material material, int timesToRecurse)
{
	//What to return in the end
	Color result = white;		
	Vector currentPosition;
	//All color components
	Color surfaceColor,
	      diffuseColor, 
	     specularColor, 
	         reflected, 
	       transmitted;
	Ray shadowRay;
	Vector toLight, reflectDirection;
	Vector dir = ray->direction;
	double thetaAngle,	//Angle
	       alphaAngle,	//Angle
       	    lightDistance, 	//Distance to light
              diffuseFrac, 	//Diffuse
             specularFrac, 	//Frac
                lightFrac;
	LightNode *lP;  	//Primitive element in our linked list
	Point lightPosition;	
	Point randomPosition;
	(void) Normalize(&normal);
	//Find intersection point 
	TIMES(currentPosition, dir, t);
	PLUS(currentPosition, ray->origin, currentPosition);
	//No texture detected here
	if(material.texture == 0)
	{
		surfaceColor = material.col;
	}
	else
	{
		surfaceColor = Texture(&material, currentPosition);
	}

	// Ambient light initial calculation
	TIMES(result, surfaceColor, material.Ka);
	reflectDirection = ReflectRay(dir, normal);
	/* Loop through each primitve in the linked list and------ 
	---perform specular, diffuse and ambient light calculation
	---and other additional calculations.-------------------*/
	for(lP = lights ; lP != NULL ; lP = lP->next){
		/*Randomize position*/
		randomPosition.v[0] = (2*frandom() -1.0) * lP->radius;
		randomPosition.v[1] = (2*frandom() -1.0) * lP->radius;
		randomPosition.v[2] = (2*frandom() -1.0) * lP->radius;
		/* Add the random position and the primitive object
		 * Position and then stubtract from the light source*/
		PLUS(lightPosition, lP->position, randomPosition);
		MINUS(toLight, lightPosition, currentPosition);
		lightDistance = Normalize(&toLight);
		shadowRay.origin = currentPosition;
		shadowRay.direction = toLight;
		//Find the angle of reflection
		thetaAngle = DOT(toLight, normal);
		/*diffuse light calculation*/
		if(thetaAngle > 0 && ShadowProbe(&shadowRay, lightDistance) != HIT){
			/*Since intensity decreases by r^2 of the distance*/
			lightFrac = lP -> intensity / (lightDistance * lightDistance);
			diffuseFrac = lightFrac *material.Kd*thetaAngle;
			TIMES(diffuseColor, surfaceColor, diffuseFrac);
			//diffuse
			PLUS( result, diffuseColor, result);
		}
		//-----------------------------------------------------------------
		/*Specular light source calculation*/
		alphaAngle = DOT(toLight, reflectDirection);
		if (alphaAngle > 0.0 && ShadowProbe(&shadowRay, lightDistance) != HIT)
		{
			specularFrac = lightFrac * material.Ks*pow(alphaAngle, (double) material.n);
			// Assuming no metals.
			TIMES (specularColor, white, specularFrac);
			//specular
			PLUS(result, specularColor, result);
		}
		/*Reflection and refraction calculations*/
		/*Do ray tracing recursively*/
		if(timesToRecurse < MAX_RECURSION)
		{
			double outerToInner = DOT(reflectDirection, normal);
			if(material.Kt > 0.0)
			{
				// i and j are just indices
				double intersectionObject, i , j;
				Vector transmitNormal;
				Material transmitMaterial;
				Ray transmissionRay;
				transmissionRay.origin = currentPosition;
				/*----------------------------------------*/
				if(outerToInner > 0.0)
				{
					i = 1.0;
					j = material.index;
				}
				else
				{
					i = material.index;
					j = 1.0;
				}
				/* Compute transmitted direction vector, given incident, normal vectors and IRs.
				 * returns 0 if maxed internal reflection
				 */
				int transmitted = TransmitRay(dir, normal, i, j, &(transmissionRay.direction));
				
				if(transmitted && IntersectScene(&transmissionRay, &intersectionObject, &transmitNormal, &transmitMaterial) == HIT)
				{
					Color transmitColor = ComputeRadiance(&transmissionRay, intersectionObject, transmitNormal, transmitMaterial, timesToRecurse + 1);
					TIMES(result, transmitColor, material.Kt);
				}
			}
			if(material.Kr > 0.0)
			{
				double relfectionObject;
				Vector reflectNormal;
				Material reflectMaterial;
				Ray reflectRay;
				/*Prepare our ray to intersetct with the scene*/
				reflectRay.origin = currentPosition;
				reflectRay.direction = ReflectRay(dir, normal);
				reflectRay.direction = ReflectRay(dir, normal);
				/*----------------------------------------*/
				//If you catch an intersection
				if(IntersectScene(&reflectRay, &relfectionObject, &reflectNormal, &reflectMaterial) == HIT)
				{
					Color reflectColor = ComputeRadiance(&reflectRay, relfectionObject, reflectNormal, reflectMaterial, timesToRecurse + 1);
					TIMES(result, reflectColor, material.Kr);
				}
			} 
		}
	}
return result;
}


Color
GetRadiance(Ray *ray)
{
	double t;
	Vector normal;
	Material material;

	if(IntersectScene(ray, &t, &normal, &material) == HIT)
		return ComputeRadiance(ray, t, normal, material, 0);
	else	return background;
}


void InitLighting()
{
	Material material;

	material.col= white;
	material.Ka= 0.2;
	material.Kd= 0.6;
	material.Ks= 0.7;
	material.n= 50.0;
	material.Kr= 0.0;
	material.Kt= 0.0;
	material.index= 1.0;
	(void) art_Material(material);
	(void) art_Background(black);

	lights= NULL;
}


void FinishLighting()
{
	LightNode *node;

	while(lights) {
		node= lights;
		lights= lights->next;

		free((void *) node);
	}
}
