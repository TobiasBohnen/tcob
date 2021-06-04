// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <unordered_map>

#include <tcob/core/data/Point.hpp>
#include <tcob/thirdparty/sigslot/signal.hpp>

typedef struct _SDL_GameController SDL_GameController;
typedef union SDL_Event SDL_Event;

namespace tcob {
//from SDL2:
enum class Scancode {
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

    RETURN = 40,
    ESCAPE = 41,
    BACKSPACE = 42,
    TAB = 43,
    SPACE = 44,

    MINUS = 45,
    EQUALS = 46,
    LEFTBRACKET = 47,
    RIGHTBRACKET = 48,
    BACKSLASH = 49,
    NONUSHASH = 50,
    SEMICOLON = 51,
    APOSTROPHE = 52,
    GRAVE = 53,
    COMMA = 54,
    PERIOD = 55,
    SLASH = 56,

    CAPSLOCK = 57,

    F1 = 58,
    F2 = 59,
    F3 = 60,
    F4 = 61,
    F5 = 62,
    F6 = 63,
    F7 = 64,
    F8 = 65,
    F9 = 66,
    F10 = 67,
    F11 = 68,
    F12 = 69,

    PRINTSCREEN = 70,
    SCROLLLOCK = 71,
    PAUSE = 72,
    INSERT = 73, /**< insert on PC, help on some Mac keyboards (but
                                   does send code 73, not 117) */
    HOME = 74,
    PAGEUP = 75,
    DEL = 76,
    END = 77,
    PAGEDOWN = 78,
    RIGHT = 79,
    LEFT = 80,
    DOWN = 81,
    UP = 82,

    NUMLOCKCLEAR = 83, /**< num lock on PC, clear on Mac keyboards
                                     */
    KP_DIVIDE = 84,
    KP_MULTIPLY = 85,
    KP_MINUS = 86,
    KP_PLUS = 87,
    KP_ENTER = 88,
    KP_1 = 89,
    KP_2 = 90,
    KP_3 = 91,
    KP_4 = 92,
    KP_5 = 93,
    KP_6 = 94,
    KP_7 = 95,
    KP_8 = 96,
    KP_9 = 97,
    KP_0 = 98,
    KP_PERIOD = 99,

    NONUSBACKSLASH = 100,
    APPLICATION = 101, /**< windows contextual menu, compose */
    POWER = 102,
    KP_EQUALS = 103,
    F13 = 104,
    F14 = 105,
    F15 = 106,
    F16 = 107,
    F17 = 108,
    F18 = 109,
    F19 = 110,
    F20 = 111,
    F21 = 112,
    F22 = 113,
    F23 = 114,
    F24 = 115,
    EXECUTE = 116,
    HELP = 117,
    MENU = 118,
    SELECT = 119,
    STOP = 120,
    AGAIN = 121, /**< redo */
    UNDO = 122,
    CUT = 123,
    COPY = 124,
    PASTE = 125,
    FIND = 126,
    MUTE = 127,
    VOLUMEUP = 128,
    VOLUMEDOWN = 129,
    KP_COMMA = 133,
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
    LANG1 = 144, /**< Hangul/English toggle */
    LANG2 = 145, /**< Hanja conversion */
    LANG3 = 146, /**< Katakana */
    LANG4 = 147, /**< Hiragana */
    LANG5 = 148, /**< Zenkaku/Hankaku */
    LANG6 = 149, /**< reserved */
    LANG7 = 150, /**< reserved */
    LANG8 = 151, /**< reserved */
    LANG9 = 152, /**< reserved */

    ALTERASE = 153, /**< Erase-Eaze */
    SYSREQ = 154,
    CANCEL = 155,
    CLEAR = 156,
    PRIOR = 157,
    RETURN2 = 158,
    SEPARATOR = 159,
    KEY_OUT = 160,
    OPER = 161,
    CLEARAGAIN = 162,
    CRSEL = 163,
    EXSEL = 164,

    KP_00 = 176,
    KP_000 = 177,
    THOUSANDSSEPARATOR = 178,
    DECIMALSEPARATOR = 179,
    CURRENCYUNIT = 180,
    CURRENCYSUBUNIT = 181,
    KP_LEFTPAREN = 182,
    KP_RIGHTPAREN = 183,
    KP_LEFTBRACE = 184,
    KP_RIGHTBRACE = 185,
    KP_TAB = 186,
    KP_BACKSPACE = 187,
    KP_A = 188,
    KP_B = 189,
    KP_C = 190,
    KP_D = 191,
    KP_E = 192,
    KP_F = 193,
    KP_XOR = 194,
    KP_POWER = 195,
    KP_PERCENT = 196,
    KP_LESS = 197,
    KP_GREATER = 198,
    KP_AMPERSAND = 199,
    KP_DBLAMPERSAND = 200,
    KP_VERTICALBAR = 201,
    KP_DBLVERTICALBAR = 202,
    KP_COLON = 203,
    KP_HASH = 204,
    KP_SPACE = 205,
    KP_AT = 206,
    KP_EXCLAM = 207,
    KP_MEMSTORE = 208,
    KP_MEMRECALL = 209,
    KP_MEMCLEAR = 210,
    KP_MEMADD = 211,
    KP_MEMSUBTRACT = 212,
    KP_MEMMULTIPLY = 213,
    KP_MEMDIVIDE = 214,
    KP_PLUSMINUS = 215,
    KP_CLEAR = 216,
    KP_CLEARENTRY = 217,
    KP_BINARY = 218,
    KP_OCTAL = 219,
    KP_DECIMAL = 220,
    KP_HEXADECIMAL = 221,

    LCTRL = 224,
    LSHIFT = 225,
    LALT = 226, /**< alt, option */
    LGUI = 227, /**< windows, command (apple), meta */
    RCTRL = 228,
    RSHIFT = 229,
    RALT = 230, /**< alt gr, option */
    RGUI = 231, /**< windows, command (apple), meta */

    MODE = 257, /**< I'm not sure if this is really not covered
                                 *   by any of the above, but since there's a
                                 *   special KMOD_MODE for it I'm adding it here
                                 */

    /* @} */ /* Usage page 0x07 */

    /**
     *  \name Usage page 0x0C
     *
     *  These values are mapped from usage page 0x0C (USB consumer page).
     */
    /* @{ */

    AUDIONEXT = 258,
    AUDIOPREV = 259,
    AUDIOSTOP = 260,
    AUDIOPLAY = 261,
    AUDIOMUTE = 262,
    MEDIASELECT = 263,
    WWW = 264,
    MAIL = 265,
    CALCULATOR = 266,
    COMPUTER = 267,
    AC_SEARCH = 268,
    AC_HOME = 269,
    AC_BACK = 270,
    AC_FORWARD = 271,
    AC_STOP = 272,
    AC_REFRESH = 273,
    AC_BOOKMARKS = 274,

    /* @} */ /* Usage page 0x0C */

    /**
     *  \name Walther keys
     *
     *  These are values that Christian Walther added (for mac keyboard?).
     */
    /* @{ */

    BRIGHTNESSDOWN = 275,
    BRIGHTNESSUP = 276,
    DISPLAYSWITCH = 277, /**< display mirroring/dual display
                                           switch, video mode switch */
    KBDILLUMTOGGLE = 278,
    KBDILLUMDOWN = 279,
    KBDILLUMUP = 280,
    EJECT = 281,
    SLEEP = 282,

    APP1 = 283,
    APP2 = 284,

    /* @} */ /* Walther keys */

    /**
     *  \name Usage page 0x0C (additional media keys)
     *
     *  These values are mapped from usage page 0x0C (USB consumer page).
     */
    /* @{ */

    AUDIOREWIND = 285,
    AUDIOFASTFORWARD = 286,

    /* @} */ /* Usage page 0x0C (additional media keys) */

    /* Add any other keys here. */

    SDL_NUM_SCANCODES = 512 /**< not a key, just marks the number of scancodes
                                 for array bounds */
};

constexpr auto ScancodeToKeycode(Scancode x) -> i32
{
    return static_cast<i32>(x) | (1 << 30);
}

enum class KeyCode {
    UNKNOWN = 0,

    RETURN = '\r',
    ESCAPE = '\033',
    BACKSPACE = '\b',
    TAB = '\t',
    SPACE = ' ',
    EXCLAIM = '!',
    QUOTEDBL = '"',
    HASH = '#',
    PERCENT = '%',
    DOLLAR = '$',
    AMPERSAND = '&',
    QUOTE = '\'',
    LEFTPAREN = '(',
    RIGHTPAREN = ')',
    ASTERISK = '*',
    PLUS = '+',
    COMMA = ',',
    MINUS = '-',
    PERIOD = '.',
    SLASH = '/',
    D0 = '0',
    D1 = '1',
    D2 = '2',
    D3 = '3',
    D4 = '4',
    D5 = '5',
    D6 = '6',
    D7 = '7',
    D8 = '8',
    D9 = '9',
    COLON = ':',
    SEMICOLON = ';',
    LESS = '<',
    EQUALS = '=',
    GREATER = '>',
    QUESTION = '?',
    AT = '@',
    /*
       Skip uppercase letters
     */
    LEFTBRACKET = '[',
    BACKSLASH = '\\',
    RIGHTBRACKET = ']',
    CARET = '^',
    UNDERSCORE = '_',
    BACKQUOTE = '`',
    a = 'a',
    b = 'b',
    c = 'c',
    d = 'd',
    e = 'e',
    f = 'f',
    g = 'g',
    h = 'h',
    i = 'i',
    j = 'j',
    k = 'k',
    l = 'l',
    m = 'm',
    n = 'n',
    o = 'o',
    p = 'p',
    q = 'q',
    r = 'r',
    s = 's',
    t = 't',
    u = 'u',
    v = 'v',
    w = 'w',
    x = 'x',
    y = 'y',
    z = 'z',

    CAPSLOCK = ScancodeToKeycode(Scancode::CAPSLOCK),

    F1 = ScancodeToKeycode(Scancode::F1),
    F2 = ScancodeToKeycode(Scancode::F2),
    F3 = ScancodeToKeycode(Scancode::F3),
    F4 = ScancodeToKeycode(Scancode::F4),
    F5 = ScancodeToKeycode(Scancode::F5),
    F6 = ScancodeToKeycode(Scancode::F6),
    F7 = ScancodeToKeycode(Scancode::F7),
    F8 = ScancodeToKeycode(Scancode::F8),
    F9 = ScancodeToKeycode(Scancode::F9),
    F10 = ScancodeToKeycode(Scancode::F10),
    F11 = ScancodeToKeycode(Scancode::F11),
    F12 = ScancodeToKeycode(Scancode::F12),

    PRINTSCREEN = ScancodeToKeycode(Scancode::PRINTSCREEN),
    SCROLLLOCK = ScancodeToKeycode(Scancode::SCROLLLOCK),
    PAUSE = ScancodeToKeycode(Scancode::PAUSE),
    INSERT = ScancodeToKeycode(Scancode::INSERT),
    HOME = ScancodeToKeycode(Scancode::HOME),
    PAGEUP = ScancodeToKeycode(Scancode::PAGEUP),
    DEL = '\177',
    END = ScancodeToKeycode(Scancode::END),
    PAGEDOWN = ScancodeToKeycode(Scancode::PAGEDOWN),
    RIGHT = ScancodeToKeycode(Scancode::RIGHT),
    LEFT = ScancodeToKeycode(Scancode::LEFT),
    DOWN = ScancodeToKeycode(Scancode::DOWN),
    UP = ScancodeToKeycode(Scancode::UP),

    NUMLOCKCLEAR = ScancodeToKeycode(Scancode::NUMLOCKCLEAR),
    KP_DIVIDE = ScancodeToKeycode(Scancode::KP_DIVIDE),
    KP_MULTIPLY = ScancodeToKeycode(Scancode::KP_MULTIPLY),
    KP_MINUS = ScancodeToKeycode(Scancode::KP_MINUS),
    KP_PLUS = ScancodeToKeycode(Scancode::KP_PLUS),
    KP_ENTER = ScancodeToKeycode(Scancode::KP_ENTER),
    KP_1 = ScancodeToKeycode(Scancode::KP_1),
    KP_2 = ScancodeToKeycode(Scancode::KP_2),
    KP_3 = ScancodeToKeycode(Scancode::KP_3),
    KP_4 = ScancodeToKeycode(Scancode::KP_4),
    KP_5 = ScancodeToKeycode(Scancode::KP_5),
    KP_6 = ScancodeToKeycode(Scancode::KP_6),
    KP_7 = ScancodeToKeycode(Scancode::KP_7),
    KP_8 = ScancodeToKeycode(Scancode::KP_8),
    KP_9 = ScancodeToKeycode(Scancode::KP_9),
    KP_0 = ScancodeToKeycode(Scancode::KP_0),
    KP_PERIOD = ScancodeToKeycode(Scancode::KP_PERIOD),

    APPLICATION = ScancodeToKeycode(Scancode::APPLICATION),
    POWER = ScancodeToKeycode(Scancode::POWER),
    KP_EQUALS = ScancodeToKeycode(Scancode::KP_EQUALS),
    F13 = ScancodeToKeycode(Scancode::F13),
    F14 = ScancodeToKeycode(Scancode::F14),
    F15 = ScancodeToKeycode(Scancode::F15),
    F16 = ScancodeToKeycode(Scancode::F16),
    F17 = ScancodeToKeycode(Scancode::F17),
    F18 = ScancodeToKeycode(Scancode::F18),
    F19 = ScancodeToKeycode(Scancode::F19),
    F20 = ScancodeToKeycode(Scancode::F20),
    F21 = ScancodeToKeycode(Scancode::F21),
    F22 = ScancodeToKeycode(Scancode::F22),
    F23 = ScancodeToKeycode(Scancode::F23),
    F24 = ScancodeToKeycode(Scancode::F24),
    EXECUTE = ScancodeToKeycode(Scancode::EXECUTE),
    HELP = ScancodeToKeycode(Scancode::HELP),
    MENU = ScancodeToKeycode(Scancode::MENU),
    SELECT = ScancodeToKeycode(Scancode::SELECT),
    STOP = ScancodeToKeycode(Scancode::STOP),
    AGAIN = ScancodeToKeycode(Scancode::AGAIN),
    UNDO = ScancodeToKeycode(Scancode::UNDO),
    CUT = ScancodeToKeycode(Scancode::CUT),
    COPY = ScancodeToKeycode(Scancode::COPY),
    PASTE = ScancodeToKeycode(Scancode::PASTE),
    FIND = ScancodeToKeycode(Scancode::FIND),
    MUTE = ScancodeToKeycode(Scancode::MUTE),
    VOLUMEUP = ScancodeToKeycode(Scancode::VOLUMEUP),
    VOLUMEDOWN = ScancodeToKeycode(Scancode::VOLUMEDOWN),
    KP_COMMA = ScancodeToKeycode(Scancode::KP_COMMA),
    KP_EQUALSAS400 = ScancodeToKeycode(Scancode::KP_EQUALSAS400),

    ALTERASE = ScancodeToKeycode(Scancode::ALTERASE),
    SYSREQ = ScancodeToKeycode(Scancode::SYSREQ),
    CANCEL = ScancodeToKeycode(Scancode::CANCEL),
    CLEAR = ScancodeToKeycode(Scancode::CLEAR),
    PRIOR = ScancodeToKeycode(Scancode::PRIOR),
    RETURN2 = ScancodeToKeycode(Scancode::RETURN2),
    SEPARATOR = ScancodeToKeycode(Scancode::SEPARATOR),
    KEY_OUT = ScancodeToKeycode(Scancode::KEY_OUT),
    OPER = ScancodeToKeycode(Scancode::OPER),
    CLEARAGAIN = ScancodeToKeycode(Scancode::CLEARAGAIN),
    CRSEL = ScancodeToKeycode(Scancode::CRSEL),
    EXSEL = ScancodeToKeycode(Scancode::EXSEL),

    KP_00 = ScancodeToKeycode(Scancode::KP_00),
    KP_000 = ScancodeToKeycode(Scancode::KP_000),
    THOUSANDSSEPARATOR = ScancodeToKeycode(Scancode::THOUSANDSSEPARATOR),
    DECIMALSEPARATOR = ScancodeToKeycode(Scancode::DECIMALSEPARATOR),
    CURRENCYUNIT = ScancodeToKeycode(Scancode::CURRENCYUNIT),
    CURRENCYSUBUNIT = ScancodeToKeycode(Scancode::CURRENCYSUBUNIT),
    KP_LEFTPAREN = ScancodeToKeycode(Scancode::KP_LEFTPAREN),
    KP_RIGHTPAREN = ScancodeToKeycode(Scancode::KP_RIGHTPAREN),
    KP_LEFTBRACE = ScancodeToKeycode(Scancode::KP_LEFTBRACE),
    KP_RIGHTBRACE = ScancodeToKeycode(Scancode::KP_RIGHTBRACE),
    KP_TAB = ScancodeToKeycode(Scancode::KP_TAB),
    KP_BACKSPACE = ScancodeToKeycode(Scancode::KP_BACKSPACE),
    KP_A = ScancodeToKeycode(Scancode::KP_A),
    KP_B = ScancodeToKeycode(Scancode::KP_B),
    KP_C = ScancodeToKeycode(Scancode::KP_C),
    KP_D = ScancodeToKeycode(Scancode::KP_D),
    KP_E = ScancodeToKeycode(Scancode::KP_E),
    KP_F = ScancodeToKeycode(Scancode::KP_F),
    KP_XOR = ScancodeToKeycode(Scancode::KP_XOR),
    KP_POWER = ScancodeToKeycode(Scancode::KP_POWER),
    KP_PERCENT = ScancodeToKeycode(Scancode::KP_PERCENT),
    KP_LESS = ScancodeToKeycode(Scancode::KP_LESS),
    KP_GREATER = ScancodeToKeycode(Scancode::KP_GREATER),
    KP_AMPERSAND = ScancodeToKeycode(Scancode::KP_AMPERSAND),
    KP_DBLAMPERSAND = ScancodeToKeycode(Scancode::KP_DBLAMPERSAND),
    KP_VERTICALBAR = ScancodeToKeycode(Scancode::KP_VERTICALBAR),
    KP_DBLVERTICALBAR = ScancodeToKeycode(Scancode::KP_DBLVERTICALBAR),
    KP_COLON = ScancodeToKeycode(Scancode::KP_COLON),
    KP_HASH = ScancodeToKeycode(Scancode::KP_HASH),
    KP_SPACE = ScancodeToKeycode(Scancode::KP_SPACE),
    KP_AT = ScancodeToKeycode(Scancode::KP_AT),
    KP_EXCLAM = ScancodeToKeycode(Scancode::KP_EXCLAM),
    KP_MEMSTORE = ScancodeToKeycode(Scancode::KP_MEMSTORE),
    KP_MEMRECALL = ScancodeToKeycode(Scancode::KP_MEMRECALL),
    KP_MEMCLEAR = ScancodeToKeycode(Scancode::KP_MEMCLEAR),
    KP_MEMADD = ScancodeToKeycode(Scancode::KP_MEMADD),
    KP_MEMSUBTRACT = ScancodeToKeycode(Scancode::KP_MEMSUBTRACT),
    KP_MEMMULTIPLY = ScancodeToKeycode(Scancode::KP_MEMMULTIPLY),
    KP_MEMDIVIDE = ScancodeToKeycode(Scancode::KP_MEMDIVIDE),
    KP_PLUSMINUS = ScancodeToKeycode(Scancode::KP_PLUSMINUS),
    KP_CLEAR = ScancodeToKeycode(Scancode::KP_CLEAR),
    KP_CLEARENTRY = ScancodeToKeycode(Scancode::KP_CLEARENTRY),
    KP_BINARY = ScancodeToKeycode(Scancode::KP_BINARY),
    KP_OCTAL = ScancodeToKeycode(Scancode::KP_OCTAL),
    KP_DECIMAL = ScancodeToKeycode(Scancode::KP_DECIMAL),
    KP_HEXADECIMAL = ScancodeToKeycode(Scancode::KP_HEXADECIMAL),

    LCTRL = ScancodeToKeycode(Scancode::LCTRL),
    LSHIFT = ScancodeToKeycode(Scancode::LSHIFT),
    LALT = ScancodeToKeycode(Scancode::LALT),
    LGUI = ScancodeToKeycode(Scancode::LGUI),
    RCTRL = ScancodeToKeycode(Scancode::RCTRL),
    RSHIFT = ScancodeToKeycode(Scancode::RSHIFT),
    RALT = ScancodeToKeycode(Scancode::RALT),
    RGUI = ScancodeToKeycode(Scancode::RGUI),

    MODE = ScancodeToKeycode(Scancode::MODE),

    AUDIONEXT = ScancodeToKeycode(Scancode::AUDIONEXT),
    AUDIOPREV = ScancodeToKeycode(Scancode::AUDIOPREV),
    AUDIOSTOP = ScancodeToKeycode(Scancode::AUDIOSTOP),
    AUDIOPLAY = ScancodeToKeycode(Scancode::AUDIOPLAY),
    AUDIOMUTE = ScancodeToKeycode(Scancode::AUDIOMUTE),
    MEDIASELECT = ScancodeToKeycode(Scancode::MEDIASELECT),
    WWW = ScancodeToKeycode(Scancode::WWW),
    MAIL = ScancodeToKeycode(Scancode::MAIL),
    CALCULATOR = ScancodeToKeycode(Scancode::CALCULATOR),
    COMPUTER = ScancodeToKeycode(Scancode::COMPUTER),
    AC_SEARCH = ScancodeToKeycode(Scancode::AC_SEARCH),
    AC_HOME = ScancodeToKeycode(Scancode::AC_HOME),
    AC_BACK = ScancodeToKeycode(Scancode::AC_BACK),
    AC_FORWARD = ScancodeToKeycode(Scancode::AC_FORWARD),
    AC_STOP = ScancodeToKeycode(Scancode::AC_STOP),
    AC_REFRESH = ScancodeToKeycode(Scancode::AC_REFRESH),
    AC_BOOKMARKS = ScancodeToKeycode(Scancode::AC_BOOKMARKS),

    BRIGHTNESSDOWN = ScancodeToKeycode(Scancode::BRIGHTNESSDOWN),
    BRIGHTNESSUP = ScancodeToKeycode(Scancode::BRIGHTNESSUP),
    DISPLAYSWITCH = ScancodeToKeycode(Scancode::DISPLAYSWITCH),
    KBDILLUMTOGGLE = ScancodeToKeycode(Scancode::KBDILLUMTOGGLE),
    KBDILLUMDOWN = ScancodeToKeycode(Scancode::KBDILLUMDOWN),
    KBDILLUMUP = ScancodeToKeycode(Scancode::KBDILLUMUP),
    EJECT = ScancodeToKeycode(Scancode::EJECT),
    SLEEP = ScancodeToKeycode(Scancode::SLEEP),
    APP1 = ScancodeToKeycode(Scancode::APP1),
    APP2 = ScancodeToKeycode(Scancode::APP2),

    AUDIOREWIND = ScancodeToKeycode(Scancode::AUDIOREWIND),
    AUDIOFASTFORWARD = ScancodeToKeycode(Scancode::AUDIOFASTFORWARD)
};

////////////////////////////////////////////////////////////

enum class KeyMod {
    None = 0x0000,
    LShift = 0x0001,
    RShift = 0x0002,
    LCtrl = 0x0040,
    RCtrl = 0x0080,
    LAlt = 0x0100,
    RAlt = 0x0200,
    LGui = 0x0400,
    RGui = 0x0800,
    Num = 0x1000,
    Caps = 0x2000,
    Mode = 0x4000,
    Ctrl = LCtrl | RCtrl,
    Shift = LShift | RShift,
    Alt = LAlt | RAlt,
    Gui = LGui | RGui
};

struct KeyboardEvent {
    bool Pressed;
    bool Repeat;
    Scancode Code;
    KeyCode Key;
    KeyMod Mod;
};

////////////////////////////////////////////////////////////

struct TextInputEvent {
    std::string Text;
};

////////////////////////////////////////////////////////////

struct TextEditingEvent {
    std::string Text;
    i32 Start;
    i32 Length;
};

////////////////////////////////////////////////////////////

struct MouseMotionEvent {
    PointI Position;
    PointI RelativeMotion;
};

////////////////////////////////////////////////////////////

enum class MouseButton {
    None = 0,
    Left = 1,
    Middle = 2,
    Right = 3,
    X1 = 4,
    X2 = 5
};

struct MouseButtonEvent {
    MouseButton Button;
    bool Pressed;
    u8 Clicks;
    PointI Position;
};

////////////////////////////////////////////////////////////

struct MouseWheelEvent {
    PointI Scroll;
    bool Flipped;
};

////////////////////////////////////////////////////////////

struct JoyAxisEvent {
    i32 JoystickID;
    u8 Axis;
    i16 Value;
};

////////////////////////////////////////////////////////////

enum class JoyHat {
    Centered = 0x00,
    Up = 0x01,
    Right = 0x02,
    Down = 0x04,
    Left = 0x08,
    RightUp = Right | Up,
    RightDown = Right | Down,
    LeftUp = Left | Up,
    LeftDown = Left | Down,
};

struct JoyHatEvent {
    i32 JoystickID;
    JoyHat Hat;
    u8 Value;
};

////////////////////////////////////////////////////////////

struct JoyButtonEvent {
    i32 JoystickID;
    u8 Button;
    bool Pressed;
};

////////////////////////////////////////////////////////////

enum class GameControllerAxis {
    Invalid = -1,
    LeftX,
    LeftY,
    RightX,
    RightY,
    TriggerLeft,
    TriggerRight,
};

struct ControllerAxisEvent {
    i32 JoystickID;
    GameControllerAxis Axis;
    i16 Value;
};

////////////////////////////////////////////////////////////

enum class GameControllerButton {
    Invalid = -1,
    A,
    B,
    X,
    Y,
    Back,
    Guide,
    Start,
    LeftStick,
    RightStick,
    LeftShoulder,
    RightShoulder,
    DPadUp,
    DPadDown,
    DPadLeft,
    DPadRight
};

struct ControllerButtonEvent {
    i32 JoystickID;
    GameControllerButton Button;
    bool Pressed;
};

////////////////////////////////////////////////////////////

class GameController final {
public:
    GameController(SDL_GameController* controller);

    auto name() const -> std::string;

    auto rumble(u16 lowFrequencyRumble, u16 highFrequencyRumble, u32 duration) const -> bool;

    auto is_button_pressed(GameControllerButton button) const -> bool;
    auto has_button(GameControllerButton button) const -> bool;
    static auto ButtonName(GameControllerButton button) -> std::string;

    auto axis_value(GameControllerAxis axis) const -> i16;
    auto has_axis(GameControllerAxis axis) const -> bool;
    static auto AxisName(GameControllerAxis axis) -> std::string;

    auto is_valid() const -> bool;

private:
    SDL_GameController* _controller;
};

////////////////////////////////////////////////////////////

enum class InputMode {
    KeyboardMouse,
    Joystick,
    Controller
};

class Input final {
public:
    Input();
    ~Input();

    sigslot::signal<KeyboardEvent&> KeyDown;
    sigslot::signal<KeyboardEvent&> KeyUp;
    sigslot::signal<TextInputEvent&> TextInput;
    sigslot::signal<TextEditingEvent&> TextEditing;
    sigslot::signal<MouseMotionEvent&> MouseMotion;
    sigslot::signal<MouseButtonEvent&> MouseButtonDown;
    sigslot::signal<MouseButtonEvent&> MouseButtonUp;
    sigslot::signal<MouseWheelEvent&> MouseWheel;
    sigslot::signal<JoyAxisEvent&> JoyAxisMotion;
    sigslot::signal<JoyHatEvent&> JoyHatMotion;
    sigslot::signal<JoyButtonEvent&> JoyButtonDown;
    sigslot::signal<JoyButtonEvent&> JoyButtonUp;
    sigslot::signal<ControllerAxisEvent&> ControllerAxisMotion;
    sigslot::signal<ControllerButtonEvent&> ControllerButtonDown;
    sigslot::signal<ControllerButtonEvent&> ControllerButtonUp;
    sigslot::signal<InputMode> InputModeChanged;

    auto controller_at(u32 index) -> GameController;
    auto controller_count() -> u32;

    auto mode() -> InputMode;
    void process_events(SDL_Event* ev);

private:
    std::unordered_map<i32, SDL_GameController*> _controllers;
    InputMode _mode { InputMode::KeyboardMouse };
};
}