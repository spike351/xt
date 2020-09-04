# I'm not good at make files, so this make file is based on the template from
# https://www.cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles.html
#
# The makefile should emulate the following commands:
#
#  To use the Execution Trace (XT) library with a project, simply define the
#  USE_XT variable on the command line as follows when building the project
#:
#     $ make USE_XT=1
#
#     Resulting commands
# gcc -g -O2 -W -Wall -std=c11 -pedantic -Wshadow -Wcast-qual -Wconversion -Wwrite-strings -fno-builtin -finstrument-functions -c test.c
# gcc -g -O2 -W -Wall -std=c11 -pedantic -Wshadow -Wcast-qual -Wconversion -Wwrite-strings -fno-builtin -finstrument-functions -c xt.c
# gcc -rdynamic test.o xt.o -ldl -o test
#
# To compile normally (without XT)
#
#     $ make
#
#     Resulting commands
# gcc -g -O2 -W -Wall -std=c11 -pedantic -Wshadow -Wcast-qual -Wconversion -Wwrite-strings -fno-builtin  -c test.c
# gcc  test.o -o test
#
# Note that no changes are needed to the project files.  The only difference is
# that the compiler will generate the hook functions, and the xt.c file will be
# compiled and linked with the projects executable file.
#
# Hint:  To see what commands will be executed without actually running them
#        use the "-n" option with make.




#-----------------------------------------------------------------------------
#    - - -  Compiler Control Section.  - - -
#
# NOTE about CFLAGS:
#      ALWAYS turn on every error and warning message a compiler can produce.
#      There is no such thing as a warning.  If you can explain a warning, then
#      you can also write code to avoid it.  If you can't explain it, it is an
#      error and needs to be fixed.
#

CC = gcc
RM = rm -f
CFLAGS = -g -O2 -W -Wall -std=c11 -pedantic -Wshadow -Wcast-qual -Wconversion -Wwrite-strings -fno-builtin
INCLUDES =
LFLAGS =
LIBS = -lm


#-----------------------------------------------------------------------------
#    - - -  Your Project Details Section.  - - -
#
# This section is where the files associated with a project are listed.  There
# should be no need to change most of the remainder of the make file.
#

MAIN = test              # Define the name of your executable.
SRCS = test.c test2.c    # List all the source files in the project.
HDRS = test.h            # Derived list of all the header files in the project.


DEFS =                   # Any compiler macros (-D options).

DEPS = ${HDRS}
OBJS = ${SRCS:.c=.o}



#-----------------------------------------------------------------------------
#    - - -  Execution Trace Control Section.  - - -
#
# The following section should not need to be changed.  Variables defined here
# are included automatically when the makefile is called with the USE_XT macro
# as described above.  This will result in the definitions necessary to compile
# the Execution Trace library and link them into the project. 
#
ifdef USE_XT

  CFLAGS += -finstrument-functions     # Generate the instrument function hooks
  LFLAGS += -rdynamic                  # Tell linker to add symbols for dlopen()
  XTSRC = xt.c                         # Execution Trace source file
  DEPS += ${XTSRC:.c=.h}               # Execution Trace header
  XTOBJ = ${XTSRC:.c=.o}               # Execution Trace object file
  LIBS := -ldl ${LIBS}                 # Library required for backtrace.

  # The following four macros are provided as a convenient means of defining
  # the operating mode of the trace library by simply passing arguments on the
  # make command line rather than needing to edit the source files directly.
  # To use this feature, simply include one or more of the macros REAL_TIME,
  # TRACE_LINES, SHOW_TREE, ADD_GAPS or TIMER to make as follows:
  #
  #  $ make USE_XT=1 REAL_TIME=1
  #
  # NOTE:  It is necessary to specify a value (1 here) so that make actually
  # recognises and defines the macro.  The only exception is the TIMER option
  # which must be set to 1 for CPU timing and 2 for elapsed (clock) time.
  #
  ifdef REAL_TIME                          # Enable realtime mode.
    DEFS := ${DEFS} -D XT_X_REAL_TIME
  endif
  ifdef TRACE_LINES                        # Draw trace lines in realtime mode
    DEFS := ${DEFS} -D XT_X_TRACE_LINES
  endif
  ifdef SHOW_TREE                          # Show call tree in non realtime
    DEFS := ${DEFS} -D XT_X_SHOW_TREE
  endif
  ifdef ADD_GAPS                           # Add gap between function blocks
    DEFS := ${DEFS} -D XT_X_ADD_GAPS
  endif
  ifdef TIMER                              # Show timer
    DEFS := ${DEFS} -D XT_X_TIMER=${TIMER}
  endif

endif





#-----------------------------------------------------------------------------
# Rules:-  Should not need to be changed.
#

all:	${MAIN}
	@ echo Build complete: ${MAIN}

${MAIN}: ${OBJS} ${XTOBJ}
	${CC} $(LFLAGS) $(INCLUDES) $(OBJS) ${XTOBJ} $(LIBS) -o $(MAIN)

${OBJS}: ${DEPS}
	$(CC) $(CFLAGS) $(INCLUDES) -c ${@:.o=.c}  -o $@

${XTOBJ}: xt.h
	${CC} $(CFLAGS) $(INCLUDES) ${DEFS} -c ${XTSRC} -o $@

clean:
	${RM} *.o ${MAIN}

depend: ${SRCS}
	makedepend ${INCLUDES} $^


# DO NOT DELETE THIS LINE -- make depend needs it

