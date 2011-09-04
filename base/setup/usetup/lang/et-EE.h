#pragma once

MUI_LAYOUTS etEELayouts[] =
{
    { L"0425", L"00000425" },
    { NULL, NULL }
};

static MUI_ENTRY etEELanguagePageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Keele valik",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Vali keel, mida paigaldamisel kasutada.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Ja vajuta ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Seda keelt kasutatakse hiljem s�steemi keelena.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = J�tka  F3 = V�lju",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY etEEWelcomePageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Tere tulemast Odysseyi paigaldama",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        6,
        11,
		"Selles paigaldamise osas kopeeritakse Odysseyi failid arvutisse ja",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "valmistatakse ette paigaldamise teine j�rk.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Vajuta ENTER, et Odyssey paigaldada.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Vajuta R, et Odysseyi parandada.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  Vajuta L, et n�ha Odysseyi litsentsi ja kasutamise tingimusi",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Vajuta F3, et v�ljuda Odysseyi paigaldamata.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "Odysseyi kohta saab rohkem infot:",
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
        "ENTER = J�tka  R = Paranda  L = Litsents  F3 = V�lju",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY etEEIntroPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Odysseyi paigaldusprogramm on varajases arendusfaasis. Praegu ei ole",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "veel k�ik korraliku paigaldusprogrammi funktsioonid toetatud.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "Kehtivad j�rgmised piirangud:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "- Sihtkettal v�ib olla ainult �ks peamine partitsioon.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "- Peamist partitsiooni ei saa kettalt kustutada",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "  kui kettal on ka laiendatud partitsioone.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "- Esimest laiendatud partitsiooni ei saa kettalt kustutada",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "  kui kettal on ka teisi laiendatud partitsioone.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "- Toetatud on ainult FAT failis�steem.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "- Failis�steemi kontrollimist veel ei tehta.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        23,
        "\x07  Vajuta ENTER, et Odyssey paigaldada.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        25,
        "\x07  Vajuta F3, et v�ljuda Odysseyi paigaldamata.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = J�tka   F3 = V�lju",
        TEXT_TYPE_STATUS| TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY etEELicensePageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        6,
        "Litsents:",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        8,
        "Odyssey kasutab GNU �ldist avalikku litsentsi(GPL),",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        9,
        "m�ned komponendid kasutavad muid �hilduvaid litsentse,",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "nagu n�iteks X11, BSD ja GNU LGPL.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "Kogu Odysseyi s�steem on seega kaitstud GPL litsentsiga",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "ning samas kehtivad ka algsed litsentsid.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "K�esoleva tarkvaraga ei anta kaasa garantiid ega m��rata kasutamise",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "piiranguid kehtiva seadusega s�testatud piirides. Odysseyi",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "litsents m��rab ainult levitamise kolmandatele osapooltele.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "Kui mingil p�hjusel ei olnud tarkvaraga kaasas GNU GPL",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "�ldist avalikku litsentsi, siis saab seda vaadata lehel",
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
        "Garantii:",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        24,
        "Tegemist on vaba tarkvaraga; kopeerimise tingimused on kirjas algkoodis.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        25,
        "Garantii puudub; pole isegi turustamiseks v�i mingil",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        26,
        "kindlal eesm�rgil kasutamiseks sobivuse garantiid",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Tagasi",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY etEEDevicePageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "J�rgnev nimekiri n�itab riistvara seadeid.",
        TEXT_STYLE_NORMAL
    },
    {
        24,
        11,
        "Arvuti:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        12,
        "Monitor:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        13,
        "Klaviatuur:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        14,
        "Klaviatuuri asetus:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        16,
        "Rakenda:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        25,
        16, "Rakenda need seaded",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        19,
        "Riistvara seadeid saab muuta �les ja alla liikudes.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        20,
        "Seadistuse muutmiseks vajuta ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        21,
        "",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "Kui seadistus on paigas, vali \"Rakenda need seaded\"",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        24,
        "ja vajuta ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = J�tka   F3 = V�lju",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY etEERepairPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Odysseyi paigaldusprogramm on varajases arendusfaasis. Praegu ei ole",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "veel k�ik korraliku paigaldusprogrammi funktsioonid toetatud.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "Parandamine ei ole veel toetatud.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Vajuta U, et s�steemi uuendada.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Vajuta R, et kasutada taastuskonsooli.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  Vajuta ESC, et minna tagasi pealehele.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Vajuta ENTER, et arvuti taask�ivitada.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ESC = Pealeht  U = Uuenda  R = Taastamine  ENTER = Taask�ivitus",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};
static MUI_ENTRY etEEComputerPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Arvuti t��bi muutmine.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Liigu �les-alla, et valida sobiv arvuti t��p.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Seej�rel vajuta ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Vajuta ESC, et minna tagasi eelmisele lehele",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ilma arvuti t��pi muutmata.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = J�tka   ESC = Katkesta   F3 = V�lju",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY etEEFlushPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "S�steem kirjutab n��d andmed kettale",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "V�ib kuluda veidi aega",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "L�petamisel taask�ivitub arvuti automaatselt",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Vahem�lu t�hjendamine",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY etEEQuitPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "Odyssey ei ole t�ielikult paigaldatud",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Eemalda flopiketas ja CD-ROMid draividest.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "Vajuta ENTER, et arvuti taask�ivitada.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Palun oota...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG,
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY etEEDisplayPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Monitori t��bi muutmine.",
        TEXT_STYLE_NORMAL
    },
    {   8,
        10,
         "\x07  Liigu �les-alla, et monitori t��pi muuta.",
         TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Seej�rel vajuta ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Vajuta ESC, et minna tagasi eelmisele lehele",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ilma monitori t��pi muutmata.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = J�tka   ESC = Katkesta   F3 = V�lju",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY etEESuccessPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "Odysseyi p�hilised komponendid on edukalt paigaldatud.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Eemalda flopiketas ja CD-ROMid draividest.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "Vajuta ENTER, et arvuti taask�ivitada.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Taask�ivita arvuti",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY etEEBootPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Alglaadurit ei saanud kettale kirjutada.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        13,
		"Sisesta vormindatud flopiketas draivi A:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "ja vajuta ENTER.",
        TEXT_STYLE_NORMAL,
    },
    {
        0,
        0,
        "ENTER = J�tka   F3 = V�lju",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }

};

static MUI_ENTRY etEESelectPartitionEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "See nimekiri n�itab partitsioone ja vaba ruumi",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "uute partitsioonide jaoks.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "\x07  Liigu �les-alla, et valida kirje.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Vajuta ENTER, et paigaldada Odyssey valitud partitsioonile.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Vajuta C, et teha uus partitsioon.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Vajuta D, et kustutada olemasolev partitsioon.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Palun oota...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY etEEFormatPartitionEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Partitsiooni vormindamine",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        10,
        "N��d vormindatakse partitsioon. Vajuta ENTER, et j�tkata.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = J�tka   F3 = V�lju",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        TEXT_STYLE_NORMAL
    }
};

static MUI_ENTRY etEEInstallDirectoryEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Odyssey paigaldatakse valitud partitsioonile.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
		"Vali kaust, kuhu Odyssey paigaldada:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "Kausta muutmiseks kustuta kirje BACKSPACE klahviga ja",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        15,
        "tr�ki asemele kaust, kuhu Odyssey installeerida.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        16,
        "",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = J�tka   F3 = V�lju",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY etEEFileCopyEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        0,
        12,
        "Palun oota, kuni Odyssey paigaldatakse sihtkausta.",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        13,
        "",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        14,
        "V�ib kuluda mitu minutit.",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        50,
        0,
        "\xB3 Palun oota...    ",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY etEEBootLoaderEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Alglaaduri paigaldamine",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "Paigalda alglaadur k�vakettale (MBR ja VBR).",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "Paigalda alglaadur k�vakettale (ainult VBR).",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "Paigalda alglaadur flopikettale.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "�ra paigalda alglaadurit.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = J�tka   F3 = V�lju",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY etEEKeyboardSettingsEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Klaviatuuri t��bi muutmine.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Vajuta �les-alla, et valida klaviatuuri t��p.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Seej�rel vajuta ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Vajuta ESC, et minna tagasi eelmisele lehele",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   klaviatuuri t��pi muutmata.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = J�tka   ESC = Katkesta   F3 = V�lju",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY etEELayoutSettingsEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Vali vaikimisi klaviatuuriasetus.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Liigu �les-alla, et valida klaviatuuriasetus.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "    Seej�rel vajuta ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Vajuta ESC, et minna tagasi eelmisele lehele",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   klaviatuuriasetust muutmata.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = J�tka   ESC = Katkesta   F3 = V�lju",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY etEEPrepareCopyEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Arvutit valmistatakse ette Odysseyi failide kopeerimiseks.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Failide nimekirja loomine...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY etEESelectFSEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        17,
        "Vali nimekirjast failis�steem.",
        0
    },
    {
        8,
        19,
        "\x07  Liigu �les-alla, et valida failis�steem.",
        0
    },
    {
        8,
        21,
        "\x07  Vajuta ENTER, et partitsioon vormindada.",
        0
    },
    {
        8,
        23,
        "\x07  Vajuta ESC, et valida muu partitsioon.",
        0
    },
    {
        0,
        0,
        "ENTER = J�tka   ESC = Katkesta   F3 = V�lju",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },

    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY etEEDeletePartitionEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Oled valinud partitsiooni kustutamise",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "\x07  Vajuta D, et partitsioon kustutada.",
        TEXT_STYLE_NORMAL
    },
    {
        11,
        19,
        "HOIATUS: K�ik andmed partitsioonil kustutatakse!",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Vajuta ESC, et katkestada.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "D = Kustuta partitsioon   ESC = Katkesta   F3 = V�lju",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY etEERegistryEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " paigaldamine ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Uuendatakse s�steemi seadistust.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Registri v�tmete loomine...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

MUI_ERROR etEEErrorEntries[] =
{
    {
        //ERROR_NOT_INSTALLED
        "Odyssey ei ole t�ielikult paigaldatud.\n"
        "Kui paigaldamine praegu katkestada, siis tuleb\n"
        "Odysseyi paigaldamiseks paigaldusprogramm uuesti k�ivitada.\n"
        "\n"
        "  \x07  Vajuta ENTER, et paigaldamist j�tkata.\n"
        "  \x07  Vajuta F3, et paigaldamie peatada.",
        "F3 = V�lju  ENTER = J�tka"
    },
    {
        //ERROR_NO_HDD
        "K�vaketast ei leitud.\n",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_NO_SOURCE_DRIVE
        "Paigaldusprogramm ei leidnud ketast, millelt see k�ivitati.\n",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_LOAD_TXTSETUPSIF
        "TXTSETUP.SIF faili ei �nnestunud laadida.\n",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_CORRUPT_TXTSETUPSIF
        "TXTSETUP.SIF on vigane.\n",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_SIGNATURE_TXTSETUPSIF,
        "TXTSETUP.SIF faili signatuur on vigane.\n",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_DRIVE_INFORMATION
        "S�steemiketta parameetreid ei �nnestunud lugeda.\n",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_WRITE_BOOT,
        "S�steemikettale ei �nnestunud kirjutada FAT alglaadimiskoodi.",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_LOAD_COMPUTER,
        "Arvutit��pide nimekirja ei �nnestunud laadida.\n",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_LOAD_DISPLAY,
        "Monitoride nimekirja ei �nnestunud laadida.\n",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_LOAD_KEYBOARD,
        "Klaviatuuri t��pide nimekirja ei �nnestunud laadida.\n",
         "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_LOAD_KBLAYOUT,
        "Klaviatuuriasetuste nimekirja ei �nnestunud laadida.\n",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_WARN_PARTITION,
          "Leiti v�hemalt �ks k�vaketas, millel on �hildamatu partitsioonitabel,\n"
          "millega ei saanud korralikult �mber k�ia!\n"
          "\n"
          "Partitsioonide loomine v�i kustutamine v�ib vigastada partitsioonitabelit.\n"
          "\n"
          "  \x07  Vajuta F3, et v�ljuda paigaldusest.."
          "  \x07  Vajuta ENTER, et j�tkata.",
          "F3= V�lju  ENTER = J�tka"
    },
    {
        //ERROR_NEW_PARTITION,
        "Uut partitsioonitabelit ei saa juba olemasoleva\n"
        "partitsiooni sisse tekitada!\n"
        "\n"
        "  * Vajuta suvalist klahvi, et j�tkata.",
        NULL
    },
    {
        //ERROR_DELETE_SPACE,
        "Partitsioneerimata kettaruumi ei saa kustutada!\n"
        "\n"
        "  * Vajuta suvalist klahvi, et j�tkata.",
        NULL
    },
    {
        //ERROR_INSTALL_BOOTCODE,
        "S�steemikettale ei �nnestunud paigaldada FAT alglaadimiskoodi.",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_NO_FLOPPY,
        "Draivis A: ei ole flopiketast.",
        "ENTER = J�tka"
    },
    {
        //ERROR_UPDATE_KBSETTINGS,
        "Klaviatuuriasetuse seadistust ei �nnestunud uuendada.",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_UPDATE_DISPLAY_SETTINGS,
        "Monitori seadistust registris ei �nnestunud uuendada.",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_IMPORT_HIVE,
        "Tarufaili ei �nnestunud importida.",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_FIND_REGISTRY
        "Registri andmete faile ei leitud.",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_CREATE_HIVE,
        "Registri tarusid ei �nnestunud luua.",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_INITIALIZE_REGISTRY,
        "Registrit ei �nnestunud luua.",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_INVALID_CABINET_INF,
        "Kapifailis ei olnud p�devaid inf faile.\n",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_CABINET_MISSING,
        "Kapifaili ei leitud.\n",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_CABINET_SCRIPT,
        "Kapifailis puudub paigaldusskript.\n",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_COPY_QUEUE,
        "Kopeeritavate failide nimekirja ei �nnestunud avada.\n",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_CREATE_DIR,
        "Paigalduskaustu ei �nnestunud luua.",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_TXTSETUP_SECTION,
		"TXTSETUP.SIF failist ei leitud 'Directories' sektsiooni.",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_CABINET_SECTION,
        "Kapifailist ei leitud 'Directories' sektsiooni.",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_CREATE_INSTALL_DIR
        "Paigalduskausta ei �nnestunud luua.",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_FIND_SETUPDATA,
        "TXTSETUP.SIF failist ei leitud 'SetupData' sektsiooni",
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_WRITE_PTABLE,
        "Partitsioonitabeleid ei �nnestunud kirjutada.\n"
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_ADDING_CODEPAGE,
        "Kooditabelit ei �nnestunud registrisse lisada.\n"
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_UPDATE_LOCALESETTINGS,
        "S�steemilokaati ei �nnestunud sedistada.\n"
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_ADDING_KBLAYOUTS,
        "Klaviatuuriasetusi ei �nnestunud registrisse lisada.\n"
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_UPDATE_GEOID,
        "Geograafilist asukohta ei �nnestunud seadistada.\n"
        "ENTER = Taask�ivita arvuti"
    },
    {
        //ERROR_INSUFFICIENT_DISKSPACE,
        "Valitud partitsioonil pole piisavalt ruumi.\n"
        "  * Vajuta suvalist klahvi, et j�tkata.",
        NULL
    },
    {
        NULL,
        NULL
    }
};


MUI_PAGE etEEPages[] =
{
    {
        LANGUAGE_PAGE,
        etEELanguagePageEntries
    },
    {
        START_PAGE,
        etEEWelcomePageEntries
    },
    {
        INSTALL_INTRO_PAGE,
        etEEIntroPageEntries
    },
    {
        LICENSE_PAGE,
        etEELicensePageEntries
    },
    {
        DEVICE_SETTINGS_PAGE,
        etEEDevicePageEntries
    },
    {
        REPAIR_INTRO_PAGE,
        etEERepairPageEntries
    },
    {
        COMPUTER_SETTINGS_PAGE,
        etEEComputerPageEntries
    },
    {
        DISPLAY_SETTINGS_PAGE,
        etEEDisplayPageEntries
    },
    {
        FLUSH_PAGE,
        etEEFlushPageEntries
    },
    {
        SELECT_PARTITION_PAGE,
        etEESelectPartitionEntries
    },
    {
        SELECT_FILE_SYSTEM_PAGE,
        etEESelectFSEntries
    },
    {
        FORMAT_PARTITION_PAGE,
        etEEFormatPartitionEntries
    },
    {
        DELETE_PARTITION_PAGE,
        etEEDeletePartitionEntries
    },
    {
        INSTALL_DIRECTORY_PAGE,
        etEEInstallDirectoryEntries
    },
    {
        PREPARE_COPY_PAGE,
        etEEPrepareCopyEntries
    },
    {
        FILE_COPY_PAGE,
        etEEFileCopyEntries
    },
    {
        KEYBOARD_SETTINGS_PAGE,
        etEEKeyboardSettingsEntries
    },
    {
        BOOT_LOADER_PAGE,
        etEEBootLoaderEntries
    },
    {
        LAYOUT_SETTINGS_PAGE,
        etEELayoutSettingsEntries
    },
    {
        QUIT_PAGE,
        etEEQuitPageEntries
    },
    {
        SUCCESS_PAGE,
        etEESuccessPageEntries
    },
    {
        BOOT_LOADER_FLOPPY_PAGE,
        etEEBootPageEntries
    },
    {
        REGISTRY_PAGE,
        etEERegistryEntries
    },
    {
        -1,
        NULL
    }
};

MUI_STRING etEEStrings[] =
{
    {STRING_PLEASEWAIT,
     "   Palun oota..."},
    {STRING_INSTALLCREATEPARTITION,
     "   ENTER = Paigalda  C = Loo partitsioon    F3 = V�lju"},
    {STRING_INSTALLDELETEPARTITION,
     "   ENTER = Paigalda  D = Kustuta partitsioon  F3 = V�lju"},
    {STRING_PARTITIONSIZE,
     "Uue partitsiooni suurus:"},
    {STRING_CHOOSENEWPARTITION,
     "Oled valinud kettale uue partitsiooni loomise"},
    {STRING_HDDSIZE,
    "Sisesta uue partitsiooni suurus megabaitides."},
    {STRING_CREATEPARTITION,
     "   ENTER = Loo partitsioon   ESC = Katkesta   F3 = V�lju"},
    {STRING_PARTFORMAT,
    "J�rgmisena vormindatakse seda partitsiooni."},
    {STRING_NONFORMATTEDPART,
    "Oled valinud Odysseyi paigaldamise uuele v�i vormindamata partitsioonile."},
    {STRING_INSTALLONPART,
    "Odyssey paigaldatakse partitsioonile"},
    {STRING_CHECKINGPART,
    "Valitud partitsiooni kontrollitakse."},
    {STRING_QUITCONTINUE,
    "F3= V�lju  ENTER = J�tka"},
    {STRING_REBOOTCOMPUTER,
    "ENTER = Taask�ivita arvuti"},
    {STRING_TXTSETUPFAILED,
     "TXTSETUP.SIF failist ei leitud '%S' sektsiooni\n"},
    {STRING_COPYING,
     "   Kopeerimine: %S"},
    {STRING_SETUPCOPYINGFILES,
     "Failide kopeerimine..."},
    {STRING_REGHIVEUPDATE,
    "   Registritarude uuendamine..."},
    {STRING_IMPORTFILE,
    "   %S importimine..."},
    {STRING_DISPLAYETTINGSUPDATE,
    "   Monitori seadistuse uuendamine registris..."},
    {STRING_LOCALESETTINGSUPDATE,
    "   Lokaadi seadistuse uuendamine..."},
    {STRING_KEYBOARDSETTINGSUPDATE,
    "   Klaviatuuriasetuse seadistuse uuendamine..."},
    {STRING_CODEPAGEINFOUPDATE,
    "   Kooditabeli info lisamine registrisse..."},
    {STRING_DONE,
    "   Valmis..."},
    {STRING_REBOOTCOMPUTER2,
    "   ENTER = Taask�ivita arvuti"},
    {STRING_CONSOLEFAIL1,
    "Konsooli ei �nnestunud avada\n\n"},
    {STRING_CONSOLEFAIL2,
    "T�en�oliselt on probleem USB klaviatuuri kasutamises\n"},
    {STRING_CONSOLEFAIL3,
    "USB klaviatuurid ei ole veel toetatud\n"},
    {STRING_FORMATTINGDISK,
    "K�vaketta vormindamine"},
    {STRING_CHECKINGDISK,
    "K�vaketta kontrollimine"},
    {STRING_FORMATDISK1,
    " Vorminda partitsioon %S failis�steemiga (kiire vormindus) "},
    {STRING_FORMATDISK2,
    " Vorminda partitsioon %S failis�steemiga "},
    {STRING_KEEPFORMAT,
    " �ra muuda praegust failis�steemi "},
    {STRING_HDINFOPARTCREATE,
    "%I64u %s  K�vaketas %lu  (Port=%hu, Siin=%hu, Id=%hu) - %wZ."},
    {STRING_HDDINFOUNK1,
    "%I64u %s  K�vaketas %lu  (Port=%hu, Siin=%hu, Id=%hu)."},
    {STRING_HDDINFOUNK2,
    "   %c%c  Type %lu    %I64u %s"},
    {STRING_HDINFOPARTDELETE,
    "%I64u %s  K�vaketas %lu  (Port=%hu, Siin=%hu, Id=%hu) - %wZ."},
    {STRING_HDDINFOUNK3,
    "%I64u %s  K�vaketas %lu  (Port=%hu, Siin=%hu, Id=%hu)."},
    {STRING_HDINFOPARTZEROED,
    "K�vaketas %lu (%I64u %s), Port=%hu, Siin=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK4,
    "%c%c  Type %lu    %I64u %s"},
    {STRING_HDINFOPARTEXISTS,
    "K�vaketas %lu (%I64u %s), Port=%hu, Siin=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK5,
    "%c%c  T��p %-3u                         %6lu %s"},
    {STRING_HDINFOPARTSELECT,
    "%6lu %s  K�vaketas %lu  (Port=%hu, Siin=%hu, Id=%hu) on %S"},
    {STRING_HDDINFOUNK6,
    "%6lu %s  K�vaketas %lu  (Port=%hu, Siin=%hu, Id=%hu)"},
    {STRING_NEWPARTITION,
    "Loodi uus partitsioon"},
    {STRING_UNPSPACE,
    "    Kasutamata kettaruum                %6lu %s"},
    {STRING_MAXSIZE,
    "MB (maks. %lu MB)"},
    {STRING_UNFORMATTED,
    "Uus (Vormindamata)"},
    {STRING_FORMATUNUSED,
    "Kasutamata"},
    {STRING_FORMATUNKNOWN,
    "Tundmatu"},
    {STRING_KB,
    "KB"},
    {STRING_MB,
    "MB"},
    {STRING_GB,
    "GB"},
    {STRING_ADDKBLAYOUTS,
    "Klaviatuuriasetuste lisamine"},
    {0, 0}
};
