/*
 * implement ray/cone intersection routine
 *
 *	John Amanatides, Oct 2017
 */


#include <math.h>
#include "artInternal.h"

extern double Normalize();

#define EPSILON	(1e-10)

/*
 * compute intersection between ray and a cone (0<= y <= 1, x^2 + z^2 <= y^2)
 * Returns MISS if no intersection; otherwise, it returns HIT and
 * sets t to the the distance  to the intersection point and sets the
 * normal vector to the outward facing normal (unit length) at the
 * intersection point.  Note: no intersection is returned if myT >= t
 * (ie my intersection point is further than something else already hit).
 */

/*
Name: Nisha Shlaymoon
Student Number: 213-415-864
Partner: Michael Founk
Student Number: 213641204
*/

int IntersectCone(Ray *ray, double *t, Vector *normal)
{
	/* your code goes here */
	double a, b, c, x, y, z;
	double d, myT,t0,t1;
	double r1, r2;
	int result = MISS;
 	Point start = ray->origin;
 	Vector direction = ray->direction;

    // a = xD^2 + yD^2 - zD^2
 	a = direction.v[0] * direction.v[0]
	+ direction.v[2] * direction.v[2]
	- (direction.v[1] * direction.v[1]);
 	//b = 2xExD + 2yEyD - 2zEzD
 	b = (2.0 * start.v[0] * direction.v[0])
    + (2.0 * start.v[2] * direction.v[2])
    - (2.0 * start.v[1] * direction.v[1]);
 	//and c= xE^2 + yE^2 - zE^2 
 	c = (start.v[0] * start.v[0])
    + (start.v[2] * start.v[2])
    - (start.v[1] * start.v[1]);
	// We then use the quadratic formula
	double b24ac = b*b - 4.0*a*c;
	// We don't deal with complex numbers here
	if (b24ac < 0.0) {return result;}
	
	if (b24ac > 0.0){
		double sqb24ac = sqrt(b24ac);
		t0 = (-b + sqb24ac) / (2.0 * a);
		t1 = (-b - sqb24ac) / (2.0 * a);

		if(b < 0.0) {
			r2= t0;
			r1= c/(a*r2);
		} else {
			r1= t1;
			r2= c/(a*r1);
		}
		if(r1 < EPSILON) {
			myT= r2;
		}
		else {	
			myT= r1;
		}
	if (myT >= EPSILON && myT < *t) {
		y = start.v[1] + myT*direction.v[1];
		if ((y <= (1.0 + EPSILON)) && (y >= -EPSILON)) {
			*t= myT;
			normal->v[1] = 0.0;
			normal->v[0] = start.v[0] + myT*direction.v[0];
			normal->v[2] = start.v[2] + myT*direction.v[2];
			result = HIT;
		}

	}
}
	myT = (1.0 - start.v[1])/direction.v[1];
	if (myT >= EPSILON && myT < *t)
	{
		x = start.v[0] + myT*direction.v[0];
		z = start.v[2] + myT*direction.v[2];
		y = start.v[1] + myT*direction.v[1];
		// Applying the equation given to us by the professor
		if(x*x + z*z <= y*y + EPSILON)
		{
			*t = myT;
			normal->v[0] = 0.0;
			normal->v[1] = 1.0; 
			normal->v[2] = 0.0;
			result = HIT;
		}
	}

	return result;
}
