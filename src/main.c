#include <fcntl.h>
#include <inttypes.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "tsio.h"
#include "table.h"
#include "ts.h"
#include "pes.h"
#include "options.h"

void dump_result(int sig)
{
	dump_tables();
	dump_pes_infos();
	dump_scte_info();
	dump_ts_info();

	res_close();
	free_tables();
	uninit_pid_processor();
	exit(sig);
}

int main(int argc, char *argv[])
{
#if __STDC_VERSION__ < 201112L
	printf("good");
	exit(0);
#endif
	fileio_init();
	udp_io_init();
	int ret;
	ret = prog_parse_args(argc, argv);
	if (ret < 0)
		return -1;
	signal(SIGINT, dump_result);

	init_pid_processor();

	ts_process();

	dump_result(0);

	return 0;
}
