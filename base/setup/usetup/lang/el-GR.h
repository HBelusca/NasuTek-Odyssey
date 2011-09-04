#pragma once

MUI_LAYOUTS elGRLayouts[] =
{
    { L"0408", L"00000408" },
    { L"0409", L"00000409" },
    { NULL, NULL }
};

static MUI_ENTRY elGRLanguagePageEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "������� ��驩��.",
        TEXT_STYLE_NORMAL
    },
    {
        7,
        10,
        "\x07 �������� ����⥫� �� ��驩� ��� �� �������������� ���� ��� �����ᩫ���.",
        TEXT_STYLE_NORMAL
    },
    {
        7,
        11,
        "  ���� ���㩫� ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        7,
        13,
        "\x07  ���� � ��驩� �� �夘� � ����������⤞ ��� �� ������ �穫���.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = ���⮜��  F3 = ����騞��",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY elGRWelcomePageEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        5,
        8,
        "���� ��婘�� ���� �����ᩫ��� ��� Odyssey",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        5,
        11,
        "���� �� �⨦� ��� �����ᩫ���� ������᭜� �� ����������� �穫��� Odyssey",
        TEXT_STYLE_NORMAL
    },
    {
        5,
        12,
        "���� ���������� ��� ��� ��������᝜� �� ��竜�� �⨦� ��� �����ᩫ����.",
        TEXT_STYLE_NORMAL
    },
    {
        7,
        15,
        "\x07  ���㩫� ENTER ��� �� ��������㩜�� �� Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        7,
        17,
        "\x07  ���㩫� R ��� �� ��������驜�� �� Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        7,
        19,
        "\x07  ���㩫� L ��� �� ��嫜 ���� 樦�� ������櫞��� ��� Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        7,
        21,
        "\x07  ���㩫� F3 ��� �� �����㩜�� ��� �� ��������㩜�� �� Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        5,
        23,
        "��� ������櫜��� ��������圪 ��� �� Odyssey, ��������磜 ���������嫜 ��:",
        TEXT_STYLE_NORMAL
    },
    {
        5,
        24,
        "http://www.odyssey.org",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        0,
        0,
        "   ENTER = ���⮜��  R = �����樟ਫ਼ F3 = ����騞��",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY elGRIntroPageEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        7,
        "� �����ᩫ��� ��� Odyssey ��婡���� �� ��頣� ��ᛠ� ��᧫���� ���",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        8,
        "��� �������坜� ��棘 梜� ��� �����櫞��� ���� ��㨦�� �����ᩫ����",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        11,
        "���禬� �� ������� ����������� :",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "- � �����ᩫ��� �� ������ �� ��������� ��� ��� ⤘ ��૜禤",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "  ����⨠��� ��� �婡�.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "- � �����ᩫ��� �� ������ �� �����ᯜ� ⤘ ��૜禤 ����⨠��� ",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "  ��� ⤘ �婡� ��橦� ��ᨮ��� �������⤘ ������婣��� ��� �婡� ����.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "- � �����ᩘ�� �� ������ �� �����ᯜ� �� ��髦 �������⤦ ����⨠��� ��� ",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "  �婡�� ��橦� ��ᨮ��� �� ᢢ� �������⤘ ������婣��� ��� �婡� ����.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "- � �����ᩫ��� �������坜� �椦 FAT ����㣘�� ������.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "- ��� �������坜��� ��棘 ⢜���� ������� ��� ���㣘�� ������.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        23,
        "\x07  ���㩫� ENTER ��� �� ��������㩜�� �� Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        25,
        "\x07  ���㩫� F3 ��� �� �����㩜�� ��� �� ��������㩜�� �� Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = ���⮜��   F3 = ����騞��",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY elGRLicensePageEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        6,
        "������櫞��:",
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
        "   ENTER = ���������",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY elGRDevicePageEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "� ������� �婫� ��室�� ��� ����婜�� �������.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "    ����������:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "       ���ᤠ��:",
        TEXT_STYLE_NORMAL,
    },
    {
        8,
        13,
        "   ��������暠�:",
        TEXT_STYLE_NORMAL
    },
    {
        2,
        14,
        "��᫘�� ����������妬:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "        �������:",
        TEXT_STYLE_NORMAL
    },
    {
        25,
        16, "������� ���� �� ����婜�",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        19,
        "�����嫜 �� ���ᥜ�� ��� ����婜�� ������ ���餫�� �� ��㡫�� ����",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        20,
        "� ���� ��� �� ����⥜�� ��� �矣���.  ",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        21,
        "���� ���㩫� �� ��㡫�� ENTER ��� �� ����⥜�� ᢢ�� ����婜��.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "� 梜� �� ����婜�� �夘� �੫�, ����⥫� ",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        24,
        "\"������� ���� �� ����婜� �������\" ��� ���㩫� ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = ���⮜��   F3 = ����騞��",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY elGRRepairPageEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "� �����ᩫ��� ��� Odyssey ��婡���� �� ��頣� ��ᛠ� ��᧫���� ���",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "��� �������坜� ��棘 梜� ��� �����櫞��� ���� ��㨦�� �����ᩫ����.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "�� ��������圪 �����樟ਫ਼� ��� ⮦�� ���������� ��棘.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  ���㩫� U ��� �����ਫ਼ ��� �������������.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  ���㩫� R ��� �� �����⩜�� ��� ����梘 �����樟ਫ਼�.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  ���㩫� ESC ��� �� ������⯜�� ���� �稠� ���囘.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  ���㩫� ENTER ��� �� ���������㩜�� ��� ����������.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ESC = �稠� ���囘  ENTER = �������夞��",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};
static MUI_ENTRY elGRComputerPageEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�⢜�� �� ���ᥜ�� ��� �秦 ��� ���������� ��� �� ������������.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  ���㩫� �� ��㡫�� ���� � ���� ��� �� ����⥜�� ��� ���������",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   �秦 ����������.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "   ���� ���㩫� ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "\x07  ���㩫� �� ��㡫�� ESC ��� �� ������⯜�� ���� ������磜��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "   ���囘 ��� �� ���ᥜ�� ��� �秦 ����������.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = ���⮜��   ESC = ���ਫ਼   F3 = ����騞��",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY elGRFlushPageEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "�� �穫��� ��������餜� �騘 櫠 梘 �� �����⤘ ⮦�� �����������",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "���� �� �᨜� �嚞 騘",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "� �����������, � ���������� ��� �� ������������� ���棘��",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   �����ᨠ�� ����ਠ�� ������...",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY elGRQuitPageEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "�� Odyssey ��� ��������៞�� ����",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "�����⩫� �� ����⫘ ��� �� A: ���",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "梘 �� CD-ROMs  ��� �� CD-Drives.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "���㩫� ENTER ��� �� ���������㩜�� ��� ����������.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   �������� �����⤜�� ...",
        TEXT_TYPE_STATUS,
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY elGRDisplayPageEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�⢜�� �� ���ᥜ�� ��� �秦 ��� ���ᤠ��� ��� �� ������������.",
        TEXT_STYLE_NORMAL
    },
    {   8,
        10,
         "\x07  ���㩫� �� ��㡫�� ���� � ���� ��� �� ����⥜�� ��� ���������.",
         TEXT_STYLE_NORMAL
    },
    {   8,
        11,
         "  �秦 ���ᤠ���.",
         TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "   ���� ���㩫� ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "\x07  ���㩫� �� ��㡫�� ESC ��� �� ������⯜�� ���� ������磜�� ",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "   ���囘 ��� �� ���ᥜ�� ��� �秦 ���ᤠ���.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = ���⮜��   ESC = ���ਫ਼   F3 = ����騞��",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY elGRSuccessPageEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "�� ������ ������� ��� Odyssey ��������៞��� �������.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "�����⩫� �� ����⫘ ��� �� A: ���",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "梘 �� CD-ROMs ��� �� CD-Drive.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "���㩫� ENTER ��� �� ���������㩜�� ��� ���������� ���.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �������夞�� ����������",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY elGRBootPageEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "� �����ᩫ��� �� ������ �� ��������㩜� ��� bootloader",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "��� ������ �婡� ��� ���������� ���",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        13,
        "�������� ���᚜�� ��� ��������⤞ ����⫘ ��� A: ���",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "���㩫� ENTER.",
        TEXT_STYLE_NORMAL,
    },
    {
        0,
        0,
        "   ENTER = ���⮜��   F3 = ����騞��",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }

};

static MUI_ENTRY elGRSelectPartitionEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "� �������� �婫� �����坜� �� ��ᨮ���� ������婣��� ���",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "��� ��� ��⬟��� �騦 ��� �� ������婣���.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "\x07  ���㩫� ���� � ���� ��� �� ����⥜�� ⤘ ������� ��� �婫��.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ���㩫� ENTER ��� �� ��������㩜�� �� Odyssey ��� �������⤦ ����⨠���.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  ���㩫� C ��� �� ��������㩜�� ⤘ �� ����⨠���.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  ���㩫� D ��� �� �����ᯜ�� ⤘ ��ᨮ�� ����⨠���.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   �������� �����⤜��...",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY elGRFormatPartitionEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "����樭ਫ਼ ������婣����",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        10,
        "� �����ᩫ��� �騘 �� �������驜� �� ����⨠���",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        11,
        "���㩫� ENTER ��� �� �����婜��.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = ���⮜��   F3 = ����騞��",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        TEXT_STYLE_NORMAL
    }
};

static MUI_ENTRY elGRInstallDirectoryEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "� �����ᩫ��� ������᭜� �� ����� ��� Odyssey ��� �������⤦ ����⨠���.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "����⥫� ��� ���� �ᡜ�� ��� �⢜�� �� ������������ �� Odyssey:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "��� �� ���ᥜ�� ��� �������棜�� �ᡜ�� ���㩫� BACKSPACE ��� ��",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        15,
        "�����ᯜ�� ������㨜� ��� ���� �����������婫� ��� �ᡜ�� ���� ����",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        16,
        "�⢜�� �� ���������� �� Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = ���⮜��   F3 = ����騞��",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY elGRFileCopyEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        0,
        12,
        "�������� �����⤜�� 橦 � �����ᩫ��� ��� Odyssey ������᭜�",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        13,
        "�� ����� ��� �ᡜ�� �����ᩫ����",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        14,
        "���� � ��������� ������ �� ����㩜� ������ �����.",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        0,
        "                                                           \xB3 �������� �����⤜��...    ",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY elGRBootLoaderEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "� �����ᩫ��� ��᭜� ��� boot loader",
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
        "�����ᩫ��� ��� bootloader �� ��� ����⫘.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "�� �� �夜� �����ᩫ��� ��� bootloader.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = ���⮜��   F3 = ����騞��",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY elGRKeyboardSettingsEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�⢜�� �� ���ᥜ�� ��� �秦 ��� ����������妬 ��� �� ������������.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  ���㩫� �� ��㡫�� ���� � ���� ��� �� ����⥜�� ��� ���������",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   �秦 ����������妬.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "   ���� ���㩫� ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "\x07  ���㩫� �� ��㡫�� ESC ��� �� ������⯜�� ���� ������磜��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "   ���囘 ��� �� ���ᥜ�� ��� �秦 ��� ����������妬.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = ���⮜��   ESC = ���ਫ਼   F3 = ����騞��",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY elGRLayoutSettingsEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�������� ����⥫� ��� ��᫘�� ��� �� ������������ � ����������⤞.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  ���㩫� �� ��㡫�� ���� � ���� ��� �� ����⥜�� ��� ����������",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   ��᫘�� ����������妬. ���� ���㩫� ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ���㩫� �� ��㡫�� ESC ��� �� ������⯜�� ���� ������磜��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ���囘 ��� �� ���ᥜ�� ��� ��᫘�� ��� ����������妬.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = ���⮜��   ESC = ���ਫ਼   F3 = ����騞��",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY elGRPrepareCopyEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "� �����ᩫ��� ��������᝜� ��� ���������� ��� ��� ��� ��������� �� ������ ��� Odyssey. ",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ��坜��� � �婫� �� ������ ���� ���������...",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY elGRSelectFSEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        17,
        "����⥫� ⤘ �穫��� ������ ��� ��� ������� �婫�.",
        0
    },
    {
        8,
        19,
        "\x07  ���㩫� �� ��㡫�� ���� � ���� ��� �� ����⥜�� �� �穫��� ������.",
        0
    },
    {
        8,
        21,
        "\x07  ���㩫� ENTER ��� �� �������驜�� �� parition.",
        0
    },
    {
        8,
        23,
        "\x07  ���㩫� ESC ��� �� ����⥜�� ᢢ� partition.",
        0
    },
    {
        0,
        0,
        "   ENTER = ���⮜��   ESC = ���ਫ਼   F3 = ����騞��",
        TEXT_TYPE_STATUS
    },

    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY elGRDeletePartitionEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "����⥘�� �� �����ᯜ�� ���� �� partition",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "\x07  ���㩫� D ��� �� �����ᯜ�� �� partition.",
        TEXT_STYLE_NORMAL
    },
    {
        11,
        19,
        "�������������:  �� �����⤘ �� ���� �� partition �� �����!",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  ���㩫� ESC ��� ���ਫ਼.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   D = �������� Partition   ESC = ���ਫ਼   F3 = ����騞��",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY elGRRegistryEntries[] =
{
    {
        4,
        3,
        " �����ᩫ��� ��� Odyssey " KERNEL_VERSION_STR,
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "� �����ᩫ��� �����餜� �� ���� ��� ����㣘���. ",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ���������礫�� �� registry hives...",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    },

};

MUI_ERROR elGRErrorEntries[] =
{
    {
        //ERROR_NOT_INSTALLED
        "�� Odyssey ��� ��������៞�� ���� ����\n"
        "���������� ���. �� �����㩜�� ��� ��� �����ᩫ��� �騘, �� ��⧜� ��\n"
        "������⥜�� ��� �����ᩫ��� ��� �� ��������㩜� �� Odyssey.\n"
        "\n"
        "  \x07  ���㩫� ENTER ��� �� �����婜�� ��� �����ᩫ���.\n"
        "  \x07  ���㩫� F3 ��� �� �����㩜�� ��� ��� �����ᩫ���.",
        "F3= ����騞��  ENTER = ���⮜��"
    },
    {
        //ERROR_NO_HDD
        "� �����ᩫ��� �� ��樜�� �� ���� �᧦��� ������ �婡�.\n",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_NO_SOURCE_DRIVE
        "Setup could not find its source drive.\n",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_LOAD_TXTSETUPSIF
        "� �����ᩫ��� �� ��樜�� �� ����驜� �� ����� TXTSETUP.SIF.\n",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_CORRUPT_TXTSETUPSIF
        "� �����ᩫ�� ��㡜 ⤘ ���������⤦ ����� TXTSETUP.SIF.\n",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_SIGNATURE_TXTSETUPSIF,
        "� �����ᩫ��� ��㡜 ��� �� ⚡��� �������� ��� TXTSETUP.SIF.\n",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_DRIVE_INFORMATION
        "� �����ᩫ��� �� ��樜�� �� ����驜� ��� ��������圪 ��� �婡�� ����㣘���.\n",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_WRITE_BOOT,
        "Setup failed to install FAT bootcode on the system partition.",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_LOAD_COMPUTER,
        "� �����ᩫ��� �� ��樜�� �� ����驜� �� �婫� ��� ����������.\n",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_LOAD_DISPLAY,
        "� �����ᩫ��� �� ��樜�� �� ����驜� �� �婫� ��� ���ᤠ���.\n",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_LOAD_KEYBOARD,
        "� �����ᩫ��� �� ��樜�� �� ����驜� �� �婫� ��� ����������妬.\n",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_LOAD_KBLAYOUT,
        "� �����ᩫ��� �� ��樜�� �� ����驜� �� �婫� ����ᥜ� ����������妬.\n",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_WARN_PARTITION,
          "� �����ᩫ��� ��㡜 櫠 ����ᮠ���� ⤘� ������ �婡�� ����⮜� ⤘ �� �������\n"
          "partition table ��� �� ������ �� �������� �੫�!\n"
          "\n"
          "� ��������� � �������� partitions ������ �� �������⯜� �� partiton table.\n"
          "\n"
          "  \x07  ���㩫� F3 ��� �� �����㩜�� ��� ��� �����ᩫ���."
          "  \x07  ���㩫� ENTER ��� �� �����婜��.",
          "F3= ����騞��  ENTER = ���⮜��"
    },
    {
        //ERROR_NEW_PARTITION,
        "�� �����嫜 �� ��������㩜�� ⤘ Partition �⩘ ��\n"
        "⤘ ᢢ� ��ᨮ�� Partition!\n"
        "\n"
        "  * ���㩫� ������㧦�� ��㡫�� ��� �� �����婜��.",
        NULL
    },
    {
        //ERROR_DELETE_SPACE,
        "�� �����嫜 �� �����ᯜ�� ⤘� �� ��������⤦ �騦 �婡��!\n"
        "\n"
        "  * ���㩫� ������㧦�� ��㡫�� ��� �� �����婜��.",
        NULL
    },
    {
        //ERROR_INSTALL_BOOTCODE,
        "Setup failed to install the FAT bootcode on the system partition.",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_NO_FLOPPY,
        "��� ��ᨮ�� ����⫘ ��� A:.",
        "ENTER = ���⮜��"
    },
    {
        //ERROR_UPDATE_KBSETTINGS,
        "� �����ᩘ�� ��⫬�� �� �����驜� ��� ����婜�� ��� �� ��᫘�� ����������妬.",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_UPDATE_DISPLAY_SETTINGS,
        "� �����ᩫ��� ��⫬�� �� �����驜� ��� ����婜�� ����馬 ��� ��� ���ᤠ��.",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_IMPORT_HIVE,
        "� �����ᩫ��� ��⫬�� �� ����驜� ⤘ hive �����.",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_FIND_REGISTRY
        "� �����ᩘ�� ��⫬�� �� ���� �� ����� ������� ��� ����馬.",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_CREATE_HIVE,
        "� �����ᩫ��� ��⫬�� �� ��������㩜� �� registry hives.",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_INITIALIZE_REGISTRY,
        "� �����ᩫ��� ��⫬�� �� ���������㩜� �� �����.",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_INVALID_CABINET_INF,
        "�� cabinet ��� ⮜� ⚡��� ����� inf.\n",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_CABINET_MISSING,
        "�� cabinet �� ��⟞��.\n",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_CABINET_SCRIPT,
        "�� cabinet ��� ⮜� ���⤘ ������ �����ᩫ����.\n",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_COPY_QUEUE,
        "� �����ᩫ��� ��⫬�� �� ���奜� ��� ���� ������ ���� ���������.\n",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_CREATE_DIR,
        "� �����ᩫ��� �� ��樜�� �� ��������㩜� ���� �����暦�� �����ᩫ����.",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_TXTSETUP_SECTION,
        "� �����ᩫ��� ��⫬�� �� ���� ��� ���� 'Directories'\n"
        "��� TXTSETUP.SIF.\n",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_CABINET_SECTION,
        "� �����ᩫ��� ��⫬�� �� ���� ��� ���� 'Directories'\n"
        "��� cabinet.\n",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_CREATE_INSTALL_DIR
        "� �����ᩫ��� �� ��樜�� �� ��������㩜� ��� ���ᢦ�� �����ᩫ����.",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_FIND_SETUPDATA,
        "� �����ᩫ��� ��⫬�� �� ���� ��� ���� 'SetupData'\n"
        "��� TXTSETUP.SIF.\n",
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_WRITE_PTABLE,
        "� �����ᩫ��� ��⫬�� �� ��ᯜ� �� partition tables.\n"
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_ADDING_CODEPAGE,
        "Setup failed to add codepage to registry.\n"
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_UPDATE_LOCALESETTINGS,
        "Setup could not set the system locale.\n"
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_ADDING_KBLAYOUTS,
        "� �����ᩫ��� ��⫬�� �� �����⩜� ��� ����ᥜ�� ������������ ��� �����.\n"
        "ENTER = �������夞�� ����������"
    },
    {
        //ERROR_UPDATE_GEOID,
        "Setup could not set the geo id.\n"
        "ENTER = Reboot computer"
    },
    {
        NULL,
        NULL
    }
};


MUI_PAGE elGRPages[] =
{
    {
        LANGUAGE_PAGE,
        elGRLanguagePageEntries
    },
    {
       START_PAGE,
       elGRWelcomePageEntries
    },
    {
        INSTALL_INTRO_PAGE,
        elGRIntroPageEntries
    },
    {
        LICENSE_PAGE,
        elGRLicensePageEntries
    },
    {
        DEVICE_SETTINGS_PAGE,
        elGRDevicePageEntries
    },
    {
        REPAIR_INTRO_PAGE,
        elGRRepairPageEntries
    },
    {
        COMPUTER_SETTINGS_PAGE,
        elGRComputerPageEntries
    },
    {
        DISPLAY_SETTINGS_PAGE,
        elGRDisplayPageEntries
    },
    {
        FLUSH_PAGE,
        elGRFlushPageEntries
    },
    {
        SELECT_PARTITION_PAGE,
        elGRSelectPartitionEntries
    },
    {
        SELECT_FILE_SYSTEM_PAGE,
        elGRSelectFSEntries
    },
    {
        FORMAT_PARTITION_PAGE,
        elGRFormatPartitionEntries
    },
    {
        DELETE_PARTITION_PAGE,
        elGRDeletePartitionEntries
    },
    {
        INSTALL_DIRECTORY_PAGE,
        elGRInstallDirectoryEntries
    },
    {
        PREPARE_COPY_PAGE,
        elGRPrepareCopyEntries
    },
    {
        FILE_COPY_PAGE,
        elGRFileCopyEntries
    },
    {
        KEYBOARD_SETTINGS_PAGE,
        elGRKeyboardSettingsEntries
    },
    {
        BOOT_LOADER_PAGE,
        elGRBootLoaderEntries
    },
    {
        LAYOUT_SETTINGS_PAGE,
        elGRLayoutSettingsEntries
    },
    {
        QUIT_PAGE,
        elGRQuitPageEntries
    },
    {
        SUCCESS_PAGE,
        elGRSuccessPageEntries
    },
    {
        BOOT_LOADER_FLOPPY_PAGE,
        elGRBootPageEntries
    },
    {
        REGISTRY_PAGE,
        elGRRegistryEntries
    },
    {
        -1,
        NULL
    }
};

MUI_STRING elGRStrings[] =
{
    {STRING_PLEASEWAIT,
     "   �������� �����⤜��..."},
    {STRING_INSTALLCREATEPARTITION,
     "   ENTER = �����ᩫ���   C = ��������� Partition   F3 = ����騞��"},
    {STRING_INSTALLDELETEPARTITION,
     "   ENTER = �����ᩫ���   D = �������� Partition   F3 = ����騞��"},
    {STRING_PARTITIONSIZE,
     "�⚜��� ��� �⦬ partition:"},
    {STRING_CHOOSENEWPARTITION,
     "����⥘�� �� ��������㩜�� ⤘ �� partition on"},
    {STRING_HDDSIZE,
    "�������� �驫� �� �⚜��� ��� partition �� megabytes."},
    {STRING_CREATEPARTITION,
     "   ENTER = ��������� Partition   ESC = ���ਫ਼   F3 = ����騞��"},
    {STRING_PARTFORMAT,
    "���� �� Partition �� ����������� ����."},
    {STRING_NONFORMATTEDPART,
    "����⥘�� �� ��������㩜�� �� Odyssey �� ⤘ �� � �� ��������⤦ Partition."},
    {STRING_INSTALLONPART,
    "Setup install Odyssey onto Partition"},
    {STRING_CHECKINGPART,
    "� �����ᩫ��� ��⚮�� �騘 �� �������⤦ partition."},
    {STRING_QUITCONTINUE,
    "F3= ����騞��  ENTER = ���⮜��"},
    {STRING_REBOOTCOMPUTER,
    "ENTER = �������夞�� ����������"},
    {STRING_TXTSETUPFAILED,
    "Setup failed to find the '%S' section\nin TXTSETUP.SIF.\n"},
    {STRING_COPYING,
     "   ������᭜��� �� �����: %S"},
    {STRING_SETUPCOPYINGFILES,
     "� �����ᩫ��� ������᭜� �����..."},
    {STRING_REGHIVEUPDATE,
    "   �夜��� �����ਫ਼ �� registry hives..."},
    {STRING_IMPORTFILE,
    "   �夜��� �������� ��� %S..."},
    {STRING_DISPLAYETTINGSUPDATE,
    "   �夜��� �����ਫ਼ �� ����婜� ���ᤠ��� ��� ����馬..."},
    {STRING_LOCALESETTINGSUPDATE,
    "   �夜��� �����ਫ਼ �� ����婜� ��驩��..."},
    {STRING_KEYBOARDSETTINGSUPDATE,
    "   �夜��� �����ਫ਼ �� ����婜� ��᫘��� ����������妬..."},
    {STRING_CODEPAGEINFOUPDATE,
    "   Adding codepage information to registry..."},
    {STRING_DONE,
    "   �������韞��..."},
    {STRING_REBOOTCOMPUTER2,
    "   ENTER = �������夞�� ����������"},
    {STRING_CONSOLEFAIL1,
    "��礘�� �� �������� � ����梘\n\n"},
    {STRING_CONSOLEFAIL2,
    "The most common cause of this is using an USB keyboard\n"},
    {STRING_CONSOLEFAIL3,
    "�� USB ��������暠� ��� �夘� ���� ���������棜�� ��棘\n"},
    {STRING_FORMATTINGDISK,
    "� �����ᩫ��� �������餜� �� �婡� ���"},
    {STRING_CHECKINGDISK,
    "� �����ᩫ��� ��⚮�� �� �婡� ���"},
    {STRING_FORMATDISK1,
    " ����樭ਫ਼ ��� partition � %S �穫��� ������ (��㚦�� ����樭ਫ਼) "},
    {STRING_FORMATDISK2,
    " ����樭ਫ਼ ��� partition � %S �穫��� ������ "},
    {STRING_KEEPFORMAT,
    " �� ������夜� �� �穫��� ������ � ⮜� (���� ������) "},
    {STRING_HDINFOPARTCREATE,
    "%I64u %s  ������ �婡�� %lu  (Port=%hu, Bus=%hu, Id=%hu) on %wZ."},
    {STRING_HDDINFOUNK1,
    "%I64u %s  ������ �婡�� %lu  (Port=%hu, Bus=%hu, Id=%hu)."},
    {STRING_HDDINFOUNK2,
    "   %c%c  Type %lu    %I64u %s"},
    {STRING_HDINFOPARTDELETE,
    "on %I64u %s  Harddisk %lu  (Port=%hu, Bus=%hu, Id=%hu) on %wZ."},
    {STRING_HDDINFOUNK3,
    "on %I64u %s  Harddisk %lu  (Port=%hu, Bus=%hu, Id=%hu)."},
    {STRING_HDINFOPARTZEROED,
    "������ �婡�� %lu (%I64u %s), Port=%hu, Bus=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK4,
    "%c%c  Type %lu    %I64u %s"},
    {STRING_HDINFOPARTEXISTS,
    "��� ������ �婡� %lu (%I64u %s), Port=%hu, Bus=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK5,
    "%c%c  Type %-3u                         %6lu %s"},
    {STRING_HDINFOPARTSELECT,
    "%6lu %s  ������ �婡�� %lu  (Port=%hu, Bus=%hu, Id=%hu) on %S"},
    {STRING_HDDINFOUNK6,
    "%6lu %s  ������ �婡�� %lu  (Port=%hu, Bus=%hu, Id=%hu)"},
    {STRING_NEWPARTITION,
    "� �����ᩫ��� �����稚��� ⤘ �� partition ���"},
    {STRING_UNPSPACE,
    "    Unpartitioned space              %6lu %s"},
    {STRING_MAXSIZE,
    "MB (���. %lu MB)"},
    {STRING_UNFORMATTED,
    "�� (�� ��������⤦)"},
    {STRING_FORMATUNUSED,
    "����������垫�"},
    {STRING_FORMATUNKNOWN,
    "ꚤ੫�"},
    {STRING_KB,
    "KB"},
    {STRING_MB,
    "MB"},
    {STRING_GB,
    "GB"},
    {STRING_ADDKBLAYOUTS,
    "�夜��� �����㡞 �� ����ᥜ� ����������妬"},
    {0, 0}
};
