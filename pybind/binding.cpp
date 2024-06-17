#include <pybind11/pybind11.h>
#include <string>

#include "ts.h"
#include "table.h"
#include "pes.h"
#include "options.h"

namespace py = pybind11;

char** convertPythonListToArgv(const py::list& args_list) {
    size_t argc = args_list.size();
    if (argc == 0) {
        return nullptr;
    }

    char** argv = new char*[argc];

    for (size_t i = 0; i < argc; ++i) {
        std::string arg = args_list[i].cast<std::string>();
		argv[i] = new char[arg.size()];
		std::strcpy(argv[i], arg.c_str());
    }

    return argv;
}

int parse_args(const py::list& args)
{
	int argc = static_cast<int>(args.size());
	char ** argv = convertPythonListToArgv(args);
	for (int i = 0; i < argc; i++) {
		printf("%s\n", argv[i]);
	}
	int ret = prog_parse_args(argc, argv);
	for (int i = 0; i < argc; i++) {
		delete argv[i];
	}
	delete argv;
	return ret;
}

int init(const py::list& args)
{
	parse_args(args);
	init_pid_processor();
	return 0;
}

void run()
{
	ts_process();
}

void result()
{
	dump_tables();
	dump_pes_infos();
	dump_scte_info();
	dump_ts_info();

}

void deinit()
{
	res_close();
	free_tables();
	uninit_pid_processor();
}

PYBIND11_MODULE(tsana, m) {
	m.doc() = "pybind11 tsanalyze";
	m.def("init", &init, "Init module processer");
	m.def("run", &run, "run the parser");
	m.def("result", &result, "show parser results");
	m.def("deinit", &deinit, "Deinit module and free");
}
