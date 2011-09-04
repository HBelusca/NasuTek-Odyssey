/*
 *  ECHO.C - internal echo commands.
 *
 *
 *  History:
 *
 *    16 Jul 1998 (Hans B Pufal)
 *        Started.
 *
 *    16 Jul 1998 (John P Price)
 *        Separated commands into individual files.
 *
 *    27-Jul-1998 (John P Price <linux-guru@gcfl.net>)
 *        Added config.h include
 *
 *    08-Dec-1998 (Eric Kohl)
 *        Added help text ("/?").
 *
 *    19-Jan-1999 (Eric Kohl)
 *        Unicode and redirection ready!
 *
 *    13-Jul-2000 (Eric Kohl)
 *        Implemented 'echo.' and 'echoerr.'.
 *
 *    28-Apr-2005 (Magnus Olsen) <magnus@greatlord.com>)
 *        Remove all hardcode string to En.rc
 */

#include <precomp.h>

BOOL
OnOffCommand(LPTSTR param, LPBOOL flag, INT message)
{
	TCHAR *p2;
	if (_tcsnicmp(param, D_OFF, sizeof(D_OFF)/sizeof(TCHAR) - 1) == 0)
	{
		p2 = param + sizeof(D_OFF)/sizeof(TCHAR) - 1;
		while (_istspace(*p2))
			p2++;
		if (*p2 == _T('\0'))
		{
			*flag = FALSE;
			return TRUE;
		}
	}
	else if (_tcsnicmp(param, D_ON, sizeof(D_ON)/sizeof(TCHAR) - 1) == 0)
	{
		p2 = param + sizeof(D_ON)/sizeof(TCHAR) - 1;
		while (_istspace(*p2))
			p2++;
		if (*p2 == _T('\0'))
		{
			*flag = TRUE;
			return TRUE;
		}
	}
	else if (*param == _T('\0'))
	{
		ConOutResPrintf(message, *flag ? D_ON : D_OFF);
		return TRUE;
	}
	return FALSE;
}

INT CommandEcho (LPTSTR param)
{
	LPTSTR p1;

	TRACE ("CommandEcho: '%s'\n", debugstr_aw(param));

                /* skip all spaces for the check of '/?', 'ON' and 'OFF' */
                p1 = param;
                while(_istspace(*p1))
                        p1++;

	        if (!_tcsncmp (p1, _T("/?"), 2))
	        {
		        ConOutResPaging(TRUE,STRING_ECHO_HELP4);
		        return 0;
	        }

	if (!OnOffCommand(p1, &bEcho, STRING_ECHO_HELP5))
	{
		/* skip the first character */
		ConOutPuts(param + 1);
	}
	return 0;
}

INT CommandEchos (LPTSTR param)
{
	TRACE ("CommandEchos: '%s'\n", debugstr_aw(param));

	if (!_tcsncmp (param, _T("/?"), 2))
	{
		ConOutResPuts(STRING_ECHO_HELP1);
		return 0;
	}

	ConOutPrintf (_T("%s"), param);
	return 0;
}


INT CommandEchoerr (LPTSTR param)
{
	TRACE ("CommandEchoerr: '%s'\n", debugstr_aw(param));

	if (!_tcsncmp (param, _T("/?"), 2))
	{
		ConOutResPuts(STRING_ECHO_HELP2);
		return 0;
	}

	ConErrPuts (param);
	return 0;
}


INT CommandEchoserr (LPTSTR param)
{
	TRACE ("CommandEchoserr: '%s'\n", debugstr_aw(param));

	if (!_tcsncmp (param, _T("/?"), 2))
	{
		ConOutResPuts(STRING_ECHO_HELP3);
		return 0;
	}

	ConErrPrintf (_T("%s"), param);
	return 0;
}

/* EOF */
