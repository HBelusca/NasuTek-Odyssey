
#ifndef HEX
#define HEX(y) 0x##y
#endif

/* Memory layout */
//#ifdef _M_AMD64
#define PML4_ADDRESS        HEX(1000) /* One page PML4 page table */
#define PDP_ADDRESS         HEX(2000) /* One page PDP page table */
#define PD_ADDRESS          HEX(3000) /* One page PD page table */
//#endif
#define STACK16ADDR         HEX(6F00) /* The 16-bit stack top will be at 0000:6F00 */
#define BSS_START           HEX(6F00)
#define FREELDR_BASE        HEX(F800)
#define FREELDR_PE_BASE    HEX(10000)
#define STACK32ADDR        HEX(98000) /* The 32-bit stack top will be at 9000:8000, or 0xA8000 */
#define STACK64ADDR	       HEX(98000) /* The 64-bit stack top will be at 98000 */
#define BIOSCALLBUFFER     HEX(98000) /* Buffer to store temporary data for any Int386() call */
#define FILESYSBUFFER      HEX(80000) /* Buffer to store file system data (e.g. cluster buffer for FAT) */
#define DISKREADBUFFER     HEX(90000) /* Buffer to store data read in from the disk via the BIOS */
#define DISKREADBUFFER_SIZE 512

#define BIOSCALLBUFSEGMENT (BIOSCALLBUFFER/16) /* Buffer to store temporary data for any Int386() call */
#define BIOSCALLBUFOFFSET   HEX(0000) /* Buffer to store temporary data for any Int386() call */

/* Layout of the REGS structure */
#define REGS_EAX 0
#define REGS_EBX 4
#define REGS_ECX 8
#define REGS_EDX 12
#define REGS_ESI 16
#define REGS_EDI 20
#define REGS_DS 24
#define REGS_ES 26
#define REGS_FS 28
#define REGS_GS 30
#define REGS_EFLAGS 32
#define REGS_SIZE 36

/* These addresses specify the realmode "BSS section" layout */
#define BSS_RealModeEntry        (BSS_START +  0)
#define BSS_CallbackAddress      (BSS_START +  4)
#define BSS_CallbackReturn       (BSS_START +  8)
#define BSS_RegisterSet          (BSS_START + 16) /* size = 36 */
#define BSS_IntVector            (BSS_START + 52)
#define BSS_PxeEntryPoint        (BSS_START + 56)
#define BSS_PxeBufferSegment     (BSS_START + 60)
#define BSS_PxeBufferOffset      (BSS_START + 64)
#define BSS_PxeFunction          (BSS_START + 68)
#define BSS_PxeResult            (BSS_START + 72)
#define BSS_PnpBiosEntryPoint    (BSS_START + 76)
#define BSS_PnpBiosDataSegment   (BSS_START + 80)
#define BSS_PnpBiosBufferSegment (BSS_START + 84)
#define BSS_PnpBiosBufferOffset  (BSS_START + 88)
#define BSS_PnpNodeSize          (BSS_START + 92)
#define BSS_PnpNodeCount         (BSS_START + 96)
#define BSS_PnpNodeNumber        (BSS_START + 100)
#define BSS_PnpResult            (BSS_START + 104)
#define BSS_BootDrive            (BSS_START + 108) // 1 byte
#define BSS_BootPartition        (BSS_START + 109) // 1 byte


/* Realmode function IDs */
#define FNID_Int386 0
#define FNID_SoftReboot 1
#define FNID_ChainLoadBiosBootSectorCode 2
#define FNID_PxeCallApi 3
#define FNID_PnpBiosGetDeviceNodeCount 4
#define FNID_PnpBiosGetDeviceNode 5
#define FNID_BootLinuxKernel 6

/* Flag Masks */
#define CR0_PE_SET	HEX(00000001)	/* OR this value with CR0 to enable pmode */
#define CR0_PE_CLR	HEX(FFFFFFFE)	/* AND this value with CR0 to disable pmode */

/* Defines needed for switching between real and protected mode */
//#ifdef _M_IX86
#define NULL_DESC	HEX(00)	/* NULL descriptor */
#define PMODE_CS	HEX(08)	/* PMode code selector, base 0 limit 4g */
#define PMODE_DS	HEX(10)	/* PMode data selector, base 0 limit 4g */
#define RMODE_CS	HEX(18)	/* RMode code selector, base 0 limit 64k */
#define RMODE_DS	HEX(20)	/* RMode data selector, base 0 limit 64k */
//#else
/* Long mode selectors */
#define LMODE_CS HEX(10)
#define LMODE_DS HEX(18)
#define CMODE_CS HEX(30)
//#endif

/* Makes "x" a global variable or label */
#define EXTERN(x)	.global x; x:
