/** Colton Beckstead - 2026
 * This file should wrap the necessary header file includes in conditional
 * compilation that detects which platform / backend the application is being
 * built for.
 *
 *
 */


#ifndef PLATFORM_H
#define PLATFORM_H

//Detect what platform is being built for:
#ifdef BUILD_CURSES

#include "../platform_curses/display.h"

#else
#include "Compile Error: valid build platform not specified"
#endif //#ifdef BUILD_CURSES




#endif //PLATFORM_H

