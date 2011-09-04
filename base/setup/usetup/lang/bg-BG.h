#pragma once

MUI_LAYOUTS bgBGLayouts[] =
{
    { L"0402", L"00000402" },
    { L"0402", L"00020402" },
    { L"0402", L"00030402" },
    { L"0409", L"00000409" },
    { NULL, NULL }
};

static MUI_ENTRY bgBGLanguagePageEntries[] =
{
    {
        4,
        3,
        " ������� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "����� �� ����",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  ������ ����, ���� �� �������� �� ᫠�����.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   ���᭥� ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ������ � �ꤥ ���ࠧ��࠭��� �� �ࠩ��� �।��.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �தꫦ�����  F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY bgBGWelcomePageEntries[] =
{
    {
        4,
        3,
        " ������� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "����⎑ �� �ਢ���⢠!",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        6,
        11,
        "���� ��� �� ����ன��� ����ᢠ ࠡ�⭠� �।�� ����⎑",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "�� �������� �� � ������� ���� ��� �� ����ன���.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  ���᭥� ENTER �� ᫠���� �� ����⎑.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  ���᭥� R �� ���ࠢ�� �� ����⎑.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  ���᭥� L, �� �� ����� ࠧ��⥫��� (��業����)",
        TEXT_STYLE_NORMAL
    },
        {
        8,
        20,
        "   ���᪢���� � �᫮��� �� ����⎑",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        22,
        "\x07  ���᭥� F3 �� ��室 ��� ᫠���� �� ����⎑.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        24,
        "�� ����� ᢥ����� �� ����⎑, �����:",
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
        "   ENTER = �தꫦ�����   R = ���ࠢ��   F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY bgBGIntroPageEntries[] =
{
    {
        4,
        3,
        " ������� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "����ன����� �� ����⎑ � � ࠭�� �⥯�� �� ࠧࠡ�⪠. �� ��",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "�ﬠ ��窨 �ꧬ������ �� ���ꫭ� ����������� ����ன��� �ਫ������.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "�����㢠� ᫥���� ��࠭�祭��:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "- ����ன����� �� ���� �� ࠡ�� � ����� �� ���� �� �� ���.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "- ����ன����� �� ���� �� ����e ��ࢨ�e� ��,",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "  ��� �� ��᪠ ��� ࠧ�७ ��." ,
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "- ����ன����� �� ���� �� ���ਥ ��ࢨ� ࠧ�७ �� �� ��᪠, ",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "  ��� �� ��᪠ ��� � ��㣨 ࠧ�७� �﫮��.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "- ����ன����� �����ঠ ᠬ� FAT.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "- �஢�ઠ� �� 䠩����� �।�� �� �� �� � ��⮢�.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        23,
        "\x07  ���᭥� ENTER �� ᫠���� �� ����⎑.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        25,
        "\x07  ���᭥� F3 �� ��室 ��� ᫠���� �� ����⎑.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �தꫦ�����   F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY bgBGLicensePageEntries[] =
{
    {
        4,
        3,
        " ������� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        6,
        "��業��࠭�:",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        8,
        "�।��� ����⎑ � ��業��࠭� �� �᫮���� �� GNU GPL",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        9,
        "� ���, ���ঠ� ��� �� ��㣨 �ꢬ��⨬� ��業�� ���",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "X11, BSD ��� GNU LGPL.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "�������⥫�� ��类 �ᨣ����, ���� � ��� ��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "����⎑, � ����த�� ��� GNU GPL, � �����ঠ��� ��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "�� �ਣ������� ��業�.",
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
        "��� ��ࠤ� �猪��� ��稭�, ������ � ����⎑ �� ��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "����稫� ����� �� GNU General Public License, �����",
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
        "��࠭��:",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        24,
        "���� � ᢮����� �����; ���� ��室��� ��� �� �᫮���� �� �ꧯந��������.",
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
        "   ENTER = ���頭�",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY bgBGDevicePageEntries[] =
{
    {
        4,
        3,
        " ����ன�� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "������� ��- ���� ������� ⥪��� ����ன�� �� ���ன�⢠�.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "       ��������:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "          ��࠭:",
        TEXT_STYLE_NORMAL,
    },
    {
        3,
        13,
        "          ���������:",
        TEXT_STYLE_NORMAL
    },
    {
        3,
        14,
        "��������ୠ ���।��:",
        TEXT_STYLE_NORMAL
    },
    {
        3,
        16,
        "            �ਥ����:",
        TEXT_STYLE_NORMAL
    },
    {
        25,
        16, "�ਥ���� �� ����ன���",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        19,
        "�� �� �஬���� ����ன��� �� ����㤢����, ���������� ��५���",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        20,
        "������ � ������. ���� ⮢� ���᭥� ENTER, �� �� ������",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        21,
        "�����⢠� ����ன��.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "����� ���ࠢ�� ��窨 ����ன��, ������ '�ਥ���� �� ����ன���'",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        24,
        "� ���᭥� ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �தꫦ�����   F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY bgBGRepairPageEntries[] =
{
    {
        4,
        3,
        " ������� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "����ன����� �� ����⎑ � � ࠭�� �⥯�� �� ࠧࠡ�⪠. �� ��",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "�ﬠ ��窨 �ꧬ������ �� ���ꫭ� ����������� ����ன��� �ਫ������.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "�ꧬ������� �� ���ࠢ�� �� �� � ��⮢�.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  ���᭥� U �� ������� �� ����樮���� ��⥬�.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  ���᭥� R �� ���⠭��� �।� (�������).",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  ���ᨭ�� ESC �� ���頭� �� ������� ��࠭��.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  ���᭥� ENTER �� �१���� �� ��������.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ESC = ������ ��࠭��  ENTER = �१����",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};
static MUI_ENTRY bgBGComputerPageEntries[] =
{
    {
        4,
        3,
        " ������� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��訫� �� �� ᬥ��� ���� �� ��������.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  ������ ���� �� ��࠭� ��� ��५��� ����� � ������ � ",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   ���᭥� ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ���᭥� ESC, �� �� � ��୥� �� �।室��� ��࠭��, ��� ��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ᬥ��� ���� �� ��������.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �தꫦ�����   ESC = �⪠�   F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY bgBGFlushPageEntries[] =
{
    {
        4,
        3,
        " ������� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "�।��� �஢��, ���� ��窨 ����� � ���࠭��� �� ��᪠ ��.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "���� � �⭥�� ����⪠.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "���������� �� � � �१���᭥ ᠬ, ����� �ਪ���.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ����⢠�� �� ᪫���",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY bgBGQuitPageEntries[] =
{
    {
        4,
        3,
        " ������� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "�������� �� ����⎑ �� � �����訫�.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "������� ��᪥�� �� ���ன�⢮ �: �",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "��窨 ���⥫� �� �� � DVD ���ன�⢠�.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "���᭥� ENTER, �� �� �१���᭥� ��������.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ��砪���...",
        TEXT_TYPE_STATUS,
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY bgBGDisplayPageEntries[] =
{
    {
        4,
        3,
        " ������� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��訫� �� �� ᬥ��� ���� �� ��࠭�.",
        TEXT_STYLE_NORMAL
    },
    {   8,
        10,
         "\x07  ������ ���� �� ��࠭� ��� ��५��� ����� � ������ � ",
         TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   ���᭥� ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ���᭥� ESC, �� �� � ��୥� �� �।室��� ��࠭��, ��� ��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ᬥ��� ���� �� ��࠭�.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �தꫦ�����   ESC = �⪠�   F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY bgBGSuccessPageEntries[] =
{
    {
        4,
        3,
        " ����ன�� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "�᭮���� ���⠢�� �� ����⎑ � ᫮���� �ᯥ譮.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "������� ��᪥�� �� ���ன�⢮ �: �",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "��窨 ���⥫� �� ���筨� ���ன�⢠ (��/DVD)",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "���᭥� ENTER, �� �� �१���᭥� ��������.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �१���᪠�� �� ��������",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY bgBGBootPageEntries[] =
{
    {
        4,
        3,
        " ����ன�� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�������� �� ��०��� (bootloader) �� ��᪠ �� �������� ��",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "�� ���ᯥ譮.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        13,
        "������ �ଠ�࠭� ��᪥� � ���ன�⢮ A:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "� ���᭥� ENTER.",
        TEXT_STYLE_NORMAL,
    },
    {
        0,
        0,
        "   ENTER = �தꫦ�����   F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }

};

static MUI_ENTRY bgBGSelectPartitionEntries[] =
{
    {
        4,
        3,
        " ������� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "������� ��- ���� ���ঠ �����㢠�� �﫮�� � �ࠧ���",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "���� �� ���� �﫮��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "\x07  ���������� ��५��� �� ����� �� ᯨ�ꪠ.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ���᭥� ENTER �� ᫠���� �� ����⎑ �� ���࠭�� ��.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  ���᭥� C �� �ꧤ����� �� ��� ��.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  ���᭥� D �� ���ਢ��� �� �����㢠� ��.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ��砪���...",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY bgBGFormatPartitionEntries[] =
{
    {
        4,
        3,
        " ������� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��ଠ�࠭� �� ��",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        10,
        "���� � �ꤥ �ଠ�࠭. ���᭥� ENTER �� �தꫦ�����.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �தꫦ�����   F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        TEXT_STYLE_NORMAL
    }
};

static MUI_ENTRY bgBGInstallDirectoryEntries[] =
{
    {
        4,
        3,
        " ����ன�� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�������� �� ����⎑ � �ꤠ� ᫮���� �� ���࠭�� ��. ������",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "�����, � ���� �� �ꤥ ᫮��� ����⎑:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "�� �ﭠ �� �।������� ����� ���᭥� BACKSPACE, �� ��",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        15,
        "���ਥ� ����� � ⮣��� ������ ������, � ���� �� �ꤥ",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        16,
        "᫮��� ����⎑.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �தꫦ�����   F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY bgBGFileCopyEntries[] =
{
    {
        4,
        3,
        " ����ன�� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        11,
        12,
        "��砪��� �� �ਪ��� ������� �� 䠩����� � ���࠭�� �����.",
        TEXT_STYLE_NORMAL
    },
    {
        30,
        13,
       // "���࠭�� �����.",
       "",
        TEXT_STYLE_NORMAL
    },
    {
        20,
        14,
        "���� ���� �� �⭥�� �类��� �����.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "                                                           \xB3 ��砪���...      ",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY bgBGBootLoaderEntries[] =
{
    {
        4,
        3,
        " ����ன�� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "���� ᫠����� �� ��०���.",
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
        "������� �� ��०��� �� ��᪥�.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "�� �� � ᫠�� ��०���.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �தꫦ�����   F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY bgBGKeyboardSettingsEntries[] =
{
    {
        4,
        3,
        " ����ன�� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�᪠� �� ᬥ��� ���� �� ����������.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  ���������� ��५���, �� �� ������ ���� �� ���������.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   ���᭥� ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ���᭥� ESC, �� �� � ��୥� �� �।室��� ��࠭��, ��� ��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ᬥ��� ���� �� ����������.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �தꫦ�����   ESC = �⪠�   F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY bgBGLayoutSettingsEntries[] =
{
    {
        4,
        3,
        " ����ன�� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "������ ���ࠧ��࠭� ��������ୠ ���।��.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  ���������� ��५���, �� �� ������ ������� ��������ୠ",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "    ���।�� � ��᫥ ���᭥� ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ���᭥� ESC, �� �� � ��୥� �� �।室��� ��࠭��, ��� ��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ᬥ��� ��������ୠ� ���।��.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �தꫦ�����   ESC = �⪠�   F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY bgBGPrepareCopyEntries[] =
{
    {
        4,
        3,
        " ����ன�� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "���������� � ������� �� ����� �� 䠩����� �� ����⎑. ",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ���⠢ﭥ �� ᯨ�ꪠ �� 䠩���� �� �����...",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY bgBGSelectFSEntries[] =
{
    {
        4,
        3,
        " ����ன�� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        17,
        "�� �� ������ 䠩���� ��⥬� �� ������ ᯨ��:",
        0
    },
    {
        8,
        19,
        "\x07  ������ 䠩���� ��⥬� ��� ��५���.",
        0
    },
    {
        8,
        21,
        "\x07  ���᭥� ENTER, �� �� �ଠ��� �﫠.",
        0
    },
    {
        8,
        23,
        "\x07  ���᭥� ESC, �� �� ������ ��� ��.",
        0
    },
    {
        0,
        0,
        "   ENTER = �தꫦ�����   ESC = �⪠�   F3 = ��室",
        TEXT_TYPE_STATUS
    },

    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY bgBGDeletePartitionEntries[] =
{
    {
        4,
        3,
        " ����ன�� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "���ࠫ� �� �� ���ਥ� ��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "\x07  ���᭥� D, �� �� ���ਥ� �﫠.",
        TEXT_STYLE_NORMAL
    },
    {
        11,
        19,
        "��������: ��窨 ����� �� ⮧� �� � �ꤠ� 㭨鮦���!",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  ���᭥� ESC, �� �� � �⪠���.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   D = ���ਢ��� �� �﫠, ESC = �⪠�    F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY bgBGRegistryEntries[] =
{
    {
        4,
        3,
        " ����ன�� �� ����⎑ " KERNEL_VERSION_STR " . ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "���� ������� �� ��⥬��� ����ன��. ",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   �ꧤ����� �� ॣ����୨� ஥��...",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    },

};

MUI_ERROR bgBGErrorEntries[] =
{
    {
        //ERROR_NOT_INSTALLED
        "����⎑ �� � ���ꫭ� ᫮��� �� ��������\n"
        "��. ��� ᥣ� ������� �� ᫠�����, � ���\n"
        "�� ��᭥� ����ன��� �⭮��, �� �� ���⠫��� ����⎑.\n"
        "\n"
        "  \x07  �� �� �தꫦ� ᫠�����, ���᭥� ENTER.\n"
        "  \x07  �� ��室 ���᭥� F3.",
        "F3 = ��室 ENTER = �தꫦ�����"
    },
    {
        //ERROR_NO_HDD
        "����ன����� �� ����� ��� ���.\n",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_NO_SOURCE_DRIVE
        "����ன����� �� ����� ��室��� � ���ன�⢮.\n",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_LOAD_TXTSETUPSIF
        "����ன����� �� ��� �� ����� 䠩� TXTSETUP.SIF.\n",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_CORRUPT_TXTSETUPSIF
        "����ன����� ����� ���।�� 䠩� TXTSETUP.SIF.\n",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_SIGNATURE_TXTSETUPSIF,
        "����ன����� ����� ��।�� ������ � TXTSETUP.SIF.\n",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_DRIVE_INFORMATION
        "����ன����� �� ��� �� ��ਥ ᢥ������ �� ��⥬��� �� ���ன�⢮.\n",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_WRITE_BOOT,
        "���ᯥ譮 ᫠���� �� ����砢�� ����� (bootcode) �� FAT � ��⥬��� ��.",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_LOAD_COMPUTER,
        "����ன����� �� ��� �� ��।� ᯨ�ꪠ � ������� �������.\n",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_LOAD_DISPLAY,
        "����ன����� �� ��� �� ��।� ᯨ�ꪠ � ���ன�� �� ������.\n",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_LOAD_KEYBOARD,
        "����ன����� �� ��� �� ��।� ᯨ�ꪠ � ������� ���������.\n",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_LOAD_KBLAYOUT,
        "����ன����� �� ��� �� ��।� ᯨ�ꪠ � ��������୨� ���।��.\n",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_WARN_PARTITION,
          "����ன����� ��⠭���, � ���� ���� ��� ��� ���ঠ ���ꢬ��⨬�\n"
          "�﫮�� ⠡���, � ���� �� ���� �� � ࠡ�� �ࠢ����!\n"
          "\n"
          "�ꧤ������ ��� ���ਢ���� �� �﫮�� ���� �� 㭨鮦� �﫮��� ⠡���.\n"
          "\n"
          "  \x07  �� ��室 ���᭥� F3."
          "  \x07  ���᭥� ENTER �� �தꫦ�����.",
          "F3 = ��室 ENTER = �தꫦ�����"
    },
    {
        //ERROR_NEW_PARTITION,
        "�� ����� �� �ꧤ���� ��� �� � ��,\n"
        "���� ��� �����㢠!\n"
        "\n"
        "  * ���᭥� ������, �� �� �தꫦ��.",
        NULL
    },
    {
        //ERROR_DELETE_SPACE,
        "�� ����� �� ���ਥ� ��ࠧ�।������ ��᪮�� ����!\n"
        "\n"
        "  * ���᭥� ������, �� �� �தꫦ��.",
        NULL
    },
    {
        //ERROR_INSTALL_BOOTCODE,
        //"Setup failed to install the FAT bootcode on the system partition.",
        "���ᯥ譮 ᫠���� �� ��㢠�� ��� �� FAT �� ��⥬��� ��.",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_NO_FLOPPY,
        "� ���ன�⢮ A: �ﬠ ���⥫.",
        "ENTER = �தꫦ�����"
    },
    {
        //ERROR_UPDATE_KBSETTINGS,
        "���ᯥ譮 ������� �� ����ன��� �� ��������ୠ� ���।��.",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_UPDATE_DISPLAY_SETTINGS,
        "���ᯥ譮 ������� �� ॣ����୨ ����ன�� �� ������.",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_IMPORT_HIVE,
        "���ᯥ譮 ����ﭥ �� ஥��� 䠩�.",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_FIND_REGISTRY
        "�� ��� ���� 䠩����� � ॣ����୨ �����.",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_CREATE_HIVE,
        "����ன����� �� ��� �� �ꧤ��� ॣ����୨� ஥��.",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_INITIALIZE_REGISTRY,
        //There is something wrong with this line.
        "���ᯥ譮 ����砢��� �� ॣ�����.",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_INVALID_CABINET_INF,
        "Cab 䠩��� �ﬠ �ࠢ���� inf 䠩�.\n",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_CABINET_MISSING,
        "Cab 䠩��� �� � ����.\n",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_CABINET_SCRIPT,
        "Cab 䠩��� �ﬠ ����஥筮 ��ᠭ��.\n",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_COPY_QUEUE,
        "���ᯥ譮 �⢠�ﭥ �� ���誠� �� 䠩���� �� �����.\n",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_CREATE_DIR,
        "���ᯥ譮 �ꧤ����� �� ������ �� ᫠����.",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_TXTSETUP_SECTION,
        "�������� 'Directories' �� �� ����\n"
        "� TXTSETUP.SIF.\n",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_CABINET_SECTION,
        "�������� 'Directories' �� �� ����\n"
        "� cab 䠩��.\n",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_CREATE_INSTALL_DIR
        "���ᯥ譮 �ꧤ����� �� ������ �� ᫠����.",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_FIND_SETUPDATA,
        "�������� 'SetupData' �� �� ����\n"
        "� TXTSETUP.SIF.\n",
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_WRITE_PTABLE,
        "���ᯥ譮 ����ᢠ�� �� �﫮��� ⠡���.\n"
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_ADDING_CODEPAGE,
        "���ᯥ譮 �����ﭥ �� �������� ����� � ॣ�����.\n"
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_UPDATE_LOCALESETTINGS,
        "���ᯥ譮 ��⠭���� �� ���⭨� ����ன��.\n"
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_ADDING_KBLAYOUTS,
        "���ᯥ譮 �����ﭥ �� ��������୨� ���।�� � ॣ�����.\n"
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_UPDATE_GEOID,
        "����ன��� �� ���� �� ��⠭��� �����⥫� �� ������᪮� ���������.\n"
        "ENTER = �१���᪠�� �� ��������"
    },
    {
        //ERROR_INSUFFICIENT_DISKSPACE,
        "�� ���࠭�� �� �ﬠ ������筮 ᢮����� ����࠭�⢮.\n"
        "  * ���᭥� ������, �� �� �தꫦ��.",
        NULL
    },
    {
        NULL,
        NULL
    }
};


MUI_PAGE bgBGPages[] =
{
    {
        LANGUAGE_PAGE,
        bgBGLanguagePageEntries
    },
    {
        START_PAGE,
        bgBGWelcomePageEntries
    },
    {
        INSTALL_INTRO_PAGE,
        bgBGIntroPageEntries
    },
    {
        LICENSE_PAGE,
        bgBGLicensePageEntries
    },
    {
        DEVICE_SETTINGS_PAGE,
        bgBGDevicePageEntries
    },
    {
        REPAIR_INTRO_PAGE,
        bgBGRepairPageEntries
    },
    {
        COMPUTER_SETTINGS_PAGE,
        bgBGComputerPageEntries
    },
    {
        DISPLAY_SETTINGS_PAGE,
        bgBGDisplayPageEntries
    },
    {
        FLUSH_PAGE,
        bgBGFlushPageEntries
    },
    {
        SELECT_PARTITION_PAGE,
        bgBGSelectPartitionEntries
    },
    {
        SELECT_FILE_SYSTEM_PAGE,
        bgBGSelectFSEntries
    },
    {
        FORMAT_PARTITION_PAGE,
        bgBGFormatPartitionEntries
    },
    {
        DELETE_PARTITION_PAGE,
        bgBGDeletePartitionEntries
    },
    {
        INSTALL_DIRECTORY_PAGE,
        bgBGInstallDirectoryEntries
    },
    {
        PREPARE_COPY_PAGE,
        bgBGPrepareCopyEntries
    },
    {
        FILE_COPY_PAGE,
        bgBGFileCopyEntries
    },
    {
        KEYBOARD_SETTINGS_PAGE,
        bgBGKeyboardSettingsEntries
    },
    {
        BOOT_LOADER_PAGE,
        bgBGBootLoaderEntries
    },
    {
        LAYOUT_SETTINGS_PAGE,
        bgBGLayoutSettingsEntries
    },
    {
        QUIT_PAGE,
        bgBGQuitPageEntries
    },
    {
        SUCCESS_PAGE,
        bgBGSuccessPageEntries
    },
    {
        BOOT_LOADER_FLOPPY_PAGE,
        bgBGBootPageEntries
    },
    {
        REGISTRY_PAGE,
        bgBGRegistryEntries
    },
    {
        -1,
        NULL
    }
};

MUI_STRING bgBGStrings[] =
{
    {STRING_PLEASEWAIT,
     "   ��砪���..."},
    {STRING_INSTALLCREATEPARTITION,
     "   ENTER = �������   C = �ꧤ����� �� ��   F3 = ��室"},
    {STRING_INSTALLDELETEPARTITION,
     "   ENTER = �������   D = ���ਢ��� �� ��   F3 = ��室"},
    {STRING_PARTITIONSIZE,
     "������ �� ����� ��:"},
    {STRING_CHOOSENEWPARTITION,
     "���ࠫ� �� �� �ꧤ���� ��� �� ��"},
    {STRING_HDDSIZE,
    "�ꢥ��� ࠧ��� �� ����� �� (� ��������)."},
    {STRING_CREATEPARTITION,
     "   ENTER = �ꧤ����� �� ��   ESC = �⪠�   F3 = ��室"},
    {STRING_PARTFORMAT,
    "�।�⮨ �ଠ�࠭� �� �﫠."},
    {STRING_NONFORMATTEDPART,
    "���ࠫ� �� �� ᫮��� ����⎑ �� ��� ��� ��ࠧ�।���� ��."},
    {STRING_INSTALLONPART,
    "������� �� ����⎑ ����� ��"},
    {STRING_CHECKINGPART,
    "��� �஢�ઠ �� ���࠭�� ��."},
    {STRING_QUITCONTINUE,
    "F3= ��室  ENTER = �தꫦ�����"},
    {STRING_REBOOTCOMPUTER,
    "ENTER = �१���᪠�� �� ��������"},
    {STRING_TXTSETUPFAILED,
    "�� �� ����७ ࠧ��� '%S'\n� TXTSETUP.SIF.\n"},
    {STRING_COPYING,
     "   ����� �� 䠩�: %S"},
    {STRING_SETUPCOPYINGFILES,
     "�������� � ����ᢠ�..."},
    {STRING_REGHIVEUPDATE,
    "   ���६���� �� ॣ����୨� ஥��..."},
    {STRING_IMPORTFILE,
    "   ����ﭥ �� %S..."},
    {STRING_DISPLAYETTINGSUPDATE,
    "   ���६���� ॣ���஢�� ����ன�� �� ��࠭�..."},
    {STRING_LOCALESETTINGSUPDATE,
    "   ���६���� �� ���⭨� ����ன��..."},
    {STRING_KEYBOARDSETTINGSUPDATE,
    "   ���६���� ����ன��� �� ��������୨� ���।��..."},
    {STRING_CODEPAGEINFOUPDATE,
    "   �����ﭥ � ॣ����� �� ᢥ����� �� �������� �����..."},
    {STRING_DONE,
    "   ��⮢�..."},
    {STRING_REBOOTCOMPUTER2,
    "   ENTER = �१���᪠�� �� ��������"},
    {STRING_CONSOLEFAIL1,
    "�⢠�ﭥ� �� �������� � ���ꧬ����\n\n"},
    {STRING_CONSOLEFAIL2,
    "���� � ��碠 ���- ��� �� 㯮�ॡ� �� USB ���������\n"},
    {STRING_CONSOLEFAIL3,
    "�����ꦪ�� �� USB � �� �� ���ꫭ�\n"},
    {STRING_FORMATTINGDISK,
    "��� �ଠ�࠭� �� ��᪠"},
    {STRING_CHECKINGDISK,
    "��� �஢�ઠ �� ��᪠"},
    {STRING_FORMATDISK1,
    " ��ଠ�࠭� �� �﫠 ��� %S 䠩���� �।�� (��৮ �ଠ�࠭�) "},
    {STRING_FORMATDISK2,
    " ��ଠ�࠭� �� �﫠 ��� %S 䠩���� �।�� "},
    {STRING_KEEPFORMAT,
    " ��������� �� 䠩����� �।�� (��� �஬���) "},
    {STRING_HDINFOPARTCREATE,
    "%I64u %s  ��� ��� %lu  (�����=%hu, ����=%hu, ��=%hu) �� %wZ."},
    {STRING_HDDINFOUNK1,
    "%I64u %s  ��� ��� %lu  (�����=%hu, ����=%hu, ��=%hu)."},
    {STRING_HDDINFOUNK2,
    "   %c%c  ��� %lu    %I64u %s"},
    {STRING_HDINFOPARTDELETE,
    "�� %I64u %s  ��� ��� %lu  (�����=%hu, ����=%hu, ��=%hu) �� %wZ."},
    {STRING_HDDINFOUNK3,
    "�� %I64u %s  ��� ��� %lu  (�����=%hu, ����=%hu, ��=%hu)."},
    {STRING_HDINFOPARTZEROED,
    "��� ��� %lu (%I64u %s), �����=%hu, ����=%hu, ��=%hu (%wZ)."},
    {STRING_HDDINFOUNK4,
    "%c%c  ��� %lu    %I64u %s"},
    {STRING_HDINFOPARTEXISTS,
    "�� ��� ��� %lu (%I64u %s), �����=%hu, ����=%hu, ��=%hu (%wZ)."},
    {STRING_HDDINFOUNK5,
    "%c%c  ��� %-3u                         %6lu %s"},
    {STRING_HDINFOPARTSELECT,
    "%6lu %s  ��� ��� %lu  (�����=%hu, ����=%hu, ��=%hu) �� %S"},
    {STRING_HDDINFOUNK6,
    "%6lu %s  ��� ��� %lu  (�����=%hu, ����=%hu, ��=%hu)"},
    {STRING_NEWPARTITION,
    "�� �ꧤ���� ��� �� ��"},
    {STRING_UNPSPACE,
    "    ��ࠧ�।����� ����              %6lu %s"},
    {STRING_MAXSIZE,
    "�� (�� %lu ��)"},
    {STRING_UNFORMATTED,
    "��� (���ଠ�࠭)"},
    {STRING_FORMATUNUSED,
    "�����������"},
    {STRING_FORMATUNKNOWN,
    "�������⥭"},
    {STRING_KB,
    "��"},
    {STRING_MB,
    "��"},
    {STRING_GB,
    "��"},
    {STRING_ADDKBLAYOUTS,
    "�����ﭥ �� ��������୨ ���।��"},
    {0, 0}
};
