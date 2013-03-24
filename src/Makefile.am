EXTRA_DIST =
CLEANFILES = *~
ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory

AM_CPPFLAGS = \
	-include $(top_builddir)/config.h \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-DLIBEXECDIR=\""$(libexecdir)"\"

AM_CFLAGS = ${my_CFLAGS} \
	${MBTOOLS_CFLAGS} \
	-fvisibility=hidden \
	-ffunction-sections \
	-fdata-sections

AM_LDFLAGS = ${MBTOOLS_LIBS} \
	-Wl,--gc-sections \
	-Wl,--as-needed

bin_PROGRAMS = mbcollect mbrecorder

mbcollect_SOURCES = \
	chrono.c \
	daemon.c \
	option.c \
	keyfile.c \
	output.c \
	collect.c

mbrecorder_SOURCES = \
	recorder.c
