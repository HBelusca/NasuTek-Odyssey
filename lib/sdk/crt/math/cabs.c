#include <math.h>

/*
 * @implemented
 */
double _cabs( struct _complex z )
{
	return sqrt( z.x*z.x + z.y*z.y );
//	return hypot(z.x,z.y);
}




