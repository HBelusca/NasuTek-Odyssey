#ifndef _ODYSSEY_ELF_MACHINE_H_
#define _ODYSSEY_ELF_MACHINE_H_ 1

#ifdef _M_IX86
#define _ODYSSEY_ELF_MACHINE_IS_TARGET
#include <elf/elf-i386.h>
#undef _ODYSSEY_ELF_MACHINE_IS_TARGET
#elif defined(_M_PPC)
#define _ODYSSEY_ELF_MACHINE_IS_TARGET
#include <elf/elf-powerpc.h>
#undef _ODYSSEY_ELF_MACHINE_IS_TARGET
#else
#error Unsupported target architecture
#endif

#endif
