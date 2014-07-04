#ifndef _ARGPARSER_H_
#define _ARGPARSER_H_

#include "dict.h"


#define ARGPARSER_MAX_ARGV   50

typedef struct {
  dict_t opts;
  int argc;
  char *argv[ARGPARSER_MAX_ARGV];
} argparser;

typedef argparser * argparser_t;


argparser_t argparser_new(int maxopts);
void argparser_destroy(argparser_t ap);
void argparser_add(argparser_t ap, const char *name, char chr, const char *dval, const char *desc);
int argparser_get_int(argparser_t ap, const char *optname);
const char * argparser_get_str(argparser_t ap, const char *optname);
void argparser_parse(argparser_t ap, int argc, const char **argv);
void argparser_show_opts(argparser_t ap);

#endif
