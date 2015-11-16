AC_REVISION($Revision$)

AC_INIT([libbuspirate], [0.4.0], [bruno.quoitin@umons.ac.be])
AC_CONFIG_SRCDIR(src/buspirate.c)
AC_PREREQ(2.57)
AC_PREFIX_DEFAULT(/usr/local)

AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE([no-define])
AM_CONFIG_HEADER(src/config.h)

AC_PROG_CC([gcc])
AC_PROG_INSTALL
AC_PROG_LN_S
AM_PROG_CC_C_O

AC_HEADER_STDC
AC_CHECK_HEADER(termios.h)

AC_C_CONST
AC_TYPE_SIZE_T

AC_PROG_LIBTOOL

CFLAGS="$CFLAGS -Wall -O2"

AC_ARG_WITH(werror,
[  --with-werror        pass -Werror flag to C compiler ],,
[test -z "$with_werror" && with_werror=no])
if test "$with_werror" != "no"; then
  CFLAGS="$CFLAGS -Werror"
fi
AC_SUBST(CFLAGS)

AC_OUTPUT(Makefile src/Makefile src/dev/Makefile)
