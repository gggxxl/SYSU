#ifndef FAT_H
#define FAT_H

#include "mydef.h"
#include "dir.h"
#include "mysys.h"

#define nSec 512

//规定软盘中各结构的扇区号
#define MBR_SEC 1
#define FAT1_SEC 2
#define FAT2_SEC 11
#define ROOT_SEC 20
#define DATA_SEC 34

typedef BYTE* SEC_PTR;

BYTE fatSec[nSec];//当前FAT块缓存
int _cntFatNum;//当前FAT块号
BYTE cntSec[nSec];//当前扇区缓存
int _cntSecNum;//当前扇区号
dirItem _cntDir;//当前目录文件索引

int _backSec;//备份当前块号
dirItem _backDir;//备份当前目录文件

//空闲块链的开头、结尾
int _headFreeClus;
int _lastFreeClus;

//簇号转换为扇区号
int clus2num(int target);
//扇区号转换为簇号
int num2clus(int target);

//扇区号转化为软盘缓存中相应位置的地址
SEC_PTR num2ptr(int secNum);

//时间与文件项时间日期值之间的转换
void time2num(TIME_DATE* timeDate,WORD* time,WORD* date);
void num2time(TIME_DATE* timeDate,WORD time,WORD date);

//从当前扇区缓存向软盘缓存读扇区
void load_sec(int secNum);
//从当前扇区缓存向软盘缓存写扇区
void save_sec(int secNum);

//加载文件的下一数据块
//如果下一块是空块，在写模式下申请新块，在读模式下返回结束
SEC_PTR next_sec(FLAG MODE);

//读取簇号为clusNum的数据块对应的FAT表项的内容
int read_fat(int clusNum);
//修改簇号为clusNum的数据块对应的FAT表项的内容
void write_fat(int clusNum,int nextClus);

//pop一个空闲块
int pop_free_clus();
//push一个空闲块
void push_free_clus(int freeClus);
//清空空闲块链
void clear_free_clus();
//清空指定数据块
void clear_clus(int clusNum);

void start_fat();

void load_fat(int secNum);

void save_fat(int secNum);

void close_fat();

void backup_fat();

#endif
