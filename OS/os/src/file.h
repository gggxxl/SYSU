#ifndef FILE_H
#define FILE_H

#include "mydef.h"
#include "dir.h"

#define MAX_FILE 5
#define MAX_SYS_FILE 10
#define MAX_BUFFER_NUM 5



#define stdin (MYFILE*)0x0
#define stdout (MYFILE*)0x1
#define stderr (MYFILE*)0x2


//注意结构体相互嵌套
//系统活动文件表
typedef struct structFif SYSFILE;
struct structFif{
  dirItem fileDir;
  dirItem catDir;
  int shareCnt;
};

//进程文件打开表
typedef struct structFor MYFILE;
struct structFor{
  int posi;
  FLAG MODE;
  SYSFILE* sysFile;
};

//缓冲块
typedef struct structSix FILE_BUFFER;
struct structSix{
  BYTE* bufferPtr;//数据
  SYSFILE* sysFile;//属于的文件
  int bufferNum;//buffer是原文件的第几块
  int bufferSize;//buffer内有效字节数
  FILE_BUFFER* next;//下一块
};

MYFILE _myFile[MAX_PROCESS][MAX_FILE];
SYSFILE _sysFile[MAX_SYS_FILE];

//暂时用固定数组空间实现缓冲区
FILE_BUFFER _fileBuffer[MAX_BUFFER_NUM];
BYTE _bufferData[MAX_BUFFER_NUM*nSec];//与FILE_BUFFER一一对应
FILE_BUFFER* _bufferHead;//缓冲区头节点

//文件管理接口
MYFILE* open_file(dirItem* catDir,dirItem* targetDir,FLAG MODE);
int close_file(MYFILE* targetFile);
int read_file(void* data,size_t size,size_t count,MYFILE* mfp);
int write_file(void* data,size_t size,size_t count,MYFILE* mfp);
int seek_file(MYFILE* mfp,size_t offset,int fromWhere);
int _file_size(MYFILE* mfp);
int _file_posi(MYFILE* mfp);

//初始化文件打开表
void init_file_list(PROCESS pc);
void init_sys_list();
//关闭文件打开表
void close_file_list(PROCESS pc);
//打印文件打开表
void show_sys_list();
void show_buffer_list();

//系统活动文件表管理函数
SYSFILE* check_sys_file(dirItem* targetDir);
SYSFILE* free_sys_file();
int set_sys_file(SYSFILE* aFile,dirItem* catDir,dirItem* targetDir);
int clear_sys_file(SYSFILE* aFile);
//进程打开文件表管理函数
MYFILE* free_file();
int set_file(MYFILE* mfp,SYSFILE* aFile,FLAG MODE);
int clear_file(MYFILE* mfp);

//缓冲区管理函数

//获取缓存文件的第bufferNum块 返回缓存地址
//如所需块不在缓冲区，缓冲之
//到达文件末尾则返回NULL（读模式），申请新块（写模式）
FILE_BUFFER* get_buffer(SYSFILE* aFile,int bufferNum,FLAG MODE);
//获得文件的第bufferNum块簇号，如果bufferNum块是末尾，则返回结束标志（读模式），申请新块（写模式）
int buffer2clus(int bufferNum,int fstClus,FLAG MODE);
//获取缓冲块空间，如果已满，删除头节点，获取尾部位置
FILE_BUFFER* free_buffer(SYSFILE* aFile);
//删除缓冲块
int clear_buffer(FILE_BUFFER* bfp);
//清空文件的所有缓冲块
int clear_all_buffer(SYSFILE* aFile);


//从软盘缓存加载缓冲数据
int load_buffer(FILE_BUFFER* bfp,int clusNum);
//向软盘缓存保存缓冲数据
int save_buffer(FILE_BUFFER* bfp);


#endif
