#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <getopt.h>



#define OPT_HELP 		"help"
#define OPT_BRIEF_LIST	"brief"
#define OPT_DETAIL_LIST	"details"
#define OPT_VERSION		"version"
#define OPT_MEMORY		"mem"
#define OPT_TABLE		"table"

enum {
	/* long options mapped to a short option */
	OPT_HELP_NUM         = 'h',
	OPT_BRIEF_LIST_NUM   = 'b',
	OPT_DETAIL_LIST_NUM  = 'd',
	OPT_VERSION_NUM      = 'v',
	OPT_MEMORY_NUM       = 'm',
	OPT_TABLE_NUM        = 's',
};


void prog_usage(FILE *fp, char* pro_name)
{
	if(fp == NULL)
		fp = stderr;
	fprintf(fp,"Usage: %s [optins]... <file>\n", pro_name);
	fprintf(fp,"  Display infomations about mpeg ts.\n\n");
	fprintf(fp,"%27s%s\n","  -"OPT_HELP_NUM", --"OPT_HELP,"Show this help");
	fprintf(fp,"%27s%s\n","  -"OPT_BRIEF_LIST_NUM", --"OPT_BRIEF_LIST,"Show all infos in brief");
	fprintf(fp,"%27s%s\n","  -"OPT_DETAIL_LIST_NUM", --"OPT_DETAIL_LIST,"Show all infos in detail");
	fprintf(fp,"%27s%s\n","  -"OPT_VERSION_NUM", --"OPT_VERSION,"Show version");
	fprintf(fp,"%27s%s\n","  -"OPT_MEMORY_NUM", --"OPT_MEMORY,"memory to use");
	fprintf(fp,"%27s%s\n","  -"OPT_TABLE_NUM", --"OPT_TABLE,"Show select tables only");
}

static int 
prog_parse_args(int argc, char**argv)
{
	int opt,ret;
	int option_index;
	const char *prgname = argv[0];

	//make a copy of the dpdk options
	const char short_options[]=
		"b:" /* brief */
		"d:" /* details */
		"h"  /* help */
		"m:" /* memory size */
		"v"  /* version */
		"s:" /* tables */
	;


	const struct option long_options[]={
			{OPT_BRIEF_LIST, 1, NULL, OPT_BRIEF_LIST_NUM},
			{OPT_DETAIL_LIST, 0, NULL, OPT_DETAIL_LIST_NUM },
			{OPT_VERSION, 1, NULL, OPT_VERSION_NUM },
			{OPT_HELP, 0, NULL, OPT_HELP_NUM },
			{OPT_MEMORY, 1, NULL, OPT_MEMORY_NUM},
			{OPT_TABLE, 0, NULL, OPT_TABLE_NUM },
			{0, 0, NULL, 0 }
		};

	while((opt = getopt_long(argc,argv,short_options,
		long_options,&option_index)) != EOF){
		if(opt == '?'){
			prog_usage(stdout,prgname);
			return -1;
		}
		switch(opt){
		case 'b':
			break;
		case 'd':
			break;
		case 's':
			break;
		default:
			break;
		}
	}
	return 0;
}

