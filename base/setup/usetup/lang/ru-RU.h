#pragma once

MUI_LAYOUTS ruRULayouts[] =
{
    { L"0409", L"00000409" },
    { L"0419", L"00000419" },
    { NULL, NULL }
};

static MUI_ENTRY ruRULanguagePageEntries[] =
{
    {
        4,
        3,
        " ��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�롮� �몠",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  ��������, �롥�� ��, �ᯮ��㥬� �� ��⠭����.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   ��⥬ ������ ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ��� �� �㤥� �ᯮ�짮������ �� 㬮�砭�� � ��⠭�������� ��⥬�.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �த������  F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ruRUWelcomePageEntries[] =
{
    {
        4,
        3,
        " ��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "���� ���������� � �ணࠬ�� ��⠭���� Odyssey",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        6,
        11,
        "�� �⮩ �⠤�� ���� ᪮��஢��� 䠩�� ����樮���� ��⥬� Odyssey",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "�� ��� �������� � �����⮢���� ���� �⠤�� ��⠭����.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  ������ ENTER ��� ��⠭���� Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  ������ R ��� ����⠭������� Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  ������ L ��� ��ᬮ�� ��業�������� ᮣ��襭�� Odyssey",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  ������ F3 ��� ��室� �� �ணࠬ�� ��⠭���� Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "��� �������⥫쭮� ���ଠ樨 � Odyssey �����:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        24,
        "http://www.odyssey.ru",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        0,
        0,
        "   ENTER = �த�������  R = ����⠭������� F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ruRUIntroPageEntries[] =
{
    {
        4,
        3,
        " ��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Odyssey ��室���� � ࠭��� �⠤�� ࠧࠡ�⪨ � �� �����ন���� ��",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "�㭪樨 ��� ������ ᮢ���⨬��� � ��⠭��������묨 �ਫ�����ﬨ.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "������� ᫥���騥 ��࠭�祭��:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "- ��⠭���� �������� ⮫쪮 �� ��ࢨ�� ࠧ��� ��᪠",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "- �� ��⠭���� ����� 㤠���� ��ࢨ�� ࠧ��� ��᪠",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "  ���� ������� ���७�� ࠧ���.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "- �� ��⠭���� ����� 㤠���� ���� ���७�� ࠧ��� � ��᪠",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "  ���� �������� ��㣨� ���७�� ࠧ����.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "- �� ��⠭���� �����ন������ ⮫쪮 䠩����� ��⥬� FAT.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "- �஢�ઠ 䠩����� ��⥬� �� �����⢫����.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        23,
        "\x07  ������ ENTER ��� ��⠭���� Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        25,
        "\x07  ������ F3 ��� ��室� �� ��⠭���� Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �த�������   F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ruRULicensePageEntries[] =
{
    {
        4,
        3,
        " ��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        6,
        "��業���:",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        8,
        "Odyssey ��業��஢��� � ᮮ⢥��⢨� � ������ ��業������",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        9,
        "ᮣ��襭��� GNU GPL � ᮤ�ন� ����������, �����࠭塞�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "� ᮢ���⨬묨 ��業��ﬨ: X11, BSD � GNU LGPL.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "�� �ணࠬ���� ���ᯥ祭�� �室�饥 � ��⥬� Odyssey ���饭�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "��� ������ ��業������ ᮣ��襭��� GNU GPL � ��࠭�����",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "��ࢮ��砫쭮� ��業���.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "������ �ணࠬ���� ���ᯥ祭�� ���⠢����� ��� �������� � ��� ��࠭�祭��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "� �ᯮ�짮�����, ��� � ���⭮�, ⠪ � ����㭠த��� �ࠢ�.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "��業��� Odyssey ࠧ�蠥� ��।��� �த�� ���쨬 ��栬.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "�᫨ �� �����-���� ��稭�� �� �� ����稫� ����� ����⮣�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "��業�������� ᮣ��襭�� GNU ����� � Odyssey, �����",
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
        "��࠭⨨:",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        24,
        "�� ᢮������ �ணࠬ���� ���ᯥ祭��; �. ���筨� ��� ��ᬮ�� �ࠢ.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        25,
        "��� ������� ��������; ��� ��࠭⨨ ��������� ��������� ���",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        26,
        "����������� ��� ���������� �����",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = ������",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ruRUDevicePageEntries[] =
{
    {
        4,
        3,
        " ��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "� ᯨ᪥ ���� �ਢ����� ���ன�⢠ � �� ��ࠬ����.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "      ��������:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "          ��࠭:",
        TEXT_STYLE_NORMAL,
    },
    {
        8,
        13,
        "     ���������:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "      ��᪫����:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "      �ਬ�����:",
        TEXT_STYLE_NORMAL
    },
    {
        25,
        16, "�ਬ����� ����� ��ࠬ���� ���ன��",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        19,
        "�� ����� �������� ��ࠬ���� ���ன��, ������� ������ ����� � ����",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        20,
        "��� �뤥����� �����, � ������� ENTER ��� �롮� ��㣨� ��ਠ�⮢",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        21,
        "��ࠬ��஢.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "����� �� ��ࠬ���� ��।�����, �롥�� \"�ਬ����� ����� ��ࠬ����",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        24,
        "���ன��\" � ������ ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �த�������   F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ruRURepairPageEntries[] =
{
    {
        4,
        3,
        " ��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Odyssey ��室���� � ࠭��� �⠤�� ࠧࠡ�⪨ � �� �����ন���� ��",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "�㭪樨 ��� ������ ᮢ���⨬��� � ��⠭��������묨 �ਫ�����ﬨ.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "�㭪�� ����⠭������� � ����� ������ ���������.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  ������ U ��� ���������� ��.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  ������ R ��� ����᪠ ���᮫� ����⠭�������.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  ������ ESC ��� ������ �� ������� ��࠭���.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  ������ ENTER ��� ��१���㧪� ��������.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ESC = ������� ��࠭��  ENTER = ��१���㧪�",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};
static MUI_ENTRY ruRUComputerPageEntries[] =
{
    {
        4,
        3,
        " ��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�� ��� �������� ��⠭��������� ⨯ ��������.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  ������ ������� ����� ��� ���� ��� �롮� �।����⥫쭮�� ⨯�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   ��������. ��⥬ ������ ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ������ ������� ESC ��� ������ � �।��饩 ��࠭�� ��� ���������",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ⨯� ��������.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �த�������   ESC = �⬥��   F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ruRUFlushPageEntries[] =
{
    {
        4,
        3,
        " ��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "���⥬� �஢����, �� �� ����� ����ᠭ� �� ���",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "�� ����� ������ �����஥ �६�.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "��᫥ �����襭�� �������� �㤥� ��⮬���᪨ ��१���㦥�.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ���⪠ ���",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ruRUQuitPageEntries[] =
{
    {
        4,
        3,
        " ��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "Odyssey ��⠭����� �� ���������",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "�������� ������ ��� �� ��᪮���� A: �",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "�� CD-ROM �� CD-��᪮�����.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "������ ENTER ��� ��१���㧪� ��������.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   �������� �������� ...",
        TEXT_TYPE_STATUS,
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ruRUDisplayPageEntries[] =
{
    {
        4,
        3,
        " ��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�� ��� �������� ��⠭��������� ⨯ �࠭�.",
        TEXT_STYLE_NORMAL
    },
    {   8,
        10,
         "\x07  ������ ������ ����� ��� ���� ��� �롮� ⨯� �࠭�.",
         TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   ��⥬ ������ ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ������ ������� ESC ��� ������ � �।��饩 ��࠭�� ��� ���������",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ⨯� �࠭�.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �த�������   ESC = �⬥��   F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ruRUSuccessPageEntries[] =
{
    {
        4,
        3,
        " ��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "�᭮��� ���������� Odyssey �뫨 �ᯥ譮 ��⠭������.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "�������� ������ ��� �� ��᪮���� A: �",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "�� CD-ROM �� CD-��᪮�����.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "������ ENTER ��� ��१���㧪� ��������.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = ��१���㧪�",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ruRUBootPageEntries[] =
{
    {
        4,
        3,
        " ��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�ணࠬ�� ��⠭���� �� ᬮ��� ��⠭����� �����稪 ��",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "���⪨� ��� ��襣� ��������.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        13,
        "�������� ��⠢�� ���ଠ�஢���� ������ ��� � ��᪮��� A: �",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "������ ENTER.",
        TEXT_STYLE_NORMAL,
    },
    {
        0,
        0,
        "   ENTER = �த�������   F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }

};

static MUI_ENTRY ruRUSelectPartitionEntries[] =
{
    {
        4,
        3,
        " ��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "� ᯨ᪥ ���� �������� �������騥 ࠧ���� � ���ᯮ��㥬��",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "����࠭�⢮ ��� ������ ࠧ����.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "\x07  ������ ����� ��� ���� ��� �롮� �����.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ������ ENTER ��� ��⠭���� Odyssey �� �뤥����� ࠧ���.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  ������ C ��� ᮧ����� ������ ࠧ����.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  ������ D ��� 㤠����� �������饣� ࠧ����.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   Please wait...",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ruRUFormatPartitionEntries[] =
{
    {
        4,
        3,
        " ��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��ଠ�஢���� ࠧ����",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        10,
        "��� ��⠭���� ࠧ��� �㤥� ���ଠ�஢��. ������ ENTER ��� �த�������.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �த�������   F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        TEXT_STYLE_NORMAL
    }
};

static MUI_ENTRY ruRUInstallDirectoryEntries[] =
{
    {
        4,
        3,
        " ��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��⠭���� 䠩��� Odyssey �� ��࠭�� ࠧ���. �롥�� ��४���,",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "� ������ �㤥� ��⠭������ ��⥬�:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "�⮡� �������� ��࠭��� ��४���, ������ BACKSPACE � 㤠��� ᨬ����,",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        15,
        "� �� ⥬ ������ ����� ��� ��४�ਨ ��� ��⠭���� Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        16,
        " ",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �த������   F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ruRUFileCopyEntries[] =
{
    {
        4,
        3,
        " ��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        11,
        12,
        "��������, ��������, ���� �ணࠬ�� ��⠭���� ᪮����� 䠩��",
        TEXT_STYLE_NORMAL
    },
    {
        11,
        13,
        "Odyssey � ��⠭������ ��४���.",
        TEXT_STYLE_NORMAL
    },
    {
        11,
        14,
        "�� ����� ������ ��᪮�쪮 �����.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "                                                           \xB3 �������� ��������...    ",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ruRUBootLoaderEntries[] =
{
    {
        4,
        3,
        " ��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��⠭���� �����稪� Odyssey:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "��⠭����� �����稪 �� ���⪨� ��� (MBR � VBR).",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "��⠭����� �����稪 �� ���⪨� ��� (⮫쪮 VBR).",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "��⠭����� �����稪 �� ������ ���.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "�� ��⠭�������� �����稪.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �த������   F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ruRUKeyboardSettingsEntries[] =
{
    {
        4,
        3,
        " ��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��������� ⨯� ����������.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  ������ ����� ��� ���� ��� �롮� �㦭��� ⨯� ����������.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   ��⥬ ������ ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ������ ������� ESC ��� ������ � �।��饩 ��࠭�� ��� ���������",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ⨯� ����������.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �த������   ESC = �⬥��   F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ruRULayoutSettingsEntries[] =
{
    {
        4,
        3,
        " ��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�������� �롥�� �᪫����, ����� �㤥� ��⠭������ �� 㬮�砭��.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  ������ ����� ��� ���� ��� �롮� �㦭�� �᪫���� ����������.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   ��⥬ ������ ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ������ ������� ESC ��� ������ � �।��饩 ��࠭�� ���",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ��������� �᪫���� ����������.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   ENTER = �த������   ESC = �⬥��   F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY ruRUPrepareCopyEntries[] =
{
    {
        4,
        3,
        " ��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�����⮢�� ��襣� �������� � ����஢���� 䠩��� Odyssey. ",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   �����⮢�� ᯨ᪠ �����㥬�� 䠩���...",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY ruRUSelectFSEntries[] =
{
    {
        4,
        3,
        " ��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        17,
        "�롥�� 䠩����� ��⥬� �� ᯨ᪠ ����.",
        0
    },
    {
        8,
        19,
        "\x07  ������ ����� ��� ���� ��� �롮� 䠩����� ��⥬�.",
        0
    },
    {
        8,
        21,
        "\x07  ������ ENTER ��� �ଠ�஢���� ࠧ����.",
        0
    },
    {
        8,
        23,
        "\x07  ������ ESC ��� �롮� ��㣮�� ࠧ����.",
        0
    },
    {
        0,
        0,
        "   ENTER = �த������   ESC = �⬥��   F3 = ��室",
        TEXT_TYPE_STATUS
    },

    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ruRUDeletePartitionEntries[] =
{
    {
        4,
        3,
        " ��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�� ��ࠫ� 㤠����� ࠧ����.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "\x07  ������ D ��� 㤠����� ࠧ����.",
        TEXT_STYLE_NORMAL
    },
    {
        11,
        19,
        "��������: �� ����� � �⮣� ࠧ���� ���� ������!",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  ������ ESC ��� �⬥��.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   D = ������� ࠧ���   ESC = �⬥��   F3 = ��室",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ruRURegistryEntries[] =
{
    {
        4,
        3,
        "��⠭���� Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�ணࠬ�� ��⠭���� �������� ���䨣���� ��⥬�. ",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "   �������� ���⮢ ��⥬���� ॥���...",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    },

};

MUI_ERROR ruRUErrorEntries[] =
{
    {
        //ERROR_NOT_INSTALLED
        "Odyssey �� �� ��������� ��⠭����� �� ���\n"
        "��������. �᫨ �� �멤�� �� ��⠭���� ᥩ��,\n"
        "� ��� �㦭� �������� �ணࠬ�� ��⠭���� ᭮��,\n"
        "�᫨ �� ��� ��⠭����� Odyssey\n"
        "  \x07  ������ ENTER ��� �த������� ��⠭����.\n"
        "  \x07  ������ F3 ��室� �� ��⠭����.",
        "F3 = ��室  ENTER = �த������"
    },
    {
        //ERROR_NO_HDD
        "�� 㤠���� ���� ���⪨� ���.\n",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_NO_SOURCE_DRIVE
        "�� 㤠���� ���� ��⠭����� ���.\n",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_LOAD_TXTSETUPSIF
        "�� 㤠���� ����㧨�� 䠩� TXTSETUP.SIF.\n",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_CORRUPT_TXTSETUPSIF
        "���� TXTSETUP.SIF ���०���.\n",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_SIGNATURE_TXTSETUPSIF,
        "�����㦥�� �����४⭠� ᨣ����� � TXTSETUP.SIF.\n",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_DRIVE_INFORMATION
        "�� 㤠���� ������� ���ଠ�� � ��⥬��� ��᪥.\n",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_WRITE_BOOT,
        "�� 㤠���� ��⠭����� �����稪 FAT �� ��⥬�� ࠧ���.",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_LOAD_COMPUTER,
        "�� 㤠���� ����㧨�� ᯨ᮪ ⨯�� ��������.\n",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_LOAD_DISPLAY,
        "�� 㤠���� ����㧨�� ᯨ᮪ ०���� �࠭�.\n",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_LOAD_KEYBOARD,
        "�� 㤠���� ����㧨�� ᯨ᮪ ⨯�� ����������.\n",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_LOAD_KBLAYOUT,
        "�� 㤠���� ����㧨�� ᯨ᮪ �᪫���� ����������.\n",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_WARN_PARTITION,
        "������ �� �ࠩ��� ��� ���� ���⪨� ���, ����� ᮤ�ন� ࠧ���\n"
        "�������ন����� Odyssey!\n"
        "\n"
        "�������� ��� 㤠����� ࠧ����� ����� ������� ⠡���� ࠧ�����.\n"
        "\n"
        "  \x07  ������ F3 ��� ��室� �� ��⠭����."
        "  \x07  ������ ENTER ��� �த�������.",
        "F3 = ��室  ENTER = �த������"
    },
    {
        //ERROR_NEW_PARTITION,
        "�� �� ����� ᮧ���� ���� ࠧ��� ��᪠ �\n"
        "㦥 �������饬 ࠧ����!\n"
        "\n"
        "  * ������ ���� ������� ��� �த�������.",
        NULL
    },
    {
        //ERROR_DELETE_SPACE,
        "�� �� ����� 㤠���� ��ࠧ�������� ��᪮��� ����࠭�⢮!\n"
        "\n"
        "  * ������ ���� ������� ��� �த�������.",
        NULL
    },
    {
        //ERROR_INSTALL_BOOTCODE,
        "�� 㤠���� ��⠭����� �����稪 FAT �� ��⥬�� ࠧ���.",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_NO_FLOPPY,
        "��� ��᪠ � ��᪮���� A:.",
        "ENTER = �த������"
    },
    {
        //ERROR_UPDATE_KBSETTINGS,
        "�� 㤠���� �������� ��ࠬ���� �᪫���� ����������.",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_UPDATE_DISPLAY_SETTINGS,
        "�� 㤠���� �������� ��ࠬ���� �࠭� � ॥���.",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_IMPORT_HIVE,
        "�� 㤠���� ������஢��� 䠩�� ���⮢ ॥���.",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_FIND_REGISTRY
        "�� 㤠���� ���� 䠩�� ��⥬���� ॥���.",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_CREATE_HIVE,
        "�� 㤠���� ᮧ���� ����� ��⥬���� ॥���.",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_INITIALIZE_REGISTRY,
        "�� 㤠���� ���樠����஢��� ��⥬�� ॥���.",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_INVALID_CABINET_INF,
        "Cabinet �� ����稫 ���४�� inf-䠩�.\n",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_CABINET_MISSING,
        "Cabinet �� ������.\n",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_CABINET_SCRIPT,
        "Cabinet �� ᬮ� ���� ��⠭����� �ਯ�.\n",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_COPY_QUEUE,
        "�� 㤠���� ������ ��।� ����஢���� 䠩���.\n",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_CREATE_DIR,
        "�� 㤠���� ᮧ���� ��⠭����� ��४�ਨ.",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_TXTSETUP_SECTION,
        "�� 㤠���� ���� ᥪ�� 'Directories'\n"
        "� 䠩�� TXTSETUP.SIF.\n",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_CABINET_SECTION,
        "�� 㤠���� ���� ᥪ�� 'Directories'\n"
        "� cabinet.\n",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_CREATE_INSTALL_DIR
        "�� 㤠���� ᮧ���� ��४��� ��� ��⠭����.",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_FIND_SETUPDATA,
        "�� 㤠���� ���� ᥪ�� 'SetupData'\n"
        "� 䠩�� TXTSETUP.SIF.\n",
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_WRITE_PTABLE,
        "�� 㤠���� ������� ⠡���� ࠧ�����.\n"
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_ADDING_CODEPAGE,
        "�� 㤠���� �������� ��ࠬ���� ����஢�� � ॥���.\n"
        "ENTER = ��१���㧪�"
    },
    {
        //ERROR_UPDATE_LOCALESETTINGS,
        "�� 㤠���� ��⠭����� ������ ��⥬�.\n"
        "ENTER = ��१���㧪�"
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


MUI_PAGE ruRUPages[] =
{
    {
        LANGUAGE_PAGE,
        ruRULanguagePageEntries
    },
    {
        START_PAGE,
        ruRUWelcomePageEntries
    },
    {
        INSTALL_INTRO_PAGE,
        ruRUIntroPageEntries
    },
    {
        LICENSE_PAGE,
        ruRULicensePageEntries
    },
    {
        DEVICE_SETTINGS_PAGE,
        ruRUDevicePageEntries
    },
    {
        REPAIR_INTRO_PAGE,
        ruRURepairPageEntries
    },
    {
        COMPUTER_SETTINGS_PAGE,
        ruRUComputerPageEntries
    },
    {
        DISPLAY_SETTINGS_PAGE,
        ruRUDisplayPageEntries
    },
    {
        FLUSH_PAGE,
        ruRUFlushPageEntries
    },
    {
        SELECT_PARTITION_PAGE,
        ruRUSelectPartitionEntries
    },
    {
        SELECT_FILE_SYSTEM_PAGE,
        ruRUSelectFSEntries
    },
    {
        FORMAT_PARTITION_PAGE,
        ruRUFormatPartitionEntries
    },
    {
        DELETE_PARTITION_PAGE,
        ruRUDeletePartitionEntries
    },
    {
        INSTALL_DIRECTORY_PAGE,
        ruRUInstallDirectoryEntries
    },
    {
        PREPARE_COPY_PAGE,
        ruRUPrepareCopyEntries
    },
    {
        FILE_COPY_PAGE,
        ruRUFileCopyEntries
    },
    {
        KEYBOARD_SETTINGS_PAGE,
        ruRUKeyboardSettingsEntries
    },
    {
        BOOT_LOADER_PAGE,
        ruRUBootLoaderEntries
    },
    {
        LAYOUT_SETTINGS_PAGE,
        ruRULayoutSettingsEntries
    },
    {
        QUIT_PAGE,
        ruRUQuitPageEntries
    },
    {
        SUCCESS_PAGE,
        ruRUSuccessPageEntries
    },
    {
        BOOT_LOADER_FLOPPY_PAGE,
        ruRUBootPageEntries
    },
    {
        REGISTRY_PAGE,
        ruRURegistryEntries
    },
    {
        -1,
        NULL
    }
};

MUI_STRING ruRUStrings[] =
{
    {STRING_PLEASEWAIT,
     "   ��������, ��������..."},
    {STRING_INSTALLCREATEPARTITION,
     "   ENTER = ��⠭�����   C = ������� ࠧ���   F3 = ��室"},
    {STRING_INSTALLDELETEPARTITION,
     "   ENTER = ��⠭�����   D = ������� ࠧ���   F3 = ��室"},
    {STRING_PARTITIONSIZE,
     "������ ������ ࠧ����:"},
    {STRING_CHOOSENEWPARTITION,
     "�� ��� ᮧ���� ���� ࠧ��� ��"},
    {STRING_HDDSIZE,
    "��������, ������ ࠧ��� ������ ࠧ���� � ���������."},
    {STRING_CREATEPARTITION,
     "   ENTER = ������� ࠧ���   ESC = �⬥��   F3 = ��室"},
    {STRING_PARTFORMAT,
    "��� ࠧ��� �㤥� ���ଠ�஢�� �����."},
    {STRING_NONFORMATTEDPART,
    "�� ��ࠫ� ��⠭���� Odyssey �� ���� �����ଠ�஢���� ࠧ���."},
    {STRING_INSTALLONPART,
    "Odyssey ��⠭���������� �� ࠧ���:"},
    {STRING_CHECKINGPART,
    "�ணࠬ�� ��⠭���� �஢���� ��࠭�� ࠧ���."},
    {STRING_QUITCONTINUE,
    "F3 = ��室  ENTER = �த�������"},
    {STRING_REBOOTCOMPUTER,
    "ENTER = ��१���㧪�"},
    {STRING_TXTSETUPFAILED,
    "�ணࠬ�� ��⠭���� �� ᬮ��� ���� ᥪ�� '%S'\n� 䠩�� TXTSETUP.SIF.\n"},
    {STRING_COPYING,
     "   ����஢����: %S"},
    {STRING_SETUPCOPYINGFILES,
     "�ணࠬ�� ��⠭���� ������� 䠩��..."},
    {STRING_REGHIVEUPDATE,
    "   ���������� ���⮢ ॥���..."},
    {STRING_IMPORTFILE,
    "   ������஢���� %S..."},
    {STRING_DISPLAYETTINGSUPDATE,
    "   ���������� ��ࠬ��஢ �࠭� � ॥���..."},
    {STRING_LOCALESETTINGSUPDATE,
    "   ���������� ��ࠬ��஢ ������..."},
    {STRING_KEYBOARDSETTINGSUPDATE,
    "   ���������� ��ࠬ��஢ �᪫���� ����������..."},
    {STRING_CODEPAGEINFOUPDATE,
    "   ���������� ���ଠ樨 � ������� ��࠭�� � ॥���..."},
    {STRING_DONE,
    "   �����襭�..."},
    {STRING_REBOOTCOMPUTER2,
    "   ENTER = ��१���㧪�"},
    {STRING_CONSOLEFAIL1,
    "�� 㤠���� ������ ���᮫�\n\n"},
    {STRING_CONSOLEFAIL2,
    "�������� ����⭠� ��稭� �⮣� - �ᯮ�짮����� USB-����������\n"},
    {STRING_CONSOLEFAIL3,
    "USB ���������� ᥩ�� �����ন������ �� ���������\n"},
    {STRING_FORMATTINGDISK,
    "�ணࠬ�� ��⠭���� �ଠ���� ��� ���"},
    {STRING_CHECKINGDISK,
    "�ணࠬ�� ��⠭���� �஢���� ��� ���"},
    {STRING_FORMATDISK1,
    " ��ଠ�஢���� ࠧ���� � 䠩����� ��⥬� %S (����஥) "},
    {STRING_FORMATDISK2,
    " ��ଠ�஢���� ࠧ���� � 䠩����� ��⥬� %S "},
    {STRING_KEEPFORMAT,
    " ��⠢��� ���������� 䠩����� ��⥬� (��� ���������) "},
    {STRING_HDINFOPARTCREATE,
    "%I64u %s  ���⪨� ��� %lu  (����=%hu, ����=%hu, Id=%hu) �� %wZ."},
    {STRING_HDDINFOUNK1,
    "%I64u %s  ���⪨� ��� %lu  (����=%hu, ����=%hu, Id=%hu)."},
    {STRING_HDDINFOUNK2,
    "   %c%c  ������ %lu    %I64u %s"},
    {STRING_HDINFOPARTDELETE,
    "�� %I64u %s  ���⪨� ��� %lu  (����=%hu, ����=%hu, Id=%hu) �� %wZ."},
    {STRING_HDDINFOUNK3,
    "�� %I64u %s  ���⪨� ��� %lu  (����=%hu, ����=%hu, Id=%hu)."},
    {STRING_HDINFOPARTZEROED,
    "���⪨� ��� %lu (%I64u %s), ����=%hu, ����=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK4,
    "%c%c  ������ %lu    %I64u %s"},
    {STRING_HDINFOPARTEXISTS,
    "�� ���⪮� ��᪥ %lu (%I64u %s), ����=%hu, ����=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK5,
    "%c%c  ������ %-3u                         %6lu %s"},
    {STRING_HDINFOPARTSELECT,
    "%6lu %s  ���⪨� ��� %lu  (����=%hu, ����=%hu, Id=%hu) �� %S"},
    {STRING_HDDINFOUNK6,
    "%6lu %s  ���⪨� ��� %lu  (����=%hu, ����=%hu, Id=%hu)"},
    {STRING_NEWPARTITION,
    "�ணࠬ�� ��⠭���� ᮧ���� ���� ࠧ��� ��:"},
    {STRING_UNPSPACE,
    "    ��ࠧ��祭��� ����࠭�⢮              %6lu %s"},
    {STRING_MAXSIZE,
    "�� (����. %lu ��)"},
    {STRING_UNFORMATTED,
    "���� (�����ଠ�஢����)"},
    {STRING_FORMATUNUSED,
    "�� �ᯮ�짮����"},
    {STRING_FORMATUNKNOWN,
    "���������"},
    {STRING_KB,
    "��"},
    {STRING_MB,
    "��"},
    {STRING_GB,
    "��"},
    {STRING_ADDKBLAYOUTS,
    "Adding keyboard layouts"},
    {0, 0}
};
