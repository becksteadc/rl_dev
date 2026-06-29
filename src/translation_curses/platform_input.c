/* Colton Beckstead - 2026
 * 
 * This file handles mapping input from platform-dependant libraries to
 * platform-independent game logic.
 * 
 * It is *not* for performing game logic for handling input. That is platform-
 * agnostic code that should be kept separate from this platform-dependant code.
 * 
 * Part of the translation_curses layer of the program
 *
 * All functions in this file shall begin with platform_input_ as a namespace indicator.
 *
 */

#include <curses.h>
#include "platform_input.h"
#include "../game_layer/input.h"

int platform_input_map_keystroke(int key)
{
    switch (key) {
    case KEY_UP:
        return gKEY_UP;
    case KEY_DOWN:
        return gKEY_DOWN;
    case KEY_LEFT:
        return gKEY_LEFT;
    case KEY_RIGHT:
        return gKEY_RIGHT;
    case KEY_A1: //upper left of keypad: (numpad 7)
        return gKEY_NUMPAD_7;
    case KEY_A3:
        return gKEY_NUMPAD_9;
    case KEY_C1:
        return gKEY_NUMPAD_1;
    case KEY_C3:
        return gKEY_NUMPAD_3;
    default:
        return key;
    }
}

////// These cases should not be necessary. Passing them in by default should be fine.
//    case '!':
//        return gKEY_EXCLAMATION;
//    case '"':
//        return gKEY_DQUOTE;
//    case '\'':
//        return gKEY_SQUOTE;
//    case '#':
//        return gKEY_POUND; // #
//    case '$':
//        return gKEY_DOLLAR; // $
//    case '%':
//        return gKEY_PERCENT;
//    case '&':
//        return gKEY_AMPERSAND;
//    case '(':
//        return gKEY_OPEN_PARENTHESES;
//    case ')':
//        return gKEY_CLOSE_PARENTHESES;
//    case '*':
//        return gKEY_ASTERISK;
//    case '=':
//        return gKEY_EQUALS;
//    case '+':
//        return gKEY_PLUS;
//    case '-':
//        return gKEY_HYPHEN;
//    case '_':
//        return gKEY_UNDERSCORE;
//    case '/':
//        return gKEY_FORWARD_SLASH;
//    return gKEY_BACK_SLASH;
//    return gKEY_LESS_THAN; // <
//    return gKEY_GREATER_THAN; //>
//    return gKEY_QUESTION_MARK;
//    return gKEY_COLON;
//    return gKEY_SEMICOLON;
//    return gKEY_LEFT_CURLY_BRACE; // {
//    return gKEY_RIGHT_CURLY_BRACE; // }
//    return gKEY_LEFT_BRACKET; // [
//    return gKEY_RIGHT_BRACKET; // ]
//    return gKEY_TILDE;
//    return gKEY_BACK_TICK;
//    return gKEY_PIPE; // |
//    return gKEY_PERIOD;
//    return gKEY_CARROT; // ^
