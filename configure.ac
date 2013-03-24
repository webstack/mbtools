#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.


AC_PREREQ([2.63])
AC_INIT(mbtools, 0.1, stephane.raimbault@webstack.fr)
AC_CONFIG_SRCDIR([src/collect.c])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign 1.11 -Wall -Wno-portability silent-rules tar-pax subdir-objects])
AC_PROG_CC_STDC
AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE
AC_CONFIG_MACRO_DIR([m4])
AM_SILENT_RULES([yes])

AC_CANONICAL_HOST

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdio.h stdlib.h string.h unistd.h])

MBTOOLS_REQUIRES="glib-2.0 >= 2.10.0 libmodbus >= 3.1.0"
PKG_CHECK_MODULES(MBTOOLS_DEPS, [$MBTOOLS_REQUIRES])
MBTOOLS_CFLAGS="-Wall -Werror $MBTOOLS_DEPS_CFLAGS"
MBTOOLS_LIBS="$MBTOOLS_DEPS_LIBS"
AC_SUBST(MBTOOLS_CFLAGS)
AC_SUBST(MBTOOLS_LIBS)

my_CFLAGS="-Wall \
-Wmissing-declarations -Wmissing-prototypes \
-Wnested-externs -Wpointer-arith \
-Wpointer-arith -Wsign-compare -Wchar-subscripts \
-Wstrict-prototypes -Wshadow \
-Wformat-security -Wtype-limits"
AC_SUBST([my_CFLAGS])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
        Makefile
        src/Makefile
])

AC_OUTPUT
AC_MSG_RESULT([
        $PACKAGE $VERSION
        ============

        prefix:                 ${prefix}
        sysconfdir:             ${sysconfdir}
        libdir:                 ${libdir}
        includedir:             ${includedir}

        compiler:               ${CC}
        cflags:                 ${CFLAGS}
        ldflags:                ${LDFLAGS}
])
