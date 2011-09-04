/*
	Compatibility <intrin_x86.h> header for GCC -- GCC equivalents of intrinsic
	Microsoft Visual C++ functions. Originally developed for the Odyssey
	(<http://www.odyssey.org/>) and TinyKrnl (<http://www.tinykrnl.org/>)
	projects.

	Copyright (c) 2006 KJK::Hyperion <hackbunny@odyssey.com>

	Permission is hereby granted, free of charge, to any person obtaining a
	copy of this software and associated documentation files (the "Software"),
	to deal in the Software without restriction, including without limitation
	the rights to use, copy, modify, merge, publish, distribute, sublicense,
	and/or sell copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
	DEALINGS IN THE SOFTWARE.
*/

#ifndef KJK_INTRIN_X86_H_
#define KJK_INTRIN_X86_H_

/*
	FIXME: review all "memory" clobbers, add/remove to match Visual C++
	behavior: some "obvious" memory barriers are not present in the Visual C++
	implementation - e.g. __stosX; on the other hand, some memory barriers that
	*are* present could have been missed
*/

/*
	NOTE: this is a *compatibility* header. Some functions may look wrong at
	first, but they're only "as wrong" as they would be on Visual C++. Our
	priority is compatibility

	NOTE: unlike most people who write inline asm for GCC, I didn't pull the
	constraints and the uses of __volatile__ out of my... hat. Do not touch
	them. I hate cargo cult programming

	NOTE: be very careful with declaring "memory" clobbers. Some "obvious"
	barriers aren't there in Visual C++ (e.g. __stosX)

	NOTE: review all intrinsics with a return value, add/remove __volatile__
	where necessary. If an intrinsic whose value is ignored generates a no-op
	under Visual C++, __volatile__ must be omitted; if it always generates code
	(for example, if it has side effects), __volatile__ must be specified. GCC
	will only optimize out non-volatile asm blocks with outputs, so input-only
	blocks are safe. Oddities such as the non-volatile 'rdmsr' are intentional
	and follow Visual C++ behavior

	NOTE: on GCC 4.1.0, please use the __sync_* built-ins for barriers and
	atomic operations. Test the version like this:

	#if (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) > 40100
		...

	Pay attention to the type of barrier. Make it match with what Visual C++
	would use in the same case
*/

#ifdef __cplusplus
extern "C" {
#endif

/*** Stack frame juggling ***/
#define _ReturnAddress() (__builtin_return_address(0))
#define _AddressOfReturnAddress() (&(((void **)(__builtin_frame_address(0)))[1]))
/* TODO: __getcallerseflags but how??? */

/* Maybe the same for x86? */
#ifdef _x86_64
#define _alloca(s) __builtin_alloca(s)
#endif

/*** Memory barriers ***/

__INTRIN_INLINE void _ReadWriteBarrier(void)
{
	__asm__ __volatile__("" : : : "memory");
}

/* GCC only supports full barriers */
#define _ReadBarrier _ReadWriteBarrier
#define _WriteBarrier _ReadWriteBarrier

__INTRIN_INLINE void _mm_mfence(void)
{
	__asm__ __volatile__("mfence" : : : "memory");
}

__INTRIN_INLINE void _mm_lfence(void)
{
	_ReadBarrier();
	__asm__ __volatile__("lfence");
	_ReadBarrier();
}

__INTRIN_INLINE void _mm_sfence(void)
{
	_WriteBarrier();
	__asm__ __volatile__("sfence");
	_WriteBarrier();
}

#ifdef _x86_64
__INTRIN_INLINE void __faststorefence(void)
{
    long local;
	__asm__ __volatile__("lock; orl $0, %0;" : : "m"(local));
}
#endif


/*** Atomic operations ***/

#if (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) > 40100

__INTRIN_INLINE char _InterlockedCompareExchange8(volatile char * const Destination, const char Exchange, const char Comperand)
{
	return __sync_val_compare_and_swap(Destination, Comperand, Exchange);
}

__INTRIN_INLINE short _InterlockedCompareExchange16(volatile short * const Destination, const short Exchange, const short Comperand)
{
	return __sync_val_compare_and_swap(Destination, Comperand, Exchange);
}

__INTRIN_INLINE long _InterlockedCompareExchange(volatile long * const Destination, const long Exchange, const long Comperand)
{
	return __sync_val_compare_and_swap(Destination, Comperand, Exchange);
}

__INTRIN_INLINE void * _InterlockedCompareExchangePointer(void * volatile * const Destination, void * const Exchange, void * const Comperand)
{
	return (void *)__sync_val_compare_and_swap(Destination, Comperand, Exchange);
}

__INTRIN_INLINE long _InterlockedExchange(volatile long * const Target, const long Value)
{
	/* NOTE: __sync_lock_test_and_set would be an acquire barrier, so we force a full barrier */
	__sync_synchronize();
	return __sync_lock_test_and_set(Target, Value);
}

#if defined(_M_AMD64)
__INTRIN_INLINE long long _InterlockedExchange64(volatile long long * const Target, const long long Value)
{
	/* NOTE: __sync_lock_test_and_set would be an acquire barrier, so we force a full barrier */
	__sync_synchronize();
	return __sync_lock_test_and_set(Target, Value);
}
#endif

__INTRIN_INLINE void * _InterlockedExchangePointer(void * volatile * const Target, void * const Value)
{
	/* NOTE: ditto */
	__sync_synchronize();
	return (void *)__sync_lock_test_and_set(Target, Value);
}

__INTRIN_INLINE long _InterlockedExchangeAdd16(volatile short * const Addend, const short Value)
{
	return __sync_fetch_and_add(Addend, Value);
}

__INTRIN_INLINE long _InterlockedExchangeAdd(volatile long * const Addend, const long Value)
{
	return __sync_fetch_and_add(Addend, Value);
}

#if defined(_M_AMD64)
__INTRIN_INLINE long long _InterlockedExchangeAdd64(volatile long long * const Addend, const long long Value)
{
	return __sync_fetch_and_add(Addend, Value);
}
#endif

__INTRIN_INLINE char _InterlockedAnd8(volatile char * const value, const char mask)
{
	return __sync_fetch_and_and(value, mask);
}

__INTRIN_INLINE short _InterlockedAnd16(volatile short * const value, const short mask)
{
	return __sync_fetch_and_and(value, mask);
}

__INTRIN_INLINE long _InterlockedAnd(volatile long * const value, const long mask)
{
	return __sync_fetch_and_and(value, mask);
}

#if defined(_M_AMD64)
__INTRIN_INLINE long _InterlockedAnd64(volatile long long * const value, const long long mask)
{
	return __sync_fetch_and_and(value, mask);
}
#endif

__INTRIN_INLINE char _InterlockedOr8(volatile char * const value, const char mask)
{
	return __sync_fetch_and_or(value, mask);
}

__INTRIN_INLINE short _InterlockedOr16(volatile short * const value, const short mask)
{
	return __sync_fetch_and_or(value, mask);
}

__INTRIN_INLINE long _InterlockedOr(volatile long * const value, const long mask)
{
	return __sync_fetch_and_or(value, mask);
}

#if defined(_M_AMD64)
__INTRIN_INLINE long _InterlockedOr64(volatile long long * const value, const long long mask)
{
	return __sync_fetch_and_or(value, mask);
}
#endif

__INTRIN_INLINE char _InterlockedXor8(volatile char * const value, const char mask)
{
	return __sync_fetch_and_xor(value, mask);
}

__INTRIN_INLINE short _InterlockedXor16(volatile short * const value, const short mask)
{
	return __sync_fetch_and_xor(value, mask);
}

__INTRIN_INLINE long _InterlockedXor(volatile long * const value, const long mask)
{
	return __sync_fetch_and_xor(value, mask);
}

#else

__INTRIN_INLINE char _InterlockedCompareExchange8(volatile char * const Destination, const char Exchange, const char Comperand)
{
	char retval = Comperand;
	__asm__("lock; cmpxchgb %b[Exchange], %[Destination]" : [retval] "+a" (retval) : [Destination] "m" (*Destination), [Exchange] "q" (Exchange) : "memory");
	return retval;
}

__INTRIN_INLINE short _InterlockedCompareExchange16(volatile short * const Destination, const short Exchange, const short Comperand)
{
	short retval = Comperand;
	__asm__("lock; cmpxchgw %w[Exchange], %[Destination]" : [retval] "+a" (retval) : [Destination] "m" (*Destination), [Exchange] "q" (Exchange): "memory");
	return retval;
}

__INTRIN_INLINE long _InterlockedCompareExchange(volatile long * const Destination, const long Exchange, const long Comperand)
{
	long retval = Comperand;
	__asm__("lock; cmpxchgl %k[Exchange], %[Destination]" : [retval] "+a" (retval) : [Destination] "m" (*Destination), [Exchange] "q" (Exchange): "memory");
	return retval;
}

__INTRIN_INLINE void * _InterlockedCompareExchangePointer(void * volatile * const Destination, void * const Exchange, void * const Comperand)
{
	void * retval = (void *)Comperand;
	__asm__("lock; cmpxchgl %k[Exchange], %[Destination]" : [retval] "=a" (retval) : "[retval]" (retval), [Destination] "m" (*Destination), [Exchange] "q" (Exchange) : "memory");
	return retval;
}

__INTRIN_INLINE long _InterlockedExchange(volatile long * const Target, const long Value)
{
	long retval = Value;
	__asm__("xchgl %[retval], %[Target]" : [retval] "+r" (retval) : [Target] "m" (*Target) : "memory");
	return retval;
}

__INTRIN_INLINE void * _InterlockedExchangePointer(void * volatile * const Target, void * const Value)
{
	void * retval = Value;
	__asm__("xchgl %[retval], %[Target]" : [retval] "+r" (retval) : [Target] "m" (*Target) : "memory");
	return retval;
}

__INTRIN_INLINE long _InterlockedExchangeAdd16(volatile short * const Addend, const short Value)
{
	long retval = Value;
	__asm__("lock; xaddw %[retval], %[Addend]" : [retval] "+r" (retval) : [Addend] "m" (*Addend) : "memory");
	return retval;
}

__INTRIN_INLINE long _InterlockedExchangeAdd(volatile long * const Addend, const long Value)
{
	long retval = Value;
	__asm__("lock; xaddl %[retval], %[Addend]" : [retval] "+r" (retval) : [Addend] "m" (*Addend) : "memory");
	return retval;
}

__INTRIN_INLINE char _InterlockedAnd8(volatile char * const value, const char mask)
{
	char x;
	char y;

	y = *value;

	do
	{
		x = y;
		y = _InterlockedCompareExchange8(value, x & mask, x);
	}
	while(y != x);

	return y;
}

__INTRIN_INLINE short _InterlockedAnd16(volatile short * const value, const short mask)
{
	short x;
	short y;

	y = *value;

	do
	{
		x = y;
		y = _InterlockedCompareExchange16(value, x & mask, x);
	}
	while(y != x);

	return y;
}

__INTRIN_INLINE long _InterlockedAnd(volatile long * const value, const long mask)
{
	long x;
	long y;

	y = *value;

	do
	{
		x = y;
		y = _InterlockedCompareExchange(value, x & mask, x);
	}
	while(y != x);

	return y;
}

__INTRIN_INLINE char _InterlockedOr8(volatile char * const value, const char mask)
{
	char x;
	char y;

	y = *value;

	do
	{
		x = y;
		y = _InterlockedCompareExchange8(value, x | mask, x);
	}
	while(y != x);

	return y;
}

__INTRIN_INLINE short _InterlockedOr16(volatile short * const value, const short mask)
{
	short x;
	short y;

	y = *value;

	do
	{
		x = y;
		y = _InterlockedCompareExchange16(value, x | mask, x);
	}
	while(y != x);

	return y;
}

__INTRIN_INLINE long _InterlockedOr(volatile long * const value, const long mask)
{
	long x;
	long y;

	y = *value;

	do
	{
		x = y;
		y = _InterlockedCompareExchange(value, x | mask, x);
	}
	while(y != x);

	return y;
}

__INTRIN_INLINE char _InterlockedXor8(volatile char * const value, const char mask)
{
	char x;
	char y;

	y = *value;

	do
	{
		x = y;
		y = _InterlockedCompareExchange8(value, x ^ mask, x);
	}
	while(y != x);

	return y;
}

__INTRIN_INLINE short _InterlockedXor16(volatile short * const value, const short mask)
{
	short x;
	short y;

	y = *value;

	do
	{
		x = y;
		y = _InterlockedCompareExchange16(value, x ^ mask, x);
	}
	while(y != x);

	return y;
}

__INTRIN_INLINE long _InterlockedXor(volatile long * const value, const long mask)
{
	long x;
	long y;

	y = *value;

	do
	{
		x = y;
		y = _InterlockedCompareExchange(value, x ^ mask, x);
	}
	while(y != x);

	return y;
}

#endif

#if (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) > 40100 && defined(__x86_64__)

__INTRIN_INLINE long long _InterlockedCompareExchange64(volatile long long * const Destination, const long long Exchange, const long long Comperand)
{
	return __sync_val_compare_and_swap(Destination, Comperand, Exchange);
}

#else

__INTRIN_INLINE long long _InterlockedCompareExchange64(volatile long long * const Destination, const long long Exchange, const long long Comperand)
{
	long long retval = Comperand;

	__asm__
	(
		"lock; cmpxchg8b %[Destination]" :
		[retval] "+A" (retval) :
			[Destination] "m" (*Destination),
			"b" ((unsigned long)((Exchange >>  0) & 0xFFFFFFFF)),
			"c" ((unsigned long)((Exchange >> 32) & 0xFFFFFFFF)) :
		"memory"
	);

	return retval;
}

#endif

__INTRIN_INLINE long _InterlockedAddLargeStatistic(volatile long long * const Addend, const long Value)
{
	__asm__
	(
		"lock; add %[Value], %[Lo32];"
		"jae LABEL%=;"
		"lock; adc $0, %[Hi32];"
		"LABEL%=:;" :
		[Lo32] "+m" (*((volatile long *)(Addend) + 0)), [Hi32] "+m" (*((volatile long *)(Addend) + 1)) :
		[Value] "ir" (Value) :
		"memory"
	);

	return Value;
}

__INTRIN_INLINE long _InterlockedDecrement(volatile long * const lpAddend)
{
	return _InterlockedExchangeAdd(lpAddend, -1) - 1;
}

__INTRIN_INLINE long _InterlockedIncrement(volatile long * const lpAddend)
{
	return _InterlockedExchangeAdd(lpAddend, 1) + 1;
}

__INTRIN_INLINE short _InterlockedDecrement16(volatile short * const lpAddend)
{
	return _InterlockedExchangeAdd16(lpAddend, -1) - 1;
}

__INTRIN_INLINE short _InterlockedIncrement16(volatile short * const lpAddend)
{
	return _InterlockedExchangeAdd16(lpAddend, 1) + 1;
}

#if defined(_M_AMD64)
__INTRIN_INLINE long long _InterlockedDecrement64(volatile long long * const lpAddend)
{
	return _InterlockedExchangeAdd64(lpAddend, -1) - 1;
}

__INTRIN_INLINE long long _InterlockedIncrement64(volatile long long * const lpAddend)
{
	return _InterlockedExchangeAdd64(lpAddend, 1) + 1;
}
#endif

__INTRIN_INLINE unsigned char _interlockedbittestandreset(volatile long * a, const long b)
{
	unsigned char retval;
	__asm__("lock; btrl %[b], %[a]; setb %b[retval]" : [retval] "=q" (retval), [a] "+m" (*a) : [b] "Ir" (b) : "memory");
	return retval;
}

#if defined(_M_AMD64)
__INTRIN_INLINE unsigned char _interlockedbittestandreset64(volatile long long * a, const long long b)
{
	unsigned char retval;
	__asm__("lock; btrq %[b], %[a]; setb %b[retval]" : [retval] "=r" (retval), [a] "+m" (*a) : [b] "Ir" (b) : "memory");
	return retval;
}
#endif

__INTRIN_INLINE unsigned char _interlockedbittestandset(volatile long * a, const long b)
{
	unsigned char retval;
	__asm__("lock; btsl %[b], %[a]; setc %b[retval]" : [retval] "=q" (retval), [a] "+m" (*a) : [b] "Ir" (b) : "memory");
	return retval;
}

#if defined(_M_AMD64)
__INTRIN_INLINE unsigned char _interlockedbittestandset64(volatile long long * a, const long long b)
{
	unsigned char retval;
	__asm__("lock; btsq %[b], %[a]; setc %b[retval]" : [retval] "=r" (retval), [a] "+m" (*a) : [b] "Ir" (b) : "memory");
	return retval;
}
#endif

/*** String operations ***/
/* NOTE: we don't set a memory clobber in the __stosX functions because Visual C++ doesn't */
__INTRIN_INLINE void __stosb(unsigned char * Dest, const unsigned char Data, size_t Count)
{
	__asm__ __volatile__
	(
		"rep; stosb" :
		[Dest] "=D" (Dest), [Count] "=c" (Count) :
		"[Dest]" (Dest), "a" (Data), "[Count]" (Count)
	);
}

__INTRIN_INLINE void __stosw(unsigned short * Dest, const unsigned short Data, size_t Count)
{
	__asm__ __volatile__
	(
		"rep; stosw" :
		[Dest] "=D" (Dest), [Count] "=c" (Count) :
		"[Dest]" (Dest), "a" (Data), "[Count]" (Count)
	);
}

__INTRIN_INLINE void __stosd(unsigned long * Dest, const unsigned long Data, size_t Count)
{
	__asm__ __volatile__
	(
		"rep; stosl" :
		[Dest] "=D" (Dest), [Count] "=c" (Count) :
		"[Dest]" (Dest), "a" (Data), "[Count]" (Count)
	);
}

#ifdef _M_AMD64
__INTRIN_INLINE void __stosq(unsigned __int64 * Dest, const unsigned __int64 Data, size_t Count)
{
	__asm__ __volatile__
	(
		"rep; stosq" :
		[Dest] "=D" (Dest), [Count] "=c" (Count) :
		"[Dest]" (Dest), "a" (Data), "[Count]" (Count)
	);
}
#endif

__INTRIN_INLINE void __movsb(unsigned char * Destination, const unsigned char * Source, size_t Count)
{
	__asm__ __volatile__
	(
		"rep; movsb" :
		[Destination] "=D" (Destination), [Source] "=S" (Source), [Count] "=c" (Count) :
		"[Destination]" (Destination), "[Source]" (Source), "[Count]" (Count)
	);
}

__INTRIN_INLINE void __movsw(unsigned short * Destination, const unsigned short * Source, size_t Count)
{
	__asm__ __volatile__
	(
		"rep; movsw" :
		[Destination] "=D" (Destination), [Source] "=S" (Source), [Count] "=c" (Count) :
		"[Destination]" (Destination), "[Source]" (Source), "[Count]" (Count)
	);
}

__INTRIN_INLINE void __movsd(unsigned long * Destination, const unsigned long * Source, size_t Count)
{
	__asm__ __volatile__
	(
		"rep; movsd" :
		[Destination] "=D" (Destination), [Source] "=S" (Source), [Count] "=c" (Count) :
		"[Destination]" (Destination), "[Source]" (Source), "[Count]" (Count)
	);
}

#ifdef _M_AMD64
__INTRIN_INLINE void __movsq(unsigned long * Destination, const unsigned long * Source, size_t Count)
{
	__asm__ __volatile__
	(
		"rep; movsq" :
		[Destination] "=D" (Destination), [Source] "=S" (Source), [Count] "=c" (Count) :
		"[Destination]" (Destination), "[Source]" (Source), "[Count]" (Count)
	);
}
#endif

#if defined(_M_AMD64)
/*** GS segment addressing ***/

__INTRIN_INLINE void __writegsbyte(const unsigned long Offset, const unsigned char Data)
{
	__asm__ __volatile__("movb %b[Data], %%gs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "ir" (Data) : "memory");
}

__INTRIN_INLINE void __writegsword(const unsigned long Offset, const unsigned short Data)
{
	__asm__ __volatile__("movw %w[Data], %%gs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "ir" (Data) : "memory");
}

__INTRIN_INLINE void __writegsdword(const unsigned long Offset, const unsigned long Data)
{
	__asm__ __volatile__("movl %k[Data], %%gs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "ir" (Data) : "memory");
}

__INTRIN_INLINE void __writegsqword(const unsigned long Offset, const unsigned __int64 Data)
{
	__asm__ __volatile__("movq %q[Data], %%gs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "ir" (Data) : "memory");
}

__INTRIN_INLINE unsigned char __readgsbyte(const unsigned long Offset)
{
	unsigned char value;
	__asm__ __volatile__("movb %%gs:%a[Offset], %b[value]" : [value] "=r" (value) : [Offset] "ir" (Offset));
	return value;
}

__INTRIN_INLINE unsigned short __readgsword(const unsigned long Offset)
{
	unsigned short value;
	__asm__ __volatile__("movw %%gs:%a[Offset], %w[value]" : [value] "=r" (value) : [Offset] "ir" (Offset));
	return value;
}

__INTRIN_INLINE unsigned long __readgsdword(const unsigned long Offset)
{
	unsigned long value;
	__asm__ __volatile__("movl %%gs:%a[Offset], %k[value]" : [value] "=r" (value) : [Offset] "ir" (Offset));
	return value;
}

__INTRIN_INLINE unsigned __int64 __readgsqword(const unsigned long Offset)
{
	unsigned __int64 value;
	__asm__ __volatile__("movq %%gs:%a[Offset], %q[value]" : [value] "=r" (value) : [Offset] "ir" (Offset));
	return value;
}

__INTRIN_INLINE void __incgsbyte(const unsigned long Offset)
{
	__asm__ __volatile__("incb %%gs:%a[Offset]" : : [Offset] "ir" (Offset) : "memory");
}

__INTRIN_INLINE void __incgsword(const unsigned long Offset)
{
	__asm__ __volatile__("incw %%gs:%a[Offset]" : : [Offset] "ir" (Offset) : "memory");
}

__INTRIN_INLINE void __incgsdword(const unsigned long Offset)
{
	__asm__ __volatile__("incl %%gs:%a[Offset]" : : [Offset] "ir" (Offset) : "memory");
}

__INTRIN_INLINE void __addgsbyte(const unsigned long Offset, const unsigned char Data)
{
	__asm__ __volatile__("addb %b[Data], %%gs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "ir" (Data) : "memory");
}

__INTRIN_INLINE void __addgsword(const unsigned long Offset, const unsigned short Data)
{
	__asm__ __volatile__("addw %w[Data], %%gs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "ir" (Data) : "memory");
}

__INTRIN_INLINE void __addgsdword(const unsigned long Offset, const unsigned int Data)
{
	__asm__ __volatile__("addl %k[Data], %%gs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "ir" (Data) : "memory");
}

__INTRIN_INLINE void __addgsqword(const unsigned long Offset, const unsigned __int64 Data)
{
	__asm__ __volatile__("addq %k[Data], %%gs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "ir" (Data) : "memory");
}

#else
/*** FS segment addressing ***/
__INTRIN_INLINE void __writefsbyte(const unsigned long Offset, const unsigned char Data)
{
	__asm__ __volatile__("movb %b[Data], %%fs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "iq" (Data) : "memory");
}

__INTRIN_INLINE void __writefsword(const unsigned long Offset, const unsigned short Data)
{
	__asm__ __volatile__("movw %w[Data], %%fs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "ir" (Data) : "memory");
}

__INTRIN_INLINE void __writefsdword(const unsigned long Offset, const unsigned long Data)
{
	__asm__ __volatile__("movl %k[Data], %%fs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "ir" (Data) : "memory");
}

__INTRIN_INLINE unsigned char __readfsbyte(const unsigned long Offset)
{
	unsigned char value;
	__asm__ __volatile__("movb %%fs:%a[Offset], %b[value]" : [value] "=q" (value) : [Offset] "ir" (Offset));
	return value;
}

__INTRIN_INLINE unsigned short __readfsword(const unsigned long Offset)
{
	unsigned short value;
	__asm__ __volatile__("movw %%fs:%a[Offset], %w[value]" : [value] "=r" (value) : [Offset] "ir" (Offset));
	return value;
}

__INTRIN_INLINE unsigned long __readfsdword(const unsigned long Offset)
{
	unsigned long value;
	__asm__ __volatile__("movl %%fs:%a[Offset], %k[value]" : [value] "=r" (value) : [Offset] "ir" (Offset));
	return value;
}

__INTRIN_INLINE void __incfsbyte(const unsigned long Offset)
{
	__asm__ __volatile__("incb %%fs:%a[Offset]" : : [Offset] "ir" (Offset) : "memory");
}

__INTRIN_INLINE void __incfsword(const unsigned long Offset)
{
	__asm__ __volatile__("incw %%fs:%a[Offset]" : : [Offset] "ir" (Offset) : "memory");
}

__INTRIN_INLINE void __incfsdword(const unsigned long Offset)
{
	__asm__ __volatile__("incl %%fs:%a[Offset]" : : [Offset] "ir" (Offset) : "memory");
}

/* NOTE: the bizarre implementation of __addfsxxx mimics the broken Visual C++ behavior */
__INTRIN_INLINE void __addfsbyte(const unsigned long Offset, const unsigned char Data)
{
	if(!__builtin_constant_p(Offset))
		__asm__ __volatile__("addb %b[Offset], %%fs:%a[Offset]" : : [Offset] "r" (Offset) : "memory");
	else
		__asm__ __volatile__("addb %b[Data], %%fs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "iq" (Data) : "memory");
}

__INTRIN_INLINE void __addfsword(const unsigned long Offset, const unsigned short Data)
{
	if(!__builtin_constant_p(Offset))
		__asm__ __volatile__("addw %w[Offset], %%fs:%a[Offset]" : : [Offset] "r" (Offset) : "memory");
	else
		__asm__ __volatile__("addw %w[Data], %%fs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "iq" (Data) : "memory");
}

__INTRIN_INLINE void __addfsdword(const unsigned long Offset, const unsigned int Data)
{
	if(!__builtin_constant_p(Offset))
		__asm__ __volatile__("addl %k[Offset], %%fs:%a[Offset]" : : [Offset] "r" (Offset) : "memory");
	else
		__asm__ __volatile__("addl %k[Data], %%fs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "iq" (Data) : "memory");
}
#endif


/*** Bit manipulation ***/
__INTRIN_INLINE unsigned char _BitScanForward(unsigned long * const Index, const unsigned long Mask)
{
	__asm__("bsfl %[Mask], %[Index]" : [Index] "=r" (*Index) : [Mask] "mr" (Mask));
	return Mask ? 1 : 0;
}

__INTRIN_INLINE unsigned char _BitScanReverse(unsigned long * const Index, const unsigned long Mask)
{
	__asm__("bsrl %[Mask], %[Index]" : [Index] "=r" (*Index) : [Mask] "mr" (Mask));
	return Mask ? 1 : 0;
}

/* NOTE: again, the bizarre implementation follows Visual C++ */
__INTRIN_INLINE unsigned char _bittest(const long * const a, const long b)
{
	unsigned char retval;

	if(__builtin_constant_p(b))
		__asm__("bt %[b], %[a]; setb %b[retval]" : [retval] "=q" (retval) : [a] "mr" (*(a + (b / 32))), [b] "Ir" (b % 32));
	else
		__asm__("bt %[b], %[a]; setb %b[retval]" : [retval] "=q" (retval) : [a] "mr" (*a), [b] "r" (b));

	return retval;
}

#ifdef _M_AMD64
__INTRIN_INLINE unsigned char _bittest64(const __int64 * const a, const __int64 b)
{
	unsigned char retval;

	if(__builtin_constant_p(b))
		__asm__("bt %[b], %[a]; setb %b[retval]" : [retval] "=q" (retval) : [a] "mr" (*(a + (b / 64))), [b] "Ir" (b % 64));
	else
		__asm__("bt %[b], %[a]; setb %b[retval]" : [retval] "=q" (retval) : [a] "mr" (*a), [b] "r" (b));

	return retval;
}
#endif

__INTRIN_INLINE unsigned char _bittestandcomplement(long * const a, const long b)
{
	unsigned char retval;

	if(__builtin_constant_p(b))
		__asm__("btc %[b], %[a]; setb %b[retval]" : [a] "+mr" (*(a + (b / 32))), [retval] "=q" (retval) : [b] "Ir" (b % 32));
	else
		__asm__("btc %[b], %[a]; setb %b[retval]" : [a] "+mr" (*a), [retval] "=q" (retval) : [b] "r" (b));

	return retval;
}

__INTRIN_INLINE unsigned char _bittestandreset(long * const a, const long b)
{
	unsigned char retval;

	if(__builtin_constant_p(b))
		__asm__("btr %[b], %[a]; setb %b[retval]" : [a] "+mr" (*(a + (b / 32))), [retval] "=q" (retval) : [b] "Ir" (b % 32));
	else
		__asm__("btr %[b], %[a]; setb %b[retval]" : [a] "+mr" (*a), [retval] "=q" (retval) : [b] "r" (b));

	return retval;
}

__INTRIN_INLINE unsigned char _bittestandset(long * const a, const long b)
{
	unsigned char retval;

	if(__builtin_constant_p(b))
		__asm__("bts %[b], %[a]; setb %b[retval]" : [a] "+mr" (*(a + (b / 32))), [retval] "=q" (retval) : [b] "Ir" (b % 32));
	else
		__asm__("bts %[b], %[a]; setb %b[retval]" : [a] "+mr" (*a), [retval] "=q" (retval) : [b] "r" (b));

	return retval;
}

__INTRIN_INLINE unsigned char _rotl8(unsigned char value, unsigned char shift)
{
	unsigned char retval;
	__asm__("rolb %b[shift], %b[retval]" : [retval] "=rm" (retval) : "[retval]" (value), [shift] "Nc" (shift));
	return retval;
}

__INTRIN_INLINE unsigned short _rotl16(unsigned short value, unsigned char shift)
{
	unsigned short retval;
	__asm__("rolw %b[shift], %w[retval]" : [retval] "=rm" (retval) : "[retval]" (value), [shift] "Nc" (shift));
	return retval;
}

__INTRIN_INLINE unsigned int _rotl(unsigned int value, int shift)
{
	unsigned long retval;
	__asm__("roll %b[shift], %k[retval]" : [retval] "=rm" (retval) : "[retval]" (value), [shift] "Nc" (shift));
	return retval;
}

__INTRIN_INLINE unsigned int _rotr(unsigned int value, int shift)
{
	unsigned long retval;
	__asm__("rorl %b[shift], %k[retval]" : [retval] "=rm" (retval) : "[retval]" (value), [shift] "Nc" (shift));
	return retval;
}

__INTRIN_INLINE unsigned char _rotr8(unsigned char value, unsigned char shift)
{
	unsigned char retval;
	__asm__("rorb %b[shift], %b[retval]" : [retval] "=qm" (retval) : "[retval]" (value), [shift] "Nc" (shift));
	return retval;
}

__INTRIN_INLINE unsigned short _rotr16(unsigned short value, unsigned char shift)
{
	unsigned short retval;
	__asm__("rorw %b[shift], %w[retval]" : [retval] "=rm" (retval) : "[retval]" (value), [shift] "Nc" (shift));
	return retval;
}

/*
	NOTE: in __ll_lshift, __ll_rshift and __ull_rshift we use the "A"
	constraint (edx:eax) for the Mask argument, because it's the only way GCC
	can pass 64-bit operands around - passing the two 32 bit parts separately
	just confuses it. Also we declare Bit as an int and then truncate it to
	match Visual C++ behavior
*/
__INTRIN_INLINE unsigned long long __ll_lshift(const unsigned long long Mask, const int Bit)
{
	unsigned long long retval = Mask;

	__asm__
	(
		"shldl %b[Bit], %%eax, %%edx; sall %b[Bit], %%eax" :
		"+A" (retval) :
		[Bit] "Nc" ((unsigned char)((unsigned long)Bit) & 0xFF)
	);

	return retval;
}

__INTRIN_INLINE long long __ll_rshift(const long long Mask, const int Bit)
{
	unsigned long long retval = Mask;

	__asm__
	(
		"shldl %b[Bit], %%eax, %%edx; sarl %b[Bit], %%eax" :
		"+A" (retval) :
		[Bit] "Nc" ((unsigned char)((unsigned long)Bit) & 0xFF)
	);

	return retval;
}

__INTRIN_INLINE unsigned long long __ull_rshift(const unsigned long long Mask, int Bit)
{
	unsigned long long retval = Mask;

	__asm__
	(
		"shrdl %b[Bit], %%eax, %%edx; shrl %b[Bit], %%eax" :
		"+A" (retval) :
		[Bit] "Nc" ((unsigned char)((unsigned long)Bit) & 0xFF)
	);

	return retval;
}

__INTRIN_INLINE unsigned short _byteswap_ushort(unsigned short value)
{
	unsigned short retval;
	__asm__("rorw $8, %w[retval]" : [retval] "=rm" (retval) : "[retval]" (value));
	return retval;
}

__INTRIN_INLINE unsigned long _byteswap_ulong(unsigned long value)
{
	unsigned long retval;
	__asm__("bswapl %[retval]" : [retval] "=r" (retval) : "[retval]" (value));
	return retval;
}

#ifdef _M_AMD64
__INTRIN_INLINE unsigned __int64 _byteswap_uint64(unsigned __int64 value)
{
	unsigned __int64 retval;
	__asm__("bswapq %[retval]" : [retval] "=r" (retval) : "[retval]" (value));
	return retval;
}
#else
__INTRIN_INLINE unsigned __int64 _byteswap_uint64(unsigned __int64 value)
{
	union {
		__int64 int64part;
		struct {
			unsigned long lowpart;
			unsigned long hipart;
		};
	} retval;
	retval.int64part = value;
	__asm__("bswapl %[lowpart]\n"
	        "bswapl %[hipart]\n"
	        : [lowpart] "=r" (retval.hipart), [hipart] "=r" (retval.lowpart)  : "[lowpart]" (retval.lowpart), "[hipart]" (retval.hipart) );
	return retval.int64part;
}
#endif

/*** 64-bit math ***/
__INTRIN_INLINE long long __emul(const int a, const int b)
{
	long long retval;
	__asm__("imull %[b]" : "=A" (retval) : [a] "a" (a), [b] "rm" (b));
	return retval;
}

__INTRIN_INLINE unsigned long long __emulu(const unsigned int a, const unsigned int b)
{
	unsigned long long retval;
	__asm__("mull %[b]" : "=A" (retval) : [a] "a" (a), [b] "rm" (b));
	return retval;
}

#ifdef _M_AMD64

__INTRIN_INLINE __int64 __mulh(__int64 a, __int64 b)
{
	__int64 retval;
	__asm__("imulq %[b]" : "=d" (retval) : [a] "a" (a), [b] "rm" (b));
	return retval;
}

__INTRIN_INLINE unsigned __int64 __umulh(unsigned __int64 a, unsigned __int64 b)
{
	unsigned __int64 retval;
	__asm__("mulq %[b]" : "=d" (retval) : [a] "a" (a), [b] "rm" (b));
	return retval;
}

#endif

/*** Port I/O ***/
__INTRIN_INLINE unsigned char __inbyte(const unsigned short Port)
{
	unsigned char byte;
	__asm__ __volatile__("inb %w[Port], %b[byte]" : [byte] "=a" (byte) : [Port] "Nd" (Port));
	return byte;
}

__INTRIN_INLINE unsigned short __inword(const unsigned short Port)
{
	unsigned short word;
	__asm__ __volatile__("inw %w[Port], %w[word]" : [word] "=a" (word) : [Port] "Nd" (Port));
	return word;
}

__INTRIN_INLINE unsigned long __indword(const unsigned short Port)
{
	unsigned long dword;
	__asm__ __volatile__("inl %w[Port], %k[dword]" : [dword] "=a" (dword) : [Port] "Nd" (Port));
	return dword;
}

__INTRIN_INLINE void __inbytestring(unsigned short Port, unsigned char * Buffer, unsigned long Count)
{
	__asm__ __volatile__
	(
		"rep; insb" :
		[Buffer] "=D" (Buffer), [Count] "=c" (Count) :
		"d" (Port), "[Buffer]" (Buffer), "[Count]" (Count) :
		"memory"
	);
}

__INTRIN_INLINE void __inwordstring(unsigned short Port, unsigned short * Buffer, unsigned long Count)
{
	__asm__ __volatile__
	(
		"rep; insw" :
		[Buffer] "=D" (Buffer), [Count] "=c" (Count) :
		"d" (Port), "[Buffer]" (Buffer), "[Count]" (Count) :
		"memory"
	);
}

__INTRIN_INLINE void __indwordstring(unsigned short Port, unsigned long * Buffer, unsigned long Count)
{
	__asm__ __volatile__
	(
		"rep; insl" :
		[Buffer] "=D" (Buffer), [Count] "=c" (Count) :
		"d" (Port), "[Buffer]" (Buffer), "[Count]" (Count) :
		"memory"
	);
}

__INTRIN_INLINE void __outbyte(unsigned short const Port, const unsigned char Data)
{
	__asm__ __volatile__("outb %b[Data], %w[Port]" : : [Port] "Nd" (Port), [Data] "a" (Data));
}

__INTRIN_INLINE void __outword(unsigned short const Port, const unsigned short Data)
{
	__asm__ __volatile__("outw %w[Data], %w[Port]" : : [Port] "Nd" (Port), [Data] "a" (Data));
}

__INTRIN_INLINE void __outdword(unsigned short const Port, const unsigned long Data)
{
	__asm__ __volatile__("outl %k[Data], %w[Port]" : : [Port] "Nd" (Port), [Data] "a" (Data));
}

__INTRIN_INLINE void __outbytestring(unsigned short const Port, const unsigned char * const Buffer, const unsigned long Count)
{
	__asm__ __volatile__("rep; outsb" : : [Port] "d" (Port), [Buffer] "S" (Buffer), "c" (Count));
}

__INTRIN_INLINE void __outwordstring(unsigned short const Port, const unsigned short * const Buffer, const unsigned long Count)
{
	__asm__ __volatile__("rep; outsw" : : [Port] "d" (Port), [Buffer] "S" (Buffer), "c" (Count));
}

__INTRIN_INLINE void __outdwordstring(unsigned short const Port, const unsigned long * const Buffer, const unsigned long Count)
{
	__asm__ __volatile__("rep; outsl" : : [Port] "d" (Port), [Buffer] "S" (Buffer), "c" (Count));
}

__INTRIN_INLINE int _inp(unsigned short Port)
{
	return __inbyte(Port);
}

__INTRIN_INLINE unsigned short _inpw(unsigned short Port)
{
	return __inword(Port);
}

__INTRIN_INLINE unsigned long _inpd(unsigned short Port)
{
	return __indword(Port);
}

__INTRIN_INLINE int _outp(unsigned short Port, int databyte)
{
	__outbyte(Port, databyte);
	return databyte;
}

__INTRIN_INLINE unsigned short _outpw(unsigned short Port, unsigned short dataword)
{
	__outword(Port, dataword);
	return dataword;
}

__INTRIN_INLINE unsigned long _outpd(unsigned short Port, unsigned long dataword)
{
	__outdword(Port, dataword);
	return dataword;
}


/*** System information ***/
__INTRIN_INLINE void __cpuid(int CPUInfo[], const int InfoType)
{
	__asm__ __volatile__("cpuid" : "=a" (CPUInfo[0]), "=b" (CPUInfo[1]), "=c" (CPUInfo[2]), "=d" (CPUInfo[3]) : "a" (InfoType));
}

__INTRIN_INLINE unsigned long long __rdtsc(void)
{
#ifdef _M_AMD64
	unsigned long long low, high;
	__asm__ __volatile__("rdtsc" : "=a"(low), "=d"(high));
	return low | (high << 32);
#else
	unsigned long long retval;
	__asm__ __volatile__("rdtsc" : "=A"(retval));
	return retval;
#endif
}

__INTRIN_INLINE void __writeeflags(uintptr_t Value)
{
	__asm__ __volatile__("push %0\n popf" : : "rim"(Value));
}

__INTRIN_INLINE uintptr_t __readeflags(void)
{
	uintptr_t retval;
	__asm__ __volatile__("pushf\n pop %0" : "=rm"(retval));
	return retval;
}

/*** Interrupts ***/
__INTRIN_INLINE void __debugbreak(void)
{
	__asm__("int $3");
}

__INTRIN_INLINE void __int2c(void)
{
	__asm__("int $0x2c");
}

__INTRIN_INLINE void _disable(void)
{
	__asm__("cli" : : : "memory");
}

__INTRIN_INLINE void _enable(void)
{
	__asm__("sti" : : : "memory");
}

__INTRIN_INLINE void __halt(void)
{
	__asm__("hlt\n\t" : : : "memory");
}

/*** Protected memory management ***/

__INTRIN_INLINE void __writecr0(const unsigned __int64 Data)
{
	__asm__("mov %[Data], %%cr0" : : [Data] "r" (Data) : "memory");
}

__INTRIN_INLINE void __writecr3(const unsigned __int64 Data)
{
	__asm__("mov %[Data], %%cr3" : : [Data] "r" (Data) : "memory");
}

__INTRIN_INLINE void __writecr4(const unsigned __int64 Data)
{
	__asm__("mov %[Data], %%cr4" : : [Data] "r" (Data) : "memory");
}

#ifdef _M_AMD64
__INTRIN_INLINE void __writecr8(const unsigned __int64 Data)
{
	__asm__("mov %[Data], %%cr8" : : [Data] "r" (Data) : "memory");
}

__INTRIN_INLINE unsigned __int64 __readcr0(void)
{
	unsigned __int64 value;
	__asm__ __volatile__("mov %%cr0, %[value]" : [value] "=r" (value));
	return value;
}

__INTRIN_INLINE unsigned __int64 __readcr2(void)
{
	unsigned __int64 value;
	__asm__ __volatile__("mov %%cr2, %[value]" : [value] "=r" (value));
	return value;
}

__INTRIN_INLINE unsigned __int64 __readcr3(void)
{
	unsigned __int64 value;
	__asm__ __volatile__("mov %%cr3, %[value]" : [value] "=r" (value));
	return value;
}

__INTRIN_INLINE unsigned __int64 __readcr4(void)
{
	unsigned __int64 value;
	__asm__ __volatile__("mov %%cr4, %[value]" : [value] "=r" (value));
	return value;
}

__INTRIN_INLINE unsigned __int64 __readcr8(void)
{
	unsigned __int64 value;
	__asm__ __volatile__("movq %%cr8, %q[value]" : [value] "=r" (value));
	return value;
}
#else
__INTRIN_INLINE unsigned long __readcr0(void)
{
	unsigned long value;
	__asm__ __volatile__("mov %%cr0, %[value]" : [value] "=r" (value));
	return value;
}

__INTRIN_INLINE unsigned long __readcr2(void)
{
	unsigned long value;
	__asm__ __volatile__("mov %%cr2, %[value]" : [value] "=r" (value));
	return value;
}

__INTRIN_INLINE unsigned long __readcr3(void)
{
	unsigned long value;
	__asm__ __volatile__("mov %%cr3, %[value]" : [value] "=r" (value));
	return value;
}

__INTRIN_INLINE unsigned long __readcr4(void)
{
	unsigned long value;
	__asm__ __volatile__("mov %%cr4, %[value]" : [value] "=r" (value));
	return value;
}
#endif

#ifdef _M_AMD64
__INTRIN_INLINE unsigned __int64 __readdr(unsigned int reg)
{
	unsigned __int64 value;
	switch (reg)
	{
		case 0:
			__asm__ __volatile__("movq %%dr0, %q[value]" : [value] "=r" (value));
			break;
		case 1:
			__asm__ __volatile__("movq %%dr1, %q[value]" : [value] "=r" (value));
			break;
		case 2:
			__asm__ __volatile__("movq %%dr2, %q[value]" : [value] "=r" (value));
			break;
		case 3:
			__asm__ __volatile__("movq %%dr3, %q[value]" : [value] "=r" (value));
			break;
		case 4:
			__asm__ __volatile__("movq %%dr4, %q[value]" : [value] "=r" (value));
			break;
		case 5:
			__asm__ __volatile__("movq %%dr5, %q[value]" : [value] "=r" (value));
			break;
		case 6:
			__asm__ __volatile__("movq %%dr6, %q[value]" : [value] "=r" (value));
			break;
		case 7:
			__asm__ __volatile__("movq %%dr7, %q[value]" : [value] "=r" (value));
			break;
	}
	return value;
}

__INTRIN_INLINE void __writedr(unsigned reg, unsigned __int64 value)
{
	switch (reg)
	{
		case 0:
			__asm__("movq %q[value], %%dr0" : : [value] "r" (value) : "memory");
			break;
		case 1:
			__asm__("movq %q[value], %%dr1" : : [value] "r" (value) : "memory");
			break;
		case 2:
			__asm__("movq %q[value], %%dr2" : : [value] "r" (value) : "memory");
			break;
		case 3:
			__asm__("movq %q[value], %%dr3" : : [value] "r" (value) : "memory");
			break;
		case 4:
			__asm__("movq %q[value], %%dr4" : : [value] "r" (value) : "memory");
			break;
		case 5:
			__asm__("movq %q[value], %%dr5" : : [value] "r" (value) : "memory");
			break;
		case 6:
			__asm__("movq %q[value], %%dr6" : : [value] "r" (value) : "memory");
			break;
		case 7:
			__asm__("movq %q[value], %%dr7" : : [value] "r" (value) : "memory");
			break;
	}
}
#else
__INTRIN_INLINE unsigned int __readdr(unsigned int reg)
{
	unsigned int value;
	switch (reg)
	{
		case 0:
			__asm__ __volatile__("mov %%dr0, %[value]" : [value] "=r" (value));
			break;
		case 1:
			__asm__ __volatile__("mov %%dr1, %[value]" : [value] "=r" (value));
			break;
		case 2:
			__asm__ __volatile__("mov %%dr2, %[value]" : [value] "=r" (value));
			break;
		case 3:
			__asm__ __volatile__("mov %%dr3, %[value]" : [value] "=r" (value));
			break;
		case 4:
			__asm__ __volatile__("mov %%dr4, %[value]" : [value] "=r" (value));
			break;
		case 5:
			__asm__ __volatile__("mov %%dr5, %[value]" : [value] "=r" (value));
			break;
		case 6:
			__asm__ __volatile__("mov %%dr6, %[value]" : [value] "=r" (value));
			break;
		case 7:
			__asm__ __volatile__("mov %%dr7, %[value]" : [value] "=r" (value));
			break;
	}
	return value;
}

__INTRIN_INLINE void __writedr(unsigned reg, unsigned int value)
{
	switch (reg)
	{
		case 0:
			__asm__("mov %[value], %%dr0" : : [value] "r" (value) : "memory");
			break;
		case 1:
			__asm__("mov %[value], %%dr1" : : [value] "r" (value) : "memory");
			break;
		case 2:
			__asm__("mov %[value], %%dr2" : : [value] "r" (value) : "memory");
			break;
		case 3:
			__asm__("mov %[value], %%dr3" : : [value] "r" (value) : "memory");
			break;
		case 4:
			__asm__("mov %[value], %%dr4" : : [value] "r" (value) : "memory");
			break;
		case 5:
			__asm__("mov %[value], %%dr5" : : [value] "r" (value) : "memory");
			break;
		case 6:
			__asm__("mov %[value], %%dr6" : : [value] "r" (value) : "memory");
			break;
		case 7:
			__asm__("mov %[value], %%dr7" : : [value] "r" (value) : "memory");
			break;
	}
}
#endif

__INTRIN_INLINE void __invlpg(void * const Address)
{
	__asm__("invlpg %[Address]" : : [Address] "m" (*((unsigned char *)(Address))) : "memory");
}


/*** System operations ***/
__INTRIN_INLINE unsigned long long __readmsr(const int reg)
{
#ifdef _M_AMD64
	unsigned long low, high;
	__asm__ __volatile__("rdmsr" : "=a" (low), "=d" (high) : "c" (reg));
	return ((unsigned long long)high << 32) | low;
#else
	unsigned long long retval;
	__asm__ __volatile__("rdmsr" : "=A" (retval) : "c" (reg));
	return retval;
#endif
}

__INTRIN_INLINE void __writemsr(const unsigned long Register, const unsigned long long Value)
{
#ifdef _M_AMD64
	__asm__ __volatile__("wrmsr" : : "a" (Value), "d" (Value >> 32), "c" (Register));
#else
	__asm__ __volatile__("wrmsr" : : "A" (Value), "c" (Register));
#endif
}

__INTRIN_INLINE unsigned long long __readpmc(const int counter)
{
	unsigned long long retval;
	__asm__ __volatile__("rdpmc" : "=A" (retval) : "c" (counter));
	return retval;
}

/* NOTE: an immediate value for 'a' will raise an ICE in Visual C++ */
__INTRIN_INLINE unsigned long __segmentlimit(const unsigned long a)
{
	unsigned long retval;
	__asm__ __volatile__("lsl %[a], %[retval]" : [retval] "=r" (retval) : [a] "rm" (a));
	return retval;
}

__INTRIN_INLINE void __wbinvd(void)
{
	__asm__ __volatile__("wbinvd" : : : "memory");
}

__INTRIN_INLINE void __lidt(void *Source)
{
	__asm__ __volatile__("lidt %0" : : "m"(*(short*)Source));
}

__INTRIN_INLINE void __sidt(void *Destination)
{
	__asm__ __volatile__("sidt %0" : : "m"(*(short*)Destination) : "memory");
}

/*** Misc operations ***/

__INTRIN_INLINE void _mm_pause(void)
{
	__asm__ __volatile__("pause" : : : "memory");
}

__INTRIN_INLINE void __nop(void)
{
	__asm__ __volatile__("nop");
}

#ifdef __cplusplus
}
#endif

#endif /* KJK_INTRIN_X86_H_ */

/* EOF */
