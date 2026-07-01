/* Colton Beckstead - 2026
 * 
 * This file is part of the game logic layer (3).
 * It provides game logic to interact with player input.
 *
 * --- Note: the namespace prefix for this file may be changed.
 * --- It is currently under consideration for how to restructure things.
 *
 */


#include "input.h"
//#include "platform_input.h"

enum Input_Result input_handle_keystroke(int key)
{
    switch (key) {
        case gKEY_UP:
            [[fallthrough]];
        case gKEY_DOWN:
            [[fallthrough]];
        case gKEY_LEFT:
            [[fallthrough]];
        case gKEY_RIGHT:
            [[fallthrough]];
        case gKEY_NUMPAD_7:
            [[fallthrough]];
        case gKEY_NUMPAD_9:
            [[fallthrough]];
        case gKEY_NUMPAD_1:
            [[fallthrough]];
        case gKEY_NUMPAD_3:
            return IR_MOVE; //NOT fallthrough
        case 'q':
            [[fallthrough]];
        case 'Q':
            return IR_QUIT;
        default:
            return IR_NONE;
    }
    return IR_NONE;
}
