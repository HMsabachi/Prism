#pragma once

namespace Prism
{
    typedef enum class KeyCode : uint16_t
    {
        // From glfw3.h
        Space = 32,
        Apostrophe = 39, /* ' */
        Comma = 44, /* , */
        Minus = 45, /* - */
        Period = 46, /* . */
        Slash = 47, /* / */

        D0 = 48, /* 0 */
        D1 = 49, /* 1 */
        D2 = 50, /* 2 */
        D3 = 51, /* 3 */
        D4 = 52, /* 4 */
        D5 = 53, /* 5 */
        D6 = 54, /* 6 */
        D7 = 55, /* 7 */
        D8 = 56, /* 8 */
        D9 = 57, /* 9 */

        Semicolon = 59, /* ; */
        Equal = 61, /* = */

        A = 65,
        B = 66,
        C = 67,
        D = 68,
        E = 69,
        F = 70,
        G = 71,
        H = 72,
        I = 73,
        J = 74,
        K = 75,
        L = 76,
        M = 77,
        N = 78,
        O = 79,
        P = 80,
        Q = 81,
        R = 82,
        S = 83,
        T = 84,
        U = 85,
        V = 86,
        W = 87,
        X = 88,
        Y = 89,
        Z = 90,

        LeftBracket = 91,  /* [ */
        Backslash = 92,  /* \ */
        RightBracket = 93,  /* ] */
        GraveAccent = 96,  /* ` */

        World1 = 161, /* non-US #1 */
        World2 = 162, /* non-US #2 */

        /* Function keys */
        Escape = 256,
        Enter = 257,
        Tab = 258,
        Backspace = 259,
        Insert = 260,
        Delete = 261,
        Right = 262,
        Left = 263,
        Down = 264,
        Up = 265,
        PageUp = 266,
        PageDown = 267,
        Home = 268,
        End = 269,
        CapsLock = 280,
        ScrollLock = 281,
        NumLock = 282,
        PrintScreen = 283,
        Pause = 284,
        F1 = 290,
        F2 = 291,
        F3 = 292,
        F4 = 293,
        F5 = 294,
        F6 = 295,
        F7 = 296,
        F8 = 297,
        F9 = 298,
        F10 = 299,
        F11 = 300,
        F12 = 301,
        F13 = 302,
        F14 = 303,
        F15 = 304,
        F16 = 305,
        F17 = 306,
        F18 = 307,
        F19 = 308,
        F20 = 309,
        F21 = 310,
        F22 = 311,
        F23 = 312,
        F24 = 313,
        F25 = 314,

        /* Keypad */
        KP0 = 320,
        KP1 = 321,
        KP2 = 322,
        KP3 = 323,
        KP4 = 324,
        KP5 = 325,
        KP6 = 326,
        KP7 = 327,
        KP8 = 328,
        KP9 = 329,
        KPDecimal = 330,
        KPDivide = 331,
        KPMultiply = 332,
        KPSubtract = 333,
        KPAdd = 334,
        KPEnter = 335,
        KPEqual = 336,

        LeftShift = 340,
        LeftControl = 341,
        LeftAlt = 342,
        LeftSuper = 343,
        RightShift = 344,
        RightControl = 345,
        RightAlt = 346,
        RightSuper = 347,
        Menu = 348
    } Key;

    inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
    {
        os << static_cast<int32_t>(keyCode);
        return os;
    }
    inline auto format_as(KeyCode keyCode) { return static_cast<int32_t>(keyCode); }
}

// From glfw3.h

/* The unknown key */ /* 一般按键 */
#define PR_KEY_SPACE           ::Prism::Key::Space
#define PR_KEY_APOSTROPHE      ::Prism::Key::Apostrophe    /* ' */
#define PR_KEY_COMMA           ::Prism::Key::Comma         /* , */
#define PR_KEY_MINUS           ::Prism::Key::Minus         /* - */
#define PR_KEY_PERIOD          ::Prism::Key::Period        /* . */
#define PR_KEY_SLASH           ::Prism::Key::Slash         /* / */
#define PR_KEY_0               ::Prism::Key::D0
#define PR_KEY_1               ::Prism::Key::D1
#define PR_KEY_2               ::Prism::Key::D2
#define PR_KEY_3               ::Prism::Key::D3
#define PR_KEY_4               ::Prism::Key::D4
#define PR_KEY_5               ::Prism::Key::D5
#define PR_KEY_6               ::Prism::Key::D6
#define PR_KEY_7               ::Prism::Key::D7
#define PR_KEY_8               ::Prism::Key::D8
#define PR_KEY_9               ::Prism::Key::D9
#define PR_KEY_SEMICOLON       ::Prism::Key::Semicolon     /* ; */
#define PR_KEY_EQUAL           ::Prism::Key::Equal         /* = */
#define PR_KEY_A               ::Prism::Key::A
#define PR_KEY_B               ::Prism::Key::B
#define PR_KEY_C               ::Prism::Key::C
#define PR_KEY_D               ::Prism::Key::D
#define PR_KEY_E               ::Prism::Key::E
#define PR_KEY_F               ::Prism::Key::F
#define PR_KEY_G               ::Prism::Key::G
#define PR_KEY_H               ::Prism::Key::H
#define PR_KEY_I               ::Prism::Key::I
#define PR_KEY_J               ::Prism::Key::J
#define PR_KEY_K               ::Prism::Key::K
#define PR_KEY_L               ::Prism::Key::L
#define PR_KEY_M               ::Prism::Key::M
#define PR_KEY_N               ::Prism::Key::N
#define PR_KEY_O               ::Prism::Key::O
#define PR_KEY_P               ::Prism::Key::P
#define PR_KEY_Q               ::Prism::Key::Q
#define PR_KEY_R               ::Prism::Key::R
#define PR_KEY_S               ::Prism::Key::S
#define PR_KEY_T               ::Prism::Key::T
#define PR_KEY_U               ::Prism::Key::U
#define PR_KEY_V               ::Prism::Key::V
#define PR_KEY_W               ::Prism::Key::W
#define PR_KEY_X               ::Prism::Key::X
#define PR_KEY_Y               ::Prism::Key::Y
#define PR_KEY_Z               ::Prism::Key::Z
#define PR_KEY_LEFT_BRACKET    ::Prism::Key::LeftBracket   /* [ */
#define PR_KEY_BACKSLASH       ::Prism::Key::Backslash     /* \ */
#define PR_KEY_RIGHT_BRACKET   ::Prism::Key::RightBracket  /* ] */
#define PR_KEY_GRAVE_ACCENT    ::Prism::Key::GraveAccent   /* ` */
#define PR_KEY_WORLD_1         ::Prism::Key::World1        /* non-US #1 */
#define PR_KEY_WORLD_2         ::Prism::Key::World2

/* Function keys */ /* 功能按键 */
#define PR_KEY_ESCAPE          ::Prism::Key::Escape
#define PR_KEY_ENTER           ::Prism::Key::Enter
#define PR_KEY_TAB             ::Prism::Key::Tab
#define PR_KEY_BACKSPACE       ::Prism::Key::Backspace
#define PR_KEY_INSERT          ::Prism::Key::Insert
#define PR_KEY_DELETE          ::Prism::Key::Delete
#define PR_KEY_RIGHT           ::Prism::Key::Right
#define PR_KEY_LEFT            ::Prism::Key::Left
#define PR_KEY_DOWN            ::Prism::Key::Down
#define PR_KEY_UP              ::Prism::Key::Up
#define PR_KEY_PAGE_UP         ::Prism::Key::PageUp
#define PR_KEY_PAGE_DOWN       ::Prism::Key::PageDown
#define PR_KEY_HOME            ::Prism::Key::Home
#define PR_KEY_END             ::Prism::Key::End
#define PR_KEY_CAPS_LOCK       ::Prism::Key::CapsLock
#define PR_KEY_SCROLL_LOCK     ::Prism::Key::ScrollLock
#define PR_KEY_NUM_LOCK        ::Prism::Key::NumLock
#define PR_KEY_PRINT_SCREEN    ::Prism::Key::PrintScreen
#define PR_KEY_PAUSE           ::Prism::Key::Pause
#define PR_KEY_F1              ::Prism::Key::F1
#define PR_KEY_F2              ::Prism::Key::F2
#define PR_KEY_F3              ::Prism::Key::F3
#define PR_KEY_F4              ::Prism::Key::F4
#define PR_KEY_F5              ::Prism::Key::F5
#define PR_KEY_F6              ::Prism::Key::F6
#define PR_KEY_F7              ::Prism::Key::F7
#define PR_KEY_F8              ::Prism::Key::F8
#define PR_KEY_F9              ::Prism::Key::F9
#define PR_KEY_F10             ::Prism::Key::F10
#define PR_KEY_F11             ::Prism::Key::F11
#define PR_KEY_F12             ::Prism::Key::F12
#define PR_KEY_F13             ::Prism::Key::F13
#define PR_KEY_F14             ::Prism::Key::F14
#define PR_KEY_F15             ::Prism::Key::F15
#define PR_KEY_F16             ::Prism::Key::F16
#define PR_KEY_F17             ::Prism::Key::F17
#define PR_KEY_F18             ::Prism::Key::F18
#define PR_KEY_F19             ::Prism::Key::F19
#define PR_KEY_F20             ::Prism::Key::F20
#define PR_KEY_F21             ::Prism::Key::F21
#define PR_KEY_F22             ::Prism::Key::F22
#define PR_KEY_F23             ::Prism::Key::F23
#define PR_KEY_F24             ::Prism::Key::F24
#define PR_KEY_F25             ::Prism::Key::F25

/* Keypad */
#define PR_KEY_KP_0            ::Prism::Key::KP0
#define PR_KEY_KP_1            ::Prism::Key::KP1
#define PR_KEY_KP_2            ::Prism::Key::KP2
#define PR_KEY_KP_3            ::Prism::Key::KP3
#define PR_KEY_KP_4            ::Prism::Key::KP4
#define PR_KEY_KP_5            ::Prism::Key::KP5
#define PR_KEY_KP_6            ::Prism::Key::KP6
#define PR_KEY_KP_7            ::Prism::Key::KP7
#define PR_KEY_KP_8            ::Prism::Key::KP8
#define PR_KEY_KP_9            ::Prism::Key::KP9
#define PR_KEY_KP_DECIMAL      ::Prism::Key::KPDecimal
#define PR_KEY_KP_DIVIDE       ::Prism::Key::KPDivide
#define PR_KEY_KP_MULTIPLY     ::Prism::Key::KPMultiply
#define PR_KEY_KP_SUBTRACT     ::Prism::Key::KPSubtract
#define PR_KEY_KP_ADD          ::Prism::Key::KPAdd
#define PR_KEY_KP_ENTER        ::Prism::Key::KPEnter
#define PR_KEY_KP_EQUAL        ::Prism::Key::KPEqual

#define PR_KEY_LEFT_SHIFT      ::Prism::Key::LeftShift
#define PR_KEY_LEFT_CONTROL    ::Prism::Key::LeftControl
#define PR_KEY_LEFT_ALT        ::Prism::Key::LeftAlt
#define PR_KEY_LEFT_SUPER      ::Prism::Key::LeftSuper
#define PR_KEY_RIGHT_SHIFT     ::Prism::Key::RightShift
#define PR_KEY_RIGHT_CONTROL   ::Prism::Key::RightControl
#define PR_KEY_RIGHT_ALT       ::Prism::Key::RightAlt
#define PR_KEY_RIGHT_SUPER     ::Prism::Key::RightSuper
#define PR_KEY_MENU            ::Prism::Key::Menu
