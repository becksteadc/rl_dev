#ifndef INPUT_H
#define INPUT_H

enum Input_Result {
    IR_NONE,
    IR_QUIT, // Signals that the program should clean up and exit normally (user quit)
    // ...TODO - make more as needed - things that can't be handled
    // by input.c alone. There should not be many here.
};


#include "main.h"

enum Input_Result input_handle_keystroke(struct State *s, int key);

//Prefixing every enum value with 'g' - think "game_key_up" for example
//This is to separate it from the curses naming of gKEY_UP, etc. ("Library" enums vs "game" ones.
enum {
    gKEY_UP,
    gKEY_DOWN,
    gKEY_LEFT,
    gKEY_RIGHT,
    gKEY_A = 'A',
    gKEY_B = 'B',
    gKEY_C = 'C',
    gKEY_D = 'D',
    gKEY_E = 'E',
    gKEY_F = 'F',
    gKEY_G = 'G',
    gKEY_H = 'H',
    gKEY_I = 'I',
    gKEY_J = 'J',
    gKEY_K = 'K',
    gKEY_L = 'L',
    gKEY_M = 'M',
    gKEY_N = 'N',
    gKEY_O = 'O',
    gKEY_P = 'P',
    gKEY_Q = 'Q',
    gKEY_R = 'R',
    gKEY_S = 'S',
    gKEY_T = 'T',
    gKEY_U = 'U',
    gKEY_V = 'V',
    gKEY_W = 'W',
    gKEY_X = 'X',
    gKEY_Y = 'Y',
    gKEY_Z = 'Z',
    gKEY_a = 'a',
    gKEY_b = 'b',
    gKEY_c = 'c',
    gKEY_d = 'd',
    gKEY_e = 'e',
    gKEY_f = 'f',
    gKEY_g = 'g',
    gKEY_h = 'h',
    gKEY_i = 'i',
    gKEY_j = 'j',
    gKEY_k = 'k',
    gKEY_l = 'l',
    gKEY_m = 'm',
    gKEY_n = 'n',
    gKEY_o = 'o',
    gKEY_p = 'p',
    gKEY_q = 'q',
    gKEY_r = 'r',
    gKEY_s = 's',
    gKEY_t = 't',
    gKEY_u = 'u',
    gKEY_v = 'v',
    gKEY_w = 'w',
    gKEY_x = 'x',
    gKEY_y = 'y',
    gKEY_z = 'z',
    gKEY_EXCLAMATION = '!',
    gKEY_DQUOTE = '"',
    gKEY_SQUOTE = '\'',
    gKEY_POUND = '#', // #
    gKEY_DOLLAR = '$', // $
    gKEY_PERCENT = '%',
    gKEY_AMPERSAND = '&',
    gKEY_OPEN_PARENTHESES = '(',
    gKEY_CLOSE_PARENTHESES = ')',
    gKEY_ASTERISK = '*',
    gKEY_EQUALS = '=',
    gKEY_PLUS = '+',
    gKEY_HYPHEN = '-',
    gKEY_UNDERSCORE = '_',
    gKEY_FORWARD_SLASH = '/',
    gKEY_BACK_SLASH = '\\',
    gKEY_LESS_THAN = '<', // <
    gKEY_GREATER_THAN = '>', //>
    gKEY_QUESTION_MARK = '?',
    gKEY_COLON = ':',
    gKEY_SEMICOLON = ';',
    gKEY_LEFT_CURLY_BRACE = '{', // {
    gKEY_RIGHT_CURLY_BRACE = '}', // }
    gKEY_LEFT_BRACKET = '[', // [
    gKEY_RIGHT_BRACKET = ']', // ]
    gKEY_TILDE = '~',
    gKEY_BACK_TICK = '`',
    gKEY_PIPE = '|', // |
    gKEY_PERIOD = '.',
    gKEY_CARROT = '^', // ^
};


#endif
