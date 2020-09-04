/* gcc -g -O2 -rdynamic -W -Wall -std=c11 -pedantic -Wshadow -Wcast-qual -Wconversion -Wwrite-strings -fno-builtin -finstrument-functions -o test -ldl -lm test.c
 */

#include <stdio.h>
#include "test.h"




int main (int argc, char *argv[])
{
    int  k, n, r;

    (void)(argv);

    k = (argc > 1) ? 1 : 3;
    n = 5;
    r = func1 (n, 0);
    r /= 2;
    n = func1 (r, k);
    r += 6;
    n = func1 (r, 2);

    return 0;
}



int func1 (int x, int f)
{
    int    y;
    int   *p = NULL;

    y = func2 (x, 2);
    y = func2 (y, 3);

    if (f != 1)    /* If f == 1, p remains a NULL so next */
        p = &y;    /* statement causes a seg fault.       */

    *p = 5;

    return 2 * y;
}
