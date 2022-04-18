#include "cglobal.h"

int clus2num(int target){
  if(target==0) return ROOT_SEC;
  if(target<2||target>BPB_TOT_SEC16-DATA_SEC+2) return -1;
  return target+(DATA_SEC-2);
}

int num2clus(int target){
  if(target==ROOT_SEC) return 0;
  if(target<DATA_SEC||target>BPB_TOT_SEC16) return -1;
  return target-(DATA_SEC-2);
}

void time2num(TIME_DATE* timeDate,WORD* time,WORD* date){
  *time=0;
  *date=0;
  *date|=((timeDate->year-1980)&0x7F)<<9;//只是保险
  *date|=((timeDate->month)&0xF)<<5;
  *date|=(timeDate->day)&0x1F;
  *time|=((timeDate->hour)&0x1F)<<11;
  *time|=((timeDate->minute)&0x3F)<<5;
  *time|=(timeDate->sec/2)&0x1F;
}

void num2time(TIME_DATE* timeDate,WORD time,WORD date){
  timeDate->year=1980+(date>>9);
  timeDate->month=(date&0x1FF)>>5;
  timeDate->day=date&0x1F;
  timeDate->sec=(time&0x1F)*2;
  timeDate->minute=(time&0x7FF)>>5;
  timeDate->hour=time>>11;
}

//目录块缓存的读写
void load_sec(int secNum){
  //切换扇区时才真正从软盘加载
  if(_cntSecNum!=secNum){
    if(_cntSecNum!=-1) save_sec(_cntSecNum);//如果不是第一次加载 就保存原缓存扇区
    //加载扇区、更新目录块号
    io_load_sec(cntSec,secNum);
    _cntSecNum=secNum;
  }
}

void save_sec(int secNum){
  io_save_sec(cntSec,secNum);
}

//FAT块缓存的读写
void load_fat(int secNum){
  //切换扇区时才真正从软盘加载
  if(_cntFatNum!=secNum){
    if(_cntFatNum!=-1) save_fat(_cntFatNum);//如果不是第一次加载 就保存原缓存扇区
    //加载扇区、更新目录块号
    io_load_sec(fatSec,secNum);
    _cntFatNum=secNum;
  }
}

void save_fat(int secNum){
  io_save_sec(fatSec,secNum);
}

int read_fat(int clusNum){
  int fatByte=clusNum/2*3;
  load_fat(fatByte/nSec+FAT1_SEC);
  fatByte%=nSec;
  int nextClus=0;
  if(clusNum%2==0){
    nextClus=fatSec[fatByte]|((fatSec[fatByte+1]&0x0F)<<8);
  }
  else{
    nextClus=(fatSec[fatByte+2]<<4)|(fatSec[fatByte+1]>>4);
  }
  return nextClus;
}

void write_fat(int clusNum,int nextClus){
  int fatByte=clusNum/2*3;
  int secNum=fatByte/nSec+FAT1_SEC;
  load_fat(secNum);
  fatByte%=nSec;
  if(clusNum%2==0){
    fatSec[fatByte]=(BYTE)(nextClus&0xFF);
    fatSec[fatByte+1]&=0xF0;
    fatSec[fatByte+1]|=(BYTE)((nextClus>>8)&0xF);
  }
  else{
    fatSec[fatByte+2]=(BYTE)((nextClus>>4)&0xFF);
    fatSec[fatByte+1]&=0x0F;
    fatSec[fatByte+1]|=(BYTE)((nextClus&0xF)<<4);
  }
}

int pop_free_clus(){
  if(_headFreeClus==-1){
    for(int i=2;i<num2clus(BPB_TOT_SEC16);i++){
      if(read_fat(i)==0){
        write_fat(i,0xFFF);
        clear_clus(i);
        return i;
      }
    }
  }
  else{
    int popClus=_headFreeClus;
    _headFreeClus=read_fat(_headFreeClus);
    if(_headFreeClus==0xFFF){
       _headFreeClus=-1;
       _lastFreeClus=-1;
    }
    write_fat(popClus,0xFFF);
    clear_clus(popClus);
    return popClus;
  }
  return -1;
}

void push_free_clus(int freeClus){
  if(_headFreeClus==-1){
    _headFreeClus=freeClus;
  }
  else{
    write_fat(_lastFreeClus,freeClus);
  }
  int cntClus=freeClus;
  int nextClus;
  while((nextClus=read_fat(cntClus))!=0xFFF) cntClus=nextClus;
  _lastFreeClus=cntClus;
}

void clear_clus(int clusNum){
  clusNum=clus2num(clusNum);
  load_fat(clusNum);
  mem_set(fatSec,nSec,0);
}

void clear_free_clus(){
  while(_headFreeClus!=-1){
    int cntClus=pop_free_clus();
    write_fat(cntClus,0);
    clear_clus(cntClus);
  }
}

SEC_PTR next_sec(FLAG MODE){
  if(clus2num(_cntDir.DIR_FST_CLUS)==ROOT_SEC){
    if(_cntSecNum+1>=DATA_SEC) return NULL;
    load_sec(_cntSecNum+1);
    return (SEC_PTR)&cntSec;
  }
  else{
    int nextClus=read_fat(num2clus(_cntSecNum));
    if(nextClus==0xFF0) return NULL;//下一块为坏块
    if(nextClus==0x000) return NULL;//
    if(nextClus==0xFFF){
      if(MODE==READ) return NULL;
      else{
        nextClus=pop_free_clus();//
        if(nextClus==-1){
          return NULL;//申请失败
        }
        write_fat(num2clus(_cntSecNum),nextClus);
      }
    }
    load_sec(clus2num(nextClus));
    return (SEC_PTR)&cntSec;
  }
}

//初始化FAT系统
void start_fat(){
  //设置为根目录文件索引
  mem_cpy(_cntDir.DIR_NAME,len_str("ROOT",8)+1,"ROOT");
  _cntDir.DIR_FST_CLUS=num2clus(ROOT_SEC);
  _cntDir.DIR_ATTR=DIR_ITEM;
  //设置为根目录首块
  _cntSecNum=-1;//表示当前没有扇区加载到目录块缓存
  load_sec(ROOT_SEC);
  _cntFatNum=-1;//表示当前没有扇区加载到FAT块缓存
  load_fat(FAT1_SEC);
  //初始化空闲块链
  _headFreeClus=-1;
  _lastFreeClus=-1;
}

void close_fat(){
  //保存最后一次的 扇区缓存 到软盘中
  save_fat(_cntFatNum);
  save_sec(_cntSecNum);
  //
  backup_fat();
}

void backup_fat(){
  for(int i=0;i<9;i++){
    io_load_sec(fatSec,FAT1_SEC+i);
    io_save_sec(fatSec,FAT2_SEC+i);
  }
}