#pragma once

MUI_LAYOUTS enUSLayouts[] =
{
    { L"0409", L"00000409" },
    { NULL, NULL }
};

static MUI_ENTRY enUSLanguagePageEntries[] =
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
        "Language Selection",
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
        "   Then press ENTER.",
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
        "ENTER = Continue  F3 = Quit",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY enUSWelcomePageEntries[] =
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
        "Welcome to Odyssey Setup",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        6,
        11,
        "This part of the setup copies the Odyssey Operating System to your",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "computer and prepares the second part of the setup.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Press ENTER to install Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Press R to repair or update Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  Press L to view the Odyssey Licensing Terms and Conditions.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Press F3 to quit without installing Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "For more information on Odyssey, please visit:",
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
        "ENTER = Continue  R = Repair  L = Licence  F3 = Quit",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY enUSIntroPageEntries[] =
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
        "Odyssey Setup is in an early development phase. It does not yet",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "support all the functions of a fully usable setup application.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "The following limitations apply:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "- Setup can not handle more than one primary partition per disk.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "- Setup can not delete a primary partition from a disk",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "  as long as extended partitions exist on this disk.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "- Setup can not delete the first extended partition from a disk",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "  as long as other extended partitions exist on this disk.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "- Setup supports FAT file systems only.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "- File system checks are not implemented yet.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        23,
        "\x07  Press ENTER to install Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        25,
        "\x07  Press F3 to quit without installing Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Continue   F3 = Quit",
        TEXT_TYPE_STATUS| TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY enUSLicensePageEntries[] =
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
        "Licensing:",
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
        "ENTER = Return",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY enUSDevicePageEntries[] =
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
        "The list below shows the current device settings.",
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
        "Display:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        13,
        "Keyboard:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        14,
        "Keyboard layout:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        16,
        "Accept:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        25,
        16, "Accept these device settings",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        19,
        "You can change the hardware settings by pressing the UP or DOWN keys",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        20,
        "to select an entry. Then press the ENTER key to select alternative",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        21,
        "settings.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "When all settings are correct, select \"Accept these device settings\"",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        24,
        "and press ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Continue   F3 = Quit",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY enUSRepairPageEntries[] =
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
        "Odyssey Setup is in an early development phase. It does not yet",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "support all the functions of a fully usable setup application.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "The repair functions are not implemented yet.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Press U for Updating OS.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Press R for the Recovery Console.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  Press ESC to return to the main page.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Press ENTER to reboot your computer.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ESC = Main page  U = Update  R = Recovery  ENTER = Reboot",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};
static MUI_ENTRY enUSComputerPageEntries[] =
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
        "You want to change the type of computer to be installed.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Press the UP or DOWN key to select the desired computer type.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Then press ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Press the ESC key to return to the previous page without changing",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   the computer type.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Continue   ESC = Cancel   F3 = Quit",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY enUSFlushPageEntries[] =
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
        "The system is now making sure all data is stored on your disk",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "This may take a minute",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "When finished, your computer will reboot automatically",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Flushing cache",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY enUSQuitPageEntries[] =
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
        "Odyssey is not completely installed",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Remove floppy disk from Drive A: and",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "all CD-ROMs from CD-Drives.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "Press ENTER to reboot your computer.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Please wait ...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG,
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY enUSDisplayPageEntries[] =
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
        "You want to change the type of display to be installed.",
        TEXT_STYLE_NORMAL
    },
    {   8,
        10,
         "\x07  Press the UP or DOWN key to select the desired display type.",
         TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Then press ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Press the ESC key to return to the previous page without changing",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   the display type.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Continue   ESC = Cancel   F3 = Quit",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY enUSSuccessPageEntries[] =
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
        "The basic components of Odyssey have been installed successfully.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Remove floppy disk from Drive A: and",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "all CD-ROMs from CD-Drive.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "Press ENTER to reboot your computer.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Reboot computer",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY enUSBootPageEntries[] =
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
        "Setup cannot install the bootloader on your computers",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "hardisk",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        13,
        "Please insert a formatted floppy disk in drive A: and",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "press ENTER.",
        TEXT_STYLE_NORMAL,
    },
    {
        0,
        0,
        "ENTER = Continue   F3 = Quit",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }

};

static MUI_ENTRY enUSSelectPartitionEntries[] =
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
        "The list below shows existing partitions and unused disk",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "space for new partitions.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "\x07  Press UP or DOWN to select a list entry.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Press ENTER to install Odyssey onto the selected partition.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Press C to create a new partition.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Press D to delete an existing partition.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Please wait...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY enUSFormatPartitionEntries[] =
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
        "Format partition",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        10,
        "Setup will now format the partition. Press ENTER to continue.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Continue   F3 = Quit",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        TEXT_STYLE_NORMAL
    }
};

static MUI_ENTRY enUSInstallDirectoryEntries[] =
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
        "Setup installs Odyssey files onto the selected partition. Choose a",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "directory where you want Odyssey to be installed:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "To change the suggested directory, press BACKSPACE to delete",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        15,
        "characters and then type the directory where you want Odyssey to",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        16,
        "be installed.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Continue   F3 = Quit",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY enUSFileCopyEntries[] =
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
        "Please wait while Odyssey Setup copies files to your Odyssey",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        13,
        "installation folder.",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        14,
        "This may take several minutes to complete.",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        50,
        0,
        "\xB3 Please wait...    ",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY enUSBootLoaderEntries[] =
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
        "Setup is installing the boot loader",
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
        "Install bootloader on a floppy disk.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "Skip install bootloader.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Continue   F3 = Quit",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY enUSKeyboardSettingsEntries[] =
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
        "You want to change the type of keyboard to be installed.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Press the UP or DOWN key to select the desired keyboard type.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Then press ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Press the ESC key to return to the previous page without changing",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   the keyboard type.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Continue   ESC = Cancel   F3 = Quit",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY enUSLayoutSettingsEntries[] =
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
        "Please select a layout to be installed by default.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Press the UP or DOWN key to select the desired keyboard",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "    layout. Then press ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Press the ESC key to return to the previous page without changing",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   the keyboard layout.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = Continue   ESC = Cancel   F3 = Quit",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY enUSPrepareCopyEntries[] =
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
        "Setup prepares your computer for copying the Odyssey files. ",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Building the file copy list...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY enUSSelectFSEntries[] =
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
        "Select a file system from the list below.",
        0
    },
    {
        8,
        19,
        "\x07  Press UP or DOWN to select a file system.",
        0
    },
    {
        8,
        21,
        "\x07  Press ENTER to format the partition.",
        0
    },
    {
        8,
        23,
        "\x07  Press ESC to select another partition.",
        0
    },
    {
        0,
        0,
        "ENTER = Continue   ESC = Cancel   F3 = Quit",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },

    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY enUSDeletePartitionEntries[] =
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
        "You have chosen to delete the partition",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "\x07  Press D to delete the partition.",
        TEXT_STYLE_NORMAL
    },
    {
        11,
        19,
        "WARNING: All data on this partition will be lost!",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Press ESC to cancel.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "D = Delete Partition   ESC = Cancel   F3 = Quit",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY enUSRegistryEntries[] =
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
        "Setup is updating the system configuration. ",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Creating registry hives...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

MUI_ERROR enUSErrorEntries[] =
{
    {
        //ERROR_NOT_INSTALLED
        "Odyssey is not completely installed on your\n"
        "computer. If you quit Setup now, you will need to\n"
        "run Setup again to install Odyssey.\n"
        "\n"
        "  \x07  Press ENTER to continue Setup.\n"
        "  \x07  Press F3 to quit Setup.",
        "F3 = Quit  ENTER = Continue"
    },
    {
        //ERROR_NO_HDD
        "Setup could not find a harddisk.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_NO_SOURCE_DRIVE
        "Setup could not find its source drive.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_LOAD_TXTSETUPSIF
        "Setup failed to load the file TXTSETUP.SIF.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_CORRUPT_TXTSETUPSIF
        "Setup found a corrupt TXTSETUP.SIF.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_SIGNATURE_TXTSETUPSIF,
        "Setup found an invalid signature in TXTSETUP.SIF.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_DRIVE_INFORMATION
        "Setup could not retrieve system drive information.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_WRITE_BOOT,
        "Setup failed to install FAT bootcode on the system partition.",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_LOAD_COMPUTER,
        "Setup failed to load the computer type list.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_LOAD_DISPLAY,
        "Setup failed to load the display settings list.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_LOAD_KEYBOARD,
        "Setup failed to load the keyboard type list.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_LOAD_KBLAYOUT,
        "Setup failed to load the keyboard layout list.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_WARN_PARTITION,
          "Setup found that at least one harddisk contains an incompatible\n"
          "partition table that can not be handled properly!\n"
          "\n"
          "Creating or deleting partitions can destroy the partition table.\n"
          "\n"
          "  \x07  Press F3 to quit Setup.\n"
          "  \x07  Press ENTER to continue.",
          "F3= Quit  ENTER = Continue"
    },
    {
        //ERROR_NEW_PARTITION,
        "You can not create a new Partition inside\n"
        "of an already existing Partition!\n"
        "\n"
        "  * Press any key to continue.",
        NULL
    },
    {
        //ERROR_DELETE_SPACE,
        "You can not delete unpartitioned disk space!\n"
        "\n"
        "  * Press any key to continue.",
        NULL
    },
    {
        //ERROR_INSTALL_BOOTCODE,
        "Setup failed to install the FAT bootcode on the system partition.",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_NO_FLOPPY,
        "No disk in drive A:.",
        "ENTER = Continue"
    },
    {
        //ERROR_UPDATE_KBSETTINGS,
        "Setup failed to update keyboard layout settings.",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_UPDATE_DISPLAY_SETTINGS,
        "Setup failed to update display registry settings.",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_IMPORT_HIVE,
        "Setup failed to import a hive file.",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_FIND_REGISTRY
        "Setup failed to find the registry data files.",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_CREATE_HIVE,
        "Setup failed to create the registry hives.",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_INITIALIZE_REGISTRY,
        "Setup failed to initialize the registry.",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_INVALID_CABINET_INF,
        "Cabinet has no valid inf file.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_CABINET_MISSING,
        "Cabinet not found.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_CABINET_SCRIPT,
        "Cabinet has no setup script.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_COPY_QUEUE,
        "Setup failed to open the copy file queue.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_CREATE_DIR,
        "Setup could not create install directories.",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_TXTSETUP_SECTION,
        "Setup failed to find the 'Directories' section\n"
        "in TXTSETUP.SIF.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_CABINET_SECTION,
        "Setup failed to find the 'Directories' section\n"
        "in the cabinet.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_CREATE_INSTALL_DIR
        "Setup could not create the install directory.",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_FIND_SETUPDATA,
        "Setup failed to find the 'SetupData' section\n"
        "in TXTSETUP.SIF.\n",
        "ENTER = Reboot computer"
    },
    {
        //ERROR_WRITE_PTABLE,
        "Setup failed to write partition tables.\n"
        "ENTER = Reboot computer"
    },
    {
        //ERROR_ADDING_CODEPAGE,
        "Setup failed to add codepage to registry.\n"
        "ENTER = Reboot computer"
    },
    {
        //ERROR_UPDATE_LOCALESETTINGS,
        "Setup could not set the system locale.\n"
        "ENTER = Reboot computer"
    },
    {
        //ERROR_ADDING_KBLAYOUTS,
        "Setup failed to add keyboard layouts to registry.\n"
        "ENTER = Reboot computer"
    },
    {
        //ERROR_UPDATE_GEOID,
        "Setup could not set the geo id.\n"
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

MUI_PAGE enUSPages[] =
{
    {
        LANGUAGE_PAGE,
        enUSLanguagePageEntries
    },
    {
        START_PAGE,
        enUSWelcomePageEntries
    },
    {
        INSTALL_INTRO_PAGE,
        enUSIntroPageEntries
    },
    {
        LICENSE_PAGE,
        enUSLicensePageEntries
    },
    {
        DEVICE_SETTINGS_PAGE,
        enUSDevicePageEntries
    },
    {
        REPAIR_INTRO_PAGE,
        enUSRepairPageEntries
    },
    {
        COMPUTER_SETTINGS_PAGE,
        enUSComputerPageEntries
    },
    {
        DISPLAY_SETTINGS_PAGE,
        enUSDisplayPageEntries
    },
    {
        FLUSH_PAGE,
        enUSFlushPageEntries
    },
    {
        SELECT_PARTITION_PAGE,
        enUSSelectPartitionEntries
    },
    {
        SELECT_FILE_SYSTEM_PAGE,
        enUSSelectFSEntries
    },
    {
        FORMAT_PARTITION_PAGE,
        enUSFormatPartitionEntries
    },
    {
        DELETE_PARTITION_PAGE,
        enUSDeletePartitionEntries
    },
    {
        INSTALL_DIRECTORY_PAGE,
        enUSInstallDirectoryEntries
    },
    {
        PREPARE_COPY_PAGE,
        enUSPrepareCopyEntries
    },
    {
        FILE_COPY_PAGE,
        enUSFileCopyEntries
    },
    {
        KEYBOARD_SETTINGS_PAGE,
        enUSKeyboardSettingsEntries
    },
    {
        BOOT_LOADER_PAGE,
        enUSBootLoaderEntries
    },
    {
        LAYOUT_SETTINGS_PAGE,
        enUSLayoutSettingsEntries
    },
    {
        QUIT_PAGE,
        enUSQuitPageEntries
    },
    {
        SUCCESS_PAGE,
        enUSSuccessPageEntries
    },
    {
        BOOT_LOADER_FLOPPY_PAGE,
        enUSBootPageEntries
    },
    {
        REGISTRY_PAGE,
        enUSRegistryEntries
    },
    {
        -1,
        NULL
    }
};

MUI_STRING enUSStrings[] =
{
    {STRING_PLEASEWAIT,
     "   Please wait..."},
    {STRING_INSTALLCREATEPARTITION,
     "   ENTER = Install   C = Create Partition   F3 = Quit"},
    {STRING_INSTALLDELETEPARTITION,
     "   ENTER = Install   D = Delete Partition   F3 = Quit"},
    {STRING_PARTITIONSIZE,
     "Size of new partition:"},
    {STRING_CHOOSENEWPARTITION,
     "You have chosen to create a new partition on"},
    {STRING_HDDSIZE,
    "Please enter the size of the new partition in megabytes."},
    {STRING_CREATEPARTITION,
     "   ENTER = Create Partition   ESC = Cancel   F3 = Quit"},
    {STRING_PARTFORMAT,
    "This Partition will be formatted next."},
    {STRING_NONFORMATTEDPART,
    "You chose to install Odyssey on a new or unformatted Partition."},
    {STRING_INSTALLONPART,
    "Setup installs Odyssey onto Partition"},
    {STRING_CHECKINGPART,
    "Setup is now checking the selected partition."},
    {STRING_QUITCONTINUE,
    "F3= Quit  ENTER = Continue"},
    {STRING_REBOOTCOMPUTER,
    "ENTER = Reboot computer"},
    {STRING_TXTSETUPFAILED,
    "Setup failed to find the '%S' section\nin TXTSETUP.SIF.\n"},
    {STRING_COPYING,
     "   Copying file: %S"},
    {STRING_SETUPCOPYINGFILES,
     "Setup is copying files..."},
    {STRING_REGHIVEUPDATE,
    "   Updating registry hives..."},
    {STRING_IMPORTFILE,
    "   Importing %S..."},
    {STRING_DISPLAYETTINGSUPDATE,
    "   Updating display registry settings..."},
    {STRING_LOCALESETTINGSUPDATE,
    "   Updating locale settings..."},
    {STRING_KEYBOARDSETTINGSUPDATE,
    "   Updating keyboard layout settings..."},
    {STRING_CODEPAGEINFOUPDATE,
    "   Adding codepage information to registry..."},
    {STRING_DONE,
    "   Done..."},
    {STRING_REBOOTCOMPUTER2,
    "   ENTER = Reboot computer"},
    {STRING_CONSOLEFAIL1,
    "Unable to open the console\n\n"},
    {STRING_CONSOLEFAIL2,
    "The most common cause of this is using an USB keyboard\n"},
    {STRING_CONSOLEFAIL3,
    "USB keyboards are not fully supported yet\n"},
    {STRING_FORMATTINGDISK,
    "Setup is formatting your disk"},
    {STRING_CHECKINGDISK,
    "Setup is checking your disk"},
    {STRING_FORMATDISK1,
    " Format partition as %S file system (quick format) "},
    {STRING_FORMATDISK2,
    " Format partition as %S file system "},
    {STRING_KEEPFORMAT,
    " Keep current file system (no changes) "},
    {STRING_HDINFOPARTCREATE,
    "%I64u %s  Harddisk %lu  (Port=%hu, Bus=%hu, Id=%hu) on %wZ."},
    {STRING_HDDINFOUNK1,
    "%I64u %s  Harddisk %lu  (Port=%hu, Bus=%hu, Id=%hu)."},
    {STRING_HDDINFOUNK2,
    "   %c%c  Type %lu    %I64u %s"},
    {STRING_HDINFOPARTDELETE,
    "on %I64u %s  Harddisk %lu  (Port=%hu, Bus=%hu, Id=%hu) on %wZ."},
    {STRING_HDDINFOUNK3,
    "on %I64u %s  Harddisk %lu  (Port=%hu, Bus=%hu, Id=%hu)."},
    {STRING_HDINFOPARTZEROED,
    "Harddisk %lu (%I64u %s), Port=%hu, Bus=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK4,
    "%c%c  Type %lu    %I64u %s"},
    {STRING_HDINFOPARTEXISTS,
    "on Harddisk %lu (%I64u %s), Port=%hu, Bus=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK5,
    "%c%c  Type %-3u                         %6lu %s"},
    {STRING_HDINFOPARTSELECT,
    "%6lu %s  Harddisk %lu  (Port=%hu, Bus=%hu, Id=%hu) on %S"},
    {STRING_HDDINFOUNK6,
    "%6lu %s  Harddisk %lu  (Port=%hu, Bus=%hu, Id=%hu)"},
    {STRING_NEWPARTITION,
    "Setup created a new partition on"},
    {STRING_UNPSPACE,
    "    Unpartitioned space              %6lu %s"},
    {STRING_MAXSIZE,
    "MB (max. %lu MB)"},
    {STRING_UNFORMATTED,
    "New (Unformatted)"},
    {STRING_FORMATUNUSED,
    "Unused"},
    {STRING_FORMATUNKNOWN,
    "Unknown"},
    {STRING_KB,
    "KB"},
    {STRING_MB,
    "MB"},
    {STRING_GB,
    "GB"},
    {STRING_ADDKBLAYOUTS,
    "Adding keyboard layouts"},
    {0, 0}
};
