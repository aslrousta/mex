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

#include "libmex.h"

#include <ctype.h>
#include <stdint.h>
#include <string.h>

typedef uint16_t token_t;

/* primitive tokens */
enum {
  tEOF = 256, /* end of file */
  tESCAPE,    /* escape '\' appears in control sequences */
  tBGROUP,    /* begin group '{' */
  tEGROUP,    /* end group '}' */
  tARG1,      /* 1st argument '\1' */
  tARG2,      /* 2nd argument '\2' */
  tARG3,      /* 3rd argument '\3' */
  tARG4,      /* 4th argument '\4' */
  tARG5,      /* 5th argument '\5' */
  tARG6,      /* 6th argument '\6' */
  tARG7,      /* 7th argument '\7' */
  tARG8,      /* 8th argument '\8' */
  tARG9,      /* 9th argument '\9' */
  tDEF,       /* macro definition '\def' */
  tSENTINEL,  /* end of primitives */
};

static struct {
  token_t t;
  token_t s[4];
  int len;
} prims[] = {
    {tEOF, {'E', 'O', 'F'}, 3}, {tESCAPE, {'\\'}, 1},
    {tBGROUP, {'{'}, 1},        {tEGROUP, {'}'}, 1},
    {tARG1, {'\\', '1'}, 2},    {tARG2, {'\\', '2'}, 2},
    {tARG3, {'\\', '3'}, 2},    {tARG4, {'\\', '4'}, 2},
    {tARG5, {'\\', '5'}, 2},    {tARG6, {'\\', '6'}, 2},
    {tARG7, {'\\', '7'}, 2},    {tARG8, {'\\', '8'}, 2},
    {tARG9, {'\\', '9'}, 2},    {tDEF, {tESCAPE, 'd', 'e', 'f'}, 4},
};

#define POOLMAX 1000000
#define TOKENMAX 500
#define BUFFERMAX (3 * TOKENMAX)
#define MACROMAX (UINT16_MAX - tSENTINEL)

static FILE *in;
static int is_start;
static int line_breaks;

static token_t buf[BUFFERMAX];
static int bpos;
static int blast;

static token_t pool[POOLMAX];
static int ppos[UINT16_MAX];
static int plen[UINT16_MAX];
static token_t pcur;
static int plast;

static token_t macro[MACROMAX];
static token_t mdef[MACROMAX];
static int margs[MACROMAX];
static int mlast;

static void init(FILE *fp) {
  int i, j;
  in = fp;
  pcur = tSENTINEL;
  bpos = blast = plast = mlast = 0;
  is_start = 1;
  line_breaks = 0;
  for (i = 0; i < 256; i++) {
    ppos[i] = plast;
    plen[i] = 1;
    pool[plast++] = i;
  }
  for (i = 0; i < sizeof(prims) / sizeof(prims[0]); i++) {
    ppos[prims[i].t] = plast;
    plen[prims[i].t] = prims[i].len;
    for (j = 0; j < prims[i].len; j++) pool[plast++] = prims[i].s[j];
  }
}

static void scan_cs(void) {
  token_t cs[TOKENMAX] = {tESCAPE};
  int i, len = 1;
  while (1) {
    int ch = fgetc(in);
    if (!isalpha(ch) || len >= TOKENMAX) {
      ungetc(ch, in);
      break;
    }
    cs[len++] = ch;
  }
  for (i = tDEF; i < pcur; i++) {
    if (plen[i] == len && !memcmp(&pool[ppos[i]], cs, len * sizeof(token_t))) {
      buf[blast++] = i;
      return;
    }
  }
  ppos[pcur] = plast;
  plen[pcur] = len;
  for (i = 0; i < len; i++) pool[plast++] = cs[i];
  buf[blast++] = pcur++;
}

static void scan(void) {
  int ch;
again:
  ch = fgetc(in);
  if (ch == EOF) {
    buf[blast++] = tEOF;
    return;
  }
  if (isspace(ch)) {
    if (!is_start) {
      while (1) {
        if (ch == '\n') line_breaks++;
        ch = fgetc(in);
        if (ch == EOF) {
          buf[blast++] = tEOF;
          return;
        }
        if (!isspace(ch)) {
          ungetc(ch, in);
          break;
        }
      }
      buf[blast++] = line_breaks > 1 ? '\n' : ' ';
      line_breaks = 0;
    }
    goto again;
  }
  if (ch == '#') {
    while (1) {
      ch = fgetc(in);
      if (ch == EOF) {
        buf[blast++] = tEOF;
        return;
      }
      if (ch == '\n') break;
    }
    goto again;
  }
  if (ch == '{')
    buf[blast++] = tBGROUP;
  else if (ch == '}')
    buf[blast++] = tEGROUP;
  else if (ch != '\\')
    buf[blast++] = ch;
  else {
    ch = fgetc(in);
    if (ch == EOF)
      buf[blast++] = tEOF;
    else if (ch >= '1' && ch <= '9') {
      buf[blast++] = tARG1 + ch - '1';
    } else if (!isalpha(ch))
      buf[blast++] = ch;
    else {
      ungetc(ch, in);
      scan_cs();
    }
  }
  is_start = 0;
}

static token_t scan_group(void) {
  token_t group[TOKENMAX];
  int i, d = 1, len = 0;
  while (1) {
    if (bpos == blast) scan();
    if (buf[bpos] == tEOF || len >= TOKENMAX) break;
    if (buf[bpos] == tBGROUP)
      d++;
    else if (buf[bpos] == tEGROUP) {
      if (--d == 0) {
        bpos++;
        break;
      }
    }
    group[len++] = buf[bpos++];
  }
  for (i = tSENTINEL; i < pcur; i++) {
    if (plen[i] == len && !memcmp(&pool[ppos[i]], group, len * sizeof(token_t)))
      return i;
  }
  ppos[pcur] = plast;
  plen[pcur] = len;
  for (i = 0; i < len; i++) pool[plast++] = group[i];
  return pcur++;
}

static void define(void) {
  int i;
  if (bpos == blast) scan();
  if (buf[bpos] < tSENTINEL) return;
  for (i = 0; i < mlast; i++) {
    if (macro[i] == buf[bpos]) break;
  }
  if (i == mlast) macro[mlast++] = buf[bpos];
  if (++bpos == blast) scan();
  if (buf[bpos] >= '0' && buf[bpos] <= '9') {
    margs[i] = buf[bpos] - '0';
    if (++bpos == blast) scan();
  } else {
    margs[i] = 0;
  }
  if (buf[bpos] == tBGROUP) {
    bpos++;
    mdef[i] = scan_group();
  } else if (buf[bpos] != tEOF) {
    mdef[i] = buf[bpos];
  }
}

static void expand(void) {
  token_t tmp[BUFFERMAX];
  token_t param[9];
  int m, i, j, k, start = bpos;
  for (m = 0; m < mlast; m++) {
    if (macro[m] == buf[bpos]) break;
  }
  bpos++;
  for (i = 0; i < margs[m]; i++) {
    if (bpos == blast) scan();
    if (buf[bpos] == tBGROUP) {
      bpos++;
      param[i] = scan_group();
    } else {
      param[i] = buf[bpos++];
    }
  }
  k = 0;
  for (i = ppos[mdef[m]], j = i + plen[mdef[m]]; i < j; i++) {
    if (pool[i] < tARG1 || pool[i] > tARG9)
      tmp[k++] = pool[i];
    else {
      int v, w, p = pool[i] - tARG1;
      for (v = ppos[param[p]], w = v + plen[param[p]]; v < w; v++)
        tmp[k++] = pool[v];
    }
  }
  j = (start + k) - bpos;
  if (j > 0) {
    for (i = blast - 1; i >= bpos; i--) buf[i] = buf[i - j];
  } else if (j < 0) {
    for (i = bpos; i < blast; i++) buf[i + j] = buf[i];
  }
  blast += j;
  for (i = 0; i < k; i++) buf[start + i] = tmp[i];
  bpos = start;
}

static token_t next(void) {
  int i;
again:
  if (bpos == blast) scan();
  if (bpos > TOKENMAX) {
    for (i = bpos; i < blast; i++) buf[i - bpos] = buf[i];
    blast -= bpos;
    bpos = 0;
  }
  if (buf[bpos] == tDEF) {
    bpos++;
    define();
    goto again;
  }
  if (buf[bpos] >= tSENTINEL) {
    expand();
    goto again;
  }
  return buf[bpos++];
}

int mxrun(FILE *fi, FILE *fo) {
  init(fi);
  while (1) {
    token_t t = next();
    if (t == tEOF) break;
    if (t < tEOF) fputc(t, fo);
  }
  return 0;
}
