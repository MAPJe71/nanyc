#include <nanyc/nanyc.h>
#include "libnanyc.h"
#include <string.h>

int nymain(const nyvm_opts_t* vmopts, const char* filename, size_t flen, uint32_t argc, const char** argv) {
	nycompile_opts_t copts;
	nyprogram_t* program;
	int exitstatus = 122;
	nysource_opts_t sources[1];

	(void) argc;
	(void) argv;
	if (unlikely(!flen or !filename))
		return 121;
	memset(&copts, 0x0, sizeof(nycompile_opts_t));
	copts.sources.count = 1;
	copts.sources.items = sources;
	memset(&sources[0], 0x0, sizeof(sources));
	sources[0].filename.c_str = filename;
	sources[0].filename.len = (uint32_t) flen;
	copts.entrypoint.c_str = "main";
	copts.entrypoint.len = 4;
	program = nyprogram_compile(&copts);
	if (program) {
		if (nytrue == nyvm_run_entrypoint(vmopts, program))
			exitstatus = 0;
		nyprogram_free(program);
	}
	return exitstatus;
}

int nymain_ex(const nyvm_opts_t* vmopts, const nyanystr_t* files, uint32_t flen, uint32_t argc, const char** argv) {
	nycompile_opts_t copts;
	nyprogram_t* program;
	int exitstatus = 122;
	uint32_t i;

	(void) argc;
	(void) argv;
	if (unlikely(!flen or !files))
		return 121;
	memset(&copts, 0x0, sizeof(nycompile_opts_t));
	copts.sources.count = flen;
	copts.sources.items = (nysource_opts_t*) calloc(flen, sizeof(nysource_opts_t));
	if (unlikely(!copts.sources.items))
		return 121;
	for (i = 0; i != flen; ++i)
		copts.sources.items[i].filename = files[i];
	copts.entrypoint.c_str = "main";
	copts.entrypoint.len = 4;
	program = nyprogram_compile(&copts);
	if (program) {
		if (nytrue == nyvm_run_entrypoint(vmopts, program))
			exitstatus = 0;
		nyprogram_free(program);
	}
	free(copts.sources.items);
	return exitstatus;
}
