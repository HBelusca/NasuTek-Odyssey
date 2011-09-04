#pragma once

extern KIRQL TcpipGetCurrentIrql(VOID);
extern VOID TcpipInitializeSpinLock( PKSPIN_LOCK SpinLock );
extern VOID TcpipAcquireSpinLock( PKSPIN_LOCK SpinLock, PKIRQL Irql );
extern VOID TcpipReleaseSpinLock( PKSPIN_LOCK SpinLock, KIRQL Irql );
extern VOID TcpipAcquireSpinLockAtDpcLevel( PKSPIN_LOCK SpinLock );
extern VOID TcpipReleaseSpinLockFromDpcLevel( PKSPIN_LOCK SpinLock );
extern VOID TcpipInterlockedInsertTailList( PLIST_ENTRY ListHead,
					    PLIST_ENTRY Item,
					    PKSPIN_LOCK Lock );
extern VOID TcpipAcquireFastMutex( PFAST_MUTEX Mutex );
extern VOID TcpipReleaseFastMutex( PFAST_MUTEX Mutex );
