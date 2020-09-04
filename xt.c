/*
 * xt.h:
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


#define _X_TRACE__
#include "xt.h"


#ifdef _X_TRACE__


/*                  - - - -  MAKE CONTROL PARAMETERS  - - - -                           */

/* This section defines several macros that allows the user to set global
 * variables directly from the command line in the make file or just the
 * C compiler.  This eliminates the necessity to edit this xt.c file
 * directly each time these variables need to be changed.  By adding the
 * variable such as "-D XT_X_REAL_TIME" to the compiler, the default state
 * of these flags can be reversed.
 */

#ifdef XT_X_REAL_TIME
#   define XT_X_RT         1                /* Switch realtime ON       */
#else
#   define XT_X_RT         0                /* Realtime - OFF           */
#endif

#ifdef XT_X_TRACE_LINES
#   define XT_X_TL         1                /* Switch trace lines ON    */
#else
#   define XT_X_TL         0                /* Tracelines - OFF         */
#endif

#ifdef XT_X_SHOW_TREE
#   define XT_X_ST         1                /* Switch tree ON           */
#else
#   define XT_X_ST         0                /* Show tree - OFF          */
#endif

#ifdef XT_X_ADD_GAPS
#   define XT_X_AG         1                /* Switch block gaps ON     */
#else
#   define XT_X_AG         0                /* Add gaps - OFF           */
#endif

#ifndef XT_X_TIMER
#   define XT_X_T         XT_TIMER_DISABLED
#else
#   if XT_X_TIMER == 1
#      define XT_X_T      XT_TIMER_CPU
#   elif XT_X_TIMER == 2
#      define XT_X_T      XT_TIMER_ELAPSED
#   endif
#endif



/*               - - - -  USER DEFINED GLOBAL VARIABLES  - - - -                        */

/* Control variables set by user.  Set values as required before compiling.
 */

/* Set this value to 1 to enable funtion call tracing and 0 to disable it.
 *  This is done before the program is compiled; it cannot be done reliably
 * under program control if since it risks producing ameaningless trace.
 */
    static int       xt_enabled       = 1;

/* Setting this value to 1 will print the call stack trace in real time as the
 * program runs.  This mode is useful when the program terminates prematurely
 * (e.g. as a result of a segmentation fault) allowing the function where the
 * program aborted to be identified.
 * Set the value to 0 to create a pretty print of the stack trace.  This can
 * only be done once the call trace has been completed since the entire
 * structure needs to be available to determine how the tree should appear.
 * Because of this, the non real time call stack is only produced immediately
 * before the main() function exits.
 */
    static int       xt_realTime      = XT_X_RT;

/* This option is available with the real time trace mode only.  If set to 0,
 * only function calls are printed.  Setting this to 1 will also draw the
 * execution trace lines associated with the program execution.  This includes
 * both calls to functions as well as function returns.  This is useful for
 * more precisely locating the exact location of unexpected program
 * terminations.
 */
    static int       xt_traceLines    = XT_X_TL;

/* This value is only meaningful in non real time mode and should be set to 1
 * to display the call tree lines, and 0 to hide them.
 */
    int              xt_showTree      = XT_X_ST;

/* This variable is also used in non real time mode only.  By settting this
 * value to 1 a blank line will be generated whenever a "block" of functions
 * exit.  This is defined as two or more function returns without an
 * intervening function call.  This may make the tree more appealing to view.
 */
    int              xt_addGaps       = XT_X_AG;

/*  By setting this variable to 1, the amount of CPU time used by each function
 * will be reported at the end of the function name.  This works both in real
 * time and non real time modes.  In non rela time mode, the time is the total
 * time spent inside a function (including the time taken by each sub function
 * it calls).  In real time mode the time reported is the total program
 * execution time from the start of the program up to when the function is
 * called.  To disable time reporting, set the value to 0.
 */
    XTTimer          xt_timer         = XT_X_T;

/* This variable defines the type of lines used to draw the non real time view.
 * The current options are for light lines (normal), heavy lines, or double
 * lines.  The types are defined in the XTType enumeration.
 */
    XTType           xt_treeType      = XT_TYP_LIGHT;

/* This variable defines the ANSI codes that will be used to specify the tree
 * line color.  Standard color definition strings are provided above, however,
 * any ANSI string can be used.  This only applies on non real time mode.
 */
    const char      *xt_pTreeCol      = XT_COL_WHITE;

/*This variable defines the ANSI codes that will be used to specify the color
 * to be used to print the function names.  As before, any of the standard ANSI
 * color control strings may be specified.
 */
    const char      *xt_pNameCol      = XT_COL_BOLD_ON;

/* This is either a NULL or a character string specifying the output file name
 * where execution trace output is sent.
 */
    const char      *xt_pOutputFile   = NULL;





/*                   - - - -  PRIVATE GLOBAL VARIABLES  - - - -                         */

/* The following global variables are used internally by the CT package and
 * should not changed.  While stack call tracing relies on the instrument
 * functions provided by GCC, this does not include an init function, so some
 * of these globals are initialised to zero or null here.  Conceivably an init
 * function could be provided by __cyg_profile_func_enter() when the main()
 * function is being processed, but it is also just as easy to perform the
 * initialisation here.
 */
    unsigned         xt_level         = 0;    /* Trace stack level count.               */
    unsigned         xt_prevLvl       = 0;    /* Previous stack level.                  */
    unsigned         xt_maxLvl        = 0;    /* Maximum stack level.                   */
    int              xt_lineNo;               /* Stores current line number (_ macro).  */
    double           xt_realTimeStart;        /* The start time of real time tracing.   */
    char             xt_teeHoriz[65];         /* UTF-8 char buffer for 't' + hor line.  */
    char             xt_vlinSpace[65];        /* UTF-8 char buffer for 'v line' + spaces*/
    char             xt_LHoriz[65];           /* UTF-8 char buffer for 'L' + hor line.  */
    char             xt_SHoriz[65];           /* UTF-8 char buffer for 'S' line.        */
    char             xt_space[65];            /* Just blank spaces.                     */

/* Each function call (tree node) is defined by a XTBranch structure.  A new
 * node is created each time the __cyg_profile_func_enter() function is called.
 * Nodes are stored as a single large dynamically allocated array.  The array
 * will automatically expand as required.  The pointer xt_pTree points to the
 * first entry of the array, while xt_treeSize represents the total number of
 * nodes currently available.  The variable xt_nextBranch is the index of the
 * next available entry in the array.
 */
    XTBranch        *xt_pTree         = NULL;   /* Array of XTBranch type objects.      */
    unsigned         xt_treeSize;               /* Number of objects in array.          */
    unsigned         xt_nextBranch;             /* Index of next abailable array item.  */

    unsigned         xt_exitNodeIndex;          /* Node where exit timer is to be set.  */

/* Since function names vary greatly in length, these are stored separately.
 * The pointer xt_funcNames points to a large character array.  Like the tree
 * array, this array isdynamically allocated and resizable.  Function names are
 * stored one after the other as null terminated strings.  Each name is
 * referenced by the associated tree node entry as the index to the first
 * character of the name from the start of the string buffer (pointers cannot
 * be used as the array may move in memory when the buffer expands).  Again,
 * xt_nameBuffSize specifies the current size of the buffer and xt_nextAvail is
 * the index to the next free character following the last entry.
 */
    char            *xt_funcNames     = NULL;   /* Buffer of null terminated names.     */
    unsigned         xt_nameBuffSize;           /* Number of characters in buffer.      */
    unsigned         xt_nextAvail;              /* Index of next abailable entry.       */

/* This is just the pointer to the file stream used by the trace facility.  It
 * is initialised by the first call to __cyg_profile_func_enter() and is either
 * a pointer to a file stream or the standard error path.  Must initially be
 * set to NULL.
 */
    FILE            *xt_fp            = NULL;





/*                         - - - -  FUNCTIONS  - - - -                              */

/*-----------------------------------------------------------------------------
 * This function is the hook inserted at the start of every C function by the
 * GCC compiler when the -finstrument_function command line option is used.
 * Note:  To avoid inserting the hook at the start of this function (or any
 * function used to implement the call trace code), it needs to be prefixed
 * by the __attribute__ ((no_instrument_function)) line.  This tells the
 * compiler not to insert the hook into these functions.  If this was not
 * done, the hook would be recursively called until the stack exploded and
 * a segmentation fault was generated.
 */

__attribute__ ((no_instrument_function))
void __cyg_profile_func_enter (void *this_fn, void *call_site)
{
    Dl_info      info;                  /* Used to get function names.      */
    char         thisName[65];

    /* Tell the compiler not to worry about this unused argument.
     */
    UNUSED (call_site);

    if (xt_enabled == 1)
    {
        /* This section is called only once.  It is used to initialise the
         * output path.  If a file name has been specified, it is opened.
         * If no file name was specified, the standard error path is used.
         * If initialisation failed, an error is generated and execution
         * tracing is disabled.
         */
        if (xt_fp == NULL)                 /* If NULL, not initialised.     */
        {
            if (xt_pOutputFile != NULL)    /* File name specified.          */
            {
                if ((xt_fp = fopen (xt_pOutputFile, "w")) != NULL)
                {
                    xt_enabled = 0;
                    fprintf (stderr, "Could not open trace path.  Tracing disabled!\n");
                    return;
                }
            }
            else
                xt_fp = stderr;         /* No file name, so use std error.  */
        }

        /* Retreive information about the symbol for this_fn addr.  Addresses
         * may be particularly useful for debugging with GDB, but they are
         * very uninformative to read.  what we want is the actual function
         * name.
         */
        if (dladdr (this_fn, & info) != 0)
        {
            strncpy (thisName, (info.dli_sname != NULL) ? info.dli_sname : "???", 64);
            XT_Trace (thisName);
        }
    }
}



/*-----------------------------------------------------------------------------
 * This function is the hook inserted immediately before the return of every
 * C function by the GCC compiler when the -finstrument_function command
 * line option is used.
 */

__attribute__ ((no_instrument_function))
void __cyg_profile_func_exit  (void *this_fn, void *call_site)
{
    unsigned   i;
    char      *pOut, lineBuff[512];
    XTBranch  *pBranch;

    /* Tell the compiler not to worry about these unused arguments.
     */
    UNUSED (this_fn);
    UNUSED (call_site);

    if (xt_enabled == 1)
    {
        xt_prevLvl = xt_level;
        xt_level--;

        if (xt_realTime == 1)
        {
            if (xt_traceLines == 1)
            {
                pOut = lineBuff;
                if (xt_prevLvl > 1)
                {
                    for (i = XT_INDENT * (xt_level - 1); i > 0;  i--)
                        *pOut++ = ' ';
                }
                if (xt_prevLvl > 0)            /* Don't output for main()   */
                {
                    strcpy (pOut, xt_SHoriz);
                    XT_OUT ("%s\n", lineBuff);
                }
            }
            else
                if (xt_level > 0 && (xt_maxLvl - xt_level) >= 2)
                {
                    XT_OUT ("\n");
                    xt_maxLvl = xt_level;
                }
//                XT_OUT ("\n");
        }
        else
        {
           /* First store the current clock ticks as the exit time for this
            * node, then set the next exit node as this nodes parent.
            */
            pBranch = & xt_pTree [xt_exitNodeIndex];
            pBranch->exitTime = XT_GetTime ();
            xt_exitNodeIndex = pBranch->parent;

            if (xt_lineNo != 0)
                pBranch->lineNo = xt_lineNo;

            /* Only need to tidy up if we are not in real time mode.
             */
            if (xt_level == 0)
            {
                /* Exiting at level 0 means we are exiting main(), so generate
                 * output and clean up after ourselves.
                 */
                XT_Print ();
                XT_Cleanup ();
            }
        }
        xt_lineNo = 0;                         /* Reset for next function.  */

        if (xt_level == 0 && xt_pOutputFile != NULL)
            fclose (xt_fp);
    }
}



/*-----------------------------------------------------------------------------
 * This function is called at the beginning of each function in the program and
 * it operates in one of two ways.  In normal mode, function calls are stored
 * in memory and when the main() function is about to exit.  In this way, the
 * call tree can be printed as a pretty structure.
 * In real time mode, the indented function name is printed immediately.  This
 * is particularly useful if the program terminates unexpectedly (e.g. such as
 * when a segmentation fault occurs).  Under these circumstances, it is not
 * possible to get the pretty print version of the call tree that happens at
 * the end of the program.
 */

__attribute__ ((no_instrument_function))
void XT_Trace (const char *func)
{
    unsigned  i;
    char     *pOut;
    char      lineBuff [512];                  /* Print line buffer.         */

    if (xt_realTime == 1)                      /*   --- REAL TIME MODE ---   */
    {
        /* If asked to print times in real time mode, get the start time of
         * the program if we are just entered the main() function.
         */
        if (xt_timer != XT_TIMER_DISABLED && xt_level == 0)
            xt_realTimeStart = XT_GetTime ();

        /* In real time mode, just output the results immediately.  This is
         * useful in situations such as segmentation faults where the normal
         * mode would crash the entire program before getting a chance to
         * output the call trace tree.  In real time mode, the output is
         * generated on the run and is available when the seg fault occurs.
         */
        XT_PrintInit ();

        lineBuff[0] = '\0';
        pOut = lineBuff;
        if (xt_traceLines == 1)
        {
            if (xt_level >= 1)
            {
                for (i = XT_INDENT * (xt_level - 1); i > 0;  i--)
                    *pOut++ = ' ';
                strcpy (pOut, xt_LHoriz);
            }
        }
        else
        {
            for (i = XT_INDENT * xt_level; i > 0;  i--)
                *pOut++ = ' ';
            *pOut = '\0';
        }
        XT_OUT ("%s%s", lineBuff, func);

        if (xt_timer != XT_TIMER_DISABLED)
            XT_PrintElapsedTime (xt_realTimeStart, XT_GetTime ());

        XT_OUT ("\n");
    }
    else                                       /*    --- NORMAL MODE ---     */
    {
        /* The normal mode.  In this mode we want to generate a pretty tree
         * structure, but we can't know how to draw some nodes before exiting
         * a function (which may not happen for some lines later).  To overcome
         * this issue, all the call tree is stored and printed out at the end
         * of the program.  The problem with this is that problems such as a
         * segmentation fault which cause the program to prematurely finish
         * prevent the call tree from being generated.
         */
        XT_AddBranch (func, xt_level);
    }

    if (xt_level++ > 1)                         /* Incr stack level.          */
        xt_prevLvl++;                           /* Incr ptrev stack level.    */
    if (xt_level > xt_maxLvl)
        xt_maxLvl = xt_level;
}



/*-----------------------------------------------------------------------------
 * This function performs all the pretty printing for the function call tree in
 * non-realtime mode.  In this mode, the data is stored in a tree structure
 * xt_pTree[] of size xt_treeSize.
 * In realtime mode, printing is done immediately in the XT_Trace() function.
 */

__attribute__ ((no_instrument_function))
void XT_Print (void)
{
    int        endOfBlock;
    unsigned   n, idx, idxp, index, level, parentIdx, node [64];
    char      *p1, *p2;
    char       lineBuff1 [128];          /* Print line buffer.                */
    char       lineBuff2 [128];
    char       lineNoBuff [16];
    XTBranch  *pBranch;

    XT_PrintInit ();                     /* Initialise elements for printing  */

    for (index = 0;  index < xt_nextBranch;  index++)
    {
        p1 = lineBuff1;
        *p1 = '\0';
        p2 = lineBuff2;
        *p2 = '\0';
        strcat (p1, xt_pTreeCol);         /* Set color of tree structure.      */
        pBranch = & xt_pTree [index];
        level = pBranch->level;

        node [level] = index;
        endOfBlock = 0;

        /* First build the remainder of the family tree for this node by
         * storing the parent and parents parent... etc in the node[] array.
         * The first entry (node[0)) should always be main(), while the last
         * entry is the function we are about to print.  The node[] array
         * therefore represents the family tree for this function.
         */
        for (n = level;  n != 0;  n--)
        {
            parentIdx = pBranch->parent;
            node [n - 1] = parentIdx;
            pBranch = & xt_pTree [parentIdx];
        }

        /* Scan the family tree creating the appropriate pretty printing strings.
         */
        for (n = 1;  n <= level;  n++)
        {
            idx = node [n];                 /* Get branch index of this node  */
            idxp = xt_pTree [idx].parent;   /* Get index of parent.           */
            pBranch = & xt_pTree [idxp];

            if (pBranch->lastChild > index)
            {
                if (n == level)
                    strcat (p2, xt_teeHoriz);
                else
                    strcat (p1, xt_vlinSpace);
            }
            else if (pBranch->lastChild < index)
               strcat (p1, xt_space);
            else                            /*  pBranch->lastChild == index   */
            {
               strcat (p2, xt_LHoriz);
               endOfBlock = (xt_pTree [idx].lastChild == 0) ? 1 : 0;
            }
        }

        n = node [level];                   /* Index of current node.         */
        n = xt_pTree [n].nameIndx;          /* Index of name of this node.    */

        /* If, by some miracle, line number information has been provided for
         * this function, we print it in the small temp buffer for that,
         * otherwise, it will just be an empty buffer.
         */
        lineNoBuff [0] = '\0';              /* better safe than sorry!!!      */
        if (xt_pTree [n].lineNo != 0)
            sprintf (lineNoBuff, "[%d] ", xt_pTree [n].lineNo);

        strcat (p2, xt_pNameCol);           /* Set color of names.            */
        XT_OUT ("%s%s%s%s" XT_COL_RESET, lineBuff1, lineBuff2, lineNoBuff, & xt_funcNames [n]);

        /* Now if we are recording execution times, calculate the total time
         * spent in this function (as well as all it's child functions).
         * NOTE:  The execution time for this function alone is the time
         * calculated here minus the execution times of all it's children.
         */
        if (xt_timer != XT_TIMER_DISABLED)
            XT_PrintElapsedTime (pBranch->enterTime, pBranch->exitTime);

        XT_OUT ("\n");

        /* Print gaps at the end of blocks if requested.
         */
        if (xt_addGaps == 1 && endOfBlock == 1)
            XT_OUT ("%s\n", lineBuff1);
    }
}



/*-----------------------------------------------------------------------------
 * This function is called to add the current function as the next entry on the
 * function call trace tree.  The trace tree is stored as an array of XTBranch
 * items.  As more function calls are added, the memory requirements are
 * automatically expaned to account for the additional requirements.  Each
 * entry stores the index to the function name stored in the xt_funcNames
 * character array, the level of the function in the tree and, line number that
 * the function was called from (usually 0 as this info is difficult to get).
 */

__attribute__ ((no_instrument_function))
void XT_AddBranch (const char *p, unsigned level)
{
    XTBranch  *pBranch;

    if (xt_pTree == NULL)            /* If NULL, no mem allocated yet.      */
    {
        xt_treeSize = 1000;          /* Initial buffer size.                */
        xt_pTree = (XTBranch *) calloc ((size_t) xt_treeSize, sizeof (XTBranch));
        xt_nextBranch = 0;           /* Init index to next avail branch.    */
    }
    else if (xt_treeSize <= xt_nextBranch)               /* Enough room for */
    {                                                    /* next symbol?    */
        xt_treeSize += 500;          /* Expanded buffer size.               */
        xt_pTree = (XTBranch *) realloc (xt_pTree, (size_t) xt_treeSize * sizeof (XTBranch));
    }

    if (xt_pTree != NULL)
    {
        xt_exitNodeIndex = xt_nextBranch;
        pBranch = & xt_pTree [xt_nextBranch];     /* Get ptr to next branch */

        pBranch->nameIndx = XT_AddFunctionName (p) - 1; /* Add name and get index */
        pBranch->level = level;           /* Store level of function call.  */
        pBranch->lineNo = xt_lineNo;      /* Save line No. if available.    */
        pBranch->enterTime = XT_GetTime ();   /* Store the current time.    */
        pBranch->lastChild = 0;
        XT_LinkToParent (pBranch);

        xt_nextBranch++;             /* Update index to next entry in array.*/
        xt_lineNo = 0;
    }
}



/*-----------------------------------------------------------------------------
 * null terminated function names are stored in along character buffer one
 * after the other.  As memory requirements grow, the buffer is resized
 * automatically.  Since resizing does not guarantee the buffer will not be
 * moved, rather than storing pointers to the required string, an index is used
 * giving the offset to the function name from the start of the buffer.
 * NOTE the returned value is 1 more than the index, so needs to be decremented
 * to use it.  If an error occurs, zero is returned.
 */

__attribute__ ((no_instrument_function))
unsigned XT_AddFunctionName (const char *p)
{
    unsigned  len, rtn;

    len = strlen (p) + 1;            /* Symbol name length including null.  */

    if (xt_funcNames == NULL)        /* If NULL, no mem allocated yet.      */
    {
        xt_nameBuffSize = 10000;     /* Initial buffer size.                */
        xt_funcNames = (char *) calloc (xt_nameBuffSize, sizeof (char));
        xt_nextAvail = 0;            /* Init index to next avail slot.      */
    }
    else if (xt_nameBuffSize - xt_nextAvail <= len)      /* Enough room for */
    {                                                    /* next symbol?    */
        xt_nameBuffSize += 5000;    /* Expanded buffer size.                */
        xt_funcNames = (char *) realloc (xt_funcNames, xt_nameBuffSize);
    }

    if (xt_funcNames != NULL)
    {
        strncpy (& xt_funcNames [xt_nextAvail], p, len);  /* Copy to buffer */
        rtn = xt_nextAvail + 1;     /* Save the index to symbol just saved. */
        xt_nextAvail += len;        /* Update next avail slot index.        */
        return (rtn);               /* Return index to current symbol.      */
    }
    return (0);                     /* Error.  Symbol not saved.            */
}



/*-----------------------------------------------------------------------------
 * Find the parent for this object by searching backwards through the tree from
 * this branch until we find the first function at the next lowest level (closer
 * to main() ).  That function must be our parent.  The parent index is stored
 * for this node and the parent's last child is updated to reflect the latest
 * node.
 */

__attribute__ ((no_instrument_function))
void XT_LinkToParent (XTBranch *pBranch)
{
    unsigned  n;

    if (xt_nextBranch > 0)
    {
        for (n = xt_nextBranch;  n != 0;  n--)    /* Scan all previous branches */
        {
            if (xt_pTree [n - 1].level < pBranch->level)
            {
                /* Found the last node added at the next highest level, so
                 * firstly we store it as the parent of this node, then
                 * tell it that this node is it's last child.
                 */
                pBranch->parent = n - 1;
                xt_pTree [n - 1].lastChild = xt_nextBranch;
                break;
            }
        }
    }
    else
         pBranch->parent = 0;    /* This should be the root i.e. main().*/
}



/*-----------------------------------------------------------------------------
 * When pretty printing, lines are drawn using box characters. These characters
 * are defined in the unicode character set \u2500 to \u257f.  The characters
 * used are the vertical line (\u2502 or │), horizontal line (\u2500 or ─),
 * right-tee piece (\u251c or ├) and 'L' shaped piece (\u2514 or └). (For more
 * information see the web site https://jrgraphix.net/r/Unicode/2500-257F).
 * While these characters can be represented using the wchar_t variable type,
 * however, this is not portable and introduces a number of other issues (e.g.
 * functions like strlen() )..  The preferred method is to use UTF-8 coding
 * which encodes 16 bit unicode characters into a 3 byte codes as follows:
 * 1110-xxxx 10xx xxxx 10xx-xxxx.
 */

__attribute__ ((no_instrument_function))
void XT_PrintInit (void)
{
    int          i;
    static int   initialised = 0;
    char        *p;
    static char  h[] = {0xe2, 0x94, 0x80,      /* \u2500 -  ─  Horiz light   */
                        0xe2, 0x94, 0x81,      /* \u2501             heavy   */
                        0xe2, 0x95, 0x90};     /* \u2550             double  */
    static char  v[] = {0xe2, 0x94, 0x82,      /* \u2502 -  │  Vert  light   */
                        0xe2, 0x94, 0x83,      /* \u2503             heavy   */
                        0xe2, 0x95, 0x91};     /* \u2551             double  */
    static char  t[] = {0xe2, 0x94, 0x9c,      /* \u251c -  ├  tee   light   */
                        0xe2, 0x94, 0xa0,      /* \u2520             heavy   */
                        0xe2, 0x95, 0xa0};     /* \u2560             double  */
    static char  l[] = {0xe2, 0x94, 0x94,      /* \u2514 -  └  L     light   */
                        0xe2, 0x94, 0x97,      /* \u2517             heavy   */
                        0xe2, 0x95, 0x9a};     /* \u255a             double  */
    static char  j[] = {0xe2, 0x94, 0x98,      /* \u2518 -  ┘  J     light   */
                        0xe2, 0x94, 0x9b,      /* \u251b             heavy   */
                        0xe2, 0x95, 0x9d};     /* \u255d             double  */
    static char  b[] = {0xe2, 0x94, 0x8c,      /* \u250c -  ┌  J     light   */
                        0xe2, 0x94, 0x8f,      /* \u250f             heavy   */
                        0xe2, 0x95, 0x94};     /* \u2554             double  */
    XTType       g;

    if (initialised == 0)
    {
        g = xt_treeType;

        p = xt_space;                             /* Build buffer with spaces only*/
        for (i = 0; i < XT_INDENT + 1; i++)
            *p++ = ' ';
        *p = '\0';

        if (xt_showTree == 1 || xt_traceLines == 1)
        {
            p = xt_vlinSpace;                     /* Build vertical line + spaces */
            *p++ = v[g+0]; *p++ = v[g+1];*p++ = v[g+2];        /*  "│    "        */
            for (i = 0; i < XT_INDENT; i++)
                *p++ = ' ';
            *p = '\0';

            p = xt_teeHoriz;                      /* Build 't' piece + horiz lines */
            *p++ = t[g+0];  *p++ = t[g+1];  *p++ = t[g+2];     /*  "├─── "         */
            for (i = 0; i < XT_INDENT - 1; i++)
            {
                *p++ = h[g+0];
                *p++ = h[g+1];
                *p++ = h[g+2];
            }
            *p++ = ' ';
            *p = '\0';

            p = xt_LHoriz;                        /* Build 'L' piece + horiz lines */
            *p++ = l[g+0];  *p++ = l[g+1];  *p++ = l[g+2];     /*  "└─── "         */
            for (i = 0; i < XT_INDENT - 1; i++)
            {
                *p++ = h[g+0];
                *p++ = h[g+1];
                *p++ = h[g+2];
            }
            *p++ = ' ';
            *p = '\0';

            p = xt_SHoriz;                        /* Build sideways 'S' piece       */
            *p++ = b[g+0];  *p++ = b[g+1];  *p++ = b[g+2];     /*  "┌───┘ "         */
            for (i = 0; i < XT_INDENT - 1; i++)
            {
                *p++ = h[g+0];
                *p++ = h[g+1];
                *p++ = h[g+2];
            }
            *p++ = j[g+0];  *p++ = j[g+1];  *p++ = j[g+2];
            *p = '\0';
        }
        else
        {
            memset (xt_vlinSpace, ' ', XT_INDENT + 1);   xt_vlinSpace [XT_INDENT + 1] = '\0';
            memset (xt_teeHoriz,  ' ', XT_INDENT + 1);   xt_teeHoriz [XT_INDENT + 1]  = '\0';
            memset (xt_LHoriz,    ' ', XT_INDENT + 1);   xt_LHoriz [XT_INDENT + 1]    = '\0';
            memset (xt_SHoriz,    ' ', XT_INDENT + 1);   xt_SHoriz [XT_INDENT + 1]    = '\0';
        }

        initialised = 1;
    }
}



/*-----------------------------------------------------------------------------
 * This function retreives the current "time" based on the type specified by
 * the xt_timer variable.
 * NOTE:  While the clock_gettime() function can theoretically be used to measure
 * times down to the nS, it is only available on a handful of *NIX systems.
 * Also, Windows may or may not support more accurate timing functions than the
 * standard time() function, but I don't support it -- I don't do Windows!
 */

__attribute__ ((no_instrument_function))
double XT_GetTime (void)
{
    double    secs = 0.0;

    switch (xt_timer)
    {
        case XT_TIMER_CPU:
            secs = (double) clock () / (double) CLOCKS_PER_SEC;
            break;

        case XT_TIMER_ELAPSED:
            {
#ifdef _WIN32                /* Windows: - gettimeofday() not supported. */
                time_t    t;

                t = time (NULL);
                secs = (double) t;
#else                        /* Linux/MacOS                              */
                struct timeval   t;

                gettimeofday (& t, NULL);
                secs = t.tv_sec;
                secs += (double) t.tv_usec / 1000000.0;
#endif
            }
            break;

        case XT_TIMER_DISABLED:
        default:
            break;
    }

    return (secs);
}



/*-----------------------------------------------------------------------------
 * This simple function outputs the elapsed time based on start and end values
 * assumed to be in seconds.
 */

__attribute__ ((no_instrument_function))
void XT_PrintElapsedTime (double start, double end)
{
    double    elapsedTime;

    elapsedTime = end - start;

    if (elapsedTime > 1.0)
        XT_OUT ("  [%.2f S]", elapsedTime);
    else if (elapsedTime > 0.001)
        XT_OUT ("  [%.2f mS]", elapsedTime * 1000.0);
    else if (elapsedTime > 0.000001)
        XT_OUT ("  [%.0f uS]", elapsedTime * 1000000.0);
    else
        XT_OUT ("  [%.0f nS]", elapsedTime * 1000000000.0);
}



/*-----------------------------------------------------------------------------
 * Release the memory associated with the function name strings as well as the memory
 * used to store each node of the call tree.
 */

__attribute__ ((no_instrument_function))
void XT_Cleanup (void)
{
    if (xt_funcNames != NULL)
    {
        free (xt_funcNames);
        xt_funcNames = NULL;
    }

    if (xt_pTree != NULL)
    {
        free (xt_pTree);
        xt_pTree = NULL;
    }
}

#endif  /* _X_TRACE__ */
