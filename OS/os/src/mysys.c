#include "cglobal.h"

char cntFlp[]="C:\\";
char userName[MAXLEN]="GXL";
FLAG isroot=0;

WORD COM_PSP=0x0100;
WORD COM_SEG=0x2000;
PCB PCB_LIST[MAX_PROCESS]={0};
WORD CURRENTPID=0;//0:core 1-8:user

//内部系统时间
TIME_DATE SYS_TIME={
  .year=2020,.month=04,.day=06,
  .hour=13,.minute=57,.sec=58
};

TIME_DATE* get_sys_time(){
  SYS_TIME.year-=2000;
  upd_sys_time();//bcd
  SYS_TIME.year=GET_BCD(SYS_TIME.year);
  SYS_TIME.year+=2000;
  SYS_TIME.month=GET_BCD(SYS_TIME.month);
  SYS_TIME.day=GET_BCD(SYS_TIME.day);
  SYS_TIME.hour=GET_BCD(SYS_TIME.hour);
  SYS_TIME.minute=GET_BCD(SYS_TIME.minute);
  SYS_TIME.sec=GET_BCD(SYS_TIME.sec);
  return &SYS_TIME;
}

void print_data(BYTE* data,int rangeLen,FLAG MODE){
  if(MODE==TEX){
    for(int i=0;i<rangeLen;i++){
      if(data[i]=='\r'){
        if(i+1<rangeLen&&data[i+1]!='\n') put_char('\n');
      }
      else put_char(data[i]);
    }
  }
  else{
    for(int i=0;i<rangeLen;i++){
      print_num(data[i],3);
      put_char(' ');
      if((i+1)%32==0) put_char('\n');
    }
  }
}

int start_sys(){
  io_init();
  start_fat();
  init_sys_list();
  init_file_list(THIS_PROCESS);
  print_str("# START FAT SYSTEM SUCCESSFUL!\n",0);
  return true;
}

int close_sys(){
  close_fat();
  close_file_list(THIS_PROCESS);
  clear_free_clus();
  print_str("# CLOSE FAT SYSTEM SUCCESSFUL!\n",0);
  return true;
}

//为每个程序分配PCB，然后将每个程序加载到内存的相应位置。跳过无法加载的程序。
void _load(int clusNum,char* fileName){
  //分配PCB
  int pid=1;
  while(pid!=MAX_PROCESS&&PCB_LIST[pid].pid!=0) pid++;
  if(pid==MAX_PROCESS){
    print_str("# TOO MUCH EXECUTED!\n",0);
    return;
  }
  //指定加载位置
  COM_SEG=0x1000*(pid+1);
  //根据首扇区加载所有扇区
  int i=0;
  while(clusNum!=0xFFF){
    if(i>40){
      print_str("# COM TO LARGE!\n",0);
      break;
    }
    if(clusNum==0||clus2num(clusNum)==-1){
      print_str("# BROKEN COM!\n",0);
      i=41;
      break;
    }
    COM_LBA=clus2num(clusNum)-1;
    COM_PTR=(void*)(COM_PSP+0x0200*i);
    load_com();
    clusNum=read_fat(clusNum);
    i++;
  }
  //加载成功
  if(i<=40){
    //加入进程队列
    PCB_LIST[pid].pid=pid;
    PCB_LIST[pid].pstate=0;
    mem_cpy(PCB_LIST[pid].pname,8,fileName);
    PCB_LIST[pid].cpuRegs.EFLAGS=1<<9;//IF=1 开启硬件中断
    //CS:IP为程序起始位置
    PCB_LIST[pid].cpuRegs.IP=COM_PSP;
    PCB_LIST[pid].cpuRegs.CS=COM_SEG;
    PCB_LIST[pid].cpuRegs.DS=COM_SEG;
    PCB_LIST[pid].cpuRegs.ES=COM_SEG;
    PCB_LIST[pid].cpuRegs.SS=COM_SEG;
    PCB_LIST[pid].cpuRegs.SP=0xFFF0;
    PCB_LIST[pid].cpuRegs.BP=0xFFF2;、
    //创建PSP
    create_psp();
  }
}

int _exec(){
  sched_com();
  if(CURRENTPID!=0){
    exec_com();
    PCB_LIST[CURRENTPID].pid=0;
  }
  return CURRENTPID;
}

void sched_com(){
  PCB_LIST[CURRENTPID].pstate=0;
  for(int i=(CURRENTPID+1)%MAX_PROCESS;i!=CURRENTPID;i=(i+1)%MAX_PROCESS){
    if(PCB_LIST[i].pid!=0){
      CURRENTPID=i;
      break;
    }
  }
  //如果没有其他进程 则CURRENTPID不变
  if(PCB_LIST[CURRENTPID].pid==0) CURRENTPID=0;
  else PCB_LIST[CURRENTPID].pstate=1;
}

//根据路径和打开方式 打开文件 返回进程文件打开表项的指针
MYFILE* user_open(char* path,int mode){
  cover_cat(NULL,NULL);
  dirItem* dirPtr=path2ptr(path,false);
  if(dirPtr==NULL||dirPtr==ROOT_DIR||dirPtr->DIR_ATTR==DIR_ITEM){
    recover_cat(NULL,NULL);
    return NULL;
  }
  MYFILE* fp=open_file(&_cntDir,dirPtr,mode);
  recover_cat(NULL,NULL);
  return fp;
}

//根据进程文件打开表项的指针 读取文件当前位置的长度为size的内容到buf中
int user_read(char* buf,int size,MYFILE* fp){
  return read_file(buf,1,size,fp);
}

//根据进程文件打开表项的指针 将长度为size的内容从buf中写入文件当前位置
int user_write(char* buf,int size,MYFILE* fp){
  return write_file(buf,1,size,fp);
}

//根据进程文件打开表项的指针 在进程文件打开表中关闭文件
void user_close(MYFILE* fp){
  close_file(fp);
}
