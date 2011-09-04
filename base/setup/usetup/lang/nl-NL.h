#pragma once

MUI_LAYOUTS nlNLLayouts[] =
{
    { L"0413", L"00020409" },
    { L"0413", L"00000413" },
    { L"0409", L"00000409" },
    { NULL, NULL }
};

static MUI_ENTRY nlNLLanguagePageEntries[] =
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
        "Taalkeuze",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Kies de taal die U voor de installatie wilt gebruiken.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Druk daarna op ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,    
        "\x07  Deze taal wordt later als standaard taal door het systeem gebruikt.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER=Doorgaan  F3=Afsluiten",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY nlNLWelcomePageEntries[] =
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
        "Welkom bij Odyssey Setup",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        6,
        11,
        "Dit gedeelte van Setup kopieeert het Odyssey besturingssysteem naar",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "uw computer en bereidt het tweede deel van setup voor.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Druk op ENTER om Odyssey te installeren.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Druk op R om Odyssey te repareren of bij te werken.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  Druk op L om de Odyssey Licensieovereenkomst te bekijken.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Druk op F3 als u Setup wilt afsluiten zonder Reactos te",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        22,
        "   installeren.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        24,
        "Voor meer informatie over Odyssey bezoekt u:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        25,
        "http://www.odyssey.org",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        0,
        0,
		"ENTER=Doorgaan  R=Herstellen  L = Licensie  F3=Afsluiten",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY nlNLIntroPageEntries[] =
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
        "Odyssey Setup is nog in een vroege ontwikkelingsfase.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "Het ondersteunt nog niet alle functies van een volledig",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        10,
        "setup programma.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "De volgende beperkingen zijn van toepassing:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "- Setup kan niet meer dan 1 primaire partitie per harde schijf aan.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "- Setup kan geen primaire partitie van een schijf verwijderen",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "  zo lang er nog uitgebreide partities bestaan op deze schijf.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "- Setup kan de eerste uitgebreide partitie van een schijf niet",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "  verwijderen zolang er nog andere uitgebreide partities",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "  op deze schijf bestaan.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "- Setup ondersteunt alleen FAT bestandssystemen.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        20,
        "- Bestandssysteemcontrole is nog niet geimplementeerd.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        23,
        "\x07  Druk op ENTER om Odyssey te installeren.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        25,
        "\x07  Druk op F3 als u Setup wilt afsluiten zonder Reactos te",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        26,
        "   installeren.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Doorgaan   F3 = Afsluiten",
        TEXT_TYPE_STATUS| TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY nlNLLicensePageEntries[] =
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
        "Licensie:",
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
        "Warranty:",
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
        "ENTER = Ga terug",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY nlNLDevicePageEntries[] =
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
        "Onderstaande lijst bevat de huidige apparaatinstellingen.",
        TEXT_STYLE_NORMAL
    },
    {
        24,
        11,
        "Computer:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        12,
        "Beeldscherm:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        13,
        "Toetsenbord:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        14,
        "Toetsenbord indeling:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        16,
        "Accepteren:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        25,
        16, "Accepteren van apparaatinstellingen",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        19,
        "Selecteer een apparaatinstelling door middel van de pijltjestoetsen",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        20,
        "Druk daarna op de ENTER toets om de instelling aan te passen",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "Als alle instellingen juist zijn, selecteer ",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        24,
        "\"Accepteren van apparaatinstellingen\" en druk op ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Doorgaan   F3 = Afsluiten",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY nlNLRepairPageEntries[] =
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
        "Odyssey Setup is nog in een vroege ontwikkelingsfase",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "Het ondersteunt nog niet alle functies van een volledig",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        10,
        "setup programma.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "De herstel functies zijn nog niet geimplementeerd.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Druk op U om Reactos bij te werken.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Druk op R voor de Herstelconsole.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  Druk op ESC om terug naar het hoofdscherm te gaan.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Druk op ENTER om de computer opnieuw op te starten.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ESC = Hoofdscherm  U = Bijwerken  R = Herstelconsole  ENTER = Reboot",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};
static MUI_ENTRY nlNLComputerPageEntries[] =
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
        "U wilt het te installeren computer type instellen.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Selecteer het gewenste computer type met behulp van de",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   pijltjestoetsen en Druk daarna op ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Druk op de ESC toets om naar het vorige scherm te gaan",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   zonder het computer type te wijzigen.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Doorgaan   ESC = Annuleren   F3 = Afsluiten",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY nlNLFlushPageEntries[] =
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
        "Het systeem is alle gegegevens naar schijf aan het wegschrijven.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Dit kan enige minuten duren",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "Wanneer gereed, zal uw computer automatisch opnieuw opstarten",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Cache wordt geleegd",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY nlNLQuitPageEntries[] =
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
        "Odyssey is niet volledig geinstalleerd",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Verwijder de floppy disk uit Station A: en",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "alle CD-ROMs uit de CD-Stations.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "Druk op ENTER om uw computer opnieuw op te starten.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Een ogenblik geduld ...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG,
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY nlNLDisplayPageEntries[] =
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
        "U wilt het te installeren beeldscherm type instellen.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Selecteer het gewenste beeldscherm type met behulp van de",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   pijltjestoetsen en Druk daarna op ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Druk op de ESC toets om naar het vorige scherm te gaan",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   zonder het beeldscherm type te wijzigen.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Doorgaan   ESC = Annuleren   F3 = Afsluiten",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY nlNLSuccessPageEntries[] =
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
        "De basisonderdelen van Odyssey zijn succesvol geinstalleerd.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Verwijder de floppy disk uit Station A: en",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "alle CD-ROMs uit de CD-Stations.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "Druk op ENTER om uw computer opnieuw op te starten.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Opnieuw op starten",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY nlNLBootPageEntries[] =
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
        "Setup kan de bootloader niet op de harde schijf van uw computer",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "installeren",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        13,
        "Voer een geformatteerde floppy disk in Station A: en",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "Druk op ENTER.",
        TEXT_STYLE_NORMAL,
    },
    {
        0,
        0,
        "ENTER = Doorgaan   F3 = Afsluiten",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }

};

static MUI_ENTRY nlNLSelectPartitionEntries[] =
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
		"Onderstaande lijst bevat de huidige partities en ongebruikte",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "ruimte voor nieuwe partities.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "\x07  Selecteer een partitie door middel van de pijltjestoetsen.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Druk op ENTER om Odyssey te installeren op de geselecteerde partitie.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Druk op C om een nieuwe partitie aan te maken.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Druk op D om een bestaande partitie te verwijderen.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Een ogenblik geduld ...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY nlNLFormatPartitionEntries[] =
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
        "Partitie formatteren",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        10,
        "Setup gaat nu de partitie formatteren. Druk op ENTER om door te gaan.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Doorgaan   F3 = Afsluiten",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        TEXT_STYLE_NORMAL
    }
};

static MUI_ENTRY nlNLInstallDirectoryEntries[] =
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
        "Setup installeert Odyssey bestanden op de geselecteerde partitie.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "kies een directory waar Odyssey geinstalleerd moet worden:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
		"Om de voorgestelde directory te wijzigen: druk op BACKSPACE om",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        15,
        "karakters te verwijderen en voer dan de directory in waar u Odyssey",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        16,
        "wilt installeren.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Doorgaan   F3 = Afsluiten",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY nlNLFileCopyEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        0,
        12,
        "Wacht u terwijl Odyssey Setup bestanden kopieeert naar",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        13,
        "uw Odyssey installatie folder.",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        14,
        "Dit kan enkele minuten duren.",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        50,
        0,
        "\xB3 Een ogenblik geduld ...",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY nlNLBootLoaderEntries[] =
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
        "Setup installeert de bootloader",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "Install bootloader on the harddisk (MBR and VBR).",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "Install bootloader on the harddisk (VBR only).",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "Installeer de bootloader op een floppy disk.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "Installeren bootloader overslaan.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Doorgaan   F3 = Afsluiten",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY nlNLkeyboardSettingsEntries[] =
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
        "U wilt het te installeren toetsenbord type instellen.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Selecteer het gewenste toetsenbord type met behulp van de",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   pijltjestoetsen en druk daarna op ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Druk op de ESC toets om naar het vorige scherm te gaan",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   zonder het toetsenbord type te wijzigen.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Doorgaan   ESC = Annuleren   F3 = Afsluiten",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY nlNLLayoutSettingsEntries[] =
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
        "U wilt de te installeren standaard indeling instellen.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Selecteer de gewenste standaard indeling met behulp van de",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   pijltjestoetsen en Druk daarna op ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Druk op de ESC toets om naar het vorige scherm te gaan",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   zonder de standaard indeling te wijzigen.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Doorgaan   ESC = Annuleren   F3 = Afsluiten",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY nlNLPrepareCopyEntries[] =
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
        "Setup bereidt uw computer voor op het kopieeren van Odyssey bestanden.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Aanmaken van de lijst van te kopieeren bestanden...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY nlNLSelectFSEntries[] =
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
        "Selecteer een bestandssysteem uit onderstaande lijst.",
        0
    },
    {
        8,
        19,
        "\x07  Gebruik de pijltjestoetsen om een bestandssysteem te selecteren.",
        0
    },
    {
        8,
        21,
        "\x07  Druk op ENTER om de partitie te formatteren.",
        0
    },
    {
        8,
        23,
        "\x07  Druk op ESC om een andere partitie te selecteren.",
        0
    },
    {
        0,
        0,
        "ENTER = Doorgaan   ESC = Annuleren   F3 = Afsluiten",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },

    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY nlNLDeletePartitionEntries[] =
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
        "U wilt de partitie verwijderen",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "\x07  Druk op D om de partitie te verwijderen.",
        TEXT_STYLE_NORMAL
    },
    {
        11,
        19,
        "WAARSCHUWING: Alle gegevens op deze partitie worden gewist!",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Druk op ESC om te annuleren.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "D = Delete Partitie   ESC = Annuleren   F3 = Afsluiten",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY nlNLRegistryEntries[] =
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
        "Setup is de systeemconfiguratie aan het bijwerken. ",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Registry hives aanmaken...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

MUI_ERROR nlNLErrorEntries[] =
{
    {
        //ERROR_NOT_INSTALLED
        "Odyssey is niet geheel geinstalleerd op uw\n"
        "computer. Als u Setup nu afsluit moet u\n"
        "Setup opnieuw starten om Odyssey te installeren.\n"
        "\n"
        "  \x07  Druk op ENTER om door te gaan met Setup.\n"
        "  \x07  Druk op F3 om Setup af te sluiten.",
        "F3 = Afsluiten  ENTER = Doorgaan"
    },
    {
        //ERROR_NO_HDD
        "Setup kan geen harde schijf vinden.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_NO_SOURCE_DRIVE
        "Setup kan zijn station met bronbestanden niet vinden.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_LOAD_TXTSETUPSIF
        "Setup kan het bestand TXTSETUP.SIF niet laden.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_CORRUPT_TXTSETUPSIF
        "Setup heeft een ongeldig TXTSETUP.SIF gevonden.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_SIGNATURE_TXTSETUPSIF,
        "Setup heeft een ongeldige signatuur in TXTSETUP.SIF gevonden.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_DRIVE_INFORMATION
        "Setup kan de systeem drive informatie niet laden.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_WRITE_BOOT,
        "Setup kan de FAT bootcode op de system partitie niet installeren.",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_LOAD_COMPUTER,
        "Setup kan de computer type lijst niet laden.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_LOAD_DISPLAY,
        "Setup kan de beeldscherm instellingen lijst niet laden.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_LOAD_KEYBOARD,
        "Setup kan de toetsenbord type lijst niet laden.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_LOAD_KBLAYOUT,
        "Setup kan de toetsenbord indeling lijst niet laden.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_WARN_PARTITION,
          "Setup ontdekte dat ten minste 1 harde schijf een niet compatibel\n"
          "partitietabel bevat dat niet goed wordt ondersteund!\n"
          "\n"
          "Aanmaken of verwijderen van partities kan de partitie tabel\n"
          "vernietigen.\n"
          "\n"
          "  \x07  Druk op F3 om Setup af te sluiten.\n"
          "  \x07  Druk op ENTER om door te gaan.",
          "F3= Afsluiten  ENTER = Doorgaan"
    },
    {
        //ERROR_NEW_PARTITION,
        "U kunt geen nieuwe partitie aanmaken binnen\n"
        "een reeds bestaande partitie!\n"
        "\n"
        "  * Druk op een willekeurige toets om door te gaan.",
        NULL
    },
    {
        //ERROR_DELETE_SPACE, 
        "U kunt ongebruikte schijfruimte voor partities niet verwijderen!\n"
        "\n"
        "  * Druk op een willekeurige toets om door te gaan.",
        NULL
    },
    {
        //ERROR_INSTALL_BOOTCODE,
        "Setup kan de FAT bootcode op de systeem partitie niet installeren.",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_NO_FLOPPY,
        "Geen schijf in station A:.",
        "ENTER = Doorgaan"
    },
    {
        //ERROR_UPDATE_KBSETTINGS,
        "Setup kan de toetsbord indeling instellingen niet bijwerken.",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_UPDATE_DISPLAY_SETTINGS,
        "Setup kan de beeldscherm registry instellingen  niet bijwerken.",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_IMPORT_HIVE,
        "Setup kan een hive bestand niet importeren.",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_FIND_REGISTRY
        "Setup kan de registry data bestanden niet vinden.",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_CREATE_HIVE,
        "Setup kan de registry hives niet aanmaken.",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_INITIALIZE_REGISTRY,
        "Setup kan de registry niet initialiseren.",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_INVALID_CABINET_INF,
        "Cabinet has no valid inf file.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_CABINET_MISSING,
        "Cabinet bestand niet gevonden.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_CABINET_SCRIPT,
        "Cabinet bestand heeft geen setup script.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_COPY_QUEUE,
        "Setup kan de ljst met te kopieeren bestanden niet vinden.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_CREATE_DIR,
        "Setup kan de installatie directories niet aanmaken.",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_TXTSETUP_SECTION,
        "Setup kan de 'Directories' sectie niet vinden\n"
        "in TXTSETUP.SIF.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_CABINET_SECTION,
        "Setup kan de 'Directories' sectie niet vinden\n"
        "in het cabinet bestand.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_CREATE_INSTALL_DIR
        "Setup kan de installatie directory niet aanmaken.",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_FIND_SETUPDATA,
        "Setup kan de 'SetupData' sectie niet vinden\n"
        "in TXTSETUP.SIF.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_WRITE_PTABLE,
        "Setup kan de partitietabellen niet schrijven.\n"
        "ENTER = Reboot computer"
    },
    {
        //ERROR_ADDING_CODEPAGE,
        "Setup kan de codepagina niet toevoegen aan de registry.\n"
        "ENTER = Reboot computer"
    },
    {
        //ERROR_UPDATE_LOCALESETTINGS,
        "Setup kan de systeemtaal niet instellen.\n"
        "ENTER = Reboot computer"
    },
    {
        //ERROR_ADDING_KBLAYOUTS,
        "Setup kan de toetsenbord indelingen niet toevoegen aan de registry.\n"
        "ENTER = Reboot computer"
    },
    {
        //ERROR_UPDATE_GEOID,
        "Setup kan de geografische positie niet instellen.\n"
        "ENTER = Reboot computer"
    },
    {
        //ERROR_INSUFFICIENT_DISKSPACE,
        "Not enough free space in the selected partition.\n"
        "  * Press any key to continue.",
        NULL
    },
    {
        NULL,
        NULL
    }
};


MUI_PAGE nlNLPages[] =
{
    {
        LANGUAGE_PAGE,
        nlNLLanguagePageEntries
    },
    {
        START_PAGE,
        nlNLWelcomePageEntries
    },
    {
        INSTALL_INTRO_PAGE,
        nlNLIntroPageEntries
    },
    {
        LICENSE_PAGE,
        nlNLLicensePageEntries
    },
    {
        DEVICE_SETTINGS_PAGE,
        nlNLDevicePageEntries
    },
    {
        REPAIR_INTRO_PAGE,
        nlNLRepairPageEntries
    },
    {
        COMPUTER_SETTINGS_PAGE,
        nlNLComputerPageEntries
    },
    {
        DISPLAY_SETTINGS_PAGE,
        nlNLDisplayPageEntries
    },
    {
        FLUSH_PAGE,
        nlNLFlushPageEntries
    },
    {
        SELECT_PARTITION_PAGE,
        nlNLSelectPartitionEntries
    },
    {
        SELECT_FILE_SYSTEM_PAGE,
        nlNLSelectFSEntries
    },
    {
        FORMAT_PARTITION_PAGE,
        nlNLFormatPartitionEntries
    },
    {
        DELETE_PARTITION_PAGE,
        nlNLDeletePartitionEntries
    },
    {
        INSTALL_DIRECTORY_PAGE,
        nlNLInstallDirectoryEntries
    },
    {
        PREPARE_COPY_PAGE,
        nlNLPrepareCopyEntries
    },
    {
        FILE_COPY_PAGE,
        nlNLFileCopyEntries
    },
    {
        KEYBOARD_SETTINGS_PAGE,
        nlNLkeyboardSettingsEntries
    },
    {
        BOOT_LOADER_PAGE,
        nlNLBootLoaderEntries
    },
    {
        LAYOUT_SETTINGS_PAGE,
        nlNLLayoutSettingsEntries
    },
    {
        QUIT_PAGE,
        nlNLQuitPageEntries
    },
    {
        SUCCESS_PAGE,
        nlNLSuccessPageEntries
    },
    {
        BOOT_LOADER_FLOPPY_PAGE,
        nlNLBootPageEntries
    },
    {
        REGISTRY_PAGE,
        nlNLRegistryEntries
    },
    {
        -1,
        NULL
    }
};

MUI_STRING nlNLStrings[] =
{
    {STRING_PLEASEWAIT,
     "   Een ogenblik geduld..."},
    {STRING_INSTALLCREATEPARTITION,
     "   ENTER = Installeren   C = Partitie aanmaken   F3 = Afsluiten"},
    {STRING_INSTALLDELETEPARTITION,
     "   ENTER = Installeren   D = Partitie verwijderen   F3 = Afsluiten"},
    {STRING_PARTITIONSIZE,
     "Grotte nieuwe partitie:"},
    {STRING_CHOOSENEWPARTITION,
     "U wilt een nieuwe partitie aanmaken op"},
    {STRING_HDDSIZE,
    "Voert u de grootte van de nieuwe partitie in in megabytes."},
    {STRING_CREATEPARTITION,
     "   ENTER = Partitie Aanmaken   ESC = Annuleren   F3 = Afsluiten"},
    {STRING_PARTFORMAT,
    "Deze partitie zal vervolgens geformatteerd worden."},
    {STRING_NONFORMATTEDPART,
    "U wilt Odyssey installeren op een nieuwe of ongeformatteerde partitie."},
    {STRING_INSTALLONPART,
    "Setup installeert Odyssey op Partitie"},
    {STRING_CHECKINGPART,
    "Setup controleert nu de geselecteerde partitie."},
    {STRING_QUITCONTINUE,
    "F3= Afsluiten  ENTER = Doorgaan"},
    {STRING_REBOOTCOMPUTER,
    "ENTER = Reboot computer"},
    {STRING_TXTSETUPFAILED,
    "Setup kan de  '%S' sectie niet vinden\nin TXTSETUP.SIF.\n"},
    {STRING_COPYING,
     "   Kopieeren bestand: %S"},
    {STRING_SETUPCOPYINGFILES,
     "Setup is bestand aan het kopieeren..."},
    {STRING_REGHIVEUPDATE,
    "   Bijwerken registry hives..."},
    {STRING_IMPORTFILE,
    "   Importeren %S..."},
    {STRING_DISPLAYETTINGSUPDATE,
    "   Bijwerken registry instellingen beeldscherm..."},
    {STRING_LOCALESETTINGSUPDATE,
    "   Bijwerken systeemtaal instellingen..."},
    {STRING_KEYBOARDSETTINGSUPDATE,
    "   Bijwerken toetsboard indeling..."},
    {STRING_CODEPAGEINFOUPDATE,
    "   Toevoegen informatie codepagina aan registry..."},
    {STRING_DONE,
    "   Voltooid..."},
    {STRING_REBOOTCOMPUTER2,
    "   ENTER = Reboot computer"},
    {STRING_CONSOLEFAIL1,
    "Kan console niet openen\n\n"},
    {STRING_CONSOLEFAIL2,
    "De meest voorkomende oorzaak is het gebruik van een USB toetsenbord\n"},
    {STRING_CONSOLEFAIL3,
    "USB toetsenborden worden nog niet volledig ondersteund\n"},
    {STRING_FORMATTINGDISK,
    "Setup is de harde schijf aan het formatteren"},
    {STRING_CHECKINGDISK,
    "Setup is de harde schijf aan het controleren"},
    {STRING_FORMATDISK1,
    " Formatteer partition als %S bestandssysteem (snel) "},
    {STRING_FORMATDISK2,
    " Formatteer partition als %S bestandssysteem "},
    {STRING_KEEPFORMAT,
    " Behoud huidig bestandssysteem (geen wijzigingen) "},
    {STRING_HDINFOPARTCREATE,
    "%I64u %s  Schijf %lu  (Port=%hu, Bus=%hu, Id=%hu) op %wZ."},
    {STRING_HDDINFOUNK1,
    "%I64u %s  Schijf %lu  (Port=%hu, Bus=%hu, Id=%hu)."},
    {STRING_HDDINFOUNK2,
    "   %c%c  Type %lu    %I64u %s"},
    {STRING_HDINFOPARTDELETE,
    "op %I64u %s  Schijf %lu  (Port=%hu, Bus=%hu, Id=%hu) op %wZ."},
    {STRING_HDDINFOUNK3,
    "op %I64u %s  Schijf %lu  (Port=%hu, Bus=%hu, Id=%hu)."},
    {STRING_HDINFOPARTZEROED,
    "Schijf %lu (%I64u %s), Port=%hu, Bus=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK4,
    "%c%c  Type %lu    %I64u %s"},
    {STRING_HDINFOPARTEXISTS,
    "op Schijf %lu (%I64u %s), Port=%hu, Bus=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK5,
    "%c%c  Type %-3u                         %6lu %s"},
    {STRING_HDINFOPARTSELECT,
    "%6lu %s  Schijf %lu  (Port=%hu, Bus=%hu, Id=%hu) op %S"},
    {STRING_HDDINFOUNK6,
    "%6lu %s  Schijf %lu  (Port=%hu, Bus=%hu, Id=%hu)"},
    {STRING_NEWPARTITION,
    "Setup heeft een nieuwe partitie aangemaakt op"},
    {STRING_UNPSPACE,
    "    Niet gepartitioneerde ruimte     %6lu %s"},
    {STRING_MAXSIZE,
    "MB (max. %lu MB)"},
    {STRING_UNFORMATTED,
    "Nieuw (Ongeformatteerd)"},
    {STRING_FORMATUNUSED,
    "Ongebruikt"},
    {STRING_FORMATUNKNOWN,
    "Onbekend"},
    {STRING_KB,
    "KB"},
    {STRING_MB,
    "MB"},
    {STRING_GB,
    "GB"},
    {STRING_ADDKBLAYOUTS,
    "Toevoegen toetsenbord indelingen"},
    {0, 0}
};
