#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>

#include "ts.h"
#include "table.h"
#include "filter.h"

int prog_parse_args(int argc, char **argv);

void dump_result(int sig)
{
	dump_tables();
	dump_ts_info();
	exit(0);
}

int main(int argc,char *argv[])
{
	int ret;
	ret = prog_parse_args(argc, argv);
	if(ret<0)
		return -1;
	signal(SIGINT,dump_result);

	init_pid_processor();

	ts_process();

	dump_result(0);
	return 0;
}
