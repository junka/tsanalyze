AUTOMAKE_OPTIONS = foreign subdir-objects
bin_PROGRAMS = tsanalyze
tsanalyze_SOURCES = src/main.c src/ts.c src/pes.c src/filter.c src/tsio.c \
		    src/ps.c src/crc32.c src/fileio.c src/descriptor.c \
		    src/table.c src/utils.c src/udp.c src/options.c src/result.c \
			src/scte.c src/subtitle.c src/teletext.c src/win_getopt.c
tsanalyze_CPPFLAGS = -I$(top_srcdir)/include/

AM_CFLAGS = -D_GNU_SOURCE -std=c11 -W -Wall -Wfatal-errors -Wstrict-prototypes -Wshadow -Wno-unused-parameter -fno-omit-frame-pointer
