EXTRA_DIST =
CLEANFILES =
ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory

AM_CPPFLAGS = \
	-include $(top_builddir)/config.h \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-DLIBEXECDIR=\""$(libexecdir)"\" \
	-I${top_srcdir}/src/small \
	-I${top_srcdir}/src

AM_CFLAGS = ${my_CFLAGS} \
	-fvisibility=hidden \
	-ffunction-sections \
	-fdata-sections

AM_LDFLAGS = \
	-Wl,--gc-sections \
	-Wl,--as-needed

SED_PROCESS = \
	$(AM_V_GEN)$(MKDIR_P) $(dir $@) && $(SED) \
	-e 's,@VERSION\@,$(VERSION),g' \
	-e 's,@prefix\@,$(prefix),g' \
	-e 's,@exec_prefix\@,$(exec_prefix),g' \
	-e 's,@libdir\@,$(libdir),g' \
	-e 's,@includedir\@,$(includedir),g' \
	< $< > $@ || rm $@

%.pc: %.pc.in Makefile
	$(SED_PROCESS)

LIBSMALL_CURRENT=1
LIBSMALL_REVISION=0
LIBSMALL_AGE=0

pkginclude_HEADERS =\
	src/small/argparser.h \
	src/small/dict.h \
	src/small/latch.h \
	src/small/list.h \
	src/small/queue.h \
	src/small/timer.h \
	src/small/util.h \
	src/small/worker.h

lib_LTLIBRARIES = src/libsmall.la

src_libsmall_la_SOURCES =\
	src/argparser.c \
	src/dict.c \
	src/latch.c \
	src/list.c \
	src/pool.c \
	src/pool.h \
	src/queue.c \
	src/slab.c \
	src/slab.h \
	src/timer.c \
	src/util.c \
	src/util-internal.h \
	src/worker.c

EXTRA_DIST += src/libsmall.sym

src_libsmall_la_LDFLAGS = $(AM_LDFLAGS) \
	-version-info $(LIBSMALL_CURRENT):$(LIBSMALL_REVISION):$(LIBSMALL_AGE) \
	-Wl,--version-script=$(top_srcdir)/src/libsmall.sym
src_libsmall_la_DEPENDENCIES = ${top_srcdir}/src/libsmall.sym

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = src/libsmall.pc
EXTRA_DIST += src/libsmall.pc.in
CLEANFILES += src/libsmall.pc

TESTS =\
	src/test-argparser \
	src/test-dict \
	src/test-latch \
	src/test-list \
	src/test-pool \
	src/test-queue \
	src/test-timer \
	src/test-worker

check_PROGRAMS =\
	src/test-argparser \
	src/test-dict \
	src/test-latch \
	src/test-list \
	src/test-pool \
	src/test-queue \
	src/test-timer \
	src/test-worker

src_test_argparser_SOURCES = src/test-argparser.c
src_test_argparser_LDADD = src/libsmall.la

src_test_dict_SOURCES = src/test-dict.c
src_test_dict_LDADD = src/libsmall.la

src_test_latch_SOURCES = src/test-latch.c
src_test_latch_LDADD = src/libsmall.la

src_test_list_SOURCES = src/test-list.c
src_test_list_LDADD = src/libsmall.la

src_test_pool_SOURCES = src/test-pool.c
src_test_pool_LDADD = src/libsmall.la

src_test_queue_SOURCES = src/test-queue.c
src_test_queue_LDADD = src/libsmall.la

src_test_timer_SOURCES = src/test-timer.c
src_test_timer_LDADD = src/libsmall.la

src_test_worker_SOURCES = src/test-worker.c
src_test_worker_LDADD = src/libsmall.la
