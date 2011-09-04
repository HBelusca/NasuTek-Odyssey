#pragma once

MUI_LAYOUTS ptBRLayouts[] =
{
    { L"0416", L"00000416" },
    { L"0409", L"00000409" },
    { NULL, NULL }
};

static MUI_ENTRY ptBRLanguagePageEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Sele��o do idioma",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Por favor, selecione o idioma a ser utilizado durante a instala��o.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Ent�o pressione ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  O idioma selecionado tamb�m ser� o idioma padr�o do sistema.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER=Continuar  F3=Sair",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ptBRWelcomePageEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Bem-vindo � instala��o do Odyssey.",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        6,
        11,
        "Esta parte da instala��o prepara o Odyssey para ser",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "executado em seu computador.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Para instalar o Odyssey agora, pressione ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Para reparar uma instala��o do Odyssey, pressione R.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  Para ver os termos e condi��es da licen�a, pressione L.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Para sair sem instalar o Odyssey, pressione F3.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "Para maiores informa��es sobre o Odyssey, visite o s�tio:",
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
        "ENTER=Continuar  R=Reparar  L=Licen�a  F3=Sair",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ptBRIntroPageEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "O instalador do Odyssey est� em fase inicial de desenvolvimento e",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "ainda n�o suporta todas as fun��es de instala��o.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        11,
        "As seguintes limita��es se aplicam:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "- O instalador n�o suporta mais de uma parti��o prim�ria por disco.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "- O instalador n�o pode excluir uma parti��o prim�ria de um disco",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "  se houverem parti��es estendidas no mesmo disco.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "- O instalador n�o pode remover a primeira parti��o estendida de um",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "  disco se existirem outras parti��es estendidas no mesmo disco.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "- O instalador suporta somente o sistema de arquivos FAT.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "- O verificador de integridade de sistema de arquivos ainda n�o est�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        20,
        "  implementado.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        25,
        "\x07  Para continuar a instala��o do Odyssey, pressione ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        27,
        "\x07  Para sair sem instalar o Odyssey, pressione F3.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER=Continuar  F3=Sair",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ptBRLicensePageEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        6,
        "Licen�a:",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        8,
        "O Odyssey est� licenciado sob os termos da licen�a",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        9,
        "GNU GPL contendo partes de c�digo licenciados sob outras",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "licen�as compat�veis, como X11 ou BSD e GNU LGPL.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "Todo o software que faz parte do Odyssey � portanto, liberado",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "sob a licen�a GNU GPL, bem como a manuten��o da licen�a",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "original.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "Este software vem sem NENHUMA GARANTIA ou restri��o de uso",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "exceto onde leis locais e internacionais s�o aplicaveis. A licen�a",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "do Odyssey abrange apenas a distribui��o a terceiros.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "Se por alguma raz�o voc� n�o recebeu uma c�pia da licen�a",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "GNU General Public License com o Odyssey por favor visite",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        20,
        "http://www.gnu.org/licenses/licenses.html",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        6,
        22,
        "Garantia:",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        24,
        "Este � um software livre; veja o c�digo fonte para condi��es de c�pia.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        25,
        "N�O h� garantia; nem mesmo para COMERCIALIZA��O ou",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        26,
        "ADEQUA��O PARA UM PROP�SITO PARTICULAR",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER=Voltar",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ptBRDevicePageEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "A lista a seguir mostra as configura��es de dispositivos atual.",
        TEXT_STYLE_NORMAL
    },
    {
        24,
        11,
        "Computador:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        12,
        "V�deo:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        13,
        "Teclado:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        14,
        "Leiaute teclado:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        16,
        "Aceitar:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        25,
        16, "Aceitar essas configura��es de dispositivo",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        19,
        "Use as teclas SETA PARA CIMA e SETA PARA BAIXO para mudar de op��o.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        20,
        "Para escolher uma configura��o alternativa, pressione ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        22,
        "Quanto finalizar os ajustes, selecione \"Aceitar essas configura��es",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "de dispositivo\" e pressione ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER=Continuar  F3=Sair",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ptBRRepairPageEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "O instalador do Odyssey est� em fase inicial de desenvolvimento e",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "ainda n�o suporta todas as fun��es de instala��o.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "As fun��es repara��o ainda n�o foram implementadas.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Para atualizar o sistema operacional, pressione U.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Para abrir o console de recupera��o, pressione R.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  Para voltar a p�gina principal, pressione ESC.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Para reiniciar o computador, pressione ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ESC=P�gina principal  U=Atualizar  R=Recuperar  ENTER=Reiniciar",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};
static MUI_ENTRY ptBRComputerPageEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "A lista a seguir mostra os tipos de computadores dispon�veis",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "para instala��o.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        11,
        "Use as teclas SETA PARA CIMA e SETA PARA BAIXO para selecionar",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "um item na lista.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "\x07  Para escolher o item selecionado, pressione ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "\x07  Para cancelar a altera��o, pressione ESC.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER=Continuar  ESC=Cancelar  F3=Sair",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ptBRFlushPageEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "O sistema est� agora certificando que todos os dados estejam sendo",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        7,
        "armazenados corretamente no disco.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Esta opera��o pode demorar um minuto.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "Quando terminar, o computador ser� reiniciado automaticamente.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Esvaziando o cache",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ptBRQuitPageEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "O Odyssey n�o foi totalmente instalado neste computador.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Se houver algum disquete na unidade A: ou disco nas unidades",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "de CD-ROM, remova-os.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "Para reiniciar o computador, pressione ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Por favor, aguarde...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG,
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ptBRDisplayPageEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "A lista a seguir mostra os tipos de v�deo dispon�veis para instala��o.",
        TEXT_STYLE_NORMAL
    },
    {   6,
        10,
         "Use as teclas SETA PARA CIMA e SETA PARA BAIXO para selecionar",
         TEXT_STYLE_NORMAL
    },
    {
        6,
        11,
        "um item na lista.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Para escolher o item selecionado, pressione ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Para cancelar a altera��o, pressione ESC.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER=Continuar  ESC=Cancelar  F3=Sair",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ptBRSuccessPageEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "Os componentes b�sicos do Odyssey foram instalados com sucesso.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Se houver algum disquete na unidade A: ou disco nas unidades",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "de CD-ROM, remova-os.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "Para reiniciar o computador, pressione ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER=Reiniciar",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ptBRBootPageEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "O instalador n�o p�de instalar o ger�nciador de inicializa��o no disco",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "r�gido do computador.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        13,
        "Por favor insira um disquete formatado na unidade A: e",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "pressione ENTER.",
        TEXT_STYLE_NORMAL,
    },
    {
        0,
        0,
        "ENTER=Continuar  F3=Sair",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }

};

static MUI_ENTRY ptBRSelectPartitionEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        7,
        "A lista a seguir mostra as parti��es existentes e os espa�os",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        8,
        "n�o-particionados neste computador.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        10,
        "Use as teclas SETA PARA CIMA e SETA PARA BAIXO para selecionar",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        11,
        "um item na lista.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Para configurar o Odyssey no item selecionado, pressione ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Para criar uma parti��o no espa�o n�o particionado, pressione C.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Para excluir a parti��o selecionada, pressione D.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Por favor, aguarde...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ptBRFormatPartitionEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Formatar parti��o",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        10,
        "O instalador ir� formatar a parti��o. Para continuar, pressione ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER=Continuar  F3=Sair",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        TEXT_STYLE_NORMAL
    }
};

static MUI_ENTRY ptBRInstallDirectoryEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "O instalador ir� copiar os arquivos para a parti��o selecionada.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "Selecione um diret�rio onde deseja que o Odyssey seja instalado:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "Para mudar o diret�rio sugerido, pressione a tecla BACKSPACE para apagar",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        15,
        "o texto e escreva o nome do diret�rio onde deseja que o Odyssey",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        16,
        "seja instalado.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER=Continuar  F3 = Sair",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ptBRFileCopyEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        0,
        12,
        "Por favor aguarde enquanto o instalador copia os",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        13,
        "arquivos do Odyssey para a pasta de instala��o.",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        14,
        "Esta opera��o pode demorar alguns minutos.",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        50,
        0,
        "\xB3 Por favor, aguarde...",     
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ptBRBootLoaderEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "O instalador ir� configurar o ger�nciador de inicializa��o",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "Instalar o ger�nciador de inic. no disco r�gido (MBR e VBR)",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "Instalar o ger�nciador de inic. no disco r�gido (apenas VBR)",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "Instalar o ger�nciador de inicializa��o em um disquete",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "Pular a instala��o do ger�nciador de inicializa��o",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER=Continuar  F3=Sair",
        TEXT_TYPE_STATUS  | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ptBRKeyboardSettingsEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "A lista a seguir mostra os tipos de teclados dispon�veis para instala��o.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        10,
        "Use as teclas SETA PARA CIMA e SETA PARA BAIXO para selecionar",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        11,
        "um item na lista.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Para escolher o item selecionado, pressione ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Para cancelar a altera��o, pressione ESC.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER=Continuar  ESC=Cancelar  F3=Sair",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ptBRLayoutSettingsEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "A lista a seguir mostra os tipos de leiautes de teclado dispon�veis",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "para instala��o.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        11,
        "Use as teclas SETA PARA CIMA e SETA PARA BAIXO para selecionar",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "um item na lista.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "\x07  Para escolher o item selecionado, pressione ENTER.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "\x07  Para cancelar a altera��o, pressione ESC.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ENTER=Continuar  ESC=Cancelar  F3=Sair",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY ptBRPrepareCopyEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "O instalador est� preparando o computador para copiar os arquivos",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "do Odyssey.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Montando a lista de arquivos a serem copiados...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY ptBRSelectFSEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        16,
        "Selecione um sistema de arquivos para a nova parti��o na lista abaixo.",
        0
    },
    {
        6,
        17,
        "Use as teclas SETA PARA CIMA e SETA PARA BAIXO para selecionar o",
        0
    },
    {
        6,
        18,
        "sistema de arquivos de arquivos desejado e pressione ENTER.",
        0
    },
    {
        8,
        20,
        "Se desejar selecionar uma parti��o diferente, pressione ESC.",
        0
    },
    {
        0,
        0,
        "ENTER=Continuar  ESC=Cancelar  F3=Sair",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },

    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ptBRDeletePartitionEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Voc� solicitou a exclus�o da parti��o",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "\x07  Para excluir esta parti��o, pressione D",
        TEXT_STYLE_NORMAL
    },
    {
        11,
        19,
        "CUIDADO: todos os dados da parti��o ser�o perdidos!",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Para retornar � tela anterior sem excluir",
        TEXT_STYLE_NORMAL
    },
    {
        11,
        22,
        "a parti��o, pressione ESC.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "D=Excluir  ESC=Cancelar  F3=Sair",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY ptBRRegistryEntries[] =
{
    {
        4,
        3,
        " Instala��o do Odyssey " KERNEL_VERSION_STR " ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "O instalador est� atualizando a configura��o do sistema.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Criando a estrutura de registro...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

MUI_ERROR ptBRErrorEntries[] =
{
    {
        //ERROR_NOT_INSTALLED
        "O Odyssey n�o est� completamente instalado no computador.\n"
        "Se voc� sair da instala��o agora, precisar� executa-la\n"
        "novamente para instalar o Odyssey.\n"
        "\n"
        "  \x07  Para continuar a instala��o, pressione ENTER.\n"
        "  \x07  Para sair da instala��o, pressione F3.",
        "F3=Sair  ENTER=Continuar"
    },
    {
        //ERROR_NO_HDD
        "N�o foi poss�vel localizar um disco r�digo.\n",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_NO_SOURCE_DRIVE
        "N�o foi poss�vel localizar a unidade de origem.\n",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_LOAD_TXTSETUPSIF
        "N�o foi poss�vel carregar o arquivo TXTSETUP.SIF.\n",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_CORRUPT_TXTSETUPSIF
        "O arquivos TXTSETUP.SIF est� corrompido.\n",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_SIGNATURE_TXTSETUPSIF,
        "O arquivo TXTSETUP.SIF est� com a assinatura incorreta.\n",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_DRIVE_INFORMATION
        "N�o foi poss�vel obter as informa��es sobre o disco do sistema.\n",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_WRITE_BOOT,
        "Erro ao escrever o c�digo de inicializa��o na parti��o do sistema.\n",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_LOAD_COMPUTER,
        "N�o foi poss�vel carregar a lista de tipos de computadores.\n",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_LOAD_DISPLAY,
        "N�o foi poss�vel carregar a lista de tipos de v�deo.\n",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_LOAD_KEYBOARD,
        "N�o foi poss�vel carregar a lista de tipos de teclado.\n",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_LOAD_KBLAYOUT,
        "N�o foi poss�vel carregar a lista de leiautes de teclado.\n",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_WARN_PARTITION,
        "O instalador encontrou uma tabela de parti��o incompat�vel\n"
        "que n�o pode ser utilizada corretamente!\n"
        "\n"
        "Criar ou excluir parti��es pode destruir a tabela de parti��o.\n"
        "\n"
        "  \x07  Para sair da instala��o, pressione F3.\n"
        "  \x07  Para continuar, pressione ENTER.",
        "F3=Sair  ENTER=Continuar"
    },
    {
        //ERROR_NEW_PARTITION,
        "Voc� n�o pode criar uma parti��o dentro de\n"
        "outra parti��o j� existente!\n"
        "\n"
        "  * Pressione qualquer tecla para continuar.",
        NULL
    },
    {
        //ERROR_DELETE_SPACE,
        "Voc� n�o pode excluir um espa�o n�o-particionado!\n"
        "\n"
        "  * Pressione qualquer tecla para continuar.",
        NULL
    },
    {
        //ERROR_INSTALL_BOOTCODE,
        "Erro ao instalar o c�digo de inicializa��o na parti��o do sistema.",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_NO_FLOPPY,
        "N�o h� disco na unidade A:.",
        "ENTER=Continuar"
    },
    {
        //ERROR_UPDATE_KBSETTINGS,
        "N�o foi poss�vel atualizar a configura��o de leiaute de teclado.",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_UPDATE_DISPLAY_SETTINGS,
        "N�o foi poss�vel atualizar a configura��o de v�deo.",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_IMPORT_HIVE,
        "N�o foi poss�vel importar o arquivo de estrutura.",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_FIND_REGISTRY
        "N�o foi poss�vel encontrar os arquivos do registro.",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_CREATE_HIVE,
        "N�o foi poss�vel criar as estruturas do registro.",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_INITIALIZE_REGISTRY,
        "N�o foi poss�vel inicializar o registro.",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_INVALID_CABINET_INF,
        "O arquivo cab n�o cont�m um arquivo inf v�lido.\n",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_CABINET_MISSING,
        "N�o foi poss�vel econtrar o arquivo cab.\n",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_CABINET_SCRIPT,
        "O arquivo cab n�o cont�m um script de instala��o.\n",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_COPY_QUEUE,
        "N�o foi poss�vel abrir a lista de arquivos para c�pia.\n",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_CREATE_DIR,
        "N�o foi poss�vel criar os diret�rios de instala��o.",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_TXTSETUP_SECTION,
        "N�o foi poss�vel encontrar a se��o 'Directories' no\n"
        "arquivo TXTSETUP.SIF.\n",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_CABINET_SECTION,
        "N�o foi poss�vel encontrar a se��o 'Directories' no\n"
        "arquivo cab.\n",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_CREATE_INSTALL_DIR
        "N�o foi poss�vel criar o diret�rio de instala��o.",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_FIND_SETUPDATA,
        "N�o foi poss�vel encontrar a se��o 'SetupData' no\n"
        "arquivo TXTSETUP.SIF.\n",
        "ENTER=Reiniciar"
    },
    {
        //ERROR_WRITE_PTABLE,
        "N�o foi poss�vel escrever a tabela de parti��es.\n"
        "ENTER=Reiniciar"
    },
    {
        //ERROR_ADDING_CODEPAGE,
        "N�o foi poss�vel adicionar o c�digo de localidade no registro.\n"
        "ENTER=Reiniciar"
    },
    {
        //ERROR_UPDATE_LOCALESETTINGS,
        "N�o foi poss�vel configurar o idioma do sistema.\n"
        "ENTER=Reiniciar"
    },
    {
        //ERROR_ADDING_KBLAYOUTS,
        "N�o foi poss�vel adicionar o leiaute do teclado no registro.\n"
        "ENTER=Reiniciar"
    },
    {
        //ERROR_UPDATE_GEOID,
        "N�o foi poss�vel configurar a identifica��o geogr�fica.\n"
        "ENTER=Reiniciar"
    },
    {
        //ERROR_INSUFFICIENT_DISKSPACE,
        "N�o h� espa�o suficiente na parti��o selecionada.\n"
        "  * Pressione qualquer tecla para continuar.",
        NULL
    },
    {
        NULL,
        NULL
    }
};

MUI_PAGE ptBRPages[] =
{
    {
        LANGUAGE_PAGE,
        ptBRLanguagePageEntries
    },
    {
        START_PAGE,
        ptBRWelcomePageEntries
    },
    {
        INSTALL_INTRO_PAGE,
        ptBRIntroPageEntries
    },
    {
        LICENSE_PAGE,
        ptBRLicensePageEntries
    },
    {
        DEVICE_SETTINGS_PAGE,
        ptBRDevicePageEntries
    },
    {
        REPAIR_INTRO_PAGE,
        ptBRRepairPageEntries
    },
    {
        COMPUTER_SETTINGS_PAGE,
        ptBRComputerPageEntries
    },
    {
        DISPLAY_SETTINGS_PAGE,
        ptBRDisplayPageEntries
    },
    {
        FLUSH_PAGE,
        ptBRFlushPageEntries
    },
    {
        SELECT_PARTITION_PAGE,
        ptBRSelectPartitionEntries
    },
    {
        SELECT_FILE_SYSTEM_PAGE,
        ptBRSelectFSEntries
    },
    {
        FORMAT_PARTITION_PAGE,
        ptBRFormatPartitionEntries
    },
    {
        DELETE_PARTITION_PAGE,
        ptBRDeletePartitionEntries
    },
    {
        INSTALL_DIRECTORY_PAGE,
        ptBRInstallDirectoryEntries
    },
    {
        PREPARE_COPY_PAGE,
        ptBRPrepareCopyEntries
    },
    {
        FILE_COPY_PAGE,
        ptBRFileCopyEntries
    },
    {
        KEYBOARD_SETTINGS_PAGE,
        ptBRKeyboardSettingsEntries
    },
    {
        BOOT_LOADER_PAGE,
        ptBRBootLoaderEntries
    },
    {
        LAYOUT_SETTINGS_PAGE,
        ptBRLayoutSettingsEntries
    },
    {
        QUIT_PAGE,
        ptBRQuitPageEntries
    },
    {
        SUCCESS_PAGE,
        ptBRSuccessPageEntries
    },
    {
        BOOT_LOADER_FLOPPY_PAGE,
        ptBRBootPageEntries
    },
    {
        REGISTRY_PAGE,
        ptBRRegistryEntries
    },
    {
        -1,
        NULL
    }
};

MUI_STRING ptBRStrings[] =
{
    {STRING_PLEASEWAIT,
    "   Por favor, aguarde..."},
    {STRING_INSTALLCREATEPARTITION,
    "   ENTER=Instalar  C=Criar parti��o  F3=Sair"},
    {STRING_INSTALLDELETEPARTITION,
    "   ENTER=Instalar  D=Apagar parti��o  F3=Sair"},
    {STRING_PARTITIONSIZE,
    "Tamanho da nova parti��o:"},
    {STRING_CHOOSENEWPARTITION,
    "Voc� solicitou a cria��o de uma nova parti��o em"},
    {STRING_HDDSIZE,
    "Por favor, insira o tamanho da nova parti��o em megabytes (MB)."},
    {STRING_CREATEPARTITION,
    "   ENTER=Criar parti��o  ESC=Cancelar  F3=Sair"},
    {STRING_PARTFORMAT,
    "Esta parti��o ser� formatada logo em seguida."},
    {STRING_NONFORMATTEDPART,
    "Voc� solicitou instalar o Odyssey em uma parti��o nova ou sem formato."},
    {STRING_INSTALLONPART,
    "O instalador instala o Odyssey na parti��o"},
    {STRING_CHECKINGPART,
    "O instalador est� verificando a parti��o selecionada."},
    {STRING_QUITCONTINUE,
    "F3=Sair  ENTER=Continuar"},
    {STRING_REBOOTCOMPUTER,
    "ENTER=Reiniciar"},
    {STRING_TXTSETUPFAILED,
    "N�o foi poss�vel econtrar a se��o '%S' no\narquivo TXTSETUP.SIF.\n"},
    {STRING_COPYING,
    "   Copiando arquivo: %S"},
    {STRING_SETUPCOPYINGFILES,
    "O instalador est� copiando os arquivos..."},
    {STRING_REGHIVEUPDATE,
    "   Atualizando a estrutura do registro..."},
    {STRING_IMPORTFILE,
    "   Importando %S..."},
    {STRING_DISPLAYETTINGSUPDATE,
    "   Atualizando as configura��es de v�deo..."},
    {STRING_LOCALESETTINGSUPDATE,
    "   Atualizando as configura��es regionais..."},
    {STRING_KEYBOARDSETTINGSUPDATE,
    "   Atualizando as configura��es de leiaute do teclado..."},
    {STRING_CODEPAGEINFOUPDATE,
    "   Adicionando as informa��es de localidade no registro..."},
    {STRING_DONE,
    "   Pronto..."},
    {STRING_REBOOTCOMPUTER2,
    "   ENTER=Reiniciar"},
    {STRING_CONSOLEFAIL1,
    "N�o foi poss�vel abrir o console\n\n"},
    {STRING_CONSOLEFAIL2,
    "A causa mais com�m � a utiliza��o de um teclado USB\n"},
    {STRING_CONSOLEFAIL3,
    "Os teclados USB ainda n�o s�o completamente suportados\n"},
    {STRING_FORMATTINGDISK,
    "O instalador est� formatando o disco"},
    {STRING_CHECKINGDISK,
    "O instalador est� verificando o disco"},
    {STRING_FORMATDISK1,
    " Formatar a parti��o utilizando o sistema de arquivos %S (R�pido) "},
    {STRING_FORMATDISK2,
    " Formatar a parti��o utilizando o sistema de arquivos %S "},
    {STRING_KEEPFORMAT,
    " Manter o sistema de arquivos atual (sem altera��es) "},
    {STRING_HDINFOPARTCREATE,
    "%I64u %s  Disco %lu  (Porta=%hu, Barramento=%hu, Id=%hu) em %wZ."},
    {STRING_HDDINFOUNK1,
    "%I64u %s  Disco %lu  (Porta=%hu, Barramento=%hu, Id=%hu)."},
    {STRING_HDDINFOUNK2,
    "   %c%c  Tipo %lu    %I64u %s"},
    {STRING_HDINFOPARTDELETE,
    "em %I64u %s  Disco %lu  (Porta=%hu, Barramento=%hu, Id=%hu) em %wZ."},
    {STRING_HDDINFOUNK3,
    "em %I64u %s  Disco %lu  (Porta=%hu, Barramento=%hu, Id=%hu)."},
    {STRING_HDINFOPARTZEROED,
    "Disco %lu (%I64u %s), Porta=%hu, Barramento=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK4,
    "%c%c  Tipo %lu    %I64u %s"},
    {STRING_HDINFOPARTEXISTS,
    "em Disco %lu (%I64u %s), Porta=%hu, Barramento=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK5,
    "%c%c  Tipo %-3u                         %6lu %s"},
    {STRING_HDINFOPARTSELECT,
    "%6lu %s  Disco %lu  (Porta=%hu, Barramento=%hu, Id=%hu) em %S"},
    {STRING_HDDINFOUNK6,
    "%6lu %s  Disco %lu  (Porta=%hu, Barramento=%hu, Id=%hu)"},
    {STRING_NEWPARTITION,
    "O instalador criou uma nova parti��o em"},
    {STRING_UNPSPACE,
    "    Espa�o n�o particionado              %6lu %s"},
    {STRING_MAXSIZE,
    "MB (max. %lu MB)"},
    {STRING_UNFORMATTED,
    "Novo (sem formato)"},
    {STRING_FORMATUNUSED,
    "Livre"},
    {STRING_FORMATUNKNOWN,
    "desconhecido"},
    {STRING_KB,
    "KB"},
    {STRING_MB,
    "MB"},
    {STRING_GB,
    "GB"},
    {STRING_ADDKBLAYOUTS,
    "Adicionando leiautes de teclado"},
    {0, 0}
};
