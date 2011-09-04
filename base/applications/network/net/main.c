
/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey net command
 * FILE:
 * PURPOSE:
 *
 * PROGRAMMERS:     Magnus Olsen (greatlord@odyssey.org)
 */

#include "net.h"



int main(int argc, char **argv)
{
    if (argc<2)
	{
      help();
      return 1;
	}

    if (_stricmp(argv[1],"ACCOUNTS")==0)
    {
		return unimplemented();
    }
    if (_stricmp(argv[1],"COMPUTER")==0)
    {
		return unimplemented();
    }
    if (_stricmp(argv[1],"CONFIG")==0)
    {
		return unimplemented();
    }
    if (_stricmp(argv[1],"CONTINUE")==0)
    {
		return unimplemented();
    }

    if (_stricmp(argv[1],"FILE")==0)
    {
		return unimplemented();
    }
    if (_stricmp(argv[1],"GROUP")==0)
    {
		return unimplemented();
    }
    if (_stricmp(argv[1],"HELP")==0)
    {
        return cmdHelp(argc,&argv[1]);
    }
    if (_stricmp(argv[1],"HELPMSG")==0)
    {
		return unimplemented();
    }
    if (_stricmp(argv[1],"LOCALGROUP")==0)
    {
		return unimplemented();
    }
    if (_stricmp(argv[1],"NAME")==0)
    {
		return unimplemented();
    }
    if (_stricmp(argv[1],"PRINT")==0)
    {
		return unimplemented();
    }
    if (_stricmp(argv[1],"SEND")==0)
    {
		return unimplemented();
    }
    if (_stricmp(argv[1],"SESSION")==0)
    {
		return unimplemented();
    }
    if (_stricmp(argv[1],"SHARE")==0)
    {
		return unimplemented();
    }

    if (_stricmp(argv[1],"START")==0)
    {
       return cmdStart(argc, &argv[1]);
    }
    if (_stricmp(argv[1],"STATISTICS")==0)
    {
		return unimplemented();
    }
    if (_stricmp(argv[1],"STOP")==0)
    {
		return cmdStop(argc, &argv[1]);
    }
    if (_stricmp(argv[1],"TIME")==0)
    {
		return unimplemented();
    }
    if (_stricmp(argv[1],"USE")==0)
    {
		return unimplemented();
    }
    if (_stricmp(argv[1],"USER")==0)
    {
		return unimplemented();
    }
    if (_stricmp(argv[1],"VIEW")==0)
    {
		return unimplemented();
    }

    help();
	return 1;
}


int unimplemented()
{
	puts("This command is not implemented yet");
	return 1;
}



















