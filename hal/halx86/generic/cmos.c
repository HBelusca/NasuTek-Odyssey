/*
 * PROJECT:         Odyssey HAL
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            hal/halx86/generic/cmos.c
 * PURPOSE:         CMOS Access Routines (Real Time Clock and LastKnownGood)
 * PROGRAMMERS:     Alex Ionescu (alex.ionescu@odyssey.org)
 *                  Eric Kohl
 */

/* INCLUDES ******************************************************************/

#include <hal.h>
#define NDEBUG
#include <debug.h>

/* GLOBALS *******************************************************************/

UCHAR HalpCmosCenturyOffset;

/* PRIVATE FUNCTIONS *********************************************************/

UCHAR
NTAPI
HalpReadCmos(IN UCHAR Reg)
{
    /* Select the register */
    WRITE_PORT_UCHAR(CMOS_CONTROL_PORT, Reg);

    /* Query the value */
    return READ_PORT_UCHAR(CMOS_DATA_PORT);
}

VOID
NTAPI
HalpWriteCmos(IN UCHAR Reg,
              IN UCHAR Value)
{
    /* Select the register */
    WRITE_PORT_UCHAR(CMOS_CONTROL_PORT, Reg);

    /* Write the value */
    WRITE_PORT_UCHAR(CMOS_DATA_PORT, Value);
}

ULONG
NTAPI
HalpGetCmosData(IN ULONG BusNumber,
                IN ULONG SlotNumber,
                IN PVOID Buffer,
                IN ULONG Length)
{
    PUCHAR Ptr = (PUCHAR)Buffer;
    ULONG Address = SlotNumber;
    ULONG Len = Length;

    /* Do nothing if we don't have a length */
    if (!Length) return 0;

    /* Acquire CMOS Lock */
    HalpAcquireSystemHardwareSpinLock();

    /* Check if this is simple CMOS */
    if (BusNumber == 0)
    {
        /* Loop the buffer up to 0xFF */
        while ((Len > 0) && (Address < 0x100))
        {
            /* Read the data */
            *Ptr = HalpReadCmos((UCHAR)Address);

            /* Update position and length */
            Ptr++;
            Address++;
            Len--;
        }
    }
    else if (BusNumber == 1)
    {
        /* Loop the buffer up to 0xFFFF */
        while ((Len > 0) && (Address < 0x10000))
        {
            /* Write the data */
            *Ptr = HalpReadCmos((UCHAR)Address);

            /* Update position and length */
            Ptr++;
            Address++;
            Len--;
        }
    }

    /* Release CMOS Lock */
    HalpReleaseCmosSpinLock();

    /* Return length read */
    return Length - Len;
}

ULONG
NTAPI
HalpSetCmosData(IN ULONG BusNumber,
                IN ULONG SlotNumber,
                IN PVOID Buffer,
                IN ULONG Length)
{
    PUCHAR Ptr = (PUCHAR)Buffer;
    ULONG Address = SlotNumber;
    ULONG Len = Length;

    /* Do nothing if we don't have a length */
    if (!Length) return 0;

    /* Acquire CMOS Lock */
    HalpAcquireSystemHardwareSpinLock();

    /* Check if this is simple CMOS */
    if (BusNumber == 0)
    {
        /* Loop the buffer up to 0xFF */
        while ((Len > 0) && (Address < 0x100))
        {
            /* Write the data */
            HalpWriteCmos((UCHAR)Address, *Ptr);

            /* Update position and length */
            Ptr++;
            Address++;
            Len--;
        }
    }
    else if (BusNumber == 1)
    {
        /* Loop the buffer up to 0xFFFF */
        while ((Len > 0) && (Address < 0x10000))
        {
            /* Write the data */
            HalpWriteCmos((UCHAR)Address, *Ptr);

            /* Update position and length */
            Ptr++;
            Address++;
            Len--;
        }
    }

    /* Release CMOS Lock */
    HalpReleaseCmosSpinLock();

    /* Return length read */
    return Length - Len;
}

VOID
NTAPI
INIT_FUNCTION
HalpInitializeCmos(VOID)
{
    /* Set default century offset byte */
    HalpCmosCenturyOffset = 50;

    /* No support for EISA or MCA */
    ASSERT(HalpBusType == MACHINE_TYPE_ISA);
}

/* PUBLIC FUNCTIONS **********************************************************/

/*
 * @implemented
 */
ARC_STATUS
NTAPI
HalGetEnvironmentVariable(IN PCH Name,
                          IN USHORT ValueLength,
                          IN PCH Value)
{
    UCHAR Val;

    /* Only variable supported on x86 */
    if (_stricmp(Name, "LastKnownGood")) return ENOENT;

    /* Acquire CMOS Lock */
    HalpAcquireSystemHardwareSpinLock();

    /* Query the current value */
    Val = HalpReadCmos(RTC_REGISTER_B) & 0x01;

    /* Release CMOS lock */
    HalpReleaseCmosSpinLock();

    /* Check the flag */
    if (Val)
    {
        /* Return false */
        strncpy(Value, "FALSE", ValueLength);
    }
    else
    {
        /* Return true */
        strncpy(Value, "TRUE", ValueLength);
    }

    /* Return success */
    return ESUCCESS;
}

/*
 * @implemented
 */
ARC_STATUS
NTAPI
HalSetEnvironmentVariable(IN PCH Name,
                          IN PCH Value)
{
    UCHAR Val;

    /* Only variable supported on x86 */
    if (_stricmp(Name, "LastKnownGood")) return ENOMEM;

    /* Check if this is true or false */
    if (!_stricmp(Value, "TRUE"))
    {
        /* It's true, acquire CMOS lock */
        HalpAcquireSystemHardwareSpinLock();

        /* Read the current value and add the flag */
        Val = HalpReadCmos(RTC_REGISTER_B) | 1;
    }
    else if (!_stricmp(Value, "FALSE"))
    {
        /* It's false, acquire CMOS lock */
        HalpAcquireSystemHardwareSpinLock();

        /* Read the current value and mask out  the flag */
        Val = HalpReadCmos(RTC_REGISTER_B) & ~1;
    }
    else
    {
        /* Fail */
        return ENOMEM;
    }

    /* Write new value */
    HalpWriteCmos(RTC_REGISTER_B, Val);

    /* Release the lock and return success */
    HalpReleaseCmosSpinLock();
    return ESUCCESS;
}

/*
 * @implemented
 */
BOOLEAN
NTAPI
HalQueryRealTimeClock(OUT PTIME_FIELDS Time)
{
    /* Acquire CMOS Lock */
    HalpAcquireSystemHardwareSpinLock();

    /* Loop while update is in progress */
    while ((HalpReadCmos(RTC_REGISTER_A)) & RTC_REG_A_UIP);

    /* Set the time data */
    Time->Second = BCD_INT(HalpReadCmos(0));
    Time->Minute = BCD_INT(HalpReadCmos(2));
    Time->Hour = BCD_INT(HalpReadCmos(4));
    Time->Weekday = BCD_INT(HalpReadCmos(6));
    Time->Day = BCD_INT(HalpReadCmos(7));
    Time->Month = BCD_INT(HalpReadCmos(8));
    Time->Year = BCD_INT(HalpReadCmos(9));
    Time->Milliseconds = 0;

    /* FIXME: Check century byte */

    /* Compensate for the century field */
    Time->Year += (Time->Year > 80) ? 1900: 2000;

    /* Release CMOS lock */
    HalpReleaseCmosSpinLock();

    /* Always return TRUE */
    return TRUE;
}

/*
 * @implemented
 */
BOOLEAN
NTAPI
HalSetRealTimeClock(IN PTIME_FIELDS Time)
{
    /* Acquire CMOS Lock */
    HalpAcquireSystemHardwareSpinLock();

    /* Loop while update is in progress */
    while ((HalpReadCmos(RTC_REGISTER_A)) & RTC_REG_A_UIP);

    /* Write time fields to CMOS RTC */
    HalpWriteCmos(0, INT_BCD(Time->Second));
    HalpWriteCmos(2, INT_BCD(Time->Minute));
    HalpWriteCmos(4, INT_BCD(Time->Hour));
    HalpWriteCmos(6, INT_BCD(Time->Weekday));
    HalpWriteCmos(7, INT_BCD(Time->Day));
    HalpWriteCmos(8, INT_BCD(Time->Month));
    HalpWriteCmos(9, INT_BCD(Time->Year % 100));

    /* FIXME: Set the century byte */

    /* Release CMOS lock */
    HalpReleaseCmosSpinLock();

    /* Always return TRUE */
    return TRUE;
}
