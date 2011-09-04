#pragma once

MUI_LAYOUTS deDELayouts[] =
{
    { L"0407", L"00000407" },
    { L"0409", L"00000409" },
    { NULL, NULL }
};

static MUI_ENTRY deDELanguagePageEntries[] =
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
        "Sprachauswahl.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Bitte w�hlen Sie die Sprache, die Sie w�hrend des Setups verwenden wollen.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Dann dr�cken Sie ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Diese Sprache wird sp�ter als Standardsprache im System verwendet.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Fortsetzen  F3 = Beenden",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEWelcomePageEntries[] =
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
        "Willkommen zum Odyssey-Setup",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        6,
        11,
        "Dieser Teil des Setups kopiert das Odyssey-Betriebssystem auf Ihren",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "Computer und bereitet den zweiten Teil des Setups vor.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Dr�cken Sie ENTER, um Odyssey zu installieren.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Dr�cken Sie R, um Odyssey zu reparieren oder aktualisieren.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  Dr�cken Sie L, um die Lizenzabkommen von Odyssey zu lesen.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Dr�cken Sie F3, um das Setup zu beenden.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "F�r weitere Informationen besuchen Sie bitte:",
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
        "ENTER = Fortsetzen  R = Reparieren  L = Lizenz  F3 = Beenden",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEIntroPageEntries[] =
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
        "Das Odyssey-Setup ist noch in einer fr�hen Entwicklungsphase. Es unter-",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "st�tzt noch nicht alle Funktionen eines vollst�ndig nutzbaren Setups.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "Es gibt folgende Beschr�nkungen:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "- Setup kann nur eine prim�re Partition auf einer HDD verwalten.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "- Setup kann keine prim�re Partition von einer HDD l�schen,",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "  so lange erweiterte Partitionen auf dieser HDD existieren.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "- Setup kann die erste erweiterte Partition nicht von der HDD l�schen,",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "  so lange weitere erweiterte Partitionen auf dieser HDD existieren.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "- Setup unterst�tzt nur FAT-Dateisysteme.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "- Dateisystem�berpr�fung ist noch nicht implementiert.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        23,
        "\x07  Dr�cken Sie ENTER, um Odyssey zu installieren.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        25,
        "\x07  Dr�cken Sie F3, um das Setup zu beenden.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Fortsetzen   F3 = Beenden",
        TEXT_TYPE_STATUS| TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDELicensePageEntries[] =
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
        "Lizenz:",
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
        "ENTER = Zur�ck",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEDevicePageEntries[] =
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
        "Die untere Liste zeigt die derzeitigen Ger�teeinstellungen.",
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
        "Bildschirm:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        13,
        "Tastatur:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        14,
        " Tastaturlayout:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        16,
        "Akzeptieren:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        25,
        16, "Diese Ger�teeinstellungen akzeptieren",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        19,
        "Sie k�nnen die Einstellungen durch die Pfeiltasten ausw�hlen.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        20,
        "Dann dr�cken Sie die Eingabetaste, um eine Einstellung zu �ndern.",
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
        "Wenn alle Einstellungen korrekt sind, w�hlen Sie \"Diese Ger�te-",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        24,
        "einstellungen akzeptieren\" und dr�cken danach die Eingabetaste.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Fortsetzen   F3 = Beenden",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDERepairPageEntries[] =
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
        "Das Odyssey-Setup ist noch in einer fr�hen Entwicklungsphase. Es unter-",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "st�tzt noch nicht alle Funktionen eines vollst�ndig nutzbaren Setups.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "Die Reparaturfunktionen sind noch nicht implementiert.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Dr�cken Sie U, um Odyssey zu aktualisieren.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Dr�cken Sie R, f�r die Wiederherstellungskonsole.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  Dr�cken Sie ESC, um zur Hauptseite zur�ckzukehren.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Dr�cken Sie ENTER, um den Computer neu zu starten.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ESC = Hauptseite  U = Update  R = Recovery  ENTER = Neustarten",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};
static MUI_ENTRY deDEComputerPageEntries[] =
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
        "Sie wollen den Computertyp �ndern, der installiert wird.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Dr�cken Sie die HOCH- oder RUNTER-Taste, um den gew�nschten",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Typ zu w�hlen. Dann dr�cken Sie ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Dr�cken Sie ESC, um zur vorherigen Seite zur�ckzukehren,",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ohne den Computertyp zu �ndern.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Fortsetzen   ESC = Abbrechen   F3 = Beenden",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEFlushPageEntries[] =
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
        "Das System vergewissert sich, dass alle Daten gespeichert sind.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Dies kann einige Minuten in Anspruch nehmen.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "Der PC wird automatisch neustarten, wenn der Vorgang beendet ist.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Cache wird geleert",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEQuitPageEntries[] =
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
        "Odyssey wurde nicht vollst�ndig installiert",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Entfernen Sie die Diskette aus Laufwerk A: und",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "alle CD-ROMs aus den CD-Laufwerken.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "Dr�cken Sie ENTER, um den Computer neuzustarten.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Bitte warten ...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEDisplayPageEntries[] =
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
        "Sie wollen den Bildschirmtyp �ndern, der installiert wird.",
        TEXT_STYLE_NORMAL
    },
    {   8,
        10,
        "\x07  Dr�cken Sie die HOCH- oder RUNTER-Taste, um den gew�nschten",
         TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Typ zu w�hlen. Dann dr�cken Sie ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Dr�cken Sie ESC, um zur vorherigen Seite zur�ckzukehren, ohne",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   den Bildschirmtyp zu �ndern.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Fortsetzen   ESC = Abbrechen   F3 = Beenden",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDESuccessPageEntries[] =
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
        "Die Standardkomponenten von Odyssey wurden erfolgreich installiert.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Entfernen Sie die Diskette aus Laufwerk A: und",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "alle CD-ROMs aus den CD-Laufwerken.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "Dr�cken Sie ENTER, um den Computer neuzustarten.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Computer neustarten",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEBootPageEntries[] =
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
        "Das Setup kann das Boot-Sektor nicht auf der",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "Festplatte Ihres Computers installieren",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        13,
        "Bitte legen Sie eine formatierte Diskette in Laufwerk A: ein und",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "dr�cken Sie ENTER.",
        TEXT_STYLE_NORMAL,
    },
    {
        0,
        0,
        "ENTER = Fortsetzen   F3 = Beenden",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }

};

static MUI_ENTRY deDESelectPartitionEntries[] =
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
        "Diese Liste zeigt existierende Partitionen an und den freien",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "Speicherplatz f�r neue Partitionen.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "\x07  Dr�cken Sie die Pfeiltasten, um eine Partition auszuw�hlen.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Dr�cken Sie die Eingabetaste, um die Auswahl zu best�tigen.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Dr�cken Sie C, um eine neue Partition zu erstellen.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Dr�cken Sie D, um eine vorhandene Partition zu l�schen.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Bitte warten...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEFormatPartitionEntries[] =
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
        "Formatiere Partition",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        10,
        "Setup wird nun die gew�nschte Partition formatieren.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        11,
        "Dr�cken Sie die Eingabetaste, um fortzufahren.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Fortfahren   F3 = Beenden",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        TEXT_STYLE_NORMAL
    }
};

static MUI_ENTRY deDEInstallDirectoryEntries[] =
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
        "Setup installiert die Odyssey-Installationsdateien in die ausgew�hlte",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "Partition. W�hlen Sie ein Installationsverzeichnis f�r Odyssey:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "Um den Vorschlag zu �ndern dr�cken sie die 'Entf'-Taste, um",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        15,
        "Zeichen zu l�schen, und geben sie dann den Namen des Verzeichnis ein",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Fortfahren   F3 = Beenden",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEFileCopyEntries[] =
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
        "Bitte warten Sie, w�hrend das Odyssey-Setup die Odyssey-Dateien",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        13,
        "in das Installationsverzeichnis kopiert.",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        14,
        "Dieser Vorgang kann mehrere Minuten in Anspruch nehmen.",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        50,
        0,
        "\xB3 Bitte warten...    ",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEBootLoaderEntries[] =
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
        "Setup installiert nun den Boot-Loader.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "Boot-Loader auf der Festplatte installieren (MBR und VBR).",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "Boot-Loader auf der Festplatte installieren (nur VBR).",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "Boot-Loader auf einer Diskette installieren.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "Boot-Loader nicht installieren.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Fortfahren   F3 = Abbrechen",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEKeyboardSettingsEntries[] =
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
        "Sie wollen den Tastaturtyp �ndern, der installiert wird.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Dr�cken Sie die HOCH- oder RUNTER-Taste, um den gew�nschten",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Typ zu w�hlen. Dann dr�cken Sie ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Dr�cken Sie ESC, um zur vorherigen Seite zur�ckzukehren,",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ohne den Tastaturtyp zu �ndern.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Fortfahren   ESC = Abbrechen   F3 = Beenden",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDELayoutSettingsEntries[] =
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
        "Bitte w�hlen Sie ein zu installierendes Standardlayout aus.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Dr�cken Sie die HOCH- oder RUNTER-Taste, um den gew�nschten",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Typ zu w�hlen. Dann dr�cken Sie ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Dr�cken Sie ESC, um zur vorherigen Seite zur�ckzukehren,",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ohne das Tastaturlayout zu �ndern.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Fortfahren   ESC = Abbrechen   F3 = Beenden",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY deDEPrepareCopyEntries[] =
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
        "Setup bereitet ihren Computer f�r die Installation vor.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Erstelle Liste der zu kopierenden Dateien...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY deDESelectFSEntries[] =
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
        "W�hlen Sie ein Dateisystem aus der folgenden Liste aus.",
        0
    },
    {
        8,
        19,
        "\x07  Dr�cken Sie die Pfeiltasten, um das Dateisystem zu �ndern.",
        0
    },
    {
        8,
        21,
        "\x07  Dr�cken Sie die Eingabetaste, um die Partition zu formatieren.",
        0
    },
    {
        8,
        23,
        "\x07  Dr�cken Sie ESC, um eine andere Partition auszuw�hlen.",
        0
    },
    {
        0,
        0,
        "ENTER = Fortfahren   ESC = Abbrechen   F3 = Beenden",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },

    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEDeletePartitionEntries[] =
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
        "Sie haben sich entschieden, diese Partition zu l�schen",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "\x07  Dr�cken Sie D, um die Partition zu l�schen.",
        TEXT_STYLE_NORMAL
    },
    {
        11,
        19,
        "Warnung: Alle Daten auf dieser Partition werden gel�scht!",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  ESC um abzubrechen.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "D = L�sche Partition   ESC = Abbrechen   F3 = Beenden",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDERegistryEntries[] =
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
        "Setup aktualisiert die Systemkonfiguration. ",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Registryzweige erstellen...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

MUI_ERROR deDEErrorEntries[] =
{
    {
        //ERROR_NOT_INSTALLED
        "Odyssey ist nicht vollst�ndig auf Ihrem System installiert.\n"
        "Wenn Sie das Setup jetzt beenden, m�ssen Sie das\n"
        "Setup erneut starten, um ROS zu installieren.\n"
        "\n"
        "  \x07  Dr�cken Sie ENTER um das Setup Fortzusetzen.\n"
        "  \x07  Dr�cken Sie F3 um das Setup zu beenden.",
        "F3 = Beenden  ENTER = Fortsetzen"
    },
    {
        //ERROR_NO_HDD
        "Setup konnte keine Festplatte finden.\n",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_NO_SOURCE_DRIVE
        "Setup konnte das Quelllaufwerk nicht finden.\n",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_LOAD_TXTSETUPSIF
        "Setup konnte TXTSETUP.SIF nicht finden.\n",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_CORRUPT_TXTSETUPSIF
        "Setup fand eine defekte TXTSETUP.SIF.\n",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_SIGNATURE_TXTSETUPSIF,
        "Setup fand eine ung�ltige Signatur in TXTSETUP.SIF.\n",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_DRIVE_INFORMATION
        "Setup konnte keine Laufwerksinformationen abfragen.\n",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_WRITE_BOOT,
        "Setup konnte den FAT-Bootcode nicht auf der Partition installieren.",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_LOAD_COMPUTER,
        "Setup konnte die Computertypenliste nicht laden.\n",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_LOAD_DISPLAY,
        "Setup konnte die Displayeinstellungsliste nicht laden.\n",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_LOAD_KEYBOARD,
        "Setup konnte die Tastaturtypenliste nicht laden.\n",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_LOAD_KBLAYOUT,
        "Setup konnte die Tastaturlayoutliste nicht laden.\n",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_WARN_PARTITION,
        "Setup hat mindestens eine Festplatte mit einer inkompatiblen Partitionstabelle\n"
        "gefunden, die nicht richtig verwendet werden kann!\n"
        "\n"
        "Partitionen zu erstellen/l�schen kann die Partitionstabelle zerst�ren.\n"
        "\n"
        "  \x07  Dr�cken Sie F3, um das Setup zu beenden."
        "  \x07  Dr�cken Sie ENTER, um das Setup Fortzusetzen.",
        "F3 = Beenden  ENTER = Fortsetzen"
    },
    {
        //ERROR_NEW_PARTITION,
        "Sie k�nnen keine neue Partition in einer bereits\n"
        "vohandenen Partition erstellen!\n"
        "\n"
        "  * * Eine beliebige Taste zum Fortsetzen dr�cken.",
        NULL
    },
    {
        //ERROR_DELETE_SPACE,
        "Sie k�nnen unpartitionieren Speicher nicht l�schen!\n"
        "\n"
        "  * Eine beliebige Taste zum Fortsetzen dr�cken.",
        NULL
    },
    {
        //ERROR_INSTALL_BOOTCODE,
        "Setup konnte den FAT-Bootcode nicht auf der Partition installieren.",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_NO_FLOPPY,
        "Keine Diskette in Laufwerk A:.",
        "ENTER = Fortsetzen"
    },
    {
        //ERROR_UPDATE_KBSETTINGS,
        "Setup konnte das Tastaturlayout nicht aktualisieren.",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_UPDATE_DISPLAY_SETTINGS,
        "Setup konnte die Display-Registrywerte nicht aktualisieren.",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_IMPORT_HIVE,
        "Setup konnte keine Hive-Datei importieren.",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_FIND_REGISTRY
        "Setup konnte die Registrydateien nicht finden.",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_CREATE_HIVE,
        "Setup konnte die Registryzweige nicht erstellen.",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_INITIALIZE_REGISTRY,
        "Setup konnte die Registry nicht initialisieren.",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_INVALID_CABINET_INF,
        "Cabinet hat keine g�ltige .inf-Datei.\n",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_CABINET_MISSING,
        "Cabinet nicht gefunden.\n",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_CABINET_SCRIPT,
        "Cabinet enth�lt kein Setup-Skript.\n",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_COPY_QUEUE,
        "Setup konnte die Liste mit zu kopierenden Dateien nicht finden.\n",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_CREATE_DIR,
        "Setup konnte die Installationspfade nicht erstellen.",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_TXTSETUP_SECTION,
        "Setup konnte die 'Ordner'-Sektion in\n"
        "TXTSETUP.SIF nicht finden.\n",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_CABINET_SECTION,
        "Setup konnte die 'Ordner'-Sektion im\n"
        "Cabinet nicht finden.\n",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_CREATE_INSTALL_DIR
        "Setup konnte den Installationspfad nicht erstellen.",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_FIND_SETUPDATA,
        "Setup konnte die 'SetupData'-Sektion in\n"
        "TXTSETUP.SIF nicht finden.\n",
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_WRITE_PTABLE,
        "Setup konnte die Partitionstabellen nicht schreiben.\n"
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_ADDING_CODEPAGE,
        "Setup konnte den Codepage-Eintrag nicht hinzuf�gen.\n"
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_UPDATE_LOCALESETTINGS,
        "Setup konnte die Systemsprache nicht einstellen.\n"
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_ADDING_KBLAYOUTS,
        "Setup konnte die Tastaturlayouts nicht in der Registry eintragen.\n"
        "ENTER = Computer neu starten"
    },
        {
        //ERROR_UPDATE_GEOID,
        "Setup konnte den geografischen Standort nicht einstellen.\n"
        "ENTER = Computer neu starten"
    },
    {
        //ERROR_INSUFFICIENT_DISKSPACE,
        "Nicht genug Speicherplatz auf der gew�hlten Partition vorhanden.\n"
        "  * Eine beliebige Taste zum Fortsetzen dr�cken.",
        NULL
    },
    {
        NULL,
        NULL
    }
};


MUI_PAGE deDEPages[] =
{
    {
        LANGUAGE_PAGE,
        deDELanguagePageEntries
    },
    {
        START_PAGE,
        deDEWelcomePageEntries
    },
    {
        INSTALL_INTRO_PAGE,
        deDEIntroPageEntries
    },
    {
        LICENSE_PAGE,
        deDELicensePageEntries
    },
    {
        DEVICE_SETTINGS_PAGE,
        deDEDevicePageEntries
    },
    {
        REPAIR_INTRO_PAGE,
        deDERepairPageEntries
    },
    {
        COMPUTER_SETTINGS_PAGE,
        deDEComputerPageEntries
    },
    {
        DISPLAY_SETTINGS_PAGE,
        deDEDisplayPageEntries
    },
    {
        FLUSH_PAGE,
        deDEFlushPageEntries
    },
    {
        SELECT_PARTITION_PAGE,
        deDESelectPartitionEntries
    },
    {
        SELECT_FILE_SYSTEM_PAGE,
        deDESelectFSEntries
    },
    {
        FORMAT_PARTITION_PAGE,
        deDEFormatPartitionEntries
    },
    {
        DELETE_PARTITION_PAGE,
        deDEDeletePartitionEntries
    },
    {
        INSTALL_DIRECTORY_PAGE,
        deDEInstallDirectoryEntries
    },
    {
        PREPARE_COPY_PAGE,
        deDEPrepareCopyEntries
    },
    {
        FILE_COPY_PAGE,
        deDEFileCopyEntries
    },
    {
        KEYBOARD_SETTINGS_PAGE,
        deDEKeyboardSettingsEntries
    },
    {
        BOOT_LOADER_PAGE,
        deDEBootLoaderEntries
    },
    {
        LAYOUT_SETTINGS_PAGE,
        deDELayoutSettingsEntries
    },
    {
        QUIT_PAGE,
        deDEQuitPageEntries
    },
    {
        SUCCESS_PAGE,
        deDESuccessPageEntries
    },
    {
        BOOT_LOADER_FLOPPY_PAGE,
        deDEBootPageEntries
    },
    {
        REGISTRY_PAGE,
        deDERegistryEntries
    },
    {
        -1,
        NULL
    }
};

MUI_STRING deDEStrings[] =
{
    {STRING_PLEASEWAIT,
     "   Bitte warten..."},
    {STRING_INSTALLCREATEPARTITION,
     "   ENTER = Installieren   C = Partition erstellen  F3 = Beenden"},
    {STRING_INSTALLDELETEPARTITION,
     "   ENTER = Installieren   D = Partition l�schen    F3 = Beenden"},
    {STRING_PARTITIONSIZE,
     "Gr��e der neuen Partition:"},
    {STRING_CHOOSENEWPARTITION,
     "Sie haben beschlossen, eine neue Partition zu erstellen auf"},
    {STRING_HDDSIZE,
    "Bitte geben Sie die Gr��e der neuen Partition in Megabyte ein."},
    {STRING_CREATEPARTITION,
     "   ENTER = Partition erstellen   ESC = Abbruch   F3 = Beenden"},
    {STRING_PARTFORMAT,
    "Diese Partition wird als n�chstes formatiert."},
    {STRING_NONFORMATTEDPART,
    "Sie wollen Odyssey auf einer neuen/unformatieren Partition installieren."},
    {STRING_INSTALLONPART,
    "Setup installiert Odyssey auf dieser Partition"},
    {STRING_CHECKINGPART,
    "Setup �berpr�ft die ausgew�hlte Partition."},
    {STRING_QUITCONTINUE,
    "F3 = Beenden  ENTER = Fortsetzen"},
    {STRING_REBOOTCOMPUTER,
    "ENTER = Computer neu starten"},
    {STRING_TXTSETUPFAILED,
    "Setup konnte die '%S'-Sektion\nin TXTSETUP.SIF nicht finden.\n"},
    {STRING_COPYING,
     "   Kopiere Datei: %S"},
    {STRING_SETUPCOPYINGFILES,
     "Setup kopiert Dateien..."},
    {STRING_REGHIVEUPDATE,
    "   Registryzweige werden aktualisiert..."},
    {STRING_IMPORTFILE,
    "   Importiere %S..."},
    {STRING_DISPLAYETTINGSUPDATE,
    "   Bildschirm-Registryeinstellungen werden aktualisiert..."},
    {STRING_LOCALESETTINGSUPDATE,
    "   Lokalisierungseinstellungen werden aktualisiert..."},
    {STRING_KEYBOARDSETTINGSUPDATE,
    "   Tastaturlayouteinstellungen werden aktualisiert..."},
    {STRING_CODEPAGEINFOUPDATE,
    "   Codepage-Informationen werden hinzugef�gt..."},
    {STRING_DONE,
    "   Fertig..."},
    {STRING_REBOOTCOMPUTER2,
    "   ENTER = Computer neu starten"},
    {STRING_CONSOLEFAIL1,
    "Konsole konnte nicht ge�ffnet werden\n\n"},
    {STRING_CONSOLEFAIL2,
    "Der h�ufigste Grund hierf�r ist die Verwendung einer USB-Tastatur\n"},
    {STRING_CONSOLEFAIL3,
    "USB-Tastaturen werden noch nicht vollst�ndig unterst�tzt\n"},
    {STRING_FORMATTINGDISK,
    "Setup formatiert Ihre Festplatte"},
    {STRING_CHECKINGDISK,
    "Setup �berpr�ft Ihre Festplatte"},
    {STRING_FORMATDISK1,
    " Formatiere Partition als %S-Dateisystem (Schnell) "},
    {STRING_FORMATDISK2,
    " Formatiere Partition als %S-Dateisystem "},
    {STRING_KEEPFORMAT,
    " Dateisystem beibehalten (Keine Ver�nderungen) "},
    {STRING_HDINFOPARTCREATE,
    "%I64u %s  Festplatte %lu  (Port=%hu, Bus=%hu, Id=%hu) auf %wZ."},
    {STRING_HDDINFOUNK1,
    "%I64u %s  Festplatte %lu  (Port=%hu, Bus=%hu, Id=%hu)."},
    {STRING_HDDINFOUNK2,
    "   %c%c  Typ %lu    %I64u %s"},
    {STRING_HDINFOPARTDELETE,
    "auf %I64u %s  Festplatte %lu  (Port=%hu, Bus=%hu, Id=%hu) auf %wZ."},
    {STRING_HDDINFOUNK3,
    "auf %I64u %s  Festplatte %lu  (Port=%hu, Bus=%hu, Id=%hu)."},
    {STRING_HDINFOPARTZEROED,
    "Festplatte %lu (%I64u %s), Port=%hu, Bus=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK4,
    "%c%c  Typ %lu    %I64u %s"},
    {STRING_HDINFOPARTEXISTS,
    "auf Festplatte %lu (%I64u %s), Port=%hu, Bus=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK5,
    "%c%c  Typ %-3u                         %6lu %s"},
    {STRING_HDINFOPARTSELECT,
    "%6lu %s  Festplatte %lu  (Port=%hu, Bus=%hu, Id=%hu) auf %S"},
    {STRING_HDDINFOUNK6,
    "%6lu %s  Festplatte %lu  (Port=%hu, Bus=%hu, Id=%hu)"},
    {STRING_NEWPARTITION,
    "Setup erstellte eine neue Partition auf"},
    {STRING_UNPSPACE,
    "    Unpartitionierter Speicher       %6lu %s"},
    {STRING_MAXSIZE,
    "MB (max. %lu MB)"},
    {STRING_UNFORMATTED,
    "Neu (Unformatiert)"},
    {STRING_FORMATUNUSED,
    "Ungenutzt"},
    {STRING_FORMATUNKNOWN,
    "Unbekannt"},
    {STRING_KB,
    "KB"},
    {STRING_MB,
    "MB"},
    {STRING_GB,
    "GB"},
    {STRING_ADDKBLAYOUTS,
    "Tastaturlayout hinzuf�gen"},
    {0, 0}
};
