/*
 *      translated by Artem Reznikov, Igor Paliychuk, 2010
 *      http://www.odyssey.org/uk/
 */ 

#pragma once

MUI_LAYOUTS ukUALayouts[] =
{
    { L"0422", L"00000422" },
    { L"0409", L"00000409" },
    { NULL, NULL }
};

static MUI_ENTRY ukUALanguagePageEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "���i� ����",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  ���-��᪠, �����i�� ����, 猪 �㤥 ������⠭� �i� �� ��⠭�������.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   i ����i�� ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  �� ���� �㤥 ������⮢㢠��� �� ������㢠��� � ��⠭�����i� ��⥬i.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = �த�����  F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ukUAWelcomePageEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��᪠�� ��ᨬ� �� �ணࠬ� ��⠭������� Odyssey",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        6,
        11,
        "�� �쮬� �⠯i ��⠭������� �i��㤥���� ���i��� Odyssey �� ���",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "����'��� i �i���⮢�� �� ��㣮�� �⠯� ��⠭�������.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  ����i�� ENTER 鮡 ��⠭���� Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  ����i�� R ��� ��������� Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  ����i�� L ��� ��ॣ��� �i業�i���� 㬮� Odyssey",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  ����i�� F3 鮡 ����. �� ��⠭������ Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "��� ��ਬ���� ��⠫�i�� i��ଠ�i� �� Odyssey, ���-��᪠ �i��i����:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        24,
        "http://www.odyssey.org/uk/",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        0,
        0,
        "ENTER = �த�����  R = �i������  L = �i業�i�  F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ukUAIntroPageEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��⠭���� Odyssey ���室����� � ࠭�i� �⠤i� ஧஡�� i � ��",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "�i��ਬ�� ��i �㭪�i� ������i���� �ணࠬ� ��⠭�������.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "�����i ����㯭i ���������:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "- ��⠭���� �� �i��ਬ�� �i��� �i� ���� ��ࢨ���� ஧�i� �� ���.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "- ��⠭���� �� ���� ������� ��ࢨ���� ஧�i� � ����",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "  ���� �� ���� ���� ஧�७�� ஧�i�.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "- ��⠭���� �� ���� ������� ���訩 ஧�७�� ஧�i� � ����",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "  ���� �� ���� i����� i��i ஧�७i ஧�i��.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "- ��⠭���� �i��ਬ�� ��� 䠩���� ��⥬� FAT.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "- ��ॢiઠ 䠩����� ��⥬� � �� ��஢������.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        23,
        "\x07  ����i�� ENTER 鮡 ��⠭���� Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        25,
        "\x07  ����i�� F3 鮡 ����, �� ��⠭������ Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = �த�����   F3 = ����",
        TEXT_TYPE_STATUS| TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ukUALicensePageEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        6,
        "�i業�i�:",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        8,
        "Odyssey �i業������ �i����i��� �� 㬮�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        9,
        "GNU GPL. ����� Odyssey �i���� ���������, �i �i業������",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "�� ��i᭨�� �i業�iﬨ: X11, BSD, GNU LGPL.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "�� �ணࠬ�� �������祭��, 瘟 �室��� � ��⥬� Odyssey, ����饭�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "�i� �i����� �i業�i���� 㣮��� GNU GPL i� ���०����",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "��ࢨ���� �i業�i�.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "���� �ணࠬ�� �������祭�� ���⠢������� ��� ������I� i ��� ��������",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "� ������⠭�i, � �� �i�楢��, ⠪ i �i���த��� �ࠢ��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "�i業�i� Odyssey �������� ��।��� �த��� ���i� �ᮡ��.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "��� �१ ���-�i ��稭� �� �� ��ਬ��� ���i� �i�����",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "�i業�i���� 㣮�� GNU ࠧ�� � Odyssey, �i��i����",
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
        "��࠭�i�:",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        24,
        "�� � �i�쭥 �ணࠬ�� �������祭��; ���. ���५� ��� ��ॣ��� �ࠢ.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        25,
        "�� ������� �I��I ��࠭�i�; �i ��࠭�i� ��������� �����, �i ",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        26,
        "����������I ��� ���������� �I���",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = ���������",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ukUADevicePageEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "� ᯨ�� ���� �ਢ����i ����i ��ࠬ��� �������.",
        TEXT_STYLE_NORMAL
    },
    {
        24,
        11,
        "����'���:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        12,
        "��࠭:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        13,
        "����i����:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        14,
        "����. ஧������:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        16,
        "�਩���:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        25,
        16, "�����㢠� ���i ��ࠬ��� �������",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        19,
        "�� ����� ��i��� ��ࠬ��� ������� ���᪠�� ����i�i ����� i ����",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        20,
        "��� ���i����� �������� i ����i�� ENTER ��� ������ i��� ���i���i�",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        21,
        "��ࠬ���i�.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "���� ��i ��ࠬ��� ����� �����祭i, �����i��",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        24,
        "\"�����㢠� ���i ��ࠬ��� �������\" i ����i�� ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = �த�����   F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ukUARepairPageEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��⠭���� Odyssey ���室����� � ࠭�i� �⠤i� ஧஡�� i � ��",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "�i��ਬ�� ��i �㭪�i� ������i���� �ணࠬ� ��⠭�������.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "�㭪�i� �i��������� � �� ��஢�����i.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  ����i�� U 鮡 ������ OS.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  ����i�� R ��� ������ ���᮫i �i���������.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  ����i�� ESC ��� ����୥��� �� �������� ���i���.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  ����i�� ENTER 鮡 ��१����⠦�� ����'���.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ESC = ������� ���i���  U = ������  R = �i������  ENTER = ��१����⠦��",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};
static MUI_ENTRY ukUAComputerPageEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��� �� ����� ��i��� ⨯ ��讣� ����'���.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  ���᪠�� ����i�i ����� � ���� ��� ������ ⨯� ��讣� ����'���",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   i ����i�� ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ����i�� ESC ��� ����୥��� �� ����।��� ���i��� ��� ��i��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ⨯� ����'���.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = �த�����   ESC = ����㢠�   F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ukUAFlushPageEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "���⥬� ��ॢi��� � ��i ���i ���०��� �� ���",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "�� ���� ������ ���i�쪠 墨���",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "�i�� �����襭�� ����'��� �㤥 ��⮬��筮 ��१����⠦���",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "����� ���",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ukUAQuitPageEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "Odyssey �� ��⠭������ ����i���",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "���i�� ������ � ��᪮���� A: �",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "��i CD-ROM � CD-�ਢ��i�.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "����i�� ENTER 鮡 ��१����⠦�� ����'���.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "���-��᪠ ��祪��� ...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG,
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ukUADisplayPageEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��� �� ����� ��i��� ⨯ ��࠭�.",
        TEXT_STYLE_NORMAL
    },
    {   8,
        10,
         "\x07  ���᪠�� ����i�i ����� � ���� ��� ������ ����i����� ⨯� ���i���",
         TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   i ����i�� ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ����i�� ESC ��� ����୥��� �� ����।��� ���i��� ��� ��i��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ⨯� ���i��.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = �த�����   ESC = ����㢠�   F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ukUASuccessPageEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "�᭮��i ��������� Odyssey �㫨 ��i譮 ��⠭�����i.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "���i�� ��᪥�� � ��᪮���� A: �",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "��i CD-ROM � CD-�ਢ��i�.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "����i�� ENTER 鮡 ��१����⠦�� ����'���.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = ��१����⠦�� ����'���",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ukUABootPageEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��⠭���� �� ���� ��⠭���� bootloader �� ����⪨� ���",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "��讣� ����'���",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        13,
        "���-��᪠ ��⠢� �i��ଠ⮢��� ��᪥�� � �������� A: �",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "����i�� ENTER.",
        TEXT_STYLE_NORMAL,
    },
    {
        0,
        0,
        "ENTER = �த�����   F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }

};

static MUI_ENTRY ukUASelectPartitionEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "���� �ਢ������ ᯨ᮪ i����� ஧�i�i� � �������⮣� �i���, �� �����",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "�⢮�� ���i ஧�i��.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "\x07  ���᪠�� ����i�i ����� � ���� ��� ������ �㭪��.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ����i�� ENTER 鮡 ��⠭���� Odyssey �� ���࠭�� ஧�i�.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  ����i�� C 鮡 �⢮�� ����� ஧�i�.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  ����i�� D 鮡 ������� i���稩 ஧�i�.",
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

static MUI_ENTRY ukUAFormatPartitionEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��ଠ�㢠��� ஧�i��",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        10,
        "��ࠧ ��⠭���� �i��ଠ��� ஧�i�. ����i�� ENTER ��� �த�������.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = �த�����   F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        TEXT_STYLE_NORMAL
    }
};

static MUI_ENTRY ukUAInstallDirectoryEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��⠭���� ��⠭����� 䠩�� Odyssey �� ���࠭�� ஧�i�. �����i��",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "��४��i�, � �� �� ��� ��⠭���� Odyssey:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "��� ��i��� ��४��i� ����i�� BACKSPACE ��� ���������",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        15,
        "ᨬ���i� �i�� 箣� ����i�� ����� ��४��i� ���",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        16,
        "��⠭������� Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = �த�����   F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ukUAFileCopyEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        0,
        12,
        "���-��᪠, ��祪��� ���� ��⠭���� Odyssey ���i�� 䠩�� ��",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        13,
        "����� �ਧ��祭��.",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        14,
        "�� ���� ������ ���i�쪠 墨���.",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        50,
        0,
        "\xB3 ���-��᪠ ��祪���...    ",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ukUABootLoaderEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��⠭���� ��⠭����� �����⠦㢠�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "��⠭���� �����⠦㢠� �� ����⪨� ��� (MBR i VBR).",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "��⠭���� �����⠦㢠� �� ����⪨� ��� (��� VBR).",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "��⠭���� �����⠦㢠� �� ��᪥��.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "�� ��⠭���� �����⠦㢠�.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = �த�����   F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ukUAKeyboardSettingsEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��� �� ����� ��i��� ⨯ ����i����.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  ���᪠�� ����i�i ����� � ���� ��� ������ ����i����� ⨯�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   ����i���� i ����i�� ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ����i�� ESC ��� ����୥��� �� ����।�� ���i��� ��� ��i��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ⨯� ����i����.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = �த�����   ESC = ����㢠�   F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ukUALayoutSettingsEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�����i�� ஧������, 猪 �㤥 ��⠭������ 猪 �⠭���⭠.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  ���᪠�� ����i�i ����� � ���� ��� ������ ����i���� ஧������",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "    ����i���� i ����i�� ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ����i�� ESC ��� ����୥��� �� ����।�� ���i��� ��� ��i��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ஧������ ����i����.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = �த�����   ESC = ����㢠�   F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY ukUAPrepareCopyEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��⠭���� ����� ��� ����'��� ��� ���i��� 䠩�i� Odyssey. ",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "������� ᯨ᮪ 䠩�i�...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY ukUASelectFSEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        17,
        "�����i�� 䠩���� ��⥬� �i ᯨ�� ����.",
        0
    },
    {
        8,
        19,
        "\x07  ���᪠�� ����i�i ����� � ���� ��� ������ 䠩����� ��⥬�.",
        0
    },
    {
        8,
        21,
        "\x07  ����i�� ENTER 鮡 �i��ଠ�㢠� ஧�i�.",
        0
    },
    {
        8,
        23,
        "\x07  ����i�� ESC ��� ������ i�讣� ஧�i��.",
        0
    },
    {
        0,
        0,
        "ENTER = �த�����   ESC = ����㢠�   F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },

    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ukUADeletePartitionEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�� ���ࠫ� ��������� ஧�i��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "\x07  ����i�� D ��� ��������� ஧�i��.",
        TEXT_STYLE_NORMAL
    },
    {
        11,
        19,
        "�����: ��i ���i �� �쮬� ஧�i�i ����� ���祭i!",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  ����i�� ESC ��� ᪠�㢠���.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "D = ������� ����i�   ESC = ����㢠�   F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ukUARegistryEntries[] =
{
    {
        4,
        3,
        " ��⠭������� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��⠭���� ������� ����i����i� ��⥬�. ",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "�⢮��� �������� ������...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

MUI_ERROR ukUAErrorEntries[] =
{
    {
        //ERROR_NOT_INSTALLED
        "Odyssey �� �� ����i��� ��⠭������� �� ���\n"
        "����'���. ��� �� ������ � ��⠭���� ��ࠧ,\n"
        "� ��� �㤥 �����i��� ������� �ணࠬ� ��⠭�������\n"
        "�����, �� �� ��� ��⠭���� Odyssey,\n"
        "\n"
        "  \x07  ����i�� ENTER 鮡 �த����� ��⠭�������.\n"
        "  \x07  ����i�� F3 ��� ��室� � ��⠭����.",
        "F3 = ����  ENTER = �த�����"
    },
    {
        //ERROR_NO_HDD
        "�� ������� ����� ����⪨� ���.\n",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_NO_SOURCE_DRIVE
        "�� ������� ����� ��⠭���筨� ���.\n",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_LOAD_TXTSETUPSIF
        "�� ������� �����⠦�� 䠩� TXTSETUP.SIF.\n",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_CORRUPT_TXTSETUPSIF
        "���� TXTSETUP.SIF ��誮������.\n",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_SIGNATURE_TXTSETUPSIF,
        "����� ����४⭨� �i���� � TXTSETUP.SIF.\n",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_DRIVE_INFORMATION
        "�� ������� ��ਬ�� ���i �� ��⥬��� ���.\n",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_WRITE_BOOT,
        "�� ������� ��⠭���� �����⠦㢠�쭨� ��� FAT �� �⥬��� ஧�i�.",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_LOAD_COMPUTER,
        "�� ������� �����⠦�� ᯨ᮪ ⨯i� ����'���.\n",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_LOAD_DISPLAY,
        "�� ������� �����⠦�� ᯨ᮪ ०��i� ��࠭�.\n",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_LOAD_KEYBOARD,
        "�� ������� �����⠦�� ᯨ᮪ ⨯i� ����i����.\n",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_LOAD_KBLAYOUT,
        "�� ������� �����⠦�� ᯨ᮪ ஧������ ����i����.\n",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_WARN_PARTITION,
          "�������� � �i�i�� ���� ����⪨� ���, � �i���� ஧�i�,\n"
          "直� �� �i��ਬ������ Odyssey!\n"
          "\n"
          "�⢮७�� � ��������� ஧�i�i� ���� ��㩭㢠� ⠡���� ஧�i�i�.\n"
          "\n"
          "  \x07  ����i�� F3 ��� ��室� � ��⠭����.\n"
          "  \x07  ����i�� ENTER 鮡 �த�����.",
          "F3= ����  ENTER = �த�����"
    },
    {
        //ERROR_NEW_PARTITION,
        "�� �� ����� �⢮�� ����� ஧�i� ��\n"
        "��� i���箬� ஧�i�i!\n"
        "\n"
        "  * ����i�� ���-�� ����i�� 鮡 �த�����.",
        NULL
    },
    {
        //ERROR_DELETE_SPACE,
        "�� ����� ������� ��஧�i祭� ������� �� ����!\n"
        "\n"
        "  * ����i�� ���-�� ����i�� 鮡 �த�����.",
        NULL
    },
    {
        //ERROR_INSTALL_BOOTCODE,
        "�� ������� ��⠭���� �����⠦㢠�쭨� ��� FAT �� �⥬��� ஧�i�.",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_NO_FLOPPY,
        "�i����� ��᪥� � ��᪮���i A:.",
        "ENTER = �த�����"
    },
    {
        //ERROR_UPDATE_KBSETTINGS,
        "�� ������� ������ ��ࠬ��� ஧������ ����i����.",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_UPDATE_DISPLAY_SETTINGS,
        "�� ������� ������ ��ࠬ��� ��࠭� � �����i.",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_IMPORT_HIVE,
        "�� ������� i�����㢠� 䠩� ���i� ������.",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_FIND_REGISTRY
        "�� ������� ����� 䠩�� ����� ������.",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_CREATE_HIVE,
        "�� ������� �⢮�� ���i ������.",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_INITIALIZE_REGISTRY,
        "�� ������� i�i�i��i�㢠� �����.",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_INVALID_CABINET_INF,
        "Cabinet ��� ����४⭨� inf-䠩�.\n",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_CABINET_MISSING,
        "Cabinet �� ��������.\n",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_CABINET_SCRIPT,
        "Cabinet �� ��� ��⠭���筮�� �業��i�.\n",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_COPY_QUEUE,
        "�� ������� �i���� ��� ���i��� 䠩�i�.\n",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_CREATE_DIR,
        "�� ������� �⢮�� ��४��i� ��� ��⠭�������.",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_TXTSETUP_SECTION,
        "�� ������� ����� ᥪ�i� 'Directories'\n"
        "� 䠩�i TXTSETUP.SIF.\n",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_CABINET_SECTION,
        "�� ������� ����� ᥪ�i� 'Directories'\n"
        "� cabinet.\n",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_CREATE_INSTALL_DIR
        "�� ������� �⢮�� ��४��i� ��� ��⠭������.",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_FIND_SETUPDATA,
        "�� ������� ����� ᥪ�i� 'SetupData'\n"
        "� 䠩�i TXTSETUP.SIF.\n",
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_WRITE_PTABLE,
        "�� ������� ������ ⠡���i ஧�i�i�.\n"
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_ADDING_CODEPAGE,
        "�� ������� ����� ��ࠬ��� ���㢠��� � �����.\n"
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_UPDATE_LOCALESETTINGS,
        "�� ������� ��⠭���� ������ ��⥬�.\n"
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_ADDING_KBLAYOUTS,
        "�� ������� ����� ஧������ ����i���� �� ������.\n"
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_UPDATE_GEOID,
        "�� ������� ��⠭���� geo id.\n"
        "ENTER = ��१����⠦�� ����'���"
    },
    {
        //ERROR_INSUFFICIENT_DISKSPACE,
        "�������� �i�쭮�� �i��� �� ��࠭��� ஧�i�i.\n"
        "  * ����i�� ���-�� ����i�� ��� �த�������.",
        NULL
    },
    {
        NULL,
        NULL
    }
};

MUI_PAGE ukUAPages[] =
{
    {
        LANGUAGE_PAGE,
        ukUALanguagePageEntries
    },
    {
        START_PAGE,
        ukUAWelcomePageEntries
    },
    {
        INSTALL_INTRO_PAGE,
        ukUAIntroPageEntries
    },
    {
        LICENSE_PAGE,
        ukUALicensePageEntries
    },
    {
        DEVICE_SETTINGS_PAGE,
        ukUADevicePageEntries
    },
    {
        REPAIR_INTRO_PAGE,
        ukUARepairPageEntries
    },
    {
        COMPUTER_SETTINGS_PAGE,
        ukUAComputerPageEntries
    },
    {
        DISPLAY_SETTINGS_PAGE,
        ukUADisplayPageEntries
    },
    {
        FLUSH_PAGE,
        ukUAFlushPageEntries
    },
    {
        SELECT_PARTITION_PAGE,
        ukUASelectPartitionEntries
    },
    {
        SELECT_FILE_SYSTEM_PAGE,
        ukUASelectFSEntries
    },
    {
        FORMAT_PARTITION_PAGE,
        ukUAFormatPartitionEntries
    },
    {
        DELETE_PARTITION_PAGE,
        ukUADeletePartitionEntries
    },
    {
        INSTALL_DIRECTORY_PAGE,
        ukUAInstallDirectoryEntries
    },
    {
        PREPARE_COPY_PAGE,
        ukUAPrepareCopyEntries
    },
    {
        FILE_COPY_PAGE,
        ukUAFileCopyEntries
    },
    {
        KEYBOARD_SETTINGS_PAGE,
        ukUAKeyboardSettingsEntries
    },
    {
        BOOT_LOADER_PAGE,
        ukUABootLoaderEntries
    },
    {
        LAYOUT_SETTINGS_PAGE,
        ukUALayoutSettingsEntries
    },
    {
        QUIT_PAGE,
        ukUAQuitPageEntries
    },
    {
        SUCCESS_PAGE,
        ukUASuccessPageEntries
    },
    {
        BOOT_LOADER_FLOPPY_PAGE,
        ukUABootPageEntries
    },
    {
        REGISTRY_PAGE,
        ukUARegistryEntries
    },
    {
        -1,
        NULL
    }
};

MUI_STRING ukUAStrings[] =
{
    {STRING_PLEASEWAIT,
     "   ���-��᪠, ��祪���..."},
    {STRING_INSTALLCREATEPARTITION,
     "   ENTER = ��⠭����   C = �⢮�� ����i�   F3 = ����"},
    {STRING_INSTALLDELETEPARTITION,
     "   ENTER = ��⠭����   D = ������� ����i�   F3 = ����"},
    {STRING_PARTITIONSIZE,
     "����i� ������ ஧�i��:"},
    {STRING_CHOOSENEWPARTITION,
     "�� ��� �⢮�� ����� ஧�i� ��"},
    {STRING_HDDSIZE,
    "���-��᪠, ����i�� ஧�i� ������ ஧�i�� � ���������."},
    {STRING_CREATEPARTITION,
     "   ENTER = �⢮�� ����i�   ESC = ����㢠�   F3 = ����"},
    {STRING_PARTFORMAT,
    "��� ஧�i� �㤥 �i��ଠ⮢���."},
    {STRING_NONFORMATTEDPART,
    "�� ���ࠫ� ��⠭������� Odyssey �� ����� ��� ���ଠ⮢���� ஧�i�."},
    {STRING_INSTALLONPART,
    "Odyssey ��⠭��������� �� ஧�i�"},
    {STRING_CHECKINGPART,
    "��⠭���� ��ॢi��� ���࠭�� ஧�i�."},
    {STRING_QUITCONTINUE,
    "F3= ����  ENTER = �த�����"},
    {STRING_REBOOTCOMPUTER,
    "ENTER = ��१����⠦�� ����'���"},
    {STRING_TXTSETUPFAILED,
    "��⠭���� �� ��i� ����� ᥪ�i� '%S' \n� 䠩�i TXTSETUP.SIF.\n"},
    {STRING_COPYING,
     "   ���i���: %S"},
    {STRING_SETUPCOPYINGFILES,
     "��⠭���� ���i�� 䠩��..."},
    {STRING_REGHIVEUPDATE,
    "   ��������� ���i� ������..."},
    {STRING_IMPORTFILE,
    "   I�����㢠��� %S..."},
    {STRING_DISPLAYETTINGSUPDATE,
    "   ��������� ��ࠬ���i� ��࠭� � �����i..."},
    {STRING_LOCALESETTINGSUPDATE,
    "   ��������� ��ࠬ���i� �����i..."},
    {STRING_KEYBOARDSETTINGSUPDATE,
    "   ��������� ��ࠬ���i� ஧������ ����i����..."},
    {STRING_CODEPAGEINFOUPDATE,
    "   ��������� ����� �� ������ ���i��� � �����..."},
    {STRING_DONE,
    "   ��⮢�..."},
    {STRING_REBOOTCOMPUTER2,
    "   ENTER = ��१����⠦�� ����'���"},
    {STRING_CONSOLEFAIL1,
    "�� ������� �i���� ���᮫�\n\n"},
    {STRING_CONSOLEFAIL2,
    "����i��� ����iୠ ��稭� �쮣� -  ������⠭�� USB ����i����\n"},
    {STRING_CONSOLEFAIL3,
    "USB ����i���� � �� �i��ਬ������ ����i���\n"},
    {STRING_FORMATTINGDISK,
    "��⠭���� �ଠ��� ��� ���"},
    {STRING_CHECKINGDISK,
    "��⠭���� ��ॢi��� ��� ���"},
    {STRING_FORMATDISK1,
    " ��ଠ�㢠� ஧�i� � 䠩���i� ��⥬i %S (袨��� �ଠ�㢠���) "},
    {STRING_FORMATDISK2,
    " ��ଠ�㢠� ஧�i� � 䠩���i� ��⥬i %S  "},
    {STRING_KEEPFORMAT,
    " ������ i����� 䠩���� ��⥬� (��� ��i�) "},
    {STRING_HDINFOPARTCREATE,
    "%I64u %s  ����⪨� ��� %lu  (����=%hu, ����=%hu, Id=%hu) on %wZ."},
    {STRING_HDDINFOUNK1,
    "%I64u %s  ����⪨� ��� %lu  (����=%hu, ����=%hu, Id=%hu)."},
    {STRING_HDDINFOUNK2,
    "   %c%c  Type %lu    %I64u %s"},
    {STRING_HDINFOPARTDELETE,
    "�� %I64u %s  ����⪨� ��� %lu  (����=%hu, ����=%hu, Id=%hu) on %wZ."},
    {STRING_HDDINFOUNK3,
    "�� %I64u %s  ����⪨� ��� %lu  (����=%hu, ����=%hu, Id=%hu)."},
    {STRING_HDINFOPARTZEROED,
    "����⪨� ��� %lu (%I64u %s), ����=%hu, ����=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK4,
    "%c%c  Type %lu    %I64u %s"},
    {STRING_HDINFOPARTEXISTS,
    "�� ����⪮�� ���� %lu (%I64u %s), ����=%hu, ����=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK5,
    "%c%c  Type %-3u                         %6lu %s"},
    {STRING_HDINFOPARTSELECT,
    "%6lu %s  ����⪨� ��� %lu  (����=%hu, ����=%hu, Id=%hu) on %S"},
    {STRING_HDDINFOUNK6,
    "%6lu %s  ����⪨� ��� %lu  (����=%hu, ����=%hu, Id=%hu)"},
    {STRING_NEWPARTITION,
    "��⠭���� �⢮ਢ ����� ஧�i� ��"},
    {STRING_UNPSPACE,
    "    ��஧�i祭� �������              %6lu %s"},
    {STRING_MAXSIZE,
    "MB (����. %lu MB)"},
    {STRING_UNFORMATTED,
    "����� (���ଠ⮢����)"},
    {STRING_FORMATUNUSED,
    "�� ������⠭�"},
    {STRING_FORMATUNKNOWN,
    "���i����"},
    {STRING_KB,
    "KB"},
    {STRING_MB,
    "MB"},
    {STRING_GB,
    "GB"},
    {STRING_ADDKBLAYOUTS,
    "��������� ஧������ ����i����"},
    {0, 0}
};
