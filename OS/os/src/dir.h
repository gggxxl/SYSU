#ifndef DIR_H
#define DIR_H

#include "mydef.h"

//目录文件项大小
#define nItem 32

//目录文件项类型信息
#define OTHER_ITEM 0x00
#define NORMAL_ITEM 0x20
#define DIR_ITEM 0x10
#define SYS_ITEM 0x27
#define FREE_ITEM 0xE5
#define BPB_TOT_SEC16 2880

//根目录索引值
#define ROOT_DIR (void*)ROOT_SEC //只用作判断 不真正读取

//“.”和".."文件项名
extern char SELF_DIR[];
extern char UP_DIR[];
extern char EXE_FILE[];

//目录文件项结构
typedef struct structTwo{
  BYTE DIR_NAME[8];
  BYTE DIR_EX_NAME[3];
  BYTE DIR_ATTR;
  BYTE RESERVE[10];
  WORD DIR_WRI_TIME;
  WORD DIR_WRI_DATE;
  WORD DIR_FST_CLUS;
  DWORD DIR_SIZE;
}__attribute__((packed)) dirItem;

//从路径获取文件项在当前扇区的地址，CHANGE指示是否改变当前路径字符串
dirItem* path2ptr(char* targetPath,FLAG CHANGE);
//前者调用的函数，处理的路径从posi开始，实现逐层扫描打开
dirItem* _path2ptr(char* targetPath,int posi,FLAG CHANGE);


//打开目录 CHANGE指示是否改变当前路径（字符串）
void open_cat(dirItem* targetDir,FLAG CHANGE);

//备份当前目录信息
void cover_cat(dirItem* targetDir,int* targetSec);
//从备份中恢复目录信息
void recover_cat(dirItem* targetDir,int* targetSec);

//初始化目录 "."和".."项
void ini_cat(dirItem* targetDir);
//当修改文件后 更新上级目录的修改日期
void upd_cat_dir();
//获取上级目录的目录文件
dirItem* get_up_dir();

//在指定目录中更新指定文件项信息
void save_item(dirItem* tarItem,dirItem* tarCat);
//在指定目录中新建文件项
dirItem* creat_item(dirItem* targetItem,dirItem* targetDir);
//复制：将原文件拼接到目标文件
int copy_item_data(dirItem* tarCat,dirItem* newItem,dirItem* srcCat,dirItem* srcItem);
//按格式显示文件项信息
void display_item(dirItem* dirPtr);

#endif
