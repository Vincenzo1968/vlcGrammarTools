AC_PREREQ([2.69])
AC_INIT([vlcGrammarTools], [1.0.0], [vincenzo.locicero@example.com])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

: ${CFLAGS="-O3"}
AC_PROG_CC
#CFLAGS="$CFLAGS -std=c11"

AC_SEARCH_LIBS([pow], [m])

AC_CONFIG_FILES([
    Makefile
    src/Makefile
])
AC_OUTPUT
