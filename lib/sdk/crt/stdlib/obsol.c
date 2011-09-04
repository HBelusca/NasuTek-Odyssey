#include <precomp.h>
#include <stdlib.h>

#undef _cpumode
unsigned char _cpumode = 0;
unsigned char *_cpumode_dll = &_cpumode;

/*
 * @implemented
 */
void _beep(unsigned nFreq, unsigned nDur)
{
	Beep(nFreq,nDur);
	return;
}

/*
 * @implemented
 */
void _sleep(unsigned long ulTime)
{
	Sleep(ulTime);
	return;
}
