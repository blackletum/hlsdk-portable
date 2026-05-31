#include "fastmath.h"

float sini( short theta )
{
    while( theta > 360 )
        theta -= 360;

    while( theta < 1 )
        theta += 360;

    return geometric_index[theta-1][0];
}

float cosi( short theta )
{    while( theta > 360 )
        theta -= 360;

    while( theta < 1 )
        theta += 360;

    return geometric_index[theta-1][1];
}

float tani( short theta )
{

    while( theta > 360 )
        theta -= 360;

    while( theta < 1 )
        theta += 360;

	if( ( theta == 90 ) || ( theta == 180 ) )
#ifdef _MSC_VER
		_asm int 3
#else
		__builtin_trap();
#endif

    return geometric_index[theta-1][2];
}

