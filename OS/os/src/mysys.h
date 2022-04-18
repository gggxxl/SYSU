#ifndef MYSYS_H
#define MYSYS_H

#include "mydef.h"
#include "dir.h"

typedef int PROCESS;
#define MAX_PROCESS 9
#define THIS_PROCESS 0

#include "file.h"

extern WORD COM_PSP;
extern WORD COM_SEG;

//时间日期结构
typedef struct structTrd{
  WORD year;
  WORD month;
  WORD day;
  WORD hour;
  WORD minute;
  WORD sec;
}__attribute__((packed)) TIME_DATE;

typedef struct{
	WORD AX,BX,CX,DX,DI,SI;//custom
	WORD SP,BP,SS,DS,ES;//stack&data
	WORD FS,GS;//other
	WORD CS,IP,EFLAGS;//state
}__attribute__((packed)) CPU_REGS;

typedef struct{
	CPU_REGS cpuRegs;//0x0 cpu running state
	WORD pid;//0x20 process identification -1:empty
	char pname[8];//0x22 fat file name
	char pstate;//0x2a 0:ready 1:running
}__attribute__((packed)) PCB;

extern PCB PCB_LIST[MAX_PROCESS];
extern WORD CURRENTPID;//0:core 1-8:user

extern char cntFlp[];
extern char userName[MAXLEN];
extern FLAG isroot;
extern TIME_DATE SYS_TIME;

//获取系统时间
TIME_DATE* get_sys_time();
//打印数据 文本模式和二进制模式
void print_data(BYTE* data,int rangeLen,FLAG MODE);

//输入：lacalCat本地文件所在目录 localDir本地文件（已建立） outName外部文件外部路径
//传入文件：返回成功与否
//int move_in(dirItem* localCat,dirItem* localDir,char* outName);
//传出文件：返回成功与否
//int move_out(dirItem* localCat,dirItem* localDir,char* outName);

MYFILE* user_open(char* path,int mode);
int user_read(char* buf,int size,MYFILE* fp);
int user_write(char* buf,int size,MYFILE* fp);
void user_close(MYFILE* fp);


//开启系统
int start_sys();
//关闭系统
int close_sys();

extern int COM_LBA;
extern void* COM_PTR;
extern void io_init();
extern void load_com();
extern void create_psp();
extern void exec_com();
extern void upd_sys_time();
void _load(int clusNum,char* fileName);
int _exec();
void sched_com();

#endif
