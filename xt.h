/*
 * xt.h
 *  The Call Tree (Execution Trace) library.
 *  Copyright (C) 2020  Peter Harris   dilbert351@gmail.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *  v 0.1   4 sep 20
 */




#ifndef _XT_H_
#define _XT_H_

#ifdef _X_TRACE__                      /* Define in xt.c before including this file.      */


#define _POSIX_C_SOURCE 2      // For popen) and pclose()

#include <stdio.h>             // fprintf()  fopen() fclose() FILE stderr
#include <string.h>            // strlen() strcpy() strncpy() strcat()
#include <stdlib.h>            // calloc() realloc() free()
#define __USE_XOPEN
#include <time.h>              // clock() CLOCKS_PER_SEC

#define __USE_GNU
#include <dlfcn.h>             // dladdr()

#ifndef _WIN32
#include <sys/time.h>          // gettimeofday() struct timeval
#endif


//#define _X_TRACE__                   /* Define to send a call trace to the stderr path. */




//#ifdef _X_TRACE__

#define _                {xt_lineNo=cygln__;}
#define _XT              __attribute__ ((no_instrument_function))

#define XT_INDENT        4                    /* Indentation for each branch of the tree.*/

#define XT_COL_NORM      "\x1B[0m"
#define XT_COL_RED       "\x1B[31m"
#define XT_COL_GREEN     "\x1B[32m"
#define XT_COL_YELL      "\x1B[33m"
#define XT_COL_BLUE      "\x1B[34m"
#define XT_COL_MAG       "\x1B[35m"
#define XT_COL_CYAN      "\x1B[36m"
#define XT_COL_WHITE     "\x1B[37m"

#define XT_COL_GRAY      "\x1B[38;5;244"
#define XT_COL_BRIGHT    "\x1B[38;5;231"

#define XT_COL_BOLD_ON   "\x1B[1m"
#define XT_COL_BOLD_OFF  "\x1B[21m"
#define XT_COL_RESET     XT_COL_NORM

#define UNUSED(x)        (void)(x)
#define XT_OUT(...)      fprintf(stderr, __VA_ARGS__)





typedef enum                                  /* Offsets into unicode character arrays. */
{                                             /* Must be multiples of 3 as there are 3  */
    XT_TYP_LIGHT         = 0,                 /* bytes in UTF-8 for each unicode        */
    XT_TYP_HEAVY         = 3,                 /* character.                             */
    XT_TYP_DOUBLE        = 6
}
XTType;

typedef enum timer_
{
    XT_TIMER_DISABLED,                        /* Timing information not printed.        */
    XT_TIMER_CPU,                             /* Print amount of CPU processing time.   */
    XT_TIMER_ELAPSED                          /* Print clock time.                      */
}
XTTimer;

typedef struct xtbranch_
{
    unsigned     level;                       /* Level in tree.                         */
    unsigned     nameIndx;                    /* Index to name in sting array.          */
    int          lineNo;                      /* Line number of call if found.          */
    double       enterTime;                   /* Clock ticks when entering function.    */
    double       exitTime;                    /* Clock ticks when exiting function.     */
    unsigned     lastChild;                   /* Index of last child.                   */
    unsigned     parent;                      /* Index of parent object.                */
}
XTBranch;






/*                    - - - -  FUNCTION PROTOTYPES  - - - -                         */

void      __cyg_profile_func_enter (void *this_fn, void *call_site)  __attribute__ ((no_instrument_function));
void      __cyg_profile_func_exit  (void *this_fn, void *call_site)  __attribute__ ((no_instrument_function));

void      XT_Trace              (const char *func)                   __attribute__ ((no_instrument_function));
void      XT_Print              (void)                               __attribute__ ((no_instrument_function));
void      XT_AddBranch          (const char *p, unsigned level)      __attribute__ ((no_instrument_function));
unsigned  XT_AddFunctionName    (const char *p)                      __attribute__ ((no_instrument_function));
void      XT_LinkToParent       (XTBranch *pBranch)                  __attribute__ ((no_instrument_function));
void      XT_PrintInit          (void)                               __attribute__ ((no_instrument_function));
double    XT_GetTime            (void)                               __attribute__ ((no_instrument_function));
void      XT_PrintElapsedTime   (double start, double end)           __attribute__ ((no_instrument_function));
void      XT_Cleanup            (void)                               __attribute__ ((no_instrument_function));


#else

#define _            {(void)(cygln__);}

#endif  /* _X_TRACE__ */
#endif  /* _XT_H_ */
