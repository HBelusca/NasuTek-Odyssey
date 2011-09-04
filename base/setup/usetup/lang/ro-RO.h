#pragma once

MUI_LAYOUTS roROLayouts[] =
{
    { L"0409", L"00000409" },
    { L"0418", L"00000418" },
    { NULL, NULL }
};

static MUI_ENTRY roROLanguagePageEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Selec�ie limb�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  V� rug�m selecta�i limba pentru procesul de instalare.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Apoi ap�sa�i ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Aceasta va fi �n final limba implicit� pentru tot sistemul.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Continuare  F3 = Ie�ire",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY roROWelcomePageEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Bun venit la instalarea Odyssey",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        6,
        11,
        "Aceast� prim� parte din instalarea Odyssey va copia fi�ierele",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "necesare �n calculatorul dumneavoastr� �i-l va preg�ti pentru",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        13,
        "cea de-a doua parte a instal�rii.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "\x07  Ap�sa�i ENTER pentru a instala Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "\x07  Tasta�i R pentru a reface un sistem deteriorat sau pentru",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "   a actualiza Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Tasta�i L pentru Termenii �i Condi�iile de Licen�iere",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        23,
        "\x07  Ap�sa�i F3 pentru a ie�i f�r� a instala Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        26,
        "Pentru mai multe informa�ii despre Odyssey, vizita�i:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        27,
        "http://www.odyssey.org",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        0,
        0,
        "ENTER = Continuare  R = Refacere  L = Licen��  F3 = Ie�ire",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY roROIntroPageEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Instalatorul Odyssey curent este �nc� �ntr-un stadiu",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "primar de dezvoltare �i nu con�ine toate func�ionalit��ile",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        10,
        "unei aplica�ii complete de instalare.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        13,
        "Sunt aplicabile urm�toarele limit�ri:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "- Instalatorul curent nu e capabil de mai mult de",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "  o parti�ie per disc.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "- Instalatorul curent nu poate �terge parti�ia primar�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "  a unui dispozitiv c�t timp exist� parti�ii extinse.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "- Instalatorul curent nu poate �terge prima parti�ie",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "  extins�, c�t timp exist� alte parti�ii extinse.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        20,
        "- Instalatorul curent poate opera doar cu sisteme",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "  de fi�iere FAT.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        22,
        "- Verific�rile de integritate pentru fi�iere nu sunt",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        23,
        "  �nc� implementate.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        26,
        "\x07  Ap�sa�i ENTER pentru a instala Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        28,
        "\x07  Ap�sa�i F3 pentru a ie�i f�r� a instala Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Continuare   F3 = Ie�ire",
        TEXT_TYPE_STATUS| TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY roROLicensePageEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        6,
        "Licen�iere:",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        8,
        "Sistemul Odyssey este oferit �n termenii GNU GPL, cu p�r�i",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        9,
        "de cod din alte licen�e compatibile, cum ar fi licen�ele",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "X11, BSD �i GNU LGPL.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "Toate softurile din componen�a sistemului Odyssey sunt prin",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "urmare oferite sub licen�a GNU GPL men�in�ndu-�i �n acela�i",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "timp �i licen�ierea original�.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "Acest soft nu are ata�at� nici restric�ie de utilizare",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "cu aplicabilitate legislativ� local� �i interna�ional�.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "Licen�ierea se refer� doar la distribu�ia Odyssey c�tre",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "p�r�i ter�e.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        20,
        "Dac� din vreun motiv careva nu a�i primit o copie a Licen�ei",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "Publice Generale GNU �mpreun� cu Odyssey, sunte�i ruga�i s�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        22,
        "vizita�i:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        23,
        "http://www.gnu.org/licenses/licenses.html",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        6,
        26,
        "Garan�ie:",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        28,
        "Acest� aplica�ie este distribuit� �n speran�a c� va fi util�,",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        29,
        "F�R� �ns� NICI O GARAN�IE; nici m�car cu garan�ia implicit� a",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        30,
        "VANDABILIT��II sau a UTILIT��II �NTR-UN SCOP ANUME.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Revenire",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY roRODevicePageEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Configurarea dispozitivelor de baz�.",
        TEXT_STYLE_NORMAL
    },
    {
        24,
        11,
        "Arh. de calcul:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        12,
        "Parametri grafici:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        13,
        "Model tastatur�:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        14,
        "Config. tastatur�:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        16,
        "Accept�:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        25,
        16, "Accept configura�ia dispozitivelor",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        19,
        "Pute�i modifica starea curent�. Utiliza�i tastele SUS/JOS pentru",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        20,
        "alegerea unui dispozitiv, apoi ap�sa�i ENTER pentru a-i modifica",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        21,
        "configura�ia ata�at�.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "C�nd configura�ia dispozitivele enumerate este cea corect�,",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        24,
        "selecta�i \"Accept configura�ia dispozitivelor\", apoi",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        25,
        "confirma�i-o ap�s�nd ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Continuare   F3 = Ie�ire",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY roRORepairPageEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Instalatorul Odyssey este �nc� �ntr-o faz� incipient� de dezvoltare",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "�i nu posed� toat� func�ionalitatea unui instalator complet.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "Func�ionalitatea de refacere �nc� nu este implementat�.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Tasta�i U pentru actualizarea SO.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Tasta�i R pentru consola de Recuperare.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  Ap�sa�i ESC pentru a reveni la pagina principal�.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Ap�sa�i ENTER pentru a reporni calculatorul.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ESC = Revenire  U = Actualizare  R = Recuperare  ENTER = Repornire",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};
static MUI_ENTRY roROComputerPageEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Dori�i specificarea arhitecturii de calcul.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Utiliza�i tastele SUS/JOS pentru a selecta o",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   arhitectur� de calcul, apoi ap�sa�i ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Ap�sa�i ESC pentru a reveni la pagina precedent�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   f�r� a specifica o alt� arhitectur� de calcul.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Continuare   ESC = Anulare   F3 = Ie�ire",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY roROFlushPageEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "Verificarea stoc�rii datelor necesare.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "La final, calculatorul va fi repornit automat.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "V� rug�m s� a�tepta�i.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Eliberare memorie...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY roROQuitPageEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "Odyssey nu a fost instalat complet.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Scoate�i discul flexibil din unitatea A:",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "�i toate CD-ROM din unit��ile CD.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "Ap�sa�i ENTER pentru a reporni calculatorul.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "V� rug�m s� a�tepta�i...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG,
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY roRODisplayPageEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Dori�i modificarea parametrilor grafici de afi�are.",
        TEXT_STYLE_NORMAL
    },
    {   8,
        10,
        "\x07  Utiliza�i tastele SUS/JOS pentru a selecta",
         TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   un set de parametri, apoi ap�sa�i ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Ap�sa�i ESC pentru a reveni la pagina precedent�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   fara a modifica parametrii grafici actuali.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Continuare   ESC = Anulare   F3 = Ie�ire",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY roROSuccessPageEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "Componentele de baz� ale Odyssey au fost instalate cu succes.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Scoate�i discul flexibil din unitatea A: �i toate CD-ROM",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "din unit��ile CD.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "Ap�sa�i ENTER pentru a reporni calculatorul.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Repornire calculator",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY roROBootPageEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Instalatorul nu poate instala aplica�ia de ini�ializare",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "a calculatorului pe discul calculatorului dumneavoastr�.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        13,
        "Sunte�i ruga�i s� introduce�i un disc flexibil formatat",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "�n unitatea A: apoi s� ap�sa�i ENTER.",
        TEXT_STYLE_NORMAL,
    },
    {
        0,
        0,
        "ENTER = Continuare   F3 = Ie�ire",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }

};

static MUI_ENTRY roROSelectPartitionEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Urm�toarea list� cuprinde parti�iile existente precum",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "�i spa�iul nefolosit pentru crearea de noi parti�ii.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "\x07  Utiliza�i tastele SUS/JOS pentru a selecta o op�iune.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Ap�sa�i ENTER pentru a instala pe parti�ia selectat�.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Tasta�i C pentru a crea o nou� parti�ie.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Tasta�i D pentru a �terge o parti�ie existent�.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Va rug�m s� a�tepta�i...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY roROFormatPartitionEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Formatare parti�ie.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        10,
        "Urmeaz� formatarea parti�iei.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        10,
        "Ap�sa�i ENTER pentru a continua.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Continuare   F3 = Ie�ire",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        TEXT_STYLE_NORMAL
    }
};

static MUI_ENTRY roROInstallDirectoryEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Alege�i un director de instalare pe parti�ia aleas�.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "Aici vor fi amplasate fi�ierele sistemului Odyssey:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "Pute�i indica un alt director. Ap�sa�i BACKSPACE pentru",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        15,
        "a �terge caractere, apoi scrie�i calea directorului unde",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        16,
        "dori�i instalarea Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Continuare   F3 = Ie�ire",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY roROFileCopyEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        0,
        12,
        "Transferarea fi�ierelor �n directorul Odyssey specificat.",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        13,
        "(poate dura c�teva minute)",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        50,
        0,
        "\xB3 V� rug�m s� a�tepta�i...    ",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY roROBootLoaderEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Instalarea aplica�iei de ini�ializare a calculatorului",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "Instaleaz� ini�ializator pe discul intern (MBR �i VBR).",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "Instaleaz� ini�ializator pe discul intern (doar VBR).",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "Instaleaz� ini�ializator pe un disc flexibil.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "Omite instalarea aplica�iei de ini�ializare.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Continuare   F3 = Ie�ire",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY roROKeyboardSettingsEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Dori�i specificarea modelului tastaturii instalate.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Utiliza�i tastele SUS/JOS pentru a selecta un model",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   de tastatur�, apoi ap�sa�i ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Ap�sa�i ESC pentru a reveni la pagina precedent� f�r�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   a schimba modelul tastaturii curente.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Continuare   ESC = Anulare   F3 = Ie�ire",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY roROLayoutSettingsEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Specifica�i pentru instalare o configura�ie implicit� a tastaturii.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Utiliza�i tastele SUS/JOS pentru a selecta o configura�ie",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "    dorit� de tastatur�, apoi ap�sa�i ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Ap�sa�i ESC pentru a reveni la pagina precedent� f�r�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   a schimba configura�ia curent� a tastaturii.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Continuare   ESC = Anulare   F3 = Ie�ire",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY roROPrepareCopyEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Se fac preg�tirile necesare pentru copierea de fi�iere...",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Crearea listei de fi�iere...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY roROSelectFSEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        14,
        "Alege�i un sistem de fi�iere din lista de mai jos.",
        0
    },
    {
        8,
        16,
        "\x07  Utiliza�i tastele SUS/JOS pentru a selecta",
        0
    },
    {
        8,
        17,
        "   un sistem de fi�iere.",
        0
    },
    {
        8,
        19,
        "\x07  Ap�sa�i ENTER pentru a formata parti�ia.",
        0
    },
    {
        8,
        21,
        "\x07  Ap�sa�i ESC pentru a alege o alt� parti�ie.",
        0
    },
    {
        0,
        0,
        "ENTER = Continuare   ESC = Anulare   F3 = Ie�ire",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },

    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY roRODeletePartitionEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "A�i ales s� �terge�i parti�ia",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "\x07  Tasta�i D pentru a �terge parti�ia.",
        TEXT_STYLE_NORMAL
    },
    {
        11,
        19,
        "ATEN�IE: Toate datele de pe aceast� parti�ie vor fi pierdute!",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Ap�sa�i ESC pentru a anula.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "D = �tergere parti�ie   ESC = Anulare   F3 = Ie�ire",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY roRORegistryEntries[] =
{
    {
        4,
        3,
        " Instalare Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Actualizarea configura�iei sistemului.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Crearea registului...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

MUI_ERROR roROErrorEntries[] =
{
    {
        //ERROR_NOT_INSTALLED
        "Odyssey nu a fost instalat complet �n calculatorul\n"
        "dumneavoastr�. Dac� abandona�i instalarea Odyssey\n"
		"acum, va fi nevoie s� o relua�i din nou alt� dat�.\n"
        "\n"
        "  \x07  Ap�sa�i ENTER pentru a continua instalarea.\n"
        "  \x07  Ap�sa�i F3 pentru a ie�i din Instalator.",
        "F3 = Ie�ire  ENTER = Continuare"
    },
    {
        //ERROR_NO_HDD
        "Nu se pot g�si discuri interne.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_NO_SOURCE_DRIVE
        "Nu se poate g�si unitatea de citire.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_LOAD_TXTSETUPSIF
        "Nu poate �nc�rca fi�ierul TXTSETUP.SIF.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_CORRUPT_TXTSETUPSIF
        "Fi�ieul TXTSETUP.SIF a fos g�sit deteriorat.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_SIGNATURE_TXTSETUPSIF,
        "Fi�ierul TXTSETUP.SIF con�ine o semn�tur� invalid�.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_DRIVE_INFORMATION
        "Nu se pot ob�ine informa�ii despre dispozitiv(ele)\n"
		"din calculator.\n",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_WRITE_BOOT,
        "Nu s-a reu�it instalarea codului FAT de ini�ializare\n"
		"pe parti�ia de sistem.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_LOAD_COMPUTER,
        "A e�uat �nc�rcarea listei cu arhitecturi de\n"
		"calcul disponibile.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_LOAD_DISPLAY,
        "A e�uat �nc�rcarea listei cu parametri de\n"
		"afi�are pentru ecran.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_LOAD_KEYBOARD,
        "A e�uat �nc�rcarea listei cu tipuri\n"
		"disponibile de tastatur�.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_LOAD_KBLAYOUT,
        "A e�uat �nc�rcarea listei de configura�ii\n"
		"ale tastaturii.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_WARN_PARTITION,
        "A fost g�sit cel pu�in un disc cu tabel� de parti�ii\n"
		"nesuportat�, ce nu pot fi gestionat� corespunz�tor!\n"
        "\n"
        "Crearea sau �tergerea de parti�ii poate astfel cauza\n"
		"distrugerea tabelei de parti�ii."
        "\n"
        "  \x07  Ap�sa�i F3 pentru a ie�i din Instalator.\n"
        "  \x07  Ap�sa�i ENTER pentru a continua.",
        "F3= Ie�ire  ENTER = Continuare"
    },
    {
        //ERROR_NEW_PARTITION,
        "O parti�ie nou� nu se creaz� �n interiorul\n"
        "unei parti�ii existente!\n"
        "\n"
        "  * Tasta�i pentru a continua.",
        NULL
    },
    {
        //ERROR_DELETE_SPACE,
        "A�i �ncercat �tergerea de spa�iu neparti�ionat,\n"
        "�ns� doar spa�iul parti�ionat poate fi �ters!\n"
        "\n"
        "  * Tasta�i pentru a continua.",
        NULL
    },
    {
        //ERROR_INSTALL_BOOTCODE,
        "Nu s-a reu�it instalarea codului FAT de ini�ializare\n"
		"pe parti�ia de sistem.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_NO_FLOPPY,
        "Nu exist� discuri flexibile �n unitatea A:",
        "ENTER = Continuare"
    },
    {
        //ERROR_UPDATE_KBSETTINGS,
        "A e�uat actualizarea configura�iei de tastatur�.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_UPDATE_DISPLAY_SETTINGS,
        "Nu s-a reu�it actualizarea registrului cu\n"
		"parametrii grafici ai ecranului!",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_IMPORT_HIVE,
		"Nu s-a reu�it importarea registrului.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_FIND_REGISTRY
        "Fi�ierele cu datele registrului\n"
		"nu au putut fi localizate.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_CREATE_HIVE,
        "Crearea registrului local a e�uat!",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_INITIALIZE_REGISTRY,
        "Ini�ializarea registrului a e�uat.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_INVALID_CABINET_INF,
        "Fi�ierul cabinet nu con�ine nici un fi�ier\n"
		"valid de tip inf.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_CABINET_MISSING,
        "Fi�ierul cabinet nu e g�sit.\n",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_CABINET_SCRIPT,
        "Fi�ierul cabinet nu con�ine nici un script\n"
		"de instalare.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_COPY_QUEUE,
        "Nu se poate deschide lista de fi�iere\n"
		"pentru copiere.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_CREATE_DIR,
        "Nu se pot crea directoarele de instalare.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_TXTSETUP_SECTION,
        "Nu se poate g�si sec�iunea de directoare\n"
		"�n fi�ierul TXTSETUP.SIF.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_CABINET_SECTION,
        "Nu se poate g�si sec�iunea de directoare\n"
		"�n fi�ierul cabinet.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_CREATE_INSTALL_DIR
        "Nu se poate crea directorul de instalare.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_FIND_SETUPDATA,
        "Nu se poate g�si sec�iunea pentru date de\n"
		"instalare din fi�ierul TXTSETUP.SIF.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_WRITE_PTABLE,
        "A e�uat scrierea tabelelor de parti�ii.\n",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_ADDING_CODEPAGE,
        "A e�uat includerea pagin�rii �n registre.\n",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_UPDATE_LOCALESETTINGS,
        "Nu se poate seta sistemul de localizare.\n",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_ADDING_KBLAYOUTS,
        "A e�uat includerea �n registre a configura�iei\n"
		"de tastatur�.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_UPDATE_GEOID,
        "Nu s-a reu�it setarea geo id.",
        "ENTER = Repornire calculator"
    },
    {
        //ERROR_INSUFFICIENT_DISKSPACE,
        "Pe parti�ia selectat� nu exist� suficient\n"
		"spa�iu liber."
        "  * Tasta�i pentru a continua.",
        NULL
    },
    {
        NULL,
        NULL
    }
};

MUI_PAGE roROPages[] =
{
    {
        LANGUAGE_PAGE,
        roROLanguagePageEntries
    },
    {
        START_PAGE,
        roROWelcomePageEntries
    },
    {
        INSTALL_INTRO_PAGE,
        roROIntroPageEntries
    },
    {
        LICENSE_PAGE,
        roROLicensePageEntries
    },
    {
        DEVICE_SETTINGS_PAGE,
        roRODevicePageEntries
    },
    {
        REPAIR_INTRO_PAGE,
        roRORepairPageEntries
    },
    {
        COMPUTER_SETTINGS_PAGE,
        roROComputerPageEntries
    },
    {
        DISPLAY_SETTINGS_PAGE,
        roRODisplayPageEntries
    },
    {
        FLUSH_PAGE,
        roROFlushPageEntries
    },
    {
        SELECT_PARTITION_PAGE,
        roROSelectPartitionEntries
    },
    {
        SELECT_FILE_SYSTEM_PAGE,
        roROSelectFSEntries
    },
    {
        FORMAT_PARTITION_PAGE,
        roROFormatPartitionEntries
    },
    {
        DELETE_PARTITION_PAGE,
        roRODeletePartitionEntries
    },
    {
        INSTALL_DIRECTORY_PAGE,
        roROInstallDirectoryEntries
    },
    {
        PREPARE_COPY_PAGE,
        roROPrepareCopyEntries
    },
    {
        FILE_COPY_PAGE,
        roROFileCopyEntries
    },
    {
        KEYBOARD_SETTINGS_PAGE,
        roROKeyboardSettingsEntries
    },
    {
        BOOT_LOADER_PAGE,
        roROBootLoaderEntries
    },
    {
        LAYOUT_SETTINGS_PAGE,
        roROLayoutSettingsEntries
    },
    {
        QUIT_PAGE,
        roROQuitPageEntries
    },
    {
        SUCCESS_PAGE,
        roROSuccessPageEntries
    },
    {
        BOOT_LOADER_FLOPPY_PAGE,
        roROBootPageEntries
    },
    {
        REGISTRY_PAGE,
        roRORegistryEntries
    },
    {
        -1,
        NULL
    }
};

MUI_STRING roROStrings[] =
{
    {STRING_PLEASEWAIT,
     "   V� rug�m s� a�tepta�i..."},
    {STRING_INSTALLCREATEPARTITION,
     "   ENTER = Instalare   C = Creare parti�ie   F3 = Ie�ire"},
    {STRING_INSTALLDELETEPARTITION,
     "   ENTER = Instalare   D = �tergere parti�ie   F3 = Ie�ire"},
    {STRING_PARTITIONSIZE,
     "M�rimea noii parti�ii:"},
    {STRING_CHOOSENEWPARTITION,
     "A�i ales crearea unei noi parti�ii pe"},
    {STRING_HDDSIZE,
    "V� rug�m s� introduce�i m�rimea noii parti�ii �n megaocte�i."},
    {STRING_CREATEPARTITION,
     "   ENTER = Creare parti�ie   ESC = Anulare   F3 = Ie�ire"},
    {STRING_PARTFORMAT,
    "Aceast� Parti�ie urmeaz� s� fie formatat�."},
    {STRING_NONFORMATTEDPART,
    "Alege�i s� instala�i Odyssey pe parti�ie nou� sau neformatat�."},
    {STRING_INSTALLONPART,
    "Instalare Odyssey pe parti�ia"},
    {STRING_CHECKINGPART,
    "Instalatorul verific� acum parti�ia aleas�."},
    {STRING_QUITCONTINUE,
    "F3= Ie�ire  ENTER = Continuare"},
    {STRING_REBOOTCOMPUTER,
    "ENTER = Repornire calculator"},
    {STRING_TXTSETUPFAILED,
    "Nu s-a reu�it g�sirea sesiunii\n'%S' �n TXTSETUP.SIF.\n"},
    {STRING_COPYING,
     "   Copiere fi�ier: %S"},
    {STRING_SETUPCOPYINGFILES,
     "Copiere fi�iere..."},
    {STRING_REGHIVEUPDATE,
    "   Actualizare registrului..."},
    {STRING_IMPORTFILE,
    "   Importare %S..."},
    {STRING_DISPLAYETTINGSUPDATE,
    "   Actualizarea �n registru a configura�iei grafice..."},
    {STRING_LOCALESETTINGSUPDATE,
    "   Actualizarea particularit��ilor locale..."},
    {STRING_KEYBOARDSETTINGSUPDATE,
    "   Actualizarea configura�iei tastaturii..."},
    {STRING_CODEPAGEINFOUPDATE,
    "   Ad�ugarea �n registru a datelor de paginare..."},
    {STRING_DONE,
    "   Terminat..."},
    {STRING_REBOOTCOMPUTER2,
    "   ENTER = Repornire calculator"},
    {STRING_CONSOLEFAIL1,
    "Nu se poate deschide consola\n\n"},
    {STRING_CONSOLEFAIL2,
    "Cea mai frecvent� cauz� pentru asta este utilizarea unei tastaturi USB\n"},
    {STRING_CONSOLEFAIL3,
    "Tastaturile USB �nc� nu sunt complet suportate\n"},
    {STRING_FORMATTINGDISK,
    "Formatarea discului"},
    {STRING_CHECKINGDISK,
    "Verificarea discului"},
    {STRING_FORMATDISK1,
    " Formatarea parti�iei ca sistem de fi�iere %S (formatare rapid�) "},
    {STRING_FORMATDISK2,
    " Formatarea parti�iei ca sistem de fi�iere %S "},
    {STRING_KEEPFORMAT,
    " P�streaz� sistemul de fi�iere actual (f�r� schimb�ri) "},
    {STRING_HDINFOPARTCREATE,
    "%I64u %s  Discul %lu  (Port=%hu, Magistrala=%hu, Id=%hu) de tip %wZ."},
    {STRING_HDDINFOUNK1,
    "%I64u %s  Discul %lu  (Port=%hu, Magistrala=%hu, Id=%hu)."},
    {STRING_HDDINFOUNK2,
    "   %c%c  Tip %lu    %I64u %s"},
    {STRING_HDINFOPARTDELETE,
    "de pe %I64u %s  Discul %lu  (Port=%hu, Magistrala=%hu, Id=%hu) de tip %wZ."},
    {STRING_HDDINFOUNK3,
    "de pe %I64u %s  Discul %lu  (Port=%hu, Magistrala=%hu, Id=%hu)."},
    {STRING_HDINFOPARTZEROED,
    "Discul %lu (%I64u %s), Port=%hu, Magistrala=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK4,
    "%c%c  Tip %lu    %I64u %s"},
    {STRING_HDINFOPARTEXISTS,
    "de pe Discul %lu (%I64u %s), Port=%hu, Magistrala=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK5,
    "%c%c  Tip %-3u                         %6lu %s"},
    {STRING_HDINFOPARTSELECT,
    "%6lu %s  Discul %lu  (Port=%hu, Magistrala=%hu, Id=%hu) de tip %S"},
    {STRING_HDDINFOUNK6,
    "%6lu %s  Discul %lu  (Port=%hu, Magistrala=%hu, Id=%hu)"},
    {STRING_NEWPARTITION,
    "O nou� parti�ie a fost creat� �n"},
    {STRING_UNPSPACE,
    "    Spa�iu neparti�ionat             %6lu %s"},
    {STRING_MAXSIZE,
    "MO (max. %lu MO)"},
    {STRING_UNFORMATTED,
    "Part. nou� (neformatat�)"},
    {STRING_FORMATUNUSED,
    "Nefolosit"},
    {STRING_FORMATUNKNOWN,
    "Necunoscut"},
    {STRING_KB,
    "KO"},
    {STRING_MB,
    "MO"},
    {STRING_GB,
    "GO"},
    {STRING_ADDKBLAYOUTS,
    "Ad�ugare configura�ii de tastatur�"},
    {0, 0}
};
