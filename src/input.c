/* Colton Beckstead - 2026
 * 
 * This file handles mapping input from platform-dependant libraries to
 * platform-independent game logic.
 * Additionally, it contains functions to interpret inputs and call the
 * necessary player code based on those inputs.
 *
 * All functions in this file shall begin with platform_input_ as a namespace indicator.
 *
 */


#include "input.h"
//#include "platform_input.h"

enum Input_Result input_handle_keystroke(struct State *s, int key)
{
    switch (key) {
        case gKEY_UP:
            s->p.y -= 1;
            break;
        case gKEY_DOWN:
            s->p.y += 1;
            break;
        case gKEY_LEFT:
            s->p.x -= 1;
            break;
        case gKEY_RIGHT:
            s->p.x += 1;
            break;
        case 'q':
            [[fallthrough]];
        case 'Q':
            return IR_QUIT;
        default:
            return IR_NONE;
    }
    return IR_NONE;
}
