
#ifdef _M_IX86
#define KUSER_SHARED_SYSCALL HEX(7ffe0300)
#define KGDT_R0_CODE 8
MACRO(STUBCODE_U, SyscallId, StackBytes)
    FPO 0, 0, 0, 0, 0, FRAME_FPO
    mov eax, SyscallId
    mov ecx, KUSER_SHARED_SYSCALL
    call dword ptr [ecx]
    ret StackBytes
ENDM
MACRO(STUBCODE_K, SyscallId, StackBytes)
    FPO 0, 0, 0, 0, 0, FRAME_FPO
    mov eax, SyscallId
    lea edx, [esp + 4]
    pushfd
    push KGDT_R0_CODE
    call _KiSystemService
    ret StackBytes
ENDM
#elif defined(_M_AMD64)
MACRO(STUBCODE_U, SyscallId, StackBytes)
    .ENDPROLOG
    mov eax, SyscallId
    mov r10, rcx
    syscall
    ret StackBytes
ENDM
MACRO(STUBCODE_K, SyscallId, StackBytes)
    .ENDPROLOG
    mov eax, SyscallId
    call KiSystemService
    ret StackBytes
ENDM
#elif defined(_M_ARM)
MACRO(STUBCODE_U, SyscallId, StackBytes)
    swi #SyscallId
    bx lr
ENDM
MACRO(STUBCODE_K, SyscallId, StackBytes)
    mov ip, lr
    swi #SyscallId
    bx ip
ENDM
#elif defined(_M_PPC)
MACRO(STUBCODE_U, SyscallId, StackBytes)
    stwu 1,-16(1)
    mflr 0
    stw  0,0(1)
    li   0, SyscallId
    sc
    lwz 0,0(1)
    mtlr 0
    addi 1,1,16
    blr
ENDM
#define STUBCODE_K STUBCODE_U
#elif defined(_M_MIPS)
MACRO(STUBCODE_U, SyscallId, StackBytes)
    li $8, KUSER_SHARED_SYSCALL
    lw $8,0($8)
    j $8
    nop
ENDM
MACRO(STUBCODE_K, SyscallId, StackBytes)
    j KiSystemService
    nop
ENDM
#else
#error unsupported architecture
#endif

#ifdef _M_IX86
MACRO(MAKE_LABEL, Name, Stackbytes)
    PUBLIC _&Name&@&Stackbytes
    _&Name&@&Stackbytes:
ENDM
MACRO(START_PROC, Name, Stackbytes)
    PUBLIC _&Name&@&Stackbytes
    .PROC &Name&@&Stackbytes
ENDM
MACRO(END_PROC, Name, Stackbytes)
    .ENDP &Name&@&Stackbytes
ENDM
#else
MACRO(MAKE_LABEL, Name, Stackbytes)
    PUBLIC &Name
    &Name:
ENDM
MACRO(START_PROC, Name, Stackbytes)
    PUBLIC &Name
    .PROC &Name
ENDM
MACRO(END_PROC, Name, Stackbytes)
    .ENDP &Name
ENDM
#endif

MACRO(STUB_U, Name, ArgCount)
    Stackbytes = 4 * &ArgCount
    MAKE_LABEL Zw&Name, %Stackbytes
    START_PROC Nt&Name, %Stackbytes
    STUBCODE_U SyscallId, %Stackbytes
    END_PROC Nt&Name, %Stackbytes
    SyscallId = SyscallId + 1
ENDM

MACRO(STUB_K, Name, ArgCount)
    Stackbytes = 4 * &ArgCount
    START_PROC Zw&Name, %Stackbytes
    STUBCODE_K SyscallId, %Stackbytes
    END_PROC Zw&Name, %Stackbytes
    SyscallId = SyscallId + 1
ENDM
