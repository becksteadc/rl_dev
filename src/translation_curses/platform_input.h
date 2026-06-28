/* Defines enum values useful for mapping platform-dependant code constants to
 * internal constants for each key type. Note that for the basic alphanumeric
 * keys, and even basic symbols, comparing to the char. value instead is allowed.
 */


#ifndef PLATFORM_INPUT_H
#define PLATFORM_INPUT_H

int platform_input_map_keystroke(int key);
#endif //PLATFORM_INPUT_H
