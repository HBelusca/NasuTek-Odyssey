/*
 * PROJECT:         Odyssey Setup
 * FILE:            \base\setup\usetup\lang\sv-SE.h  
 * PURPOSE:         Swedish resource file
 * Translation:     Jaix Bly plus perhaps GreatLord if blame and translate.odyssey.se is consulted.
 */
#pragma once

MUI_LAYOUTS svSELayouts[] =
{
    { L"041D", L"0000041D" },
    { L"0409", L"00000409" },
    { NULL, NULL }
};

static MUI_ENTRY svSELanguagePageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Language Selection.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Please choose the language used for the installation process.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Then Tryck ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  This Language will be the default language for the final system.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = Forts�tt  F3 = Avsluta",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY svSEWelcomePageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "V�lkommen till Odyssey Setup!",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        6,
        11,
        "Denna del av installationen kopierar Odyssey till eran",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "dator och f�rbereder den andra delen av installationen.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Tryck p� ENTER f�r att installera Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Tryck p� R f�r att reparera Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  Tryck p� L f�r att l�sa licensavtalet till Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Tryck p� F3 f�r att avbryta installationen av Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "F�r mer information om Odyssey, bes�k:",
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
        "   ENTER = Forts�tt  R = Reparera F3 = Avbryt",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY svSEIntroPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Odyssey Setup �r i en tidig utvecklingsfas och saknar d�rf�r ett antal",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "funktioner som kan f�rv�ntas av ett fullt anv�ndbart setup-program.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "F�ljande begr�nsningar g�ller:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "- Setup kan ej hantera mer �n 1 prim�r partition per h�rddisk.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "- Setup kan ej radera en prim�r partition fr�n en h�rddisk",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "  om ut�kade partitioner existerar p� h�rddisken.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "- Setup kan ej radera den f�rsta ut�kade partitionen fr�n en h�rddisk",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "  om andra ut�kade partitioner existerar p� h�rddisken.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "- Setup st�der endast filsystem av typen FAT.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "- Kontrollering av h�rddiskens filsystem st�ds (�nnu) ej.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        23,
        "\x07  Tryck p� ENTER f�r att installera Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        25,
        "\x07  Tryck p� F3 f�r att avbryta installationen.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = Forts�tt   F3 = Avbryt",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY svSELicensePageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        6,
        "Licensering:",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        8,
        "Odyssey �r licenserad under GNU GPL med delar",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        9,
        "av den medf�ljande koden licenserad under GPL-f�renliga",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "licenser s�som X11-, BSD- och GNU LGPL-licenserna.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "All mjukvara som �r del av Odyssey �r publicerad",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "under GNU GPL, men �ven den ursprungliga",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "licensen �r uppr�tth�llen.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "Denna mjukvara har INGEN GARANTI eller begr�nsing p� anv�ndning",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "bortsett fr�n till�mplig lokal och internationell lag. Licenseringen av",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "Odyssey t�cker endast distrubering till tredje part.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "Om Ni av n�gon anledning ej f�tt en kopia av",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "GNU General Public License med Odyssey, bes�k",
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
        "Garanti:",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        24,
        "Detta �r gratis mjukvara; se k�llkoden f�r restriktioner ang�ende kopiering.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        25,
        "INGEN GARANTI ges; inte ens f�r S�LJBARHET eller PASSANDE F�R ETT",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        26,
        "SPECIELLT SYFTE. ALL ANV�NDNING SKER P� EGEN RISK!",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �terv�nd",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY svSEDevicePageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Listan nedanf�r visar inst�llningarna f�r maskinvaran.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "       Dator:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "        Bildsk�rm:",
        TEXT_STYLE_NORMAL,
    },
    {
        8,
        13,
        "       Tangentbord:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "Tangentbordslayout:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "         Acceptera:",
        TEXT_STYLE_NORMAL
    },
    {
        25,
        16, "Acceptera dessa maskinvaruinst�llningar",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        19,
        "�ndra inst�llningarna genom att trycka p� UPP- och NED-piltangenterna",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        20,
        "f�r att markera en inst�llning, och tryck p� ENTER f�r att v�lja",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        21,
        "inst�llningen.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "N�r alla inst�llningar �r korrekta, v�lj \"Acceptera dessa maskinvaruinst�llningar\"",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        24,
        "och tryck p� ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = Forts�tt   F3 = Avbryt",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY svSERepairPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Odyssey Setup �r i en tidig utvecklingsfas och saknar d�rf�r ett antal",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "funktioner som kan f�rv�ntas av ett fullt anv�ndbart setup-program.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "Reparations- och uppdateringsfunktionerna fungerar ej.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Tryck p� U f�r att uppdatera Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Tryck p� R f�r �terst�llningskonsolen.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  Tryck p� ESC f�r att �terv�nda till f�reg�ende sida.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Tryck p� ENTER f�r att starta om datorn.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ESC = G� till f�reg�ende sida  ENTER = Starta om datorn",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};
static MUI_ENTRY svSEComputerPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�ndra vilken typ av dator som ska installeras.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Anv�nd UPP- och NED-piltangenterna f�r att v�lja �nskad datortyp.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Tryck sen p� ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Tryck p� ESC f�r att �terv�nda till den f�reg�ende sidan utan",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   att �ndra datortypen.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = Forts�tt   ESC = �terv�nd   F3 = Avbryt",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY svSEFlushPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "Datorn f�rs�krar sig om att all data �r lagrad p� h�rdisken.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Detta kommer att ta en stund.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "N�r detta �r f�rdigt kommer datorn att startas om automatiskt.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   Rensar cachen",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY svSEQuitPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "Installationen av Odyssey har inte slutf�rts.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Se till att ingen floppy-disk finns i floppy-l�sare A:",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "och tag ur alla skivor fr�n CD/DVD-l�sarna.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "Tryck p� ENTER f�r att starta om datorn.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   Var god v�nta ...",
        TEXT_TYPE_STATUS,
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY svSEDisplayPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�ndra vilken typ av bildsk�rmsinst�llning som ska installeras.",
        TEXT_STYLE_NORMAL
    },
    {   8,
        10,
         "\x07  Anv�nd UPP- och NED-piltangenterna f�r att v�lja �nskad inst�llning.",
         TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Tryck sedan p� ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Tryck p� ESC f�r att �terv�nda till den f�reg�ende sidan utan",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   att �ndra bildsk�rmsinst�llningen.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = Forts�tt   ESC = �terv�nd   F3 = Avbryt",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY svSESuccessPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "Odyssey har nu installerats p� datorn.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Se till att ingen floppy-disk finns i floppy-l�sare A:",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "och tag ur alla skivor fr�n CD/DVD-l�sarna.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "Tryck p� ENTER f�r att starta om datorn.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = Starta om datorn",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY svSEBootPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Setup misslyckades med att installera bootloadern p� datorns",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "h�rddisk",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        13,
        "Var god s�tt in en formatterad floppy-disk i l�sare A: och",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "tryck p� ENTER.",
        TEXT_STYLE_NORMAL,
    },
    {
        0,
        0,
        "   ENTER = Forts�tt   F3 = Avbryt",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }

};

static MUI_ENTRY svSESelectPartitionEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Lista nedan visar befintliga partitioner och oanv�ndt",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "diskutrymme f�r nya partitioner.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "\x07  Tryck UPP eller NER tangenten f�r att v�lja i listan.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Tryck ENTER f�r att installerara Odyssey till vald partition.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Tryck C f�r att skapa en ny partition.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Tryck D f�r att ta bort en befintlig partititon.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   Var V�nlig V�nta...",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY svSEFormatPartitionEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Formatera partition",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        10,
        "Partitionen kommer nu att formaters Tryck ENTER f�r att forts�tta.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = Forts�tt   F3 = Avsluta",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        TEXT_STYLE_NORMAL
    }
};

static MUI_ENTRY svSEInstallDirectoryEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Odyssey installeras till vald partition. V�lj en",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "mapp som du vill installera Odyssey till.:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "F�r att �ndra den f�reslagna mappen, tryck BACKSTEG f�r att radera",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        15,
        "bokst�ver och skriv sedan in mappen dit du vill att Odyssey ska bli",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        16,
        "installerad.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = Forts�tt   F3 = Avsluta",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY svSEFileCopyEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        11,
        12,
        "Var v�nlig v�nta medans Odyssey Setup kopieras till din Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        30,
        13,
        "installationsmapp.",
        TEXT_STYLE_NORMAL
    },
    {
        20,
        14,
        "Detta kan ta flera minuter att fullf�lja.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "                                                           \xB3 Var V�nlig V�nta...    ",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY svSEBootLoaderEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Setup installerar boot-loadern",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "Installera bootloadern till harddisken (MBR and VBR).",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "Installera bootloadern till h�rddisken (VBR only).",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "Installera bootloadern till en diskett.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "Skippa installation av bootloader.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = Forts�tt   F3 = Avsluta",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY svSEKeyboardSettingsEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Du vill �ndra tangentbordstyp som ska intealleras.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Tryck UP eller NER tangenten f�r att v�lja �nskat tangentbordstyp.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Tryck sedan ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Tryck ESC tangenten f�r att �terg� till f�rra sidan utan att �ndra n�got.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   tangentbordstyp.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = Forts�tt   ESC = Avbryt   F3 = Avsluta",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY svSELayoutSettingsEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Var v�nlig och v�lj layout du vill installera som standard.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Tryck UPP eller NER tangenten f�r att v�lja �nskad",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "    tangentbordslayout. Tryck sedan ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Tryck ESC tangenten f�r att �terg� till f�rra sidan utan att �ndra",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   tangentbordslayout.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = Forts�tt   ESC = Avbryt   F3 = Avsluta",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY svSEPrepareCopyEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Setup f�rbereder din dator f�r kopiering av Odyssey filer. ",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   Sammanst�ller filkopieringslistan...",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY svSESelectFSEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        17,
        "V�lj ett filsystem i listan nedan.",
        0
    },
    {
        8,
        19,
        "\x07  Tryck UPP or NER tangenten f�r att v�lja filsystem.",
        0
    },
    {
        8,
        21,
        "\x07  Tryck ENTER f�r att formatera partitionen.",
        0
    },
    {
        8,
        23,
        "\x07  Tryck ESC f�r att v�lja en annan partition.",
        0
    },
    {
        0,
        0,
        "   ENTER = Forts�tt   ESC = Avbryt   F3 = Avsluta",
        TEXT_TYPE_STATUS
    },

    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY svSEDeletePartitionEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Du har valt att ta bort partitionen",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "\x07  Tryck D f�r att ta bort partitionen.",
        TEXT_STYLE_NORMAL
    },
    {
        11,
        19,
        "VARNING: Alla data p� denna partition kommer att f�rloras!",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Tryck ESC f�r att avbryta.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   D = Tar bort Partitionen   ESC = Avbryt   F3 = Avsluta",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY svSERegistryEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Setup uppdaterar systemkonfigurationen. ",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   Skapar regististerdatafiler...",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    },

};

MUI_ERROR svSEErrorEntries[] =
{
    {
        //ERROR_NOT_INSTALLED
        "Odyssey installerades inte fullst�ndigt p� din\n"
        "dator. Om du avslutar Setup nu, kommer du att beh�va\n"
        "k�ra Setup igen f�r att installera Odyssey.\n"
        "\n"
        "  \x07  Tryck ENTER f�r att forts�tta Setup.\n"
        "  \x07  Tryck F3 f�r att avsluta Setup.",
        "F3= Avsluta  ENTER = Forts�tta"
    },
    {
        //ERROR_NO_HDD
        "Setup kunde inte hitta n�gon h�rddisk.\n",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_NO_SOURCE_DRIVE
        "Setup kunde inte hitta sin k�lldisk.\n",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_LOAD_TXTSETUPSIF
        "Setup misslyckades att l�sa in filen TXTSETUP.SIF.\n",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_CORRUPT_TXTSETUPSIF
        "Setup fann en korrupt TXTSETUP.SIF.\n",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_SIGNATURE_TXTSETUPSIF,
        "Setup hittade en ogiltig signatur i TXTSETUP.SIF.\n",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_DRIVE_INFORMATION
        "Setup kunde inte l�sa in informationen om systemenheten.\n",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_WRITE_BOOT,
        "Setup misslyckades installera FAT bootkod p� systempartitionen.",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_LOAD_COMPUTER,
        "Setup misslyckades att l�sa datortypslistan.\n",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_LOAD_DISPLAY,
        "Setup misslyckades att l�sa in sk�rminst�llningslistan.\n",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_LOAD_KEYBOARD,
        "Setup misslyckades att l�sa in tangentbordstypslistan.\n",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_LOAD_KBLAYOUT,
        "Setup misslyckades att l�sa in tangentbordslayoutslistan.\n",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_WARN_PARTITION,
        "Setup fann att minst en h�rdisk inneh�ller en partitionstabell\n"
        "inte �r kompatibel och inte kan hanteras korrekt!\n"
        "\n"
        "Skapa eller ta bort partitioner kan f�rst�ra partitionstabellen.\n"
        "\n"
        "  \x07  Tryck F3 f�r att avsluta Setup."
        "  \x07  Tryck ENTER f�r att forts�tta.",
        "F3= Avsluta  ENTER = Forts�tt"
    },
    {
        //ERROR_NEW_PARTITION,
        "Du kan inte skapa en partition inuti\n"
        "en redat befintlig partition!\n"
        "\n"
        "  * Tryck valfri tangent f�r att forts�tta.",
        NULL
    },
    {
        //ERROR_DELETE_SPACE,
        "Du kan inte ta bort opartitionerrat diskutrymme!\n"
        "\n"
        "  * Tryck valfri tangent f�r att forts�tta.",
        NULL
    },
    {
        //ERROR_INSTALL_BOOTCODE,
        "Setup misslyckades att installera FAT bootkoden p� systempartitionen.",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_NO_FLOPPY,
        "Ingen disk i enhet A:.",
        "ENTER = Forts�tt"
    },
    {
        //ERROR_UPDATE_KBSETTINGS,
        "Setup misslyckades att updatera inst�llninarna f�r tangentbordslayout.",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_UPDATE_DISPLAY_SETTINGS,
        "Setup misslyckades att uppdatera sk�rmregisterinst�llningen.",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_IMPORT_HIVE,
        "Setup misslyckades att improterea en registerdatafil.",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_FIND_REGISTRY
        "Setup misslyckades att hitta registerdatafilerna.",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_CREATE_HIVE,
        "Setup misslyckades att skapa registerdatafilerna.",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_INITIALIZE_REGISTRY,
        "Setup misslyckades att initialisera registret.",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_INVALID_CABINET_INF,
        "Kabinettet has inen giltig inf fil.\n",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_CABINET_MISSING,
        "Kabinettet hittades inte.\n",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_CABINET_SCRIPT,
        "Kabinettet har inget installationsskript.\n",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_COPY_QUEUE,
        "Setup misslyckades att �ppna filkopierningsk�n.\n",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_CREATE_DIR,
        "Setup kunnde inte skapa installationsmapparna.",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_TXTSETUP_SECTION,
        "Setup misslyckades att hitta 'Directories' sektionen\n"
        "i TXTSETUP.SIF.\n",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_CABINET_SECTION,
        "Setup misslyckades att hitta 'Directories' sektionen\n"
        "i kabinettet.\n",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_CREATE_INSTALL_DIR
        "Setup kunnde inte skapa installationsmappen.",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_FIND_SETUPDATA,
        "Setup misslyckades att hitta 'SetupData' sektionen\n"
        "i TXTSETUP.SIF.\n",
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_WRITE_PTABLE,
        "Setup misslyckades att skriva partitionstabellen.\n"
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_ADDING_CODEPAGE,
        "Setup misslyckades att l�gga till vald codepage till registret.\n"
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_UPDATE_LOCALESETTINGS,
        "Setup kunnde inte st�lla in 'system locale'.\n"
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_ADDING_KBLAYOUTS,
        "Setup misslyckades att l�gga till tangentbordslayouten till registret.\n"
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_UPDATE_GEOID,
        "Setup kunde inte stalla int 'geo id'.\n"
        "ENTER = Starta om datorn"
    },
    {
        //ERROR_INSUFFICIENT_DISKSPACE,
        "Inte tillr�ckligt mycket fritt utrymme p� den valda partitionen.\n"
        "  * Tryck valfri tangent f�r att forts�tta.",
        NULL
    },
    {
        NULL,
        NULL
    }
};

MUI_PAGE svSEPages[] =
{
    {
        LANGUAGE_PAGE,
        svSELanguagePageEntries
    },
    {
       START_PAGE,
       svSEWelcomePageEntries
    },
    {
        INSTALL_INTRO_PAGE,
        svSEIntroPageEntries
    },
    {
        LICENSE_PAGE,
        svSELicensePageEntries
    },
    {
        DEVICE_SETTINGS_PAGE,
        svSEDevicePageEntries
    },
    {
        REPAIR_INTRO_PAGE,
        svSERepairPageEntries
    },
    {
        COMPUTER_SETTINGS_PAGE,
        svSEComputerPageEntries
    },
    {
        DISPLAY_SETTINGS_PAGE,
        svSEDisplayPageEntries
    },
    {
        FLUSH_PAGE,
        svSEFlushPageEntries
    },
    {
        SELECT_PARTITION_PAGE,
        svSESelectPartitionEntries
    },
    {
        SELECT_FILE_SYSTEM_PAGE,
        svSESelectFSEntries
    },
    {
        FORMAT_PARTITION_PAGE,
        svSEFormatPartitionEntries
    },
    {
        DELETE_PARTITION_PAGE,
        svSEDeletePartitionEntries
    },
    {
        INSTALL_DIRECTORY_PAGE,
        svSEInstallDirectoryEntries
    },
    {
        PREPARE_COPY_PAGE,
        svSEPrepareCopyEntries
    },
    {
        FILE_COPY_PAGE,
        svSEFileCopyEntries
    },
    {
        KEYBOARD_SETTINGS_PAGE,
        svSEKeyboardSettingsEntries
    },
    {
        BOOT_LOADER_PAGE,
        svSEBootLoaderEntries
    },
    {
        LAYOUT_SETTINGS_PAGE,
        svSELayoutSettingsEntries
    },
    {
        QUIT_PAGE,
        svSEQuitPageEntries
    },
    {
        SUCCESS_PAGE,
        svSESuccessPageEntries
    },
    {
        BOOT_LOADER_FLOPPY_PAGE,
        svSEBootPageEntries
    },
    {
        REGISTRY_PAGE,
        svSERegistryEntries
    },
    {
        -1,
        NULL
    }
};

MUI_STRING svSEStrings[] =
{
    {STRING_PLEASEWAIT,
     "   Var v�nlig v�nta..."},
    {STRING_INSTALLCREATEPARTITION,
     "   ENTER = Installera   C = Skapa Partition   F3 = Avsluta"},
    {STRING_INSTALLDELETEPARTITION,
     "   ENTER = Installera   D = Ta bort Partition   F3 = Avsluta"},
    {STRING_PARTITIONSIZE,
     "Storlek p� den nya partitionen:"},
    {STRING_CHOOSENEWPARTITION,
     "Du har valt att skapa en ny partiton p�"},
    {STRING_HDDSIZE,
    "V�nligen skriv in storleken av den nya partitionen i megabytes."},
    {STRING_CREATEPARTITION,
     "   ENTER = Skapa Partition   ESC = Avbryt   F3 = Avsluta"},
    {STRING_PARTFORMAT,
    "Denna Partition kommer att bli formaterad h�rn�st."},
    {STRING_NONFORMATTEDPART,
    "Du valde att installera Odyssey p� en oformaterad partition."},
    {STRING_INSTALLONPART,
    "Setup installerar Odyssey till Partitionen"},
    {STRING_CHECKINGPART,
    "Setup unders�ker nu den valda partitionen."},
    {STRING_QUITCONTINUE,
    "F3= Avsluta  ENTER = Forts�tt"},
    {STRING_REBOOTCOMPUTER,
    "ENTER = Starta om datorn"},
    {STRING_TXTSETUPFAILED,
    "Setup misslyckades att hitta '%S' sektionen\ni TXTSETUP.SIF.\n"},
    {STRING_COPYING,
     "   Kopierar fil: %S"},
    {STRING_SETUPCOPYINGFILES,
     "Setup kopierar filer..."},
    {STRING_REGHIVEUPDATE,
    "   Uppdaterar registerdatafiler..."},
    {STRING_IMPORTFILE,
    "   Importerar %S..."},
    {STRING_DISPLAYETTINGSUPDATE,
    "   Uppdaterar sk�rmregisterinst�llningar..."},
    {STRING_LOCALESETTINGSUPDATE,
    "   Uppdaterar lokala inst�llningar..."},
    {STRING_KEYBOARDSETTINGSUPDATE,
    "   Uppdaterar tangentbordslayoutinst�llningar..."},
    {STRING_CODEPAGEINFOUPDATE,
    "   L�gger till information om codepage till registret..."},
    {STRING_DONE,
    "   F�rdigt..."},
    {STRING_REBOOTCOMPUTER2,
    "   ENTER = Starta om datorn"},
    {STRING_CONSOLEFAIL1,
    "Det g�r inte �ppna Konsollen\n\n"},
    {STRING_CONSOLEFAIL2,
    "Den vanligaste orsaken till detta �r att ett USB tangentbord anv�nds\n"},
    {STRING_CONSOLEFAIL3,
    "USB tangentbord �r itne helt st�tt �n\n"},
    {STRING_FORMATTINGDISK,
    "Setup formaterar din disk"},
    {STRING_CHECKINGDISK,
    "Setup under�ker din disk"},
    {STRING_FORMATDISK1,
    " Formaterar partition som %S filsystem (snabbformatering) "},
    {STRING_FORMATDISK2,
    " Formaterar partition som %S filsystem "},
    {STRING_KEEPFORMAT,
    " Beh�ll nuvarande filsystem (inga f�r�ndringar) "},
    {STRING_HDINFOPARTCREATE,
    "%I64u %s  H�rddisk %lu  (Port=%hu, Bus=%hu, Id=%hu) p� %wZ."},
    {STRING_HDDINFOUNK1,
    "%I64u %s  H�rddisk %lu  (Port=%hu, Bus=%hu, Id=%hu)."},
    {STRING_HDDINFOUNK2,
    "   %c%c  Typ %lu    %I64u %s"},
    {STRING_HDINFOPARTDELETE,
    "on %I64u %s  H�rddisk %lu  (Port=%hu, Bus=%hu, Id=%hu) p� %wZ."},
    {STRING_HDDINFOUNK3,
    "on %I64u %s  H�rddisk %lu  (Port=%hu, Bus=%hu, Id=%hu)."},
    {STRING_HDINFOPARTZEROED,
    "H�rddisk %lu (%I64u %s), Port=%hu, Bus=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK4,
    "%c%c  Typ %lu    %I64u %s"},
    {STRING_HDINFOPARTEXISTS,
    "p� H�rddisk %lu (%I64u %s), Port=%hu, Bus=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK5,
    "%c%c  Typ %-3u                         %6lu %s"},
    {STRING_HDINFOPARTSELECT,
    "%6lu %s  H�rddisk %lu  (Port=%hu, Bus=%hu, Id=%hu) p� %S"},
    {STRING_HDDINFOUNK6,
    "%6lu %s  H�rddisk %lu  (Port=%hu, Bus=%hu, Id=%hu)"},
    {STRING_NEWPARTITION,
    "Setup skapade en ny partition p�"},
    {STRING_UNPSPACE,
    "    Opartitionerat utrymme              %6lu %s"},
    {STRING_MAXSIZE,
    "MB (max. %lu MB)"},
    {STRING_UNFORMATTED,
    "Ny (Oformaterad)"},
    {STRING_FORMATUNUSED,
    "Oanv�nt"},
    {STRING_FORMATUNKNOWN,
    "Ok�nd"},
    {STRING_KB,
    "KB"},
    {STRING_MB,
    "MB"},
    {STRING_GB,
    "GB"},
    {STRING_ADDKBLAYOUTS,
    "L�gger till tangentbordslayouter"},
    {0, 0}
};
