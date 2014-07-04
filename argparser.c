/*
 * a generic, extensible, argument parser
 *
 * example:
 *
 *    argparser_t ap = argparser_new();
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

#include "argparser.h"
#include "list.h"
#include "util.h"


typedef struct {
  char c;
  char *value;
  char *desc;
} opt;

typedef opt * opt_t;

argparser_t argparser_new(int max_opts)
{
  argparser_t ap = safe_alloc(sizeof(argparser));
  ap->opts = dict_new(max_opts);
  dict_use_string_keys(ap->opts);
  return ap;
}

/* TODO: free opt_defs */
void argparser_destroy(argparser_t ap)
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
void argparser_add(argparser_t ap, const char *name, char chr, const char *dval, const char *desc)
{
  opt_t opt_def = safe_alloc(sizeof(opt));

  opt_def->c = chr;
  opt_def->value = strdup(dval);
  opt_def->desc = strdup(desc);

  dict_set(ap->opts, strdup(name), opt_def);
}

static char * build_optstring(argparser_t ap)
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

static struct option * build_options(argparser_t ap)
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

static char * key_from_char(argparser_t ap, char c)
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

static void help(argparser_t ap)
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

void argparser_parse(argparser_t ap, int argc, const char **argv)
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

void argparser_show_opts(argparser_t ap)
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

int argparser_get_int(argparser_t ap, const char *optname)
{
  return atoi(argparser_get_str(ap, optname));
}

const char * argparser_get_str(argparser_t ap, const char *optname)
{
  opt_t opt = (opt_t)dict_get(ap->opts, (char *)optname);

  assert(opt);
  assert(opt->value);

  return opt->value;
}


#ifdef RUN_TESTS

static void test_no_opts(void)
{
  int argc = 1;
  const char *argv[] = { "./prog" };
  argparser_t ap = argparser_new(2);

  argparser_add(ap, "max-events", 'e', "20", "max events for foo");
  argparser_add(ap, "path", 'p', "/tmp", "path for bar");

  argparser_parse(ap, argc, argv);

  info("Checking default values are still there... ");
  assert(argparser_get_int(ap, "max-events") == 20);
  assert(strcmp(argparser_get_str(ap, "path"), "/tmp") == 0);

  info("Checking there's no argv left...");
  assert(ap->argc == 0);

  argparser_destroy(ap);
}

static void test_basic(void)
{
  int argc = 7;
  const char *argv[] = {
    "./prog",
    "--max-events",
    "40",
    "--path",
    "/var/tmp",
    "localhost",
    "8080",
  };
  argparser_t ap = argparser_new(2);

  argparser_add(ap, "max-events", 'e', "20", "max events for foo");
  argparser_add(ap, "path", 'p', "/tmp", "path for bar");

  argparser_parse(ap, argc, argv);

  info("Checking default values were replaced... ");
  assert(argparser_get_int(ap, "max-events") == 40);
  assert(strcmp(argparser_get_str(ap, "path"), "/var/tmp") == 0);

  info("Checking there's 2 arg in argv left...");
  assert(ap->argc == 2);
  assert(strcmp(ap->argv[0], "localhost") == 0);
  assert(strcmp(ap->argv[1], "8080") == 0);

  argparser_destroy(ap);
}

int main(int argc, char **argv)
{
  run_test("no opts - just defaults", &test_no_opts);
  run_test("basic opts", &test_basic);

  return 0;
}


#endif
