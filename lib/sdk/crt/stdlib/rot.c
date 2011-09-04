/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey system libraries
 * FILE:        lib/msvcrt/stdlib/rot.c
 * PURPOSE:     Performs a bitwise rotation
 * PROGRAMER:   Ariadne
 * UPDATE HISTORY:
 *              03/04/99: Created
 */

#include <stdlib.h>

#ifdef _MSC_VER
#pragma function(_rotr, _rotl, _rotr, _lrotl, _lrotr)
#endif

unsigned int _rotr( unsigned int value, int shift );
/*
 * @implemented
 */
unsigned int _rotl( unsigned int value, int shift )
{
	int max_bits = sizeof(unsigned int)<<3;
	if ( shift < 0 )
		return _rotr(value,-shift);

	if ( shift > max_bits )
		shift = shift % max_bits;
	return (value << shift) | (value >> (max_bits-shift));
}

/*
 * @implemented
 */
unsigned int _rotr( unsigned int value, int shift )
{
	int max_bits = sizeof(unsigned int)<<3;
	if ( shift < 0 )
		return _rotl(value,-shift);

	if ( shift > max_bits<<3 )
		shift = shift % max_bits;
	return (value >> shift) | (value <<  (max_bits-shift));
}


/*
 * @implemented
 */
unsigned long _lrotl( unsigned long value, int shift )
{
	int max_bits = sizeof(unsigned long)<<3;
	if ( shift < 0 )
		return _lrotr(value,-shift);

	if ( shift > max_bits )
		shift = shift % max_bits;
	return (value << shift) | (value >> (max_bits-shift));
}

/*
 * @implemented
 */
unsigned long _lrotr( unsigned long value, int shift )
{
	int max_bits = sizeof(unsigned long)<<3;
	if ( shift < 0 )
		return _lrotl(value,-shift);

	if ( shift > max_bits )
		shift = shift % max_bits;
	return (value >> shift) | (value << (max_bits-shift));
}
