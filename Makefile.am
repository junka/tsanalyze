AUTOMAKE_OPTIONS = foreign subdir-objects
bin_PROGRAMS = tsanalyze
tsanalyze_SOURCES = src/main.c src/ts.c src/pes.c src/filter.c src/io.c \
		    src/ps.c src/crc32.c src/fileio.c src/descriptor.c \
		    src/table.c src/utils.c src/udp.c src/options.c src/result.c
tsanalyze_CPPFLAGS = -I$(top_srcdir)/include/
