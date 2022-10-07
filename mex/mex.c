/*
 Copyright (c) 2022 Ali AslRousta <aslrousta@gmail.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <getopt.h>
#include <libmex.h>
#include <stdio.h>
#include <stdlib.h>

static struct option opts[] = {{"help", no_argument, 0, 'h'},
                               {"output", required_argument, 0, 'o'},
                               {0, 0, 0, 0}};

static void intro(void) {
  printf("MeX - A TeX-inspired macro preprocessor\n");
  printf("(c) 2022 Ali AslRousta <aslrousta@gmail.com>\n\n");
}

static void usage(void) {
  intro();
  printf("usage: mex [options] <input>\n\n");
  printf("options:\n");
  printf(" --output, -o\t\toutput file (default: stdout)\n");
  printf(" --help, -h\t\tprints this help information\n");
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  FILE *in = NULL, *out = NULL;
  int c, res = EXIT_SUCCESS;
  while ((c = getopt_long(argc, argv, "ho:", opts, NULL)) != -1) {
    switch (c) {
      case 'h':
        usage();
      case 'o':
        if (!(out = fopen(optarg, "wt"))) {
          perror("output failed");
          res = EXIT_FAILURE;
          goto term;
        }
    }
  }
  if (optind < argc) {
    if (!(in = fopen(argv[optind], "rt"))) {
      perror("input failed");
      res = EXIT_FAILURE;
      goto term;
    }
  }
  if (!in) in = stdin;
  if (!out) out = stdout;
  mxrun(in, out);
term:
  if (in && in != stdin) fclose(in);
  if (out && out != stdout) fclose(out);
  return res;
}
