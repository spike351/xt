/* gcc -g -O2 -rdynamic -W -Wall -std=c11 -pedantic -Wshadow -Wcast-qual -Wconversion -Wwrite-strings -fno-builtin -finstrument-functions -o test -ldl -lm test.c
 */


int main (int argc, char *argv[]);
int func1 (int x, int f);
int func2 (int a, int b);
int func3 (void);
