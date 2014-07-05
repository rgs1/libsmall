/*-*- Mode: C; c-basic-offset: 2; indent-tabs-mode: nil -*-*/

/*
 * a generic, extensible, argument parser
 *
 * example:
 *
 *    argparser *ap = argparser_new();
 *    argparser_add(ap, "max-events", 'e', 20, "some desc");
 *    argparser_add(ap, "path", 'p', "/tmp", "other desc");
 *    ...
 *    argparser_parse(ap, argc, argv);
 *    argparser_get_int(ap, "max-events");
 *    argparser_get_str(ap, "path");
 *
 */

#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <small/argparser.h>
#include <small/dict.h>
#include <small/list.h>
#include <small/util.h>


struct argparser {
  dict_t opts;
  int argc;
  char *argv[ARGPARSER_MAX_ARGV];
};

typedef struct {
  char c;
  char *value;
  char *desc;
} opt_value;

typedef opt_value * opt_t;

SMALL_EXPORT argparser *argparser_new(int max_opts)
{
  argparser *ap = safe_alloc(sizeof(argparser));
  ap->opts = dict_new(max_opts);
  dict_use_string_keys(ap->opts);
  return ap;
}

/* TODO: free opt_defs */
SMALL_EXPORT void argparser_destroy(argparser *ap)
{
  assert(ap);
  assert(ap->opts);

  dict_destroy(ap->opts);
  free(ap);
}

/*
 * Note: the caller owns dval
 *
 * TODO: chr == 'h' is illegal
 */
SMALL_EXPORT void argparser_add(argparser *ap, const char *name, char chr, const char *dval, const char *desc)
{
  opt_t opt_def = safe_alloc(sizeof(opt_value));

  opt_def->c = chr;
  opt_def->value = strdup(dval);
  opt_def->desc = strdup(desc);

  dict_set(ap->opts, strdup(name), opt_def);
}

static char * build_optstring(argparser *ap)
{
  char *key;
  char *optstring;
  int keys_cnt, i;
  list_t keys;
  list_item_t item;
  opt_t opt;

  keys_cnt = dict_count(ap->opts);
  optstring = safe_alloc((keys_cnt * 2) + 3); /* '+h' + (num_keys * 2) + NULL */
  optstring[0] = '+';
  optstring[1] = 'h';

  i = 2;
  keys = dict_keys(ap->opts);
  list_for_each(item, key, keys) {
    opt = dict_get(ap->opts, key);
    optstring[i++] = opt->c;
    optstring[i++] = ':';
  }
  optstring[i] = '\0';

  return optstring;
}

static struct option * build_options(argparser *ap)
{
  char *key;
  int keys_cnt, i;
  list_t keys;
  list_item_t item;
  opt_t opt;
  struct option * options;

  /* help + NULL */
  keys_cnt = dict_count(ap->opts);
  options = safe_alloc(sizeof(struct option) * (keys_cnt + 2));

  options[0].name = "help";
  options[0].has_arg = no_argument;
  options[0].flag = NULL;
  options[0].val = 'h';

  keys = dict_keys(ap->opts);
  i = 1;
  list_for_each(item, key, keys) {
    opt = dict_get(ap->opts, key);
    options[i].name = key;
    options[i].has_arg = required_argument;
    options[i].flag = NULL;
    options[i].val = opt->c;
    i++;
  }

  return options;
}

static char * key_from_char(argparser *ap, char c)
{
  char *key;
  list_t keys;
  list_item_t item;
  opt_t opt;

  keys = dict_keys(ap->opts);
  list_for_each(item, key, keys) {
    opt = dict_get(ap->opts, key);
    if (opt->c == c)
      return key;
  }

  return NULL;
}

static void help(argparser *ap)
{
  char *key;
  list_t keys;
  list_item_t item;
  opt_t opt;

  printf("%s [OPTIONS...] [args]\n\n", program_invocation_short_name);
  printf("  --help,\t\t\t-h\tShow this help\n");

  /* iterate over opts */
  keys = dict_keys(ap->opts);
  list_for_each(item, key, keys) {
    opt = dict_get(ap->opts, key);
    printf("  --%s,\t\t\t-%c\t%s\n", key, opt->c, opt->desc);
  }
}

SMALL_EXPORT void argparser_parse(argparser *ap, int argc, const char **argv)
{
  char *sopts = build_optstring(ap);
  struct option * options = build_options(ap);
  int c, i;

  assert(argc >= 0);
  assert(argv);

  while ((c = getopt_long(argc, (char **)argv, sopts, options, NULL)) >= 0) {
    char *key;
    opt_t opt;

    if (c == '?' || c == 'h') {
      help(ap);
      exit(1);
    }

    key = key_from_char(ap, c);
    if (!key) {
      error(EXIT_BAD_PARAMS, "Bad option %c\n", (char)c);
    }

    opt = dict_get(ap->opts, key);
    assert(opt);
    opt->value = strdup(optarg); /* leak.. */
  }

  /* consume the remaining args... */
  i = optind;
  while (i < argc && ap->argc <  ARGPARSER_MAX_ARGV) {
    ap->argv[ap->argc++] = strdup(argv[i++]);
  }

  argparser_show_opts(ap);

  /* cleanup */
  free(sopts);
  free(options);
}

SMALL_EXPORT void argparser_show_opts(argparser *ap)
{
  char *key;
  int i;
  list_t keys;
  list_item_t item;
  opt_t opt;

  printf("Options:\n");
  keys = dict_keys(ap->opts);
  list_for_each(item, key, keys) {
    opt = dict_get(ap->opts, key);
    printf("%s = %s\n", key, opt->value);
  }
  printf("\nargv: ");
  for (i=0; i < ap->argc; i++) {
      printf("%s ", ap->argv[i]);
  }
  printf("\n\n");
}

SMALL_EXPORT int argparser_get_int(argparser *ap, const char *optname)
{
  return atoi(argparser_get_str(ap, optname));
}

SMALL_EXPORT const char * argparser_get_str(argparser *ap, const char *optname)
{
  opt_t opt = (opt_t)dict_get(ap->opts, (char *)optname);

  assert(opt);
  assert(opt->value);

  return opt->value;
}

SMALL_EXPORT char * argparser_get_argv(argparser *ap, int argi)
{
  assert(argi < ap->argc);
  return ap->argv[argi];
}

SMALL_EXPORT int argparser_get_argc(argparser *ap)
{
  return ap->argc;
}
