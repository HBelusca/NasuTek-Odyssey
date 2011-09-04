/* FILE:        setup/usetup/lang/cs-CZ.rc
 * TRANSLATOR:  Radek Liska aka Black_Fox (radekliska at gmail dot com)
 * THANKS TO:   preston for bugfix advice at line 842
 * UPDATED:     2011-03-31
 */

#pragma once

MUI_LAYOUTS csCZLayouts[] =
{
    { L"0405", L"00000405" },
    { L"0405", L"00010405" },
    { L"0409", L"00000409" },
    { NULL, NULL }
};

static MUI_ENTRY csCZLanguagePageEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "V�b�r jazyka",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Pros�m zvolte jazyk, kter� bude b�hem instalace pou�it.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Pot� stiskn�te ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Tento jazyk bude v�choz�m jazykem v nainstalovan�m syst�mu.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Pokra�ovat  F3 = Ukon�it",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY csCZWelcomePageEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "V�tejte v instalaci Odyssey",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        6,
        11,
        "Tato ��st instalace nakop�ruje opera�n� syst�m Odyssey do va�eho",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "po��ta�e a p�iprav� druhou ��st instalace.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Stisknut�m kl�vesy ENTER zah�j�te instalaci Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Stisknut�m R zah�j�te opravu nebo aktualizaci Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  Stiskut�m L zobraz�te Licen�n� podm�nky Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Stisknut�m F3 ukon��te instalaci Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "V�ce informac� o Odyssey naleznete na adrese:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        24,
        "http://www.odyssey.org",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        0,
        0,
        "ENTER = Pokra�ovat  R = Opravit  L = Licence  F3 = Ukon�it",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY csCZIntroPageEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Instalace Odyssey je v ran� v�vojov� f�zi. Zat�m nejsou podporov�ny",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "v�echny funkce pln� pou�iteln� instala�n� aplikace.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "Plat� n�sleduj�c� omezen�:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "- Instalace nedok��e zpracovat v�ce ne� jeden prim�rn� odd�l na disku.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "- Instalace nedok��e vymazat prim�rn� odd�l z disku,",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "  pokud na tomto disku existuj� roz��en� odd�ly.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "- Instalace nedok��e vymazat prvn� roz��en� odd�l z disku,",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "  pokud na tomto disku existuj� dal� roz��en� odd�ly.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "- Instalace podporuje pouze souborov� syst�m FAT.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "- Kontroly souborov�ch syst�m� zat�m nejsou implementov�ny.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        23,
        "\x07  Stisknut�m kl�vesy ENTER zah�j�te instalaci Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        25,
        "\x07  Stisknut�m F3 ukon��te instalaci Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Pokra�ovat   F3 = Ukon�it",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY csCZLicensePageEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        6,
        "Licence:",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        8,
        "The Odyssey System is licensed under the terms of the",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        9,
        "GNU GPL with parts containing code from other compatible",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "licenses such as the X11 or BSD and GNU LGPL licenses.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "All software that is part of the Odyssey system is",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "therefore released under the GNU GPL as well as maintaining",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "the original license.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "This software comes with NO WARRANTY or restrictions on usage",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "save applicable local and international law. The licensing of",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "Odyssey only covers distribution to third parties.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "If for some reason you did not receive a copy of the",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "GNU General Public License with Odyssey please visit",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        20,
        "http://www.gnu.org/licenses/licenses.html",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        22,
        "Z�ruka:",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        24,
        "This is free software; see the source for copying conditions.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        25,
        "There is NO warranty; not even for MERCHANTABILITY or",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        26,
        "FITNESS FOR A PARTICULAR PURPOSE",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Zp�t",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY csCZDevicePageEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "N�sleduj�c� seznam zobrazuje sou�asn� nastaven� za��zen�.",
        TEXT_STYLE_NORMAL
    },
    {
        24,
        11,
        "Po��ta�:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        12,
        "Obrazovka:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        13,
        "Kl�vesnice:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        14,
        "Rozlo�en� kl�ves:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        16,
        "P�ijmout:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        25,
        16, "P�ijmout toto nastaven� za��zen�",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        19,
        "Nastaven� hardwaru lze zm�nit stiskem kl�vesy ENTER na ��dku",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        20,
        "zvolen�m �ipkami nahoru a dol�. Pot� lze zvolit jin� nastaven�.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        21,
        " ",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "Jakmile budou v�echna nastaven� v po��dku, ozna�te \"P�ijmout toto",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        24,
        "nastaven� za��zen�\" a stiskn�te ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Pokra�ovat   F3 = Ukon�it",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY csCZRepairPageEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Instalace Odyssey je v ran� v�vojov� f�zi. Zat�m nejsou",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "podporov�ny v�echny funkce pln� pou�iteln� instala�n� aplikace.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "Opravn� funkce zat�m nejsou implementov�ny.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Stisknut�m U zah�j�te Update syst�mu.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Stisknut�m R spust�te Konzoli obnoven�.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  Stisknut�m ESC se vr�t�te na hlavn� str�nku.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Stisknut�m kl�vesy ENTER restartujete po��ta�.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ESC = Hlavn� str�nka  U = Aktualizovat  R = Z�chrana  ENTER = Restartovat",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};
static MUI_ENTRY csCZComputerPageEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Chcete zm�nit typ po��ta�e, kter� bude nainstalov�n.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Po�adovan� typ po��ta�e zvolte pomoc� �ipek nahoru a dol�.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Pot� stiskn�te ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Stisknut�m ESC se vr�t�te na p�edchoz� str�nku bez zm�ny typu",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   po��ta�e.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Pokra�ovat   ESC = Zru�it   F3 = Ukon�it",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY csCZFlushPageEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "Syst�m se nyn� uji�uje, �e v�echna data budou ulo�ena na disk.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Toto m��e trvat n�kolik minut.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "Po dokon�en� bude po��ta� automaticky zrestartov�n.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Uvol�uji cache",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY csCZQuitPageEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "Odyssey nen� kompletn� nainstalov�n",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Vyjm�te disketu z jednotky A:",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "a v�echny CD-ROM z CD mechanik.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "Stisknut�m kl�vesy ENTER restartujete po��ta�.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   �ekejte, pros�m...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG,
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY csCZDisplayPageEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Chcete zm�nit typ obrazovky, kter� bude nainstalov�na.",
        TEXT_STYLE_NORMAL
    },
    {   8,
        10,
         "\x07  Po�adovan� typ obrazovky zvolte pomoc� �ipek nahoru a dol�.",
         TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Pot� stiskn�te ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Stisknut�m ESC se vr�t�te na p�edchoz� str�nku bez zm�ny typu",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   obrazovky.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Pokra�ovat   ESC = Zru�it   F3 = Ukon�it",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY csCZSuccessPageEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "Z�kladn� sou��sti Odyssey byly �sp��n� nainstalov�ny.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Vyjm�te disketu z jednotky A:",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "a v�echny CD-ROM z CD mechanik.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "Stisknut�m kl�vesy ENTER restartujete po��ta�.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Restartovat po��ta�",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY csCZBootPageEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Instala�n� aplikace nedok��e nainstalovat zavad؟ na tento",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "disk",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        13,
        "Vlo�te naform�tovanou disketu do jednotky A:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "a stiskn�te ENTER.",
        TEXT_STYLE_NORMAL,
    },
    {
        0,
        0,
        "ENTER = Pokra�ovat   F3 = Ukon�it",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }

};

static MUI_ENTRY csCZSelectPartitionEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Na n�sleduj�c�m seznamu jsou existuj�c� odd�ly a nevyu�it�",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "m�sto pro nov� odd�ly.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "\x07  Vyberte polo�ku v seznamu pomoc� �ipek nahoru a dol�.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Stisknut�m kl�vesy ENTER nainstalujete Odyssey na zvolen� odd�l.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Stisknut�m C umo�n�te vytvo�en� nov�ho odd�lu.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Stisknut�m D umo�n�te smaz�n� existuj�c�ho odd�lu.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Cekejte, prosim...", //MUSI ZUSTAT BEZ DIAKRITIKY!
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY csCZFormatPartitionEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Form�tov�n� odd�lu",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        10,
        "Tento odd�l bude nyn� zform�tov�n. Stisknut�m kl�vesy ENTER za�nete.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Pokra�ovat   F3 = Ukon�it",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        TEXT_STYLE_NORMAL
    }
};

static MUI_ENTRY csCZInstallDirectoryEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Instalace nyn� na zvolen� odd�l nakop�ruje soubory Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "Zvolte adres��, kam bude Odyssey nainstalov�n:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "Chcete-li zm�nit navrhnut� adres��, stisknut�m kl�vesy BACKSPACE",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        15,
        "vyma�te text cesty a pot� zapi�te cestu, do kter� chcete Odyssey",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        16,
        "nainstalovat.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Pokra�ovat   F3 = Ukon�it",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY csCZFileCopyEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        0,
        12,
        "�ekejte, pros�m, instalace nyn� kop�ruje soubory do zvolen�ho adres��e.",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        13,
        " ",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        14,
        "Toto m��e trvat n�kolik minut.",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        49,
        0,
        "\xB3 �ekejte, pros�m...    ",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY csCZBootLoaderEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Instalace nyn� nainstaluje zavad؟.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "Nainstalovat zavad؟ na pevn� disk (MBR a VBR).",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "Nainstalovat zavad؟ na pevn� disk (pouze VBR).",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "Nainstalovat zavad؟ na disketu.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "P�esko�it instalaci zavad؟e.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Pokra�ovat   F3 = Ukon�it",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY csCZKeyboardSettingsEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Chcete zm�nit typ kl�vesnice, kter� bude nainstalov�na.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Po�adovan� typ kl�vesnice zvolte pomoc� �ipek nahoru a dol�.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Pot� stiskn�te ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Stisknut�m ESC se vr�t�te na p�edchoz� str�nku bez zm�ny typu",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   kl�vesnice.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Pokra�ovat   ESC = Zru�it   F3 = Ukon�it",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY csCZLayoutSettingsEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Pros�m zvolte rozlo�en�, kter� bude implicitn� nainstalov�no.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Po�adovan� rozlo�en� kl�ves zvolte pomoc� �ipek nahoru a dol�.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "    Pot� stiskn�te ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Stisknut�m ESC se vr�t�te na p�edchoz� str�nku bez zm�ny rozlo�en�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   kl�ves.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Pokra�ovat   ESC = Zru�it   F3 = Ukon�it",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY csCZPrepareCopyEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Instalace p�iprav� po��ta� na kop�rov�n� soubor� Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Sestavuji seznam soubor� ke zkop�rov�n�...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY csCZSelectFSEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        17,
        "Zvolte souborov� syst�m z n�sleduj�c�ho seznamu.",
        0
    },
    {
        8,
        19,
        "\x07  Souborov� syst�m zvol�te �ipkami nahoru a dol�.",
        0
    },
    {
        8,
        21,
        "\x07  Stisknut�m kl�vesy ENTER zform�tujete odd�l.",
        0
    },
    {
        8,
        23,
        "\x07  Stisknut�m ESC se vr�t�te na v�b�r odd�lu.",
        0
    },
    {
        0,
        0,
        "ENTER = Pokra�ovat   ESC = Zru�it   F3 = Ukon�it",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },

    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY csCZDeletePartitionEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Zvolili jste odstran�n� odd�lu",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "\x07  Stisknut�m D odstran�te odd�l.",
        TEXT_STYLE_NORMAL
    },
    {
        11,
        19,
        "POZOR: V�echna data na tomto odd�lu budou ztracena!",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Stisknut�m ESC zru�te akci.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "D = Odstranit odd�l   ESC = Zru�it   F3 = Ukon�it",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY csCZRegistryEntries[] =
{
    {
        4,
        3,
        " Instalace Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Instalace aktualizuje nastaven� syst�mu.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Vytv���m registry...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

MUI_ERROR csCZErrorEntries[] =
{
    {
        //ERROR_NOT_INSTALLED
        "Odyssey nen� ve va�em po��ta�i kompletn� nainstalov�n.\n"
        "Pokud nyn� instalaci ukon��te, budete ji muset pro\n"
        "nainstalov�n� Odyssey spustit znovu.\n"
        "\n"
        "  \x07  Stisknut�m kl�vesy ENTER budete pokra�ovat v instalaci.\n"
        "  \x07  Stisknut�m F3 ukon��te instalaci.",
        "F3 = Ukon�it  ENTER = Pokra�ovat"
    },
    {
        //ERROR_NO_HDD
        "Instalace nedok�zala naj�t harddisk.\n",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_NO_SOURCE_DRIVE
        "Instalace nedok�zala naj�t svou zdrojovou mechaniku.\n",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_LOAD_TXTSETUPSIF
        "Nepoda�ilo se na��st soubor TXTSETUP.SIF.\n",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_CORRUPT_TXTSETUPSIF
        "Soubor TXTSETUP.SIF je po�kozen.\n",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_SIGNATURE_TXTSETUPSIF,
        "Soubor TXTSETUP.SIF je neplatn� podepsan�.\n",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_DRIVE_INFORMATION
        "Instalace nedok�zala z�skat informace o syst�mov�ch disc�ch.\n",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_WRITE_BOOT,
        "Nepoda�ilo se nainstalovat FAT zavad؟ na syst�mov� odd�l.",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_LOAD_COMPUTER,
        "Nepoda�ilo se na��st seznam typ� po��ta�e.\n",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_LOAD_DISPLAY,
        "Nepoda�ilo se na��st seznam nastaven� obrazovek.\n",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_LOAD_KEYBOARD,
        "Nepoda�ilo se na��st seznam typ� kl�vesnic.\n",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_LOAD_KBLAYOUT,
        "Nepoda�ilo se na��st seznam rozlo�en� kl�ves.\n",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_WARN_PARTITION,
          "Instalace zjistila, �e alespo� jeden pevn� disk obsahuje\n"
          "nekompatibiln� tabulku odd�l�, kter� nem��e b�t spr�vn� zpracov�na!\n"
          "\n"
          "Vytv��en� nebo odstra�ov�n� odd�l� m��e tuto tabulku odd�l� zni�it.\n"
          "\n"
          "  \x07  Stisknut�m F3 ukon��te instalaci.\n"
          "  \x07  Stisknut�m ENTER budete pokra�ovat v instalaci.",
          "F3= Ukon�it  ENTER = Pokra�ovat"
    },
    {
        //ERROR_NEW_PARTITION,
        "Nelze vytvo�it nov� odd�l uvnit� ji�\n"
        "existuj�c�ho odd�lu!\n"
        "\n"
        "  * Pokra�ujte stisknut�m libovoln� kl�vesy.",
        NULL
    },
    {
        //ERROR_DELETE_SPACE,
        "Nelze vymazat m�sto na disku, kter� nepat�� ��dn�mu odd�lu!\n"
        "\n"
        "  * Pokra�ujte stisknut�m libovoln� kl�vesy.",
        NULL
    },
    {
        //ERROR_INSTALL_BOOTCODE,
        "Nepoda�ilo se nainstalovat FAT zavad؟ na syst�mov� odd�l.",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_NO_FLOPPY,
        "V jednotce A: nen� disketa.",
        "ENTER = Pokra�ovat"
    },
    {
        //ERROR_UPDATE_KBSETTINGS,
        "Nepoda�ilo se aktualizovat nastaven� rozlo�en� kl�ves.",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_UPDATE_DISPLAY_SETTINGS,
        "Nepoda�ilo se aktualizovat nastaven� zobrazen� registru.", //display registry settings
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_IMPORT_HIVE,
        "Nepoda�ilo se naimportovat soubor registru.",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_FIND_REGISTRY
        "Nepoda�ilo se nal�zt datov� soubory registru.",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_CREATE_HIVE,
        "Nepoda�ilo se zalo�it registr.",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_INITIALIZE_REGISTRY,
        "Nepoda�ilo se inicializovat registry.",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_INVALID_CABINET_INF,
        "V archivu nen� platn� soubor inf.\n",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_CABINET_MISSING,
        "Archiv nebyl nalezen.\n",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_CABINET_SCRIPT,
        "Archiv neobsahuje instala�n� skript.\n",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_COPY_QUEUE,
        "Nepoda�ilo se otev��t frontu kop�rov�n�.\n",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_CREATE_DIR,
        "Nepoda�ilo se vytvo�it instala�n� adres��e.",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_TXTSETUP_SECTION,
        "Nepoda�ilo se nal�zt sekci 'Directories' v souboru\n"
        "TXTSETUP.SIF.\n",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_CABINET_SECTION,
        "Nepoda�ilo se nal�zt sekci 'Directories' v archivu.\n"
        "\n",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_CREATE_INSTALL_DIR
        "Nepoda�ilo se vytvo�it instala�n� adres��.",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_FIND_SETUPDATA,
        "Nepoda�ilo se nal�zt sekci 'SetupData' v souboru\n"
        "TXTSETUP.SIF.\n",
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_WRITE_PTABLE,
        "Nepoda�ilo se zapsat tabulky odd�l�.\n"
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_ADDING_CODEPAGE,
        "Nepoda�ilo se p�idat k�dovou str�nku do registru.\n"
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_UPDATE_LOCALESETTINGS,
        "Nepoda�ilo se nastavit m�stn� nastaven�.\n"
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_ADDING_KBLAYOUTS,
        "Nepoda�ilo se p�idat rozlo�en� kl�vesnice do registru.\n"
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_UPDATE_GEOID,
        "Nepoda�ilo se nastavit geo id.\n"
        "ENTER = Restartovat po��ta�"
    },
    {
        //ERROR_INSUFFICIENT_DISKSPACE,
        "Na zvolen�m odd�lu nen� dost voln�ho m�sta.\n"
        "  * Pokra�ujte stisknut�m libovoln� kl�vesy.",
        NULL
    },
    {
        NULL,
        NULL
    }
};


MUI_PAGE csCZPages[] =
{
    {
        LANGUAGE_PAGE,
        csCZLanguagePageEntries
    },
    {
        START_PAGE,
        csCZWelcomePageEntries
    },
    {
        INSTALL_INTRO_PAGE,
        csCZIntroPageEntries
    },
    {
        LICENSE_PAGE,
        csCZLicensePageEntries
    },
    {
        DEVICE_SETTINGS_PAGE,
        csCZDevicePageEntries
    },
    {
        REPAIR_INTRO_PAGE,
        csCZRepairPageEntries
    },
    {
        COMPUTER_SETTINGS_PAGE,
        csCZComputerPageEntries
    },
    {
        DISPLAY_SETTINGS_PAGE,
        csCZDisplayPageEntries
    },
    {
        FLUSH_PAGE,
        csCZFlushPageEntries
    },
    {
        SELECT_PARTITION_PAGE,
        csCZSelectPartitionEntries
    },
    {
        SELECT_FILE_SYSTEM_PAGE,
        csCZSelectFSEntries
    },
    {
        FORMAT_PARTITION_PAGE,
        csCZFormatPartitionEntries
    },
    {
        DELETE_PARTITION_PAGE,
        csCZDeletePartitionEntries
    },
    {
        INSTALL_DIRECTORY_PAGE,
        csCZInstallDirectoryEntries
    },
    {
        PREPARE_COPY_PAGE,
        csCZPrepareCopyEntries
    },
    {
        FILE_COPY_PAGE,
        csCZFileCopyEntries
    },
    {
        KEYBOARD_SETTINGS_PAGE,
        csCZKeyboardSettingsEntries
    },
    {
        BOOT_LOADER_PAGE,
        csCZBootLoaderEntries
    },
    {
        LAYOUT_SETTINGS_PAGE,
        csCZLayoutSettingsEntries
    },
    {
        QUIT_PAGE,
        csCZQuitPageEntries
    },
    {
        SUCCESS_PAGE,
        csCZSuccessPageEntries
    },
    {
        BOOT_LOADER_FLOPPY_PAGE,
        csCZBootPageEntries
    },
    {
        REGISTRY_PAGE,
        csCZRegistryEntries
    },
    {
        -1,
        NULL
    }
};

MUI_STRING csCZStrings[] =
{
    {STRING_PLEASEWAIT,
     "   �ekejte, pros�m..."},
    {STRING_INSTALLCREATEPARTITION,
     "   ENTER = Instalovat   C = Vytvo�it odd�l    F3 = Ukon�it"},
    {STRING_INSTALLDELETEPARTITION,
     "   ENTER = Instalovat   D = Odstranit odd�l   F3 = Ukon�it"},
    {STRING_PARTITIONSIZE,
     "Velikost nov�ho odd�lu:"},
    {STRING_CHOOSENEWPARTITION,
     "Zvolili jste vytvo�en� nov�ho odd�lu na"},
    {STRING_HDDSIZE,
    "Zadejte velikost nov�ho odd�lu v megabajtech."},
    {STRING_CREATEPARTITION,
     "   ENTER = Vytvo�it odd�l   ESC = Zru�it   F3 = Ukon�it"},
    {STRING_PARTFORMAT,
    "Tento odd�l bude zform�tov�n."},
    {STRING_NONFORMATTEDPART,
    "Zvolili jste instalaci Odyssey na nov� nebo nezform�tovan� odd�l."},
    {STRING_INSTALLONPART,
    "Instalace nakop�ruje Odyssey na odd�l"},
    {STRING_CHECKINGPART,
    "Instalace nyn� kontroluje zvolen� odd�l."},
    {STRING_QUITCONTINUE,
    "F3= Ukon�it  ENTER = Pokra�ovat"},
    {STRING_REBOOTCOMPUTER,
    "ENTER = Restartovat po��ta�"},
    {STRING_TXTSETUPFAILED,
    "Nepoda�ilo se naj�t sekci '%S' v souboru\n TXTSETUP.SIF.\n"},
    {STRING_COPYING,
     "   Kop�ruji soubor: %S"},
    {STRING_SETUPCOPYINGFILES,
     "Instalace kop�ruje soubory..."},
    {STRING_REGHIVEUPDATE,
    "   Aktualizuji registr..."},
    {STRING_IMPORTFILE,
    "   Importuji %S..."},
    {STRING_DISPLAYETTINGSUPDATE,
    "   Aktualizuji nastaven� zobrazen� registru..."}, //display registry settings
    {STRING_LOCALESETTINGSUPDATE,
    "   Aktualizuji m�stn� nastaven�..."},
    {STRING_KEYBOARDSETTINGSUPDATE,
    "   Aktualizuji nastaven� rozlo�en� kl�ves..."},
    {STRING_CODEPAGEINFOUPDATE,
    "   P�id�v�m do registru informaci o znakov� str�nce..."},
    {STRING_DONE,
    "   Hotovo..."},
    {STRING_REBOOTCOMPUTER2,
    "   ENTER = Restartovat po��ta�"},
    {STRING_CONSOLEFAIL1,
    "Nelze otev��t konzoli\n\n"},
    {STRING_CONSOLEFAIL2,
    "Nejbاn�j� p���inou je pou��v�n� USB kl�vesnice\n"},
    {STRING_CONSOLEFAIL3,
    "USB kl�vesnice zat�m nejsou pln� podporov�ny\n"},
    {STRING_FORMATTINGDISK,
    "Instalace form�tuje disk"},
    {STRING_CHECKINGDISK,
    "Instalace kontroluje disk"},
    {STRING_FORMATDISK1,
    " Zform�tovat odd�l na souborov� syst�m %S (rychle) "},
    {STRING_FORMATDISK2,
    " Zform�tovat odd�l na souborov� syst�m %S "},
    {STRING_KEEPFORMAT,
    " Ponechat sou�asn� souborov� syst�m (bez zm�ny) "},
    {STRING_HDINFOPARTCREATE,
    "%I64u %s  Harddisk %lu  (Port=%hu, Bus=%hu, Id=%hu) na %wZ."},
    {STRING_HDDINFOUNK1,
    "%I64u %s  Harddisk %lu  (Port=%hu, Bus=%hu, Id=%hu)."},
    {STRING_HDDINFOUNK2,
    "   %c%c  Typ %lu    %I64u %s"},
    {STRING_HDINFOPARTDELETE,
    "na %I64u %s  Harddisk %lu  (Port=%hu, Bus=%hu, Id=%hu) na %wZ."},
    {STRING_HDDINFOUNK3,
    "na %I64u %s  Harddisk %lu  (Port=%hu, Bus=%hu, Id=%hu)."},
    {STRING_HDINFOPARTZEROED,
    "Harddisk %lu (%I64u %s), Port=%hu, Bus=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK4,
    "%c%c  Typ %lu    %I64u %s"},
    {STRING_HDINFOPARTEXISTS,
    "na harddisku %lu (%I64u %s), Port=%hu, Bus=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK5,
    "%c%c  Typ %-3u                        %6lu %s"},
    {STRING_HDINFOPARTSELECT,
    "%6lu %s  Harddisk %lu  (Port=%hu, Bus=%hu, Id=%hu) na %S"},
    {STRING_HDDINFOUNK6,
    "%6lu %s  Harddisk %lu  (Port=%hu, Bus=%hu, Id=%hu)"},
    {STRING_NEWPARTITION,
    "Instalace vytvo�ila nov� odd�l na"},
    {STRING_UNPSPACE,
    "    M�sto bez odd�l�                 %6lu %s"},
    {STRING_MAXSIZE,
    "MB (max. %lu MB)"},
    {STRING_UNFORMATTED,
    "Nov� (Nenaform�tovan�)"},
    {STRING_FORMATUNUSED,
    "Nepou�it�"},
    {STRING_FORMATUNKNOWN,
    "Nezn�m�"},
    {STRING_KB,
    "KB"},
    {STRING_MB,
    "MB"},
    {STRING_GB,
    "GB"},
    {STRING_ADDKBLAYOUTS,
    "P�id�v�m rozlo�en� kl�ves"},
    {0, 0}
};
