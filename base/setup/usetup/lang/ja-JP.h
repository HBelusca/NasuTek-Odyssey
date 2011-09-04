#pragma once

MUI_LAYOUTS jaJPLayouts[] =
{
//    { L"0411", L"e0010411" },
    { L"0411", L"00000411" },
    { NULL, NULL }
};

static MUI_ENTRY jaJPLanguagePageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��ݺ�� ����",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  �ݽİټ�� �ֳ�� ��ݺަ ���� �� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   ·�Ƥ ENTER ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ���� ���� �� ��ݺ�� �����÷� ����� �ò� ��ݺ�� ��ò��Ͻ�",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = �ޯ��  F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY jaJPWelcomePageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Odyssey ��ı���� ֳ��",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        6,
        11,
        "�� ��ı���� ��ݶ���� Odyssey ���ڰèݸ޼��Ѧ",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "���߭��� ��߰�� ��ı���� ·�� ��ݶ��� �ޭ��ަ �Ͻ�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Odyssey � �ݽİ� ���� ENTER ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Odyssey � ���̸ Ӽ�� ���� ���� R ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  Odyssey � ײ�ݽ�ޮ��ݦ ˮ��� ���� L ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Odyssey � �ݽİ� ���� ������ �ޱ�� F3 ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "Odyssey � ������ �ޮ�γ� �ݼ�� ��� ���� ��޻�:",
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
        "ENTER = �ޯ��  R = ���̸  L = ײ�ݽ  F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY jaJPIntroPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Odyssey ��ı���� ���������ݶ�� ��Ͻ� ���Ҥ ���",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "�ޭ����� �ֳ�޷� ��ı��� ���ع����� ����� �ɳ� ��߰� ��Ͼݡ",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "·�� ��Ը�� ÷ֳ ��Ͻ:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "- ��ı���� 1�� �ި��� · 1²�ޮ�� ��ײ�� �߰è��ݦ �¶��Ķ� �޷Ͼݡ",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "- ����� �߰è��ݶ� �� �ި���ޮ�� �ݻ޲�� �ޱ�� ��ı����",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "  ��ײ�� �߰è��ݦ �ި���� ���ޮ �޷Ͼݡ",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "- �� ����� خ����� �� �ި���ޮ�� �ݻ޲ �ò� �ޱ�� ��ı����",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "  ����� ����� خ���� �ި���� ���ޮ �޷Ͼݡ",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "- ��ı���� FAT ̧�ټ��� �� ��߰� �Ͻ�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "- ̧�ټ���� �����ɳ� ��� �ޯ�� ��� �Ͼݡ",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        23,
        "\x07  Odyssey � �ݽİ� ���� ENTER ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        25,
        "\x07  Odyssey � �ݽİپ��� ���� �� �ޱ�� F3 ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = �ޯ��   F3 = ����",
        TEXT_TYPE_STATUS| TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY jaJPLicensePageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        6,
        "�ֳ ���޸:",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        8,
        "�� Odyssey ����� �бܾ �ɳ� ײ�ݽ(X11 ԤBSD ����",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        9,
        "GNU LGPL ײ�ݽ ���)� ���ަ ̸� �߰�� ��� GNU GPL �",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "�ޮ���� ��� ײ�ݽ ��òϽ�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "Odyssey ����� ����� ��ĳ��� ����� Odyssey ����� ������",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "��մ� GNU GPL �޹��Ÿ� �� ��ĳ��� �ؼ��� ײ�ݽ� ����",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "�ذ� ��� �Ͻ�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "�� ��ĳ��� '�μ��'�� ó��� �ڤ ���γ� ����γ�� ÷�����",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "�ޱ�� ɿ޲ä �ֳ ����ݦ ��Ͻ� Odyssey � ײ�ݽ �����",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "�޲�ݼ��� ���޲� ̾�� �޹�޽�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "GNU General Public License � Odyssey � ��� ����Ŷ��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "�ޱ�ʤ ·ަ �޻ݼ�� ��޻�",
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
        "�ݼ� μ��:",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        24,
        "��� �ذ ��ĳ���޽� ������ ���� ��߰�ޮ��ݦ ���� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        25,
        "μ��� ϯ�� '��Ͼ�'� '��ޮ���' � '÷�޳��'",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        26,
        "Ƶ��� �޳ֳ�޽�",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY jaJPDevicePageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��� ؽ�� ��ݻ޲� ���޲� ��ò�޽�",
        TEXT_STYLE_NORMAL
    },
    {
        24,
        11,
        "���߭��:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        12,
        "�ި���ڲ:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        13,
        "���ް��:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        14,
        "���ް�� ڲ���:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        16,
        "�ޭ�޸:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        25,
        16, "���� ���޲�� ��ò� �ޭ�޸ ��",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        19,
        "UP Ӽ�� DOWN ��� ��� ���ذ� ���� �� �����",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        20,
        "ʰ�޳��� ��ò� �ݺ� �޷Ͻ� ���� ��פ ENTER ��� ��� ÷��� ��ò�",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        21,
        "���� �Ͻ�",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "���� ��ò�� ���� ��޼� �ޱ�ʤ \"���� ���޲�� ��ò� �ޭ�޸ ��\" � ���� �ä",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        24,
        "ENTER ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = �ޯ��   F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY jaJPRepairPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Odyssey ��ı���� ������� ��ݶ�� ��Ͻ� ���Ҥ ���",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "�ޭ����� �ֳ �޷� ��ı��� ���ع����� ����� �ɳ� ��߰� ��Ͼݡ",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "���̸�ɳ� ��� �ޯ�� ��� �Ͼݡ",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  OS � ���� ���� U ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  ��̸ �ݿ�٦ �׸�� R ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  Ҳ��߰��� ������ ESC ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  ���߭��� ����޳ ���� ENTER ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ESC = Ҳ��߰��  U = ����  R = ��̸  ENTER = ����޳",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};
static MUI_ENTRY jaJPComputerPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�ݽİ� ��� ���߭��� ��ٲ� �ݺ� �� �Ķ� ���� ��ϼ��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  UP Ӽ�� DOWN ��� ��� ÷�� ���߭��� ��ٲ� ���� �� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   ·�� ENTER ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ���߭��� ��ٲ� �ݺ����� ϴ� �߰��� ���� �ޱ�� ESC ���",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = �ޯ��   ESC = ��ݾ�   F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY jaJPFlushPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "����� ������ ����� �ް��� �ި���ޮ�� ο�� ��� ֳ� �òϽ",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "���� ������ �޶ݶ� ��� �ޱ��� ��Ͻ",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "��خ��ޤ ���߭��� ���޳�� ����޳ ��Ͻ",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "������ ����� �� �Ͻ",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY jaJPQuitPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "Odyssey � �ݾ��� �ݽİ� ��Ͼ��޼�",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "��ײ�� A: �� �ۯ�߰ �ި��� CD ��ײ�޶�",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "����� CD-ROM � ���޼� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "���߭��� ����޳ ���� ENTER ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "��� ��޻� ...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG,
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY jaJPDisplayPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�ݽİ� ��� �ި���ڲ� ��ٲ� �ݺ� �� �Ķ� ���� ��ϼ��",
        TEXT_STYLE_NORMAL
    },
    {   8,
        10,
         "\x07  UP Ӽ�� DOWN ��� ��� ÷�� �ި���ڲ� ��ٲ� ���� �� ��޻��",
         TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   ·�� ENTER ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  �ި���ڲ� ��ٲ� �ݺ����� ϴ� �߰��� ������ ESC ���",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = �ޯ��   ESC = ��ݾ�   F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY jaJPSuccessPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "Odyssey � ��������� �ݽİ�� ���� �ϼ��",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "��ײ�� A: �� �ۯ�߰ �ި��� CD ��ײ�޶�",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "����� CD-ROM � ���޼� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "���߭��� ����޳ ���� ENTER ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = ���߭��� ����޳",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY jaJPBootPageEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��ı���� �ް�۰�ަ ���߭��� ʰ���ި���ޮ�� �ݽİ�",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "�޷Ͼ��޼�",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        13,
        "��ײ�� A: � ̫�ϯ� ��� �ۯ�߰ �ި��� ��ä",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "ENTER ��� ��� ��޻��",
        TEXT_STYLE_NORMAL,
    },
    {
        0,
        0,
        "ENTER = �ޯ��   F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }

};

static MUI_ENTRY jaJPSelectPartitionEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��� ؽ�� �ݻ޲�� �߰è���� �ݷ �߰è���� ÷�� мֳ�",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "�ި�� ��߰�� �����޽�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "\x07  UP Ӽ�� DOWN ��� ��� ؽ� ���ذ� ���� �� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ������� �߰è���� Odyssey � �ݽİ� ���� ENTER ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  ��׼� �߰è��� � ���� ���� C ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  ���� �߰è��ݦ ���ޮ ���� D ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "��� ��޻�...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY jaJPFormatPartitionEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�߰è���� ̫�ϯ�",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        10,
        "��ı���� �߰è��ݦ ̫�ϯ� �Ͻ� �ޯ�� ���� ENTER ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = �ޯ��   F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        TEXT_STYLE_NORMAL
    }
};

static MUI_ENTRY jaJPInstallDirectoryEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��ı���� Odyssey � ̧�٦ ���� ��� �߰è��ݼޮ�� �ݽİ� �Ͻ� Odyssey �",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "�ݽİٽ� �ިڸ�ئ ��ò �� ��޻�:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "����� �ިڸ�ئ �ݺ����ʤ BACKSPACE ���� Ӽަ ���ޮ �� ��Ƥ",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        15,
        "Odyssey � �ݽİ� �� �ިڸ�ئ ƭ�خ�",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        16,
        "�� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = �ޯ��   F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY jaJPFileCopyEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        0,
        12,
        "Odyssey ��ı��߶� Odyssey �ݽİ� ̫���� ̧�٦",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        13,
        "��߰ �� ���� ���׸ ��� ��޻��",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        14,
        "��خ����� ���� ��� �ޱ��� ��Ͻ�",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        50,
        0,
        "\xB3 ��� ��޻�...    ",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY jaJPBootLoaderEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��ı���� �ް� ۰�ަ �ݽİ� �Ͻ",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "ʰ���ި��� �ް�۰�ް� �ݽİ� �Ͻ (MBR � VBR).",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "ʰ���ި��� �ް�۰�ް� �ݽİ� �Ͻ (VBR ��).",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "�ް�۰�ަ �ۯ�߰ �ި��� �ݽİ� �١",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "�ް�۰��� �ݽİ٦ ����� �١",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = �ޯ��   F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY jaJPKeyboardSettingsEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�ݽİ� ��� ���ް��� ��ٲ� �ݺ� �� �Ķ� ���� ��ϼ��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  UP Ӽ�� DOWN ��� ��� ÷�� ���ް��� ��ٲ� ���� �� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   ·�� ENTER ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ���ް��� ��ٲ� �ݺ� ���� ϴ� �߰��� ������ ESC���",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = �ޯ��   ESC = ��ݾ�   F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY jaJPLayoutSettingsEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�ò� ڲ���ļ� �ݽİ� ��� ڲ��Ħ ���� �� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  UP Ӽ�� DOWN ��� ��� ÷�� ���ް�� ڲ��Ħ ���� �ä",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "    ·�� ENTER ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  ���ް�� ڲ��Ħ �ݺ� ���� ϴ� �߰��� ������ ESC ���",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER = �ޯ��   ESC = ��ݾ�   F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY jaJPPrepareCopyEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��ı���� Odyssey �̧�٦ ���߭��� ��߰ �� �ޭ��ަ �� �Ͻ� ",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "��߰ �� ̧���ؽĦ ���� ���...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY jaJPSelectFSEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        17,
        "��� ؽĶ� ̧�� ���Ѧ ���� �� ��޻��",
        0
    },
    {
        8,
        19,
        "\x07  UP Ӽ�� DOWN ��� ��� ̧�� ���Ѧ ���� �� ��޻��",
        0
    },
    {
        8,
        21,
        "\x07  �߰è��ݦ ̫�ϯ� ���� ENTER ��� ��� ��޻��",
        0
    },
    {
        8,
        23,
        "\x07  ���� �߰è��ݦ ���� �� �ޱ�� ESC ��� ��� ��޻��",
        0
    },
    {
        0,
        0,
        "ENTER = �ޯ��   ESC = ��ݾ�   F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },

    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY jaJPDeletePartitionEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "�� �߰è��ݦ ���ޮ �� �Ķ� ���� ��ϼ�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "\x07  �� �߰è��ݦ ���ޮ ���� D ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        11,
        19,
        "����(WARNING): �� �߰è��ݼޮ�� ����� �ް�� �����Ͻ!",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  ��ݾ� ���� ESC ��� ��� ��޻��",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "D = �߰ü�� ���ޮ   ESC = ��ݾ�   F3 = ����",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY jaJPRegistryEntries[] =
{
    {
        4,
        3,
        " Odyssey " KERNEL_VERSION_STR " ��ı��� ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "��ı���� ����� ����� ���� �òϽ� ",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ڼ޽�� ʲ�ަ ���� ���...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

MUI_ERROR jaJPErrorEntries[] =
{
    {
        //ERROR_NOT_INSTALLED
        "Odyssey � ���߭��� �ݾ��� �ݽİ�\n"
        "��Ͼݡ ��ı��ߦ ���� �� �ޱ�� Odyssey � �ݽİ� ���� ��ı��ߦ\n"
        "ӳ���� �ޯ�� �� ��ֳ�� ��Ͻ�\n"
        "\n"
        "  \x07  ��ı��ߦ �ޯ�� ���� ENTER ��� ��� ��޻��\n"
        "  \x07  ��ı��ߦ ���� ���� F3 ��� ��� ��޻��",
        "F3 = ����  ENTER = �ޯ��"
    },
    {
        //ERROR_NO_HDD
        "��ı���� ʰ���ި��� �ݼ�� �޷Ͼ� �޼��\n",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_NO_SOURCE_DRIVE
        "��ı���� ��� ��ײ�ަ �ݼ�� �޷Ͼ� �޼��\n",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_LOAD_TXTSETUPSIF
        "��ı���� ̧�� TXTSETUP.SIF � �к�� ���߲ �ϼ��\n",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_CORRUPT_TXTSETUPSIF
        "��ı����  TXTSETUP.SIF �� ʿ� �ò� �Ħ �ݼ�� �ϼ��\n",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_SIGNATURE_TXTSETUPSIF,
        "��ı���� TXTSETUP.SIF � Ѻ�� ��Ҳ� �ݼ�� �ϼ��\n",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_DRIVE_INFORMATION
        "��ı���� ���� ��ײ��� �ޮ�γ� ���� �޷Ͼ� �޼��\n",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_WRITE_BOOT,
        "��ı���� ���� �߰è��� �ޮ��� FAT �ްĺ���� �ݽİ�� ���߲ �ϼ��",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_LOAD_COMPUTER,
        "��ı���� ���߭��� ��ٲ ؽ�� �к�� ���߲ �ϼ��\n",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_LOAD_DISPLAY,
        "��ı���� �ި���ڲ� ��ò ؽ�� �к�� ���߲ �ϼ��\n",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_LOAD_KEYBOARD,
        "��ı���� ���ް��� ��ٲ ؽ�� �к�� ���߲ �ϼ��\n",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_LOAD_KBLAYOUT,
        "��ı���� ���ް�� ڲ��� ؽ�� �к�� ���߲ �ϼ��\n",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_WARN_PARTITION,
          "��ı���� ��Ÿ�� 1�� ʰ���ި���� ÷��� �¶�Ų �޶ݾ�� Ų\n"
          "�߰è��� ð��٦ ̸� �Ħ �ݼ�� �ϼ�!\n"
          "\n"
          "�߰è��ݦ ���� ��� ���ޮ �ä �� �߰è��� ð��٦ ʶ� �޷Ͻ\n"
          "\n"
          "  \x07  ��ı��ߦ ���� ���� F3 ��� ��� ��޻��\n"
          "  \x07  �ޯ�� ���� ENTER ��� ��� ��޻��",
          "F3= ����  ENTER = �ޯ��"
    },
    {
        //ERROR_NEW_PARTITION,
        "��׼� �߰è��ݦ ���� �ݻ޲��\n"
        "�߰è���� Ų��� ���� �ٺ�� �޷Ͼ�!\n"
        "\n"
        "  * �ޯ�� ���� �ƶ ��� ��� ��޻��",
        NULL
    },
    {
        //ERROR_DELETE_SPACE,
        "���ݶ� � �ި�� ��߰� � ���ޮ �ٺ�� �޷Ͼ�!\n"
        "\n"
        "  * �ޯ�� ���� �ƶ ��� ��� ��޻��",
        NULL
    },
    {
        //ERROR_INSTALL_BOOTCODE,
        "��ı���� ���� �߰è��� �ޮ��� FAT �ް� ں��� � �ݽİ�� ���߲ �ϼ��",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_NO_FLOPPY,
        "��ײ�� A: � �ި���� ��Ͼݡ",
        "ENTER = �ޯ��"
    },
    {
        //ERROR_UPDATE_KBSETTINGS,
        "��ı���� ���ް�� ڲ��� � ��ò� ����� ���߲ �ϼ��",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_UPDATE_DISPLAY_SETTINGS,
        "��ı���� �ި���ڲ� ڼ޽�� ��ò� ����� ���߲ �ϼ��",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_IMPORT_HIVE,
        "��ı���� ʲ�� ̧��� ���߰�� ���߲ �ϼ��",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_FIND_REGISTRY
        "��ı���� ڼ޽�� �ް� ̧��� �ݼ��� ���߲ �ϼ��",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_CREATE_HIVE,
        "��ı���� ڼ޽�� ʲ��� ����� ���߲ �ϼ��",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_INITIALIZE_REGISTRY,
        "��ı���� ڼ޽��� ����� ���߲ �ϼ��",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_INVALID_CABINET_INF,
        "����ȯ�� Ѻ�� inf ̧�ٶ� ̸��� �Ͻ�\n",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_CABINET_MISSING,
        "����ȯĶ� �¶�Ͼݡ\n",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_CABINET_SCRIPT,
        "����ȯ�Ų� ��ı��� �����Ķ� �¶�Ͼݡ\n",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_COPY_QUEUE,
        "��ı���� ��߰ ̧�� ���� ������ ���߲ �ϼ��\n",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_CREATE_DIR,
        "��ı���� �ݽİ� �ިڸ�ئ ���� �޷Ͼ� �޼��",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_TXTSETUP_SECTION,
        "��ı���� TXTSETUP.SIF Ų� 'Directories' ������ �ݻ��\n"
        "���߲ �ϼ��\n",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_CABINET_SECTION,
        "��ı���� ����ȯ�Ų� 'Directories' ������ �ݻ��\n"
        "���߲ �ϼ��\n",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_CREATE_INSTALL_DIR
        "��ı���� �ݽİ� �ިڸ�ئ ���� �޷Ͼ� �޼��",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_FIND_SETUPDATA,
        "��ı���� TXTSETUP.SIF Ų� 'SetupData' ������ �ݻ��\n"
        "���߲ �ϼ��\n",
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_WRITE_PTABLE,
        "��ı���� �߰è��� ð���� ����� ���߲ �ϼ��\n"
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_ADDING_CODEPAGE,
        "��ı���� ڼ޽���� �����߰��� ²�� ���߲ �ϼ��\n"
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_UPDATE_LOCALESETTINGS,
        "��ı���� ���� ۹�٦ ��ò �޷Ͼ� �޼��\n"
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_ADDING_KBLAYOUTS,
        "��ı���� ڼ޽���� ���ް�� ڲ���� ²�� ���߲ �ϼ��\n"
        "ENTER = ���߭��� ����޳"
    },
    {
        //ERROR_UPDATE_GEOID,
        "��ı���� geo id � ��ò �޷Ͼ� �޼��\n"
        "ENTER = ���߭��� ����޳"
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

MUI_PAGE jaJPPages[] =
{
    {
        LANGUAGE_PAGE,
        jaJPLanguagePageEntries
    },
    {
        START_PAGE,
        jaJPWelcomePageEntries
    },
    {
        INSTALL_INTRO_PAGE,
        jaJPIntroPageEntries
    },
    {
        LICENSE_PAGE,
        jaJPLicensePageEntries
    },
    {
        DEVICE_SETTINGS_PAGE,
        jaJPDevicePageEntries
    },
    {
        REPAIR_INTRO_PAGE,
        jaJPRepairPageEntries
    },
    {
        COMPUTER_SETTINGS_PAGE,
        jaJPComputerPageEntries
    },
    {
        DISPLAY_SETTINGS_PAGE,
        jaJPDisplayPageEntries
    },
    {
        FLUSH_PAGE,
        jaJPFlushPageEntries
    },
    {
        SELECT_PARTITION_PAGE,
        jaJPSelectPartitionEntries
    },
    {
        SELECT_FILE_SYSTEM_PAGE,
        jaJPSelectFSEntries
    },
    {
        FORMAT_PARTITION_PAGE,
        jaJPFormatPartitionEntries
    },
    {
        DELETE_PARTITION_PAGE,
        jaJPDeletePartitionEntries
    },
    {
        INSTALL_DIRECTORY_PAGE,
        jaJPInstallDirectoryEntries
    },
    {
        PREPARE_COPY_PAGE,
        jaJPPrepareCopyEntries
    },
    {
        FILE_COPY_PAGE,
        jaJPFileCopyEntries
    },
    {
        KEYBOARD_SETTINGS_PAGE,
        jaJPKeyboardSettingsEntries
    },
    {
        BOOT_LOADER_PAGE,
        jaJPBootLoaderEntries
    },
    {
        LAYOUT_SETTINGS_PAGE,
        jaJPLayoutSettingsEntries
    },
    {
        QUIT_PAGE,
        jaJPQuitPageEntries
    },
    {
        SUCCESS_PAGE,
        jaJPSuccessPageEntries
    },
    {
        BOOT_LOADER_FLOPPY_PAGE,
        jaJPBootPageEntries
    },
    {
        REGISTRY_PAGE,
        jaJPRegistryEntries
    },
    {
        -1,
        NULL
    }
};

MUI_STRING jaJPStrings[] =
{
    {STRING_PLEASEWAIT,
     "   ��� ��޻�..."},
    {STRING_INSTALLCREATEPARTITION,
     "   ENTER = �ݽİ�   C = �߰è��� ����   F3 = ����"},
    {STRING_INSTALLDELETEPARTITION,
     "   ENTER = �ݽİ�   D = �߰è��� ���ޮ   F3 = ����"},
    {STRING_PARTITIONSIZE,
     "��׼� �߰è���� ����:"},
    {STRING_CHOOSENEWPARTITION,
     "��׼� �߰è��ݦ ·�� ������ �Ķ� ���� ��ϼ�:"},
    {STRING_HDDSIZE,
    "��׼� �߰è���� ���ަ Ҷ��޲� �ݲ�� ƭ�خ� �ø�޻��"},
    {STRING_CREATEPARTITION,
     "   ENTER = �߰è��� ����   ESC = ��ݾ�   F3 = ����"},
    {STRING_PARTFORMAT,
    "�� �߰è���� ·�� üޭ��� ̫�ϯ� ��Ͻ�"},
    {STRING_NONFORMATTEDPART,
    "Odyssey� �ݷ ��� �̫�ϯ�� �߰è���� �ݽİٽ� �Ķ� ���� ��ϼ��"},
    {STRING_INSTALLONPART,
    "��ı���� Odyssey� �߰è��� �ޮ�� �ݽİټϽ�"},
    {STRING_CHECKINGPART,
    "��ı���� ���� ��� �߰è��ݦ �ݻ �òϽ�"},
    {STRING_QUITCONTINUE,
    "F3= ����  ENTER = �ޯ��"},
    {STRING_REBOOTCOMPUTER,
    "ENTER = ���߭��� ����޳"},
    {STRING_TXTSETUPFAILED,
    "��ı���� TXTSETUP.SIF � '%S' ������ �ݼ���\n���߲ �ϼ��\n"},
    {STRING_COPYING,
     "   ��߰ ���� ̧��: %S"},
    {STRING_SETUPCOPYINGFILES,
     "��ı���� ̧�٦ ��߰ �� �Ͻ..."},
    {STRING_REGHIVEUPDATE,
    "   ڼ޽�� ʲ��� ���� ���..."},
    {STRING_IMPORTFILE,
    "   %S � ���߰� ���..."},
    {STRING_DISPLAYETTINGSUPDATE,
    "   �ި���ڲ ڼ޽�� ��ò� ���� ���..."},
    {STRING_LOCALESETTINGSUPDATE,
    "   ��� ��ò� ���� ���..."},
    {STRING_KEYBOARDSETTINGSUPDATE,
    "   ���ް�� ڲ���� ��ò� ���� ���..."},
    {STRING_CODEPAGEINFOUPDATE,
    "   ���� �߰�� � �ޮ�γ� ڼ޽��� ²� ���..."},
    {STRING_DONE,
    "   ��خ�..."},
    {STRING_REBOOTCOMPUTER2,
    "   ENTER = ���߭��� ����޳"},
    {STRING_CONSOLEFAIL1,
    "�ݿ�٦ ����� �޷Ͼ�\n\n"},
    {STRING_CONSOLEFAIL2,
    "�����÷� ��ݲ� ļ� USB ���ް�� � ¶�� �ٺĶ� �ݶ޴�� Ͻ\n"},
    {STRING_CONSOLEFAIL3,
    "USB ���ް�� � ��� �ݾ��� ��߰� ��� �Ͼ�\n"},
    {STRING_FORMATTINGDISK,
    "��ı���� �ި��� ̫�ϯ� �òϽ"},
    {STRING_CHECKINGDISK,
    "��ı���� �ި��� �ݻ �òϽ�"},
    {STRING_FORMATDISK1,
    " �߰è��ݦ %S ̧�� ���� ��̫�ϯ� (���� ̫�ϯ�) "},
    {STRING_FORMATDISK2,
    " �߰è��ݦ %S ̧�� ���� ��̫�ϯ� "},
    {STRING_KEEPFORMAT,
    " ��ݻ޲� ̧�� ����� �� (�ݺ� �Ų) "},
    {STRING_HDINFOPARTCREATE,
    "%I64u %s  ʰ���ި�� %lu  (�߰�=%hu, �޽=%hu, Id=%hu) on %wZ."},
    {STRING_HDDINFOUNK1,
    "%I64u %s  ʰ���ި�� %lu  (�߰�=%hu, �޽=%hu, Id=%hu)."},
    {STRING_HDDINFOUNK2,
    "   %c%c  ��ٲ %lu    %I64u %s"},
    {STRING_HDINFOPARTDELETE,
    "on %I64u %s  ʰ���ި�� %lu  (�߰�=%hu, �޽=%hu, Id=%hu) on %wZ."},
    {STRING_HDDINFOUNK3,
    "on %I64u %s  ʰ���ި�� %lu  (�߰�=%hu, �޽=%hu, Id=%hu)."},
    {STRING_HDINFOPARTZEROED,
    "ʰ���ި�� %lu (%I64u %s), �߰�=%hu, �޽=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK4,
    "%c%c  ��ٲ %lu    %I64u %s"},
    {STRING_HDINFOPARTEXISTS,
    "on ʰ���ި�� %lu (%I64u %s), �߰�=%hu, �޽=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK5,
    "%c%c  ��ٲ %-3u                         %6lu %s"},
    {STRING_HDINFOPARTSELECT,
    "%6lu %s  ʰ���ި�� %lu  (�߰�=%hu, �޽=%hu, Id=%hu) on %S"},
    {STRING_HDDINFOUNK6,
    "%6lu %s  ʰ���ި�� %lu  (�߰�=%hu, �޽=%hu, Id=%hu)"},
    {STRING_NEWPARTITION,
    "��ı���� ��׼� �߰è��ݦ ·�� �����ϼ�:"},
    {STRING_UNPSPACE,
    "    ���ݶ�� ��߰�              %6lu %s"},
    {STRING_MAXSIZE,
    "MB (���޲. %lu MB)"},
    {STRING_UNFORMATTED,
    "�ݷ (�̫�ϯ�)"},
    {STRING_FORMATUNUSED,
    "мֳ"},
    {STRING_FORMATUNKNOWN,
    "�Ҳ"},
    {STRING_KB,
    "KB"},
    {STRING_MB,
    "MB"},
    {STRING_GB,
    "GB"},
    {STRING_ADDKBLAYOUTS,
    "���ް�� ڲ���� ²� ���"},
    {0, 0}
};
