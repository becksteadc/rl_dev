/** This file is not part of the RL project.
 * It is instead a utility to be changed to generate text for
 * use in the source files of the project. (e.g., generating the KEY_ enum values
 */

#include <stdio.h>

int main()
{
    int i = 0;
    FILE *f = fopen("out.txt", "w");
    while (i < 26) {
        fprintf(f, "gKEY_");
        fprintf(f, "%c = '%c',\n", 'a' + i, 'a' + i);
        ++i;
    }
    return 0;
}
