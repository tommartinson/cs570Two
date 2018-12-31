/*
Thomas Martinson
Program 2
Professor Carroll
CS570
Due: 12/5/18
*/

//Header file for p2.h

#include <stdio.h>
#include "getword.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <signal.h>

#define MAXITEM 100 /* max number of words per line */
#define MAXSTORAGE 25500

int main();
int parse();
void handler();