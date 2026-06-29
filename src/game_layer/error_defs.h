/* Colton Beckstead - 2026
 * This header provides for the generalized error handling of the program.
 * It defines a simple enum for error types, as well as extern declarations
 * for various namespaced errnos to check for different error types.
 *
 * This file has no .c file it directly corresponds to.
 */


#ifndef ERROR_H
#define ERROR_H

enum {
    E_DISPLAY, //An unspecified display error occurred - display_errno
    E_FATAL, //Unspecified fatal error that should terminate the application safely fatal_errno
    E_OS //Errors returned by the OS or C standard library (Check errno)
};

extern int display_errno;

#endif //ERROR_H
