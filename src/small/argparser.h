#ifndef _ARGPARSER_H_
#define _ARGPARSER_H_

#define ARGPARSER_MAX_ARGV   50

typedef struct argparser argparser;

argparser * argparser_new(int maxopts);
void argparser_destroy(argparser *ap);
void argparser_add(argparser *ap, const char *name, char chr, const char *dval, const char *desc);
int argparser_get_int(argparser *ap, const char *optname);
const char * argparser_get_str(argparser *ap, const char *optname);
char * argparser_get_argv(argparser *ap, int argi);
int argparser_get_argc(argparser *ap);
void argparser_parse(argparser *ap, int argc, const char **argv);
void argparser_show_opts(argparser *ap);

#endif
