/* gcc -g -O2 -rdynamic -W -Wall -std=c11 -pedantic -Wshadow -Wcast-qual -Wconversion -Wwrite-strings -fno-builtin -finstrument-functions -o test -ldl -lm test.c
 */

#include <stdio.h>
#include <unistd.h>
#include "test.h"



int func2 (int a, int b)
{
    func3 ();
    return (a + b);
}



int func3 (void)
{
    sleep (1);
    return (2);
}
