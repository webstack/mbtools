EXTRA_DIST =
CLEANFILES = *~
ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory

AM_CFLAGS = ${my_CFLAGS} \
    ${MBTOOLS_CFLAGS} \
    -fvisibility=hidden \
    -ffunction-sections \
    -fdata-sections

AM_LDFLAGS = \
    -Wl,--gc-sections \
    -Wl,--as-needed

noinst_PROGRAMS = \
	unit-test-server \
	unit-test-client

unit_test_server_SOURCES = unit-test-server.c
unit_test_client_SOURCES = unit-test-client.c

unit_test_server_LDADD = ${MBTOOLS_LIBS} 
unit_test_client_LDADD = ${MBTOOLS_LIBS} 
