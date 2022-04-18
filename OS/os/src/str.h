#ifndef STR_H
#define STR_H

#include "mydef.h"

//获取单个字符输入（同时消除多余的换行符，不会影响后续输入流）
char my_getc();
//大小写不敏感地比较字符
int check_char(char target,char source);
//检查数据是否全是0
int all_zero(char* target,int rangeLen);
//指定长度内相同 rangeLen=max(target,source)可避免前缀相同影响
int check_str(char* target,int rangeLen,char* source);
//从字符串获取整数
int itoa(char* str);

//在rangeLen或者target长度范围内将source赋给target
int save_str(char* target,int rangeLen,char* source);
//将source中的空格替换为结束符分隔字符串，分隔位置记录在rangeRec中
int div_str(char* target,int* rangeRec,char* source);

//从source复制rangeLen个元素到target
void mem_cpy(char* target,int rangeLen,char* source);
//将target的前rangeLen个字符都设置为source
void mem_set(char* target,int rangeLen,char source);
//获取target字符串的长度 以'\0'和空格为结束符 最多返回maxLen
int len_str(char* target,int maxLen);

//在路径后面添加文件名
void apd_path_str(char* target,int rangeLen,char* source,int maxLen);
//去除路径最后一级
void cut_path_str(char* source);
//从路径中获取最后一级文件名和扩展名 并从路径中去除该文件名 如果路径中没有这样的格式 返回失败
int path2name(char* srcPath,char* newItemName,char* newExName);

#endif
