// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

namespace tcob::input {
// from SDL2:
enum class scan_code {
    UNKNOWN = 0,

    /**
     *  \name Usage page 0x07
     *
     *  These values are from usage page 0x07 (USB keyboard page).
     */
    /* @{ */

    A = 4,
    B = 5,
    C = 6,
    D = 7,
    E = 8,
    F = 9,
    G = 10,
    H = 11,
    I = 12,
    J = 13,
    K = 14,
    L = 15,
    M = 16,
    N = 17,
    O = 18,
    P = 19,
    Q = 20,
    R = 21,
    S = 22,
    T = 23,
    U = 24,
    V = 25,
    W = 26,
    X = 27,
    Y = 28,
    Z = 29,

    D1 = 30,
    D2 = 31,
    D3 = 32,
    D4 = 33,
    D5 = 34,
    D6 = 35,
    D7 = 36,
    D8 = 37,
    D9 = 38,
    D0 = 39,

    RETURN    = 40,
    ESCAPE    = 41,
    BACKSPACE = 42,
    TAB       = 43,
    SPACE     = 44,

    MINUS        = 45,
    EQUALS       = 46,
    LEFTBRACKET  = 47,
    RIGHTBRACKET = 48,
    BACKSLASH    = 49,
    NONUSHASH    = 50,
    SEMICOLON    = 51,
    APOSTROPHE   = 52,
    GRAVE        = 53,
    COMMA        = 54,
    PERIOD       = 55,
    SLASH        = 56,

    CAPSLOCK = 57,

    F1  = 58,
    F2  = 59,
    F3  = 60,
    F4  = 61,
    F5  = 62,
    F6  = 63,
    F7  = 64,
    F8  = 65,
    F9  = 66,
    F10 = 67,
    F11 = 68,
    F12 = 69,

    PRINTSCREEN = 70,
    SCROLLLOCK  = 71,
    PAUSE       = 72,
    INSERT      = 73, /**< insert on PC, help on some Mac keyboards (but
                                        does send code 73, not 117) */
    HOME        = 74,
    PAGEUP      = 75,
    DEL         = 76,
    END         = 77,
    PAGEDOWN    = 78,
    RIGHT       = 79,
    LEFT        = 80,
    DOWN        = 81,
    UP          = 82,

    NUMLOCKCLEAR = 83, /**< num lock on PC, clear on Mac keyboards
                        */
    KP_DIVIDE    = 84,
    KP_MULTIPLY  = 85,
    KP_MINUS     = 86,
    KP_PLUS      = 87,
    KP_ENTER     = 88,
    KP_1         = 89,
    KP_2         = 90,
    KP_3         = 91,
    KP_4         = 92,
    KP_5         = 93,
    KP_6         = 94,
    KP_7         = 95,
    KP_8         = 96,
    KP_9         = 97,
    KP_0         = 98,
    KP_PERIOD    = 99,

    NONUSBACKSLASH = 100,
    APPLICATION    = 101, /**< windows contextual menu, compose */
    POWER          = 102,
    KP_EQUALS      = 103,
    F13            = 104,
    F14            = 105,
    F15            = 106,
    F16            = 107,
    F17            = 108,
    F18            = 109,
    F19            = 110,
    F20            = 111,
    F21            = 112,
    F22            = 113,
    F23            = 114,
    F24            = 115,
    EXECUTE        = 116,
    HELP           = 117,
    MENU           = 118,
    SELECT         = 119,
    STOP           = 120,
    AGAIN          = 121, /**< redo */
    UNDO           = 122,
    CUT            = 123,
    COPY           = 124,
    PASTE          = 125,
    FIND           = 126,
    MUTE           = 127,
    VOLUMEUP       = 128,
    VOLUMEDOWN     = 129,
    KP_COMMA       = 133,
    KP_EQUALSAS400 = 134,

    INTERNATIONAL1 = 135,
    INTERNATIONAL2 = 136,
    INTERNATIONAL3 = 137, /**< Yen */
    INTERNATIONAL4 = 138,
    INTERNATIONAL5 = 139,
    INTERNATIONAL6 = 140,
    INTERNATIONAL7 = 141,
    INTERNATIONAL8 = 142,
    INTERNATIONAL9 = 143,
    LANG1          = 144, /**< Hangul/English toggle */
    LANG2          = 145, /**< Hanja conversion */
    LANG3          = 146, /**< Katakana */
    LANG4          = 147, /**< Hiragana */
    LANG5          = 148, /**< Zenkaku/Hankaku */
    LANG6          = 149, /**< reserved */
    LANG7          = 150, /**< reserved */
    LANG8          = 151, /**< reserved */
    LANG9          = 152, /**< reserved */

    ALTERASE   = 153,     /**< Erase-Eaze */
    SYSREQ     = 154,
    CANCEL     = 155,
    CLEAR      = 156,
    PRIOR      = 157,
    RETURN2    = 158,
    SEPARATOR  = 159,
    KEY_OUT    = 160,
    OPER       = 161,
    CLEARAGAIN = 162,
    CRSEL      = 163,
    EXSEL      = 164,

    KP_00              = 176,
    KP_000             = 177,
    THOUSANDSSEPARATOR = 178,
    DECIMALSEPARATOR   = 179,
    CURRENCYUNIT       = 180,
    CURRENCYSUBUNIT    = 181,
    KP_LEFTPAREN       = 182,
    KP_RIGHTPAREN      = 183,
    KP_LEFTBRACE       = 184,
    KP_RIGHTBRACE      = 185,
    KP_TAB             = 186,
    KP_BACKSPACE       = 187,
    KP_A               = 188,
    KP_B               = 189,
    KP_C               = 190,
    KP_D               = 191,
    KP_E               = 192,
    KP_F               = 193,
    KP_XOR             = 194,
    KP_POWER           = 195,
    KP_PERCENT         = 196,
    KP_LESS            = 197,
    KP_GREATER         = 198,
    KP_AMPERSAND       = 199,
    KP_DBLAMPERSAND    = 200,
    KP_VERTICALBAR     = 201,
    KP_DBLVERTICALBAR  = 202,
    KP_COLON           = 203,
    KP_HASH            = 204,
    KP_SPACE           = 205,
    KP_AT              = 206,
    KP_EXCLAM          = 207,
    KP_MEMSTORE        = 208,
    KP_MEMRECALL       = 209,
    KP_MEMCLEAR        = 210,
    KP_MEMADD          = 211,
    KP_MEMSUBTRACT     = 212,
    KP_MEMMULTIPLY     = 213,
    KP_MEMDIVIDE       = 214,
    KP_PLUSMINUS       = 215,
    KP_CLEAR           = 216,
    KP_CLEARENTRY      = 217,
    KP_BINARY          = 218,
    KP_OCTAL           = 219,
    KP_DECIMAL         = 220,
    KP_HEXADECIMAL     = 221,

    LCTRL  = 224,
    LSHIFT = 225,
    LALT   = 226, /**< alt, option */
    LGUI   = 227, /**< windows, command (apple), meta */
    RCTRL  = 228,
    RSHIFT = 229,
    RALT   = 230, /**< alt gr, option */
    RGUI   = 231, /**< windows, command (apple), meta */

    MODE = 257,   /**< I'm not sure if this is really not covered
                   *   by any of the above, but since there's a
                   *   special KMOD_MODE for it I'm adding it here
                   */

    /* @} */      /* Usage page 0x07 */

    /**
     *  \name Usage page 0x0C
     *
     *  These values are mapped from usage page 0x0C (USB consumer page).
     */
    /* @{ */

    AUDIONEXT    = 258,
    AUDIOPREV    = 259,
    AUDIOSTOP    = 260,
    AUDIOPLAY    = 261,
    AUDIOMUTE    = 262,
    MEDIASELECT  = 263,
    WWW          = 264,
    MAIL         = 265,
    CALCULATOR   = 266,
    COMPUTER     = 267,
    AC_SEARCH    = 268,
    AC_HOME      = 269,
    AC_BACK      = 270,
    AC_FORWARD   = 271,
    AC_STOP      = 272,
    AC_REFRESH   = 273,
    AC_BOOKMARKS = 274,

    /* @} */ /* Usage page 0x0C */

    /**
     *  \name Walther keys
     *
     *  These are values that Christian Walther added (for mac keyboard?).
     */
    /* @{ */

    BRIGHTNESSDOWN = 275,
    BRIGHTNESSUP   = 276,
    DISPLAYSWITCH  = 277, /**< display mirroring/dual display
                                            switch, video mode switch */
    KBDILLUMTOGGLE = 278,
    KBDILLUMDOWN   = 279,
    KBDILLUMUP     = 280,
    EJECT          = 281,
    SLEEP          = 282,

    APP1 = 283,
    APP2 = 284,

    /* @} */ /* Walther keys */

    /**
     *  \name Usage page 0x0C (additional media keys)
     *
     *  These values are mapped from usage page 0x0C (USB consumer page).
     */
    /* @{ */

    AUDIOREWIND      = 285,
    AUDIOFASTFORWARD = 286,

    /* @} */ /* Usage page 0x0C (additional media keys) */

    /**
     *  \name Mobile keys
     *
     *  These are values that are often used on mobile phones.
     */
    /* @{ */

    SOFTLEFT  = 287, /**< Usually situated below the display on phones and
                                       used as a multi-function feature key for selecting
                                       a software defined function shown on the bottom left
                                       of the display. */
    SOFTRIGHT = 288, /**< Usually situated below the display on phones and
                                       used as a multi-function feature key for selecting
                                       a software defined function shown on the bottom right
                                       of the display. */
    CALL      = 289, /**< Used for accepting phone calls. */
    ENDCALL   = 290, /**< Used for rejecting phone calls. */

    /* @} */         /* Mobile keys */

    /* Add any other keys here. */

    SDL_NUM_SCANCODES = 512 /**< not a key, just marks the number of scancodes
                                 for array bounds */
};

consteval auto scancode_to_keycode(scan_code x) -> i32
{
    return static_cast<i32>(x) | (1 << 30);
}

enum class key_code {
    UNKNOWN = 0,

    RETURN       = '\r',
    ESCAPE       = '\033',
    BACKSPACE    = '\b',
    TAB          = '\t',
    SPACE        = ' ',
    EXCLAIM      = '!',
    QUOTEDBL     = '"',
    HASH         = '#',
    PERCENT      = '%',
    DOLLAR       = '$',
    AMPERSAND    = '&',
    QUOTE        = '\'',
    LEFTPAREN    = '(',
    RIGHTPAREN   = ')',
    ASTERISK     = '*',
    PLUS         = '+',
    COMMA        = ',',
    MINUS        = '-',
    PERIOD       = '.',
    SLASH        = '/',
    D0           = '0',
    D1           = '1',
    D2           = '2',
    D3           = '3',
    D4           = '4',
    D5           = '5',
    D6           = '6',
    D7           = '7',
    D8           = '8',
    D9           = '9',
    COLON        = ':',
    SEMICOLON    = ';',
    LESS         = '<',
    EQUALS       = '=',
    GREATER      = '>',
    QUESTION     = '?',
    AT           = '@',
    /*
       Skip uppercase letters
     */
    LEFTBRACKET  = '[',
    BACKSLASH    = '\\',
    RIGHTBRACKET = ']',
    CARET        = '^',
    UNDERSCORE   = '_',
    BACKQUOTE    = '`',
    a            = 'a',
    b            = 'b',
    c            = 'c',
    d            = 'd',
    e            = 'e',
    f            = 'f',
    g            = 'g',
    h            = 'h',
    i            = 'i',
    j            = 'j',
    k            = 'k',
    l            = 'l',
    m            = 'm',
    n            = 'n',
    o            = 'o',
    p            = 'p',
    q            = 'q',
    r            = 'r',
    s            = 's',
    t            = 't',
    u            = 'u',
    v            = 'v',
    w            = 'w',
    x            = 'x',
    y            = 'y',
    z            = 'z',

    CAPSLOCK = scancode_to_keycode(scan_code::CAPSLOCK),

    F1  = scancode_to_keycode(scan_code::F1),
    F2  = scancode_to_keycode(scan_code::F2),
    F3  = scancode_to_keycode(scan_code::F3),
    F4  = scancode_to_keycode(scan_code::F4),
    F5  = scancode_to_keycode(scan_code::F5),
    F6  = scancode_to_keycode(scan_code::F6),
    F7  = scancode_to_keycode(scan_code::F7),
    F8  = scancode_to_keycode(scan_code::F8),
    F9  = scancode_to_keycode(scan_code::F9),
    F10 = scancode_to_keycode(scan_code::F10),
    F11 = scancode_to_keycode(scan_code::F11),
    F12 = scancode_to_keycode(scan_code::F12),

    PRINTSCREEN = scancode_to_keycode(scan_code::PRINTSCREEN),
    SCROLLLOCK  = scancode_to_keycode(scan_code::SCROLLLOCK),
    PAUSE       = scancode_to_keycode(scan_code::PAUSE),
    INSERT      = scancode_to_keycode(scan_code::INSERT),
    HOME        = scancode_to_keycode(scan_code::HOME),
    PAGEUP      = scancode_to_keycode(scan_code::PAGEUP),
    DEL         = '\177',
    END         = scancode_to_keycode(scan_code::END),
    PAGEDOWN    = scancode_to_keycode(scan_code::PAGEDOWN),
    RIGHT       = scancode_to_keycode(scan_code::RIGHT),
    LEFT        = scancode_to_keycode(scan_code::LEFT),
    DOWN        = scancode_to_keycode(scan_code::DOWN),
    UP          = scancode_to_keycode(scan_code::UP),

    NUMLOCKCLEAR = scancode_to_keycode(scan_code::NUMLOCKCLEAR),
    KP_DIVIDE    = scancode_to_keycode(scan_code::KP_DIVIDE),
    KP_MULTIPLY  = scancode_to_keycode(scan_code::KP_MULTIPLY),
    KP_MINUS     = scancode_to_keycode(scan_code::KP_MINUS),
    KP_PLUS      = scancode_to_keycode(scan_code::KP_PLUS),
    KP_ENTER     = scancode_to_keycode(scan_code::KP_ENTER),
    KP_1         = scancode_to_keycode(scan_code::KP_1),
    KP_2         = scancode_to_keycode(scan_code::KP_2),
    KP_3         = scancode_to_keycode(scan_code::KP_3),
    KP_4         = scancode_to_keycode(scan_code::KP_4),
    KP_5         = scancode_to_keycode(scan_code::KP_5),
    KP_6         = scancode_to_keycode(scan_code::KP_6),
    KP_7         = scancode_to_keycode(scan_code::KP_7),
    KP_8         = scancode_to_keycode(scan_code::KP_8),
    KP_9         = scancode_to_keycode(scan_code::KP_9),
    KP_0         = scancode_to_keycode(scan_code::KP_0),
    KP_PERIOD    = scancode_to_keycode(scan_code::KP_PERIOD),

    APPLICATION    = scancode_to_keycode(scan_code::APPLICATION),
    POWER          = scancode_to_keycode(scan_code::POWER),
    KP_EQUALS      = scancode_to_keycode(scan_code::KP_EQUALS),
    F13            = scancode_to_keycode(scan_code::F13),
    F14            = scancode_to_keycode(scan_code::F14),
    F15            = scancode_to_keycode(scan_code::F15),
    F16            = scancode_to_keycode(scan_code::F16),
    F17            = scancode_to_keycode(scan_code::F17),
    F18            = scancode_to_keycode(scan_code::F18),
    F19            = scancode_to_keycode(scan_code::F19),
    F20            = scancode_to_keycode(scan_code::F20),
    F21            = scancode_to_keycode(scan_code::F21),
    F22            = scancode_to_keycode(scan_code::F22),
    F23            = scancode_to_keycode(scan_code::F23),
    F24            = scancode_to_keycode(scan_code::F24),
    EXECUTE        = scancode_to_keycode(scan_code::EXECUTE),
    HELP           = scancode_to_keycode(scan_code::HELP),
    MENU           = scancode_to_keycode(scan_code::MENU),
    SELECT         = scancode_to_keycode(scan_code::SELECT),
    STOP           = scancode_to_keycode(scan_code::STOP),
    AGAIN          = scancode_to_keycode(scan_code::AGAIN),
    UNDO           = scancode_to_keycode(scan_code::UNDO),
    CUT            = scancode_to_keycode(scan_code::CUT),
    COPY           = scancode_to_keycode(scan_code::COPY),
    PASTE          = scancode_to_keycode(scan_code::PASTE),
    FIND           = scancode_to_keycode(scan_code::FIND),
    MUTE           = scancode_to_keycode(scan_code::MUTE),
    VOLUMEUP       = scancode_to_keycode(scan_code::VOLUMEUP),
    VOLUMEDOWN     = scancode_to_keycode(scan_code::VOLUMEDOWN),
    KP_COMMA       = scancode_to_keycode(scan_code::KP_COMMA),
    KP_EQUALSAS400 = scancode_to_keycode(scan_code::KP_EQUALSAS400),

    ALTERASE   = scancode_to_keycode(scan_code::ALTERASE),
    SYSREQ     = scancode_to_keycode(scan_code::SYSREQ),
    CANCEL     = scancode_to_keycode(scan_code::CANCEL),
    CLEAR      = scancode_to_keycode(scan_code::CLEAR),
    PRIOR      = scancode_to_keycode(scan_code::PRIOR),
    RETURN2    = scancode_to_keycode(scan_code::RETURN2),
    SEPARATOR  = scancode_to_keycode(scan_code::SEPARATOR),
    KEY_OUT    = scancode_to_keycode(scan_code::KEY_OUT),
    OPER       = scancode_to_keycode(scan_code::OPER),
    CLEARAGAIN = scancode_to_keycode(scan_code::CLEARAGAIN),
    CRSEL      = scancode_to_keycode(scan_code::CRSEL),
    EXSEL      = scancode_to_keycode(scan_code::EXSEL),

    KP_00              = scancode_to_keycode(scan_code::KP_00),
    KP_000             = scancode_to_keycode(scan_code::KP_000),
    THOUSANDSSEPARATOR = scancode_to_keycode(scan_code::THOUSANDSSEPARATOR),
    DECIMALSEPARATOR   = scancode_to_keycode(scan_code::DECIMALSEPARATOR),
    CURRENCYUNIT       = scancode_to_keycode(scan_code::CURRENCYUNIT),
    CURRENCYSUBUNIT    = scancode_to_keycode(scan_code::CURRENCYSUBUNIT),
    KP_LEFTPAREN       = scancode_to_keycode(scan_code::KP_LEFTPAREN),
    KP_RIGHTPAREN      = scancode_to_keycode(scan_code::KP_RIGHTPAREN),
    KP_LEFTBRACE       = scancode_to_keycode(scan_code::KP_LEFTBRACE),
    KP_RIGHTBRACE      = scancode_to_keycode(scan_code::KP_RIGHTBRACE),
    KP_TAB             = scancode_to_keycode(scan_code::KP_TAB),
    KP_BACKSPACE       = scancode_to_keycode(scan_code::KP_BACKSPACE),
    KP_A               = scancode_to_keycode(scan_code::KP_A),
    KP_B               = scancode_to_keycode(scan_code::KP_B),
    KP_C               = scancode_to_keycode(scan_code::KP_C),
    KP_D               = scancode_to_keycode(scan_code::KP_D),
    KP_E               = scancode_to_keycode(scan_code::KP_E),
    KP_F               = scancode_to_keycode(scan_code::KP_F),
    KP_XOR             = scancode_to_keycode(scan_code::KP_XOR),
    KP_POWER           = scancode_to_keycode(scan_code::KP_POWER),
    KP_PERCENT         = scancode_to_keycode(scan_code::KP_PERCENT),
    KP_LESS            = scancode_to_keycode(scan_code::KP_LESS),
    KP_GREATER         = scancode_to_keycode(scan_code::KP_GREATER),
    KP_AMPERSAND       = scancode_to_keycode(scan_code::KP_AMPERSAND),
    KP_DBLAMPERSAND    = scancode_to_keycode(scan_code::KP_DBLAMPERSAND),
    KP_VERTICALBAR     = scancode_to_keycode(scan_code::KP_VERTICALBAR),
    KP_DBLVERTICALBAR  = scancode_to_keycode(scan_code::KP_DBLVERTICALBAR),
    KP_COLON           = scancode_to_keycode(scan_code::KP_COLON),
    KP_HASH            = scancode_to_keycode(scan_code::KP_HASH),
    KP_SPACE           = scancode_to_keycode(scan_code::KP_SPACE),
    KP_AT              = scancode_to_keycode(scan_code::KP_AT),
    KP_EXCLAM          = scancode_to_keycode(scan_code::KP_EXCLAM),
    KP_MEMSTORE        = scancode_to_keycode(scan_code::KP_MEMSTORE),
    KP_MEMRECALL       = scancode_to_keycode(scan_code::KP_MEMRECALL),
    KP_MEMCLEAR        = scancode_to_keycode(scan_code::KP_MEMCLEAR),
    KP_MEMADD          = scancode_to_keycode(scan_code::KP_MEMADD),
    KP_MEMSUBTRACT     = scancode_to_keycode(scan_code::KP_MEMSUBTRACT),
    KP_MEMMULTIPLY     = scancode_to_keycode(scan_code::KP_MEMMULTIPLY),
    KP_MEMDIVIDE       = scancode_to_keycode(scan_code::KP_MEMDIVIDE),
    KP_PLUSMINUS       = scancode_to_keycode(scan_code::KP_PLUSMINUS),
    KP_CLEAR           = scancode_to_keycode(scan_code::KP_CLEAR),
    KP_CLEARENTRY      = scancode_to_keycode(scan_code::KP_CLEARENTRY),
    KP_BINARY          = scancode_to_keycode(scan_code::KP_BINARY),
    KP_OCTAL           = scancode_to_keycode(scan_code::KP_OCTAL),
    KP_DECIMAL         = scancode_to_keycode(scan_code::KP_DECIMAL),
    KP_HEXADECIMAL     = scancode_to_keycode(scan_code::KP_HEXADECIMAL),

    LCTRL  = scancode_to_keycode(scan_code::LCTRL),
    LSHIFT = scancode_to_keycode(scan_code::LSHIFT),
    LALT   = scancode_to_keycode(scan_code::LALT),
    LGUI   = scancode_to_keycode(scan_code::LGUI),
    RCTRL  = scancode_to_keycode(scan_code::RCTRL),
    RSHIFT = scancode_to_keycode(scan_code::RSHIFT),
    RALT   = scancode_to_keycode(scan_code::RALT),
    RGUI   = scancode_to_keycode(scan_code::RGUI),

    MODE = scancode_to_keycode(scan_code::MODE),

    AUDIONEXT    = scancode_to_keycode(scan_code::AUDIONEXT),
    AUDIOPREV    = scancode_to_keycode(scan_code::AUDIOPREV),
    AUDIOSTOP    = scancode_to_keycode(scan_code::AUDIOSTOP),
    AUDIOPLAY    = scancode_to_keycode(scan_code::AUDIOPLAY),
    AUDIOMUTE    = scancode_to_keycode(scan_code::AUDIOMUTE),
    MEDIASELECT  = scancode_to_keycode(scan_code::MEDIASELECT),
    WWW          = scancode_to_keycode(scan_code::WWW),
    MAIL         = scancode_to_keycode(scan_code::MAIL),
    CALCULATOR   = scancode_to_keycode(scan_code::CALCULATOR),
    COMPUTER     = scancode_to_keycode(scan_code::COMPUTER),
    AC_SEARCH    = scancode_to_keycode(scan_code::AC_SEARCH),
    AC_HOME      = scancode_to_keycode(scan_code::AC_HOME),
    AC_BACK      = scancode_to_keycode(scan_code::AC_BACK),
    AC_FORWARD   = scancode_to_keycode(scan_code::AC_FORWARD),
    AC_STOP      = scancode_to_keycode(scan_code::AC_STOP),
    AC_REFRESH   = scancode_to_keycode(scan_code::AC_REFRESH),
    AC_BOOKMARKS = scancode_to_keycode(scan_code::AC_BOOKMARKS),

    BRIGHTNESSDOWN = scancode_to_keycode(scan_code::BRIGHTNESSDOWN),
    BRIGHTNESSUP   = scancode_to_keycode(scan_code::BRIGHTNESSUP),
    DISPLAYSWITCH  = scancode_to_keycode(scan_code::DISPLAYSWITCH),
    KBDILLUMTOGGLE = scancode_to_keycode(scan_code::KBDILLUMTOGGLE),
    KBDILLUMDOWN   = scancode_to_keycode(scan_code::KBDILLUMDOWN),
    KBDILLUMUP     = scancode_to_keycode(scan_code::KBDILLUMUP),
    EJECT          = scancode_to_keycode(scan_code::EJECT),
    SLEEP          = scancode_to_keycode(scan_code::SLEEP),
    APP1           = scancode_to_keycode(scan_code::APP1),
    APP2           = scancode_to_keycode(scan_code::APP2),

    AUDIOREWIND      = scancode_to_keycode(scan_code::AUDIOREWIND),
    AUDIOFASTFORWARD = scancode_to_keycode(scan_code::AUDIOFASTFORWARD),

    SOFTLEFT  = scancode_to_keycode(scan_code::SOFTLEFT),
    SOFTRIGHT = scancode_to_keycode(scan_code::SOFTRIGHT),
    CALL      = scancode_to_keycode(scan_code::CALL),
    ENDCALL   = scancode_to_keycode(scan_code::ENDCALL)
};
}
