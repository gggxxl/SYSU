#ifndef RUN_H
#define RUN_H

#include "mydef.h"
#include "dir.h"

char cmdLine[MAXLEN];
char cmdArgs[MAXLEN];
int argsRec[MAXLEN/2];
int argsNum;

void run();
int check_arg(int least,int most);
void cmd_su();
void cmd_ls();
void cmd_tree();
void _cmd_tree(dirItem* targetDir,int depth);
void cmd_cd();
void cmd_md();
void cmd_rd();
void cmd_rm();
void cmd_new();
void cmd_read();
void cmd_copy();
void cmd_cls();
void cmd_exec();
extern char cntPath[MAXLEN];


#endif
