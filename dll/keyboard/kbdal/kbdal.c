/*
 * Odyssey Albanian Keyboard layout
 * Copyright (C) 2008 Odyssey
 * Author: Dmitry Chapyshev
 * License: LGPL, see: LGPL.txt
 *
 * Thanks to: http://www.barcodeman.com/altek/mule/scandoc.php
 * and http://win.tue.nl/~aeb/linux/kbd/scancodes-1.html
 */

#include <windows.h>
#include <internal/kbd.h>

#ifdef _M_IA64
#define ROSDATA static __declspec(allocate(".data"))
#else
#ifdef _MSC_VER
#pragma data_seg(".data")
#define ROSDATA static
#else
#define ROSDATA static __attribute__((section(".data")))
#endif
#endif

#define VK_EMPTY 0xff   /* The non-existent VK */
#define KSHIFT   0x001  /* Shift modifier */
#define KCTRL    0x002  /* Ctrl modifier */
#define KALT     0x004  /* Alt modifier */
#define KEXT     0x100  /* Extended key code */
#define KMULTI   0x200  /* Multi-key */
#define KSPEC    0x400  /* Special key */
#define KNUMP    0x800  /* Number-pad */
#define KNUMS    0xc00  /* Special + number pad */
#define KMEXT    0x300  /* Multi + ext */

#define SHFT_INVALID 0x0F

ROSDATA USHORT scancode_to_vk[] = {
  /* Numbers Row */
  /* - 00 - */
  /* 1 ...         2 ...         3 ...         4 ... */
  VK_EMPTY,     VK_ESCAPE,    '1',          '2',
  '3',          '4',          '5',          '6',
  '7',          '8',          '9',          '0',
  VK_OEM_MINUS, VK_OEM_PLUS,  VK_BACK,
  /* - 0f - */
  /* First Letters Row */
  VK_TAB,       'Q',          'W',          'E',
  'R',          'T',          'Y',          'U',
  'I',          'O',          'P',
  VK_OEM_4,     VK_OEM_6,     VK_RETURN,
  /* - 1d - */
  /* Second Letters Row */
  VK_LCONTROL,
  'A',          'S',          'D',          'F',
  'G',          'H',          'J',          'K',
  'L',          VK_OEM_1,     VK_OEM_7,     VK_OEM_3,
  VK_LSHIFT,    VK_OEM_5,
  /* - 2c - */
  /* Third letters row */
  'Z',          'X',          'C',          'V',
  'B',          'N',          'M',          VK_OEM_COMMA,
  VK_OEM_PERIOD,VK_OEM_2,     VK_RSHIFT | KEXT,
  /* - 37 - */
  /* Bottom Row */
  0x26a,  VK_LMENU,     VK_SPACE,     VK_CAPITAL,

  /* - 3b - */
  /* F-Keys */
  VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6,
  VK_F7, VK_F8, VK_F9, VK_F10,
  /* - 45 - */
  /* Locks */
  VK_NUMLOCK | KMEXT,
  VK_SCROLL | KMULTI,
  /* - 47 - */
  /* Number-Pad */
  VK_HOME | KNUMS,      VK_UP | KNUMS,         VK_PRIOR | KNUMS, VK_SUBTRACT,
  VK_LEFT | KNUMS,      VK_CLEAR | KNUMS,      VK_RIGHT | KNUMS, VK_ADD,
  VK_END | KNUMS,       VK_DOWN | KNUMS,       VK_NEXT | KNUMS,
  VK_INSERT | KNUMS,    VK_DELETE | KNUMS,
  /* - 54 - */
  /* Presumably PrtSc */
  VK_SNAPSHOT,
  /* - 55 - */
  /* Oddities, and the remaining standard F-Keys */
  VK_EMPTY,     VK_OEM_102,     VK_F11,       VK_F12,
  /* - 59 - */
  VK_CLEAR,     VK_OEM_WSCTRL,VK_OEM_FINISH,VK_OEM_JUMP,  VK_EREOF, /* EREOF */
  VK_OEM_BACKTAB,    VK_OEM_AUTO,  VK_EMPTY,    VK_ZOOM,            /* ZOOM */
  VK_HELP,
  /* - 64 - */
  /* Even more F-Keys (for example, NCR keyboards from the early 90's) */
  VK_F13, VK_F14, VK_F15, VK_F16, VK_F17, VK_F18, VK_F19, VK_F20,
  VK_F21, VK_F22, VK_F23,
  /* - 6f - */
  /* Not sure who uses these codes */
  VK_OEM_PA3, VK_EMPTY, VK_OEM_RESET,
  /* - 72 - */
  VK_EMPTY, 0xc1, VK_EMPTY, VK_EMPTY,
  /* - 76 - */
  /* One more f-key */
  VK_F24,
  /* - 77 - */
  VK_EMPTY, VK_EMPTY, VK_EMPTY, VK_EMPTY,
  VK_OEM_PA1, VK_TAB, 0xc2, 0, /* PA1 */
  0,
  /* - 80 - */
  0
};

ROSDATA VSC_VK extcode0_to_vk[] = {
  { 0x10, VK_MEDIA_PREV_TRACK | KEXT },
  { 0x19, VK_MEDIA_NEXT_TRACK | KEXT },
  { 0x1D, VK_RCONTROL | KEXT },
  { 0x20, VK_VOLUME_MUTE | KEXT },
  { 0x21, VK_LAUNCH_APP2 | KEXT },
  { 0x22, VK_MEDIA_PLAY_PAUSE | KEXT },
  { 0x24, VK_MEDIA_STOP | KEXT },
  { 0x2E, VK_VOLUME_DOWN | KEXT },
  { 0x30, VK_VOLUME_UP | KEXT },
  { 0x32, VK_BROWSER_HOME | KEXT },
  { 0x35, VK_DIVIDE | KEXT },
  { 0x37, VK_SNAPSHOT | KEXT },
  { 0x38, VK_RMENU | KEXT },
  { 0x47, VK_HOME | KEXT },
  { 0x48, VK_UP | KEXT },
  { 0x49, VK_PRIOR | KEXT },
  { 0x4B, VK_LEFT | KEXT },
  { 0x4D, VK_RIGHT | KEXT },
  { 0x4F, VK_END | KEXT },
  { 0x50, VK_DOWN | KEXT },
  { 0x51, VK_NEXT | KEXT },
  { 0x52, VK_INSERT | KEXT },
  { 0x53, VK_DELETE | KEXT },
  { 0x5B, VK_LWIN | KEXT },
  { 0x5C, VK_RWIN | KEXT },
  { 0x5D, VK_APPS | KEXT },
  { 0x5F, VK_SLEEP | KEXT },
  { 0x65, VK_BROWSER_SEARCH | KEXT },
  { 0x66, VK_BROWSER_FAVORITES | KEXT },
  { 0x67, VK_BROWSER_REFRESH | KEXT },
  { 0x68, VK_BROWSER_STOP | KEXT },
  { 0x69, VK_BROWSER_FORWARD | KEXT },
  { 0x6A, VK_BROWSER_BACK | KEXT },
  { 0x6B, VK_LAUNCH_APP1 | KEXT },
  { 0x6C, VK_LAUNCH_MAIL | KEXT },
  { 0x6D, VK_LAUNCH_MEDIA_SELECT | KEXT },
  { 0x1C, VK_RETURN | KEXT },
  { 0x46, VK_CANCEL | KEXT },
  { 0, 0 },
};

ROSDATA VSC_VK extcode1_to_vk[] = {
  { 0x1d, VK_PAUSE},
  { 0, 0 },
};

ROSDATA VK_TO_BIT modifier_keys[] = {
  { VK_SHIFT,   KSHIFT },
  { VK_CONTROL, KCTRL },
  { VK_MENU,    KALT },
  { 0,          0 }
};

ROSDATA MODIFIERS modifier_bits = {
  modifier_keys,
  6,
  { 0, 1, 3, 4, SHFT_INVALID, SHFT_INVALID, 2 }
};

#define NOCAPS 0
#define CAPS   KSHIFT /* Caps -> shift */

ROSDATA VK_TO_WCHARS2 key_to_chars_2mod[] = {
  { VK_OEM_3,    NOCAPS, {'\\', '|'} },
  { 'E',         CAPS,   {'e',  'E'} },
  { 'R',         CAPS,   {'r',  'R'} },
  { 'T',         CAPS,   {'t',  'T'} },
  { 'Z',         CAPS,   {'z',  'Z'} },
  { 'U',         CAPS,   {'u',  'U'} },
  { 'I',         CAPS,   {'i',  'I'} },
  { 'O',         CAPS,   {'o',  'O'} },
  { 'P',         CAPS,   {'p',  'P'} },
  { 'A',         CAPS,   {'a',  'A'} },
  { 'H',         CAPS,   {'h',  'H'} },
  { 'J',         CAPS,   {'j',  'J'} },
  { 'Y',         CAPS,   {'y',  'Y'} },
  { 'X',         CAPS,   {'x',  'X'} },
  { 'C',         CAPS,   {'c',  'C'} },
  { VK_OEM_2,    NOCAPS, {'/',  '?'} },
  { VK_DECIMAL,  NOCAPS, {',',  ','} },
  { VK_TAB,      NOCAPS, {'\t', '\t'}},
  { VK_ADD,      NOCAPS, {'+',  '+'} },
  { VK_DIVIDE,   NOCAPS, {'/',  '/'} },
  { VK_MULTIPLY, NOCAPS, {'*',  '*'} },
  { VK_SUBTRACT, NOCAPS, {'-',  '-'} },
  { 0, 0 }
};

ROSDATA VK_TO_WCHARS3 key_to_chars_3mod[] = {
  /* Normal, Shifted, Ctrl */
  /* Legacy (telnet-style) ascii escapes */
  { '1',           NOCAPS, {'1',      '!',      '~'     } },
  { '2',           NOCAPS, {'2',      '\"',     WCH_DEAD} },
  { 0xff,          NOCAPS, {WCH_NONE, WCH_NONE, 0x02c7  } },
  { '3',           NOCAPS, {'3',      '#',      WCH_DEAD} },
  { 0xff,          NOCAPS, {WCH_NONE, WCH_NONE, '^'     } },
  { '4',           NOCAPS, {'4',      '$',      WCH_DEAD} },
  { 0xff,          NOCAPS, {WCH_NONE, WCH_NONE, 0x02d8  } },
  { '5',           NOCAPS, {'5',      '%',      WCH_DEAD} },
  { 0xff,          NOCAPS, {WCH_NONE, WCH_NONE, 0x00b0  } },
  { '7',           NOCAPS, {'7',      '&',      '`'     } },
  { '8',           NOCAPS, {'8',      '*',      WCH_DEAD} },
  { 0xff,          NOCAPS, {WCH_NONE, WCH_NONE, 0x02d9  } },
  { '9',           NOCAPS, {'9',      '(',      WCH_DEAD} },
  { 0xff,          NOCAPS, {WCH_NONE, WCH_NONE, 0x00b4  } },
  { '0',           NOCAPS, {'0',      ')',      WCH_DEAD} },
  { 0xff,          NOCAPS, {WCH_NONE, WCH_NONE, 0x02dd  } },
  { VK_OEM_PLUS,   NOCAPS, {'=',      '+',      WCH_DEAD} },
  { 0xff,          NOCAPS, {WCH_NONE, WCH_NONE, 0x00b8  } },
  { 'Q',           CAPS,   {'q',      'Q',      '\\'    } },
  { 'W',           CAPS,   {'w',      'W',      '|'     } },
  { VK_OEM_4,      CAPS,   {0x00e7,   0x00c7,   0x00f7  } },
  { 'S',           CAPS,   {'s',      'S',      0x0111  } },
  { 'D',           CAPS,   {'d',      'D',      0x0110  } },
  { 'F',           CAPS,   {'f',      'F',      '['     } },
  { 'G',           CAPS,   {'g',      'G',      ']'     } },
  { 'K',           CAPS,   {'k',      'K',      0x0142  } },
  { 'L',           CAPS,   {'l',      'L',      0x0141  } },
  { VK_OEM_7,      NOCAPS, {'[',      '{',      0x00df  } },
  { 'V',           CAPS,   {'v',      'V',      '@'     } },
  { 'B',           CAPS,   {'b',      'B',      '{'     } },
  { 'N',           CAPS,   {'n',      'N',      '}'     } },
  { 'M',           CAPS,   {'m',      'M',      0x00a7  } },
  { VK_OEM_COMMA,  NOCAPS, {',',      ';',      '<'     } },
  { VK_OEM_PERIOD, NOCAPS, {'.',      ':',      '>'     } },
  { 0,0 }
};

ROSDATA VK_TO_WCHARS4 key_to_chars_4mod[] = {
  /* Normal, Shifted, Ctrl-Alt, Ctrl */
  /* Legacy Ascii generators */
  { VK_OEM_MINUS, NOCAPS, {'-',      '_',      WCH_DEAD, 0x001f  } },
  { 0xff,         NOCAPS, {WCH_NONE, WCH_NONE, 0x00a8,   WCH_NONE} },
  { VK_OEM_6,     NOCAPS, {'@',      '\'',     0x00d7,   0x001b  } },
  { VK_OEM_5,     NOCAPS, {']',      '}',      0x00a4,   0x001c  } },
  { VK_OEM_1,     CAPS,   {0x00eb,   0x00cb,   '$',      0x001d  } },
  { VK_OEM_102,   NOCAPS, {'<',      '>',      WCH_NONE, 0x001c  } },
  { VK_BACK,      NOCAPS, {'\b',     '\b',     WCH_NONE, 0x007f  } },
  { VK_ESCAPE,    NOCAPS, {0x001b,   0x001b,   WCH_NONE, 0x001b  } },
  { VK_RETURN,    NOCAPS, {'\r',     '\r',     WCH_NONE, '\n'    } },
  { VK_SPACE,     NOCAPS, {' ',      ' ',      WCH_NONE, ' '     } },
  { VK_CANCEL,    NOCAPS, {0x0003,   0x0003,   WCH_NONE, 0x0003  } },
  { 0, 0 }
};

ROSDATA VK_TO_WCHARS5 key_to_chars_5mod[] = {
  /* Normal, Shifted, Ctrl-Alt, Ctrl, S+Ctrl */
  /* Legacy Ascii generators */
  { '6',  NOCAPS, {'6',      '^',      WCH_DEAD, WCH_NONE, 0x001e  } },
  { 0xff, NOCAPS, {WCH_NONE, WCH_NONE, 0x02db,   WCH_NONE, WCH_NONE} },
  { 0, 0 }
};

ROSDATA VK_TO_WCHARS1 keypad_numbers[] = {
  { VK_NUMPAD0, 0, {'0'} },
  { VK_NUMPAD1, 0, {'1'} },
  { VK_NUMPAD2, 0, {'2'} },
  { VK_NUMPAD3, 0, {'3'} },
  { VK_NUMPAD4, 0, {'4'} },
  { VK_NUMPAD5, 0, {'5'} },
  { VK_NUMPAD6, 0, {'6'} },
  { VK_NUMPAD7, 0, {'7'} },
  { VK_NUMPAD8, 0, {'8'} },
  { VK_NUMPAD9, 0, {'9'} },
  { 0,0 }
};

#define vk_master(n,x) { (PVK_TO_WCHARS1)x, n, sizeof(x[0]) }

ROSDATA VK_TO_WCHAR_TABLE vk_to_wchar_master_table[] = {
  vk_master(3,key_to_chars_3mod),
  vk_master(4,key_to_chars_4mod),
  vk_master(5,key_to_chars_5mod),
  vk_master(2,key_to_chars_2mod),
  vk_master(1,keypad_numbers),
  { 0,0,0 }
};

ROSDATA VSC_LPWSTR key_names[] = {
  { 0x01, L"Esc" },
  { 0x0e, L"Backspace" },
  { 0x0f, L"Tab" },
  { 0x1c, L"Enter" },
  { 0x1d, L"Ctrl" },
  { 0x2a, L"Shift" },
  { 0x36, L"Right Shift" },
  { 0x37, L"Num *" },
  { 0x38, L"Alt" },
  { 0x39, L"Space" },
  { 0x3a, L"Caps Lock" },
  { 0x3b, L"F1" },
  { 0x3c, L"F2" },
  { 0x3d, L"F3" },
  { 0x3e, L"F4" },
  { 0x3f, L"F5" },
  { 0x40, L"F6" },
  { 0x41, L"F7" },
  { 0x42, L"F8" },
  { 0x43, L"F9" },
  { 0x44, L"F10" },
  { 0x45, L"Pause" },
  { 0x46, L"Scroll Lock" },
  { 0x47, L"Num 7" },
  { 0x48, L"Num 8" },
  { 0x49, L"Num 9" },
  { 0x4a, L"Num -" },
  { 0x4b, L"Num 4" },
  { 0x4c, L"Num 5" },
  { 0x4d, L"Num 6" },
  { 0x4e, L"Num +" },
  { 0x4f, L"Num 1" },
  { 0x50, L"Num 2" },
  { 0x51, L"Num 3" },
  { 0x52, L"Num 0" },
  { 0x53, L"Num Del" },
  { 0x54, L"Sys Req" },
  { 0x57, L"F11" },
  { 0x58, L"F12" },
  { 0x7c, L"F13" },
  { 0x7d, L"F14" },
  { 0x7e, L"F15" },
  { 0x7f, L"F16" },
  { 0x80, L"F17" },
  { 0x81, L"F18" },
  { 0x82, L"F19" },
  { 0x83, L"F20" },
  { 0x84, L"F21" },
  { 0x85, L"F22" },
  { 0x86, L"F23" },
  { 0x87, L"F24" },
  { 0, NULL }
};

ROSDATA VSC_LPWSTR extended_key_names[] = {
  { 0x1c, L"Num Enter" },
  { 0x1d, L"Right Control" },
  { 0x35, L"Num /" },
  { 0x37, L"Prnt Scrn" },
  { 0x38, L"Right Alt" },
  { 0x45, L"Num Lock" },
  { 0x46, L"Break" },
  { 0x47, L"Home" },
  { 0x48, L"Up" },
  { 0x49, L"Page Up" },
  { 0x4b, L"Left" },
//{ 0x4c, L"Center" },
  { 0x4d, L"Right" },
  { 0x4f, L"End" },
  { 0x50, L"Down" },
  { 0x51, L"Page Down" },
  { 0x52, L"Insert" },
  { 0x53, L"Delete" },
  { 0x54, L"<Odyssey>" },
  { 0x56, L"Help" },
  { 0x5b, L"Left <Odyssey>" },
  { 0x5c, L"Right <Odyssey>" },
  { 0x5d, L"Application" },
  { 0, NULL }
};

/* Finally, the master table */
ROSDATA KBDTABLES keyboard_layout_table = {
  /* modifier assignments */
  &modifier_bits,

  /* character from vk tables */
  vk_to_wchar_master_table,

  /* diacritical marks */
  NULL,

  /* Key names */
  (VSC_LPWSTR *)key_names,
  (VSC_LPWSTR *)extended_key_names,
  NULL, /* Dead key names */

  /* scan code to virtual key maps */
  scancode_to_vk,
  sizeof(scancode_to_vk) / sizeof(scancode_to_vk[0]),
  extcode0_to_vk,
  extcode1_to_vk,

  MAKELONG(1,1), /* Version 1.0 */

  /* Ligatures -- Albanian doesn't have any */
  0,
  0,
  NULL
};

PKBDTABLES WINAPI KbdLayerDescriptor(VOID) {
  return &keyboard_layout_table;
}

INT WINAPI
DllMain(
  PVOID hinstDll,
  ULONG dwReason,
  PVOID reserved)
{
  return 1;
}

