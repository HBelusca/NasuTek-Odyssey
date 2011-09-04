/*
 * PROJECT:         Odyssey Kernel
 * LICENSE:         BSD - See COPYING.ARM in the top level directory
 * FILE:            ntoskrnl/ex/hdlsterm.c
 * PURPOSE:         Headless Terminal Support
 * PROGRAMMERS:     Odyssey Portable Systems Group
 */

/* INCLUDES ******************************************************************/

#include <ntoskrnl.h>
#include <debug.h>

/* GLOBALS *******************************************************************/

PHEADLESS_GLOBALS HeadlessGlobals;

/* FUNCTIONS *****************************************************************/

VOID
NTAPI
HdlspSendStringAtBaud(
	IN PUCHAR String
	)
{
	/* Send every byte */
	while (*String++ != ANSI_NULL)
	{
		InbvPortPutByte(HeadlessGlobals->TerminalPort, *String);
	}
}

NTSTATUS
NTAPI
HdlspEnableTerminal(
	IN BOOLEAN Enable
	)
{
	/* Enable if requested, as long as this isn't a PCI serial port crashing */
	if ((Enable) &&
		!(HeadlessGlobals->TerminalEnabled) &&
		!((HeadlessGlobals->IsMMIODevice) && (HeadlessGlobals->InBugCheck)))
	{
		/* Initialize the COM port with cportlib */
		HeadlessGlobals->TerminalEnabled = InbvPortInitialize(
			HeadlessGlobals->TerminalBaudRate,
			HeadlessGlobals->TerminalPortNumber,
			HeadlessGlobals->TerminalPortAddress,
			&HeadlessGlobals->TerminalPort,
			HeadlessGlobals->IsMMIODevice);
        if (!HeadlessGlobals->TerminalEnabled) return STATUS_UNSUCCESSFUL;

		/* Cleanup the screen and reset the cursor */
		HdlspSendStringAtBaud((PUCHAR)"\x1B[2J");
		HdlspSendStringAtBaud((PUCHAR)"\x1B[H");

		/* Enable FIFO */
		InbvPortEnableFifo(HeadlessGlobals->TerminalPort, TRUE);
	}
	else if (!Enable)
	{
		/* Specific case when headless is being disabled */
		InbvPortTerminate(HeadlessGlobals->TerminalPort);
		HeadlessGlobals->TerminalPort = 0;
		HeadlessGlobals->TerminalEnabled = FALSE;
	}
	return STATUS_SUCCESS;
}

VOID
NTAPI
INIT_FUNCTION
HeadlessInit(
	IN PLOADER_PARAMETER_BLOCK LoaderBlock
	)
{
	PHEADLESS_LOADER_BLOCK HeadlessBlock;

	HeadlessBlock = LoaderBlock->Extension->HeadlessLoaderBlock;
	if (!HeadlessBlock) return;
	if ((HeadlessBlock->PortNumber > 4) && (HeadlessBlock->UsedBiosSettings)) return;

	HeadlessGlobals = ExAllocatePoolWithTag(
		NonPagedPool,
		sizeof(HEADLESS_GLOBALS),
		'sldH');
	if (!HeadlessGlobals) return;

	/* Zero and copy loader data */
	RtlZeroMemory(HeadlessGlobals, sizeof(HEADLESS_GLOBALS));
	HeadlessGlobals->TerminalPortNumber = HeadlessBlock->PortNumber;
	HeadlessGlobals->TerminalPortAddress = HeadlessBlock->PortAddress;
	HeadlessGlobals->TerminalBaudRate = HeadlessBlock->BaudRate;
	HeadlessGlobals->TerminalParity = HeadlessBlock->Parity;
	HeadlessGlobals->TerminalStopBits = HeadlessBlock->StopBits;
	HeadlessGlobals->UsedBiosSettings = HeadlessBlock->UsedBiosSettings;
	HeadlessGlobals->IsMMIODevice = HeadlessBlock->IsMMIODevice;
	HeadlessGlobals->TerminalType = HeadlessBlock->TerminalType;
	HeadlessGlobals->SystemGUID = HeadlessBlock->SystemGUID;

	/* These two are opposites of each other */
	if (HeadlessGlobals->IsMMIODevice) HeadlessGlobals->IsNonLegacyDevice = TRUE;

	/* Check for a PCI device, warn that this isn't supported */
	if (HeadlessBlock->PciDeviceId != PCI_INVALID_VENDORID)
	{
		DPRINT1("PCI Serial Ports not supported\n");
	}

	/* Log entries are not yet supported */
	DPRINT1("FIXME: No Headless logging support\n");

	/* Allocate temporary buffer */
	HeadlessGlobals->TmpBuffer = ExAllocatePoolWithTag(NonPagedPool, 80, 'sldH');
	if (!HeadlessGlobals->TmpBuffer) return;

	/* Windows seems to apply some special hacks for 9600 bps */
	if (HeadlessGlobals->TerminalBaudRate == 9600)
	{
		DPRINT1("Please use other baud rate than 9600bps for now\n");
	}

	/* Enable the terminal */
	HdlspEnableTerminal(TRUE);
}

VOID
NTAPI
HdlspPutString(
	IN PUCHAR String
	)
{
	PUCHAR Dest = HeadlessGlobals->TmpBuffer;
	UCHAR Char = 0;

	/* Scan each character */
	while (*String != ANSI_NULL)
	{
		/* Check for rotate, send existing buffer and restart from where we are */
		if (Dest >= &HeadlessGlobals->TmpBuffer[79])
		{
			HeadlessGlobals->TmpBuffer[79] = ANSI_NULL;
			HdlspSendStringAtBaud(HeadlessGlobals->TmpBuffer);
			Dest = HeadlessGlobals->TmpBuffer;
		}
		else
		{
			/* Get the current character and check for special graphical chars */
			Char = *String;
			if (Char & 0x80)
			{
				switch (Char)
				{
					case 0xB0: case 0xB3: case 0xBA:
						Char = '|';
						break;
					case 0xB1: case 0xDC: case 0xDD: case 0xDE: case 0xDF:
						Char = '%';
						break;
					case 0xB2: case 0xDB:
						Char = '#';
						break;
					case 0xA9: case 0xAA: case 0xBB: case 0xBC: case 0xBF:
					case 0xC0: case 0xC8: case 0xC9: case 0xD9: case 0xDA:
						Char = '+';
						break;
					case 0xC4:
						Char = '-';
						break;
					case 0xCD:
						Char = '=';
						break;
					}
			}

			/* Anything else must be Unicode */
			if (Char & 0x80)
			{
				/* Can't do Unicode yet */
				UNIMPLEMENTED;
			}
			else
			{
				/* Add the modified char to the temporary buffer */
				*Dest++ = Char;
			}
			
			/* Check the next char */
			String++;
		}
	}

	/* Finish and send */
	*Dest = ANSI_NULL;
	HdlspSendStringAtBaud(HeadlessGlobals->TmpBuffer);
}

NTSTATUS
NTAPI
HdlspDispatch(
	IN HEADLESS_CMD Command,
	IN PVOID InputBuffer,
	IN SIZE_T InputBufferSize,
	OUT PVOID OutputBuffer,
	OUT PSIZE_T OutputBufferSize
	)
{
	NTSTATUS Status = STATUS_NOT_IMPLEMENTED;
	ASSERT(HeadlessGlobals != NULL);
//	ASSERT(HeadlessGlobals->PageLockHandle != NULL);

	/* FIXME: This should be using the headless spinlock */

	/* Ignore non-reentrant commands */
	if ((Command != HeadlessCmdAddLogEntry) &&
		(Command != HeadlessCmdStartBugCheck) &&
		(Command != HeadlessCmdSendBlueScreenData) &&
		(Command != HeadlessCmdDoBugCheckProcessing))
	{
		if (HeadlessGlobals->ProcessingCmd) return STATUS_UNSUCCESSFUL;

		/* Don't allow these commands next time */
		HeadlessGlobals->ProcessingCmd = TRUE;
	}

	/* Handle each command */
	switch (Command)
	{
		case HeadlessCmdEnableTerminal:
			break;
		case HeadlessCmdCheckForReboot:
			break;

		case HeadlessCmdPutString:

			/* Validate the existence of an input buffer */
			if (!InputBuffer)
			{
				Status = STATUS_INVALID_PARAMETER;
				goto Reset;
			}

			/* Terminal should be on */
			if (HeadlessGlobals->TerminalEnabled)
			{
				/* Print each byte in the string making sure VT100 chars are used */
				PHEADLESS_CMD_PUT_STRING PutString = (PVOID)InputBuffer;
				HdlspPutString(PutString->String);
			}

			/* Return success either way */
			Status = STATUS_SUCCESS;
			break;
		case HeadlessCmdClearDisplay:
			break;
		case HeadlessCmdClearToEndOfDisplay:
			break;
		case HeadlessCmdClearToEndOfLine:
			break;
		case HeadlessCmdDisplayAttributesOff:
			break;
		case HeadlessCmdDisplayInverseVideo:
			break;
		case HeadlessCmdSetColor:
			break;
		case HeadlessCmdPositionCursor:
			break;
		case HeadlessCmdTerminalPoll:
			break;
		case HeadlessCmdGetByte:
			break;
		case HeadlessCmdGetLine:
			break;
		case HeadlessCmdStartBugCheck:
			break;
		case HeadlessCmdDoBugCheckProcessing:
			break;
		case HeadlessCmdQueryInformation:
			break;
		case HeadlessCmdAddLogEntry:
			break;
		case HeadlessCmdDisplayLog:
			break;
		case HeadlessCmdSetBlueScreenData:
			break;
		case HeadlessCmdSendBlueScreenData:
			break;
		case HeadlessCmdQueryGUID:
			break;
		case HeadlessCmdPutData:
			break;
		default:
			break;
	}

Reset:
	/* Unset prcessing state */
	if ((Command != HeadlessCmdAddLogEntry) &&
		(Command != HeadlessCmdStartBugCheck) &&
		(Command != HeadlessCmdSendBlueScreenData) &&
		(Command != HeadlessCmdDoBugCheckProcessing))
	{
		ASSERT(HeadlessGlobals->ProcessingCmd == TRUE);
		HeadlessGlobals->ProcessingCmd = FALSE;
	}

	//UNIMPLEMENTED;
	return STATUS_SUCCESS;
}

/*
 * @unimplemented
 */
NTSTATUS
NTAPI
HeadlessDispatch(
	IN HEADLESS_CMD Command,
	IN PVOID InputBuffer,
	IN SIZE_T InputBufferSize,
	OUT PVOID OutputBuffer,
	OUT PSIZE_T OutputBufferSize
	)
{
	/* Check for stubs that will expect something even with headless off */
	if (!HeadlessGlobals)
	{
		/* Don't allow the SAC to connect */
		if (Command == HeadlessCmdEnableTerminal) return STATUS_UNSUCCESSFUL;

		/* Send bogus reply */
		if ((Command == HeadlessCmdQueryInformation) ||
			(Command == HeadlessCmdGetByte) ||
			(Command == HeadlessCmdGetLine) ||
			(Command == HeadlessCmdCheckForReboot) ||
			(Command == HeadlessCmdTerminalPoll))
		{
			if (!(OutputBuffer) || !(OutputBufferSize)) return STATUS_INVALID_PARAMETER;
			RtlZeroMemory(OutputBuffer, *OutputBufferSize);
		}
		return STATUS_SUCCESS;
	}
	
	/* Do the real work */
	return HdlspDispatch(
		Command, 
		InputBuffer,
		InputBufferSize,
		OutputBuffer,
		OutputBufferSize);
}    

/* EOF */
