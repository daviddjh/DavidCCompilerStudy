#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <windows.h>

#include "debug.h"
#include "wfio.h"

typedef unsigned char uchar;

void ii_io(HANDLE (*open_funct)(), int (*close_funct)(), int (*read_funct)());

HANDLE ii_newfile(const char* name);

char* ii_text();
int ii_length();
int ii_lineno();
char* ii_ptext();
int ii_plength();
int ii_plineno();

char* ii_mark_start();

char* ii_mark_end();

char* ii_move_start();

char* ii_to_mark();

char* ii_mark_prev();

uchar ii_advance();

int ii_flush(int force);

int ii_fillbuf(uchar* starting_at);

int ii_look(unsigned long n);

int ii_pushback(int n);

void ii_term();

void ii_unterm();

int ii_input();

void ii_unput(char c);

int ii_lookahead(unsigned long n);

int ii_flushbuf();


