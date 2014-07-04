#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <small/argparser.h>
#include <small/list.h>
#include <small/util.h>


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
