#include "cglobal.h"

MYFILE* open_file(dirItem* catDir,dirItem* targetDir,FLAG MODE){
  SYSFILE* aFile=check_sys_file(targetDir);//只检查系统活动文件表 认为进程不会重复打开
  if(aFile==NULL){
    aFile=free_sys_file();
    if(aFile==NULL){
      print_str("# TOO MUCH FILE OPEND IN SYSTEM!\n",0);
      return NULL;
    }
    else{
      set_sys_file(aFile,catDir,targetDir);
    }
  }
  else{
    aFile->shareCnt++;
  }
  MYFILE* mfp=free_file(THIS_PROCESS);
  if(mfp==NULL){
    print_str("# TOO MUCH FILE OPEND IN PROCESS!\n",0);
    return NULL;
  }
  else{
    set_file(mfp,aFile,MODE);
    return mfp;
  }
}

int close_file(MYFILE* targetFile){
  //认为该文件已经打开 不会出现文件没打开的情况
  SYSFILE* aFile=targetFile->sysFile;
  aFile->shareCnt--;
  if(aFile->shareCnt==0){
    clear_sys_file(aFile);
  }
  clear_file(targetFile);
  return true;
}

SYSFILE* check_sys_file(dirItem* targetDir){
  for(int i=0;i<MAX_SYS_FILE;i++){
    if(_sysFile[i].shareCnt!=0&&_sysFile[i].fileDir.DIR_FST_CLUS==targetDir->DIR_FST_CLUS){
      return _sysFile+i;
    }
  }
  return NULL;
}

SYSFILE* free_sys_file(){
  for(int i=0;i<MAX_SYS_FILE;i++){
    if(_sysFile[i].shareCnt==0){
      return _sysFile+i;
    }
  }
  return NULL;
}

int set_sys_file(SYSFILE* aFile,dirItem* catDir,dirItem* targetDir){
  aFile->catDir=*catDir;
  aFile->fileDir=*targetDir;
  aFile->shareCnt=1;
  //afile->fileBuffer=
  get_buffer(aFile,1,READ);//预读
}

//在shareCnt=0后进行
int clear_sys_file(SYSFILE* aFile){
  clear_all_buffer(aFile);
  save_item(&aFile->fileDir,&aFile->catDir);
  //aFile->fileBuffer=NULL;
  return true;
}

MYFILE* free_file(){
  for(int i=0;i<MAX_FILE;i++){
    if(_myFile[THIS_PROCESS][i].sysFile==NULL){
      return _myFile[THIS_PROCESS]+i;
    }
  }
  return NULL;
}

int set_file(MYFILE* mfp,SYSFILE* aFile,FLAG MODE){
  mfp->posi=0;
  mfp->MODE=MODE;
  mfp->sysFile=aFile;
  return true;
}

int clear_file(MYFILE* mfp){
  mfp->sysFile=NULL;
  return true;
}

//仅用于进程已打开的文件
int _file_size(MYFILE* mfp){
  return mfp->sysFile->fileDir.DIR_SIZE;
}


void init_file_list(PROCESS pc){
  for(int i=0;i<MAX_FILE;i++) _myFile[pc][i].sysFile=NULL;
}

void init_sys_list(){
  for(int i=0;i<MAX_SYS_FILE;i++) _sysFile[i].shareCnt=0;
  for(int i=0;i<MAX_BUFFER_NUM;i++) _fileBuffer[i].sysFile=NULL;
  _bufferHead=NULL;
}

void close_file_list(PROCESS pc){
  for(int i=0;i<MAX_FILE;i++){
    if(_myFile[pc][i].sysFile!=NULL) close_file(_myFile[pc]+i);
  }
}

//后面是缓冲区管理函数

//认为fstClus不是0xFFF，新建文件时至少分配一个数据块，虽然size可以为0
//返回当前clus而不是下一clus
//对于WRITE模式 若当前对应的簇是结束簇（无内容） 则申请新的簇 所以应确保上一簇已被写满
int buffer2clus(int bufferNum,int fstClus,FLAG MODE){
  int clusNum=fstClus;
  for(int i=1;i<bufferNum;i++){
    int nextClus=read_fat(clusNum);
    if(nextClus==0XFFF){
      if(MODE==WRITE){
        nextClus=pop_free_clus();
        if(nextClus!=-1) write_fat(clusNum,nextClus);
      }
      return nextClus;//READ:0xFFF WRITE:new or -1
    }
    clusNum=nextClus;
  }
  return clusNum;
}

FILE_BUFFER* get_buffer(SYSFILE* aFile,int bufferNum,FLAG MODE){
  //目标块是否存在
  int bufferClus=buffer2clus(bufferNum,aFile->fileDir.DIR_FST_CLUS,MODE);
  if(bufferClus==0xFFF||bufferClus==-1) return NULL;
  //目标块存在时 先申请free的buffer空间 再进行加载

  //链表方式 缓冲是否已存在
  FILE_BUFFER* nbfp=_bufferHead;
  //找到已加载buffer直接返回
  //头为NULL时不用查找
  while(_bufferHead){
    if(nbfp->sysFile==aFile&&nbfp->bufferNum==bufferNum) return nbfp;
    nbfp=nbfp->next;
    if(nbfp==_bufferHead) break;
  }
  //未找到时申请free buffer
  nbfp=free_buffer(aFile);//确定buffer ptr、next、sysFile
  nbfp->bufferNum=bufferNum;
  nbfp->bufferSize=min(aFile->fileDir.DIR_SIZE-(bufferNum-1)*nSec,nSec);//写入申请新空间时 为0
  load_buffer(nbfp,bufferClus);//修改buffer ptr指向的内容
  return nbfp;
}

//如果么有空闲空间，清理出一个来；更新链表
//buffer内容在返回后加载
FILE_BUFFER* free_buffer(SYSFILE* aFile){
  if(_bufferHead==NULL){
    _bufferHead=_fileBuffer;
    _bufferHead->bufferPtr=_bufferData;
    _bufferHead->sysFile=aFile;
    _bufferHead->next=_bufferHead;//循环队列
    return _bufferHead;
  }
  else{
    //查找数组相当于malloc申请新空间
    FILE_BUFFER* nbfp=NULL;
    for(int i=0;i<MAX_BUFFER_NUM;i++){
      if(_fileBuffer[i].sysFile==NULL){
        nbfp=_fileBuffer+i;
        nbfp->bufferPtr=_bufferData+nSec*i;
        nbfp->next=_bufferHead;
        FILE_BUFFER* now=_bufferHead;
        while(now->next!=_bufferHead) now=now->next;
        now->next=nbfp;
        break;
      }
    }
    if(nbfp==NULL){
      //没有缓存空间 删除队头 加入队尾
      clear_buffer(_bufferHead);
      nbfp=free_buffer(aFile);
    }
    nbfp->sysFile=aFile;
    return nbfp;
  }
}

//加载缓冲数据
int load_buffer(FILE_BUFFER* bfp,int clusNum){
  clusNum=clus2num(clusNum);
  load_fat(clusNum);
  mem_cpy(bfp->bufferPtr,nSec,fatSec);
  return true;
}

int save_buffer(FILE_BUFFER* bfp){
  int secnum=clus2num(buffer2clus(bfp->bufferNum,bfp->sysFile->fileDir.DIR_FST_CLUS,READ));
  load_fat(secnum);
  mem_cpy(fatSec,nSec,bfp->bufferPtr);
  //save_fat(secnum);//add 5/25
  return true;
}

int clear_all_buffer(SYSFILE* aFile){
  FILE_BUFFER* nbfp=_bufferHead;
  if(nbfp==NULL) return true;
  while(nbfp->sysFile!=aFile){
    nbfp=nbfp->next;
    if(nbfp==_bufferHead) return true;
  }
  clear_buffer(nbfp);
  return clear_all_buffer(aFile);
}

int clear_buffer(FILE_BUFFER* bfp){
  //保存内容
  save_buffer(bfp);
  //更新链表
  if(bfp==_bufferHead) _bufferHead=bfp->next;
  FILE_BUFFER* nbfp=_bufferHead;
  while(nbfp->next!=bfp) nbfp=nbfp->next;
  nbfp->next=bfp->next;
  //清空buffer项
  bfp->bufferPtr=NULL;
  bfp->sysFile=NULL;
  bfp->bufferNum=0;
  bfp->bufferSize=0;
  bfp->next=NULL;
  if(_bufferHead->next==NULL) _bufferHead=NULL;
  return true;
}

int read_file(void* data,size_t size,size_t count,MYFILE* mfp){
  if(mfp==stdin){
    if(size*count==1){
      *((char*)data)=get_char();
      return 1;
    }
    return get_str(data,size*count); 
  }

  int total=size*count;
  int finish=0;
  FILE_BUFFER* bfp=NULL;

  while(finish<total){
    bfp=get_buffer(mfp->sysFile,mfp->posi/nSec+1,READ);
    if(bfp==NULL) break;//下一块是文件末尾
    int partSize=min(bfp->bufferSize-mfp->posi%nSec,total-finish);
    mem_cpy(data+finish,partSize,bfp->bufferPtr+mfp->posi%nSec);
    finish+=partSize;
    mfp->posi+=partSize;
    if(bfp->bufferSize<nSec) break;//文件末尾在当前块内
  }

  return finish;
}

int write_file(void* data,size_t size,size_t count,MYFILE* mfp){
  if(mfp==stdout){
    if(size*count==1){
      put_char(*((char*)data));
      return 1;
    }
    return print_str(data,size*count); 
  }
  
  if(mfp->MODE==READ) return 0;

  int total=size*count;
  int finish=0;
  FILE_BUFFER* bfp=NULL;

  while(finish<total){
    bfp=get_buffer(mfp->sysFile,mfp->posi/nSec+1,WRITE);//下一块或新空间的缓存
    if(bfp==NULL){
      print_str("# LACK OF STORAGE!\n",0);
      break;
    }//申请空间失败
    int partSize=min(total-finish,nSec-mfp->posi%nSec);
    mem_cpy(bfp->bufferPtr+mfp->posi%nSec,partSize,data+finish);
    mfp->posi+=partSize;//要么在写完所有内容后的最后一块某处 要么在过程中的最后一块的最后位置（1开始）（即posi是nSec的倍数）
    if(mfp->sysFile->fileDir.DIR_SIZE<mfp->posi){
      bfp->bufferSize=+mfp->posi-mfp->sysFile->fileDir.DIR_SIZE;
      mfp->sysFile->fileDir.DIR_SIZE=mfp->posi;
    }//发生了超出范围的写入 新的长度是当前写入位置
    finish+=partSize;
  }

  time2num(get_sys_time(),&(mfp->sysFile->fileDir.DIR_WRI_TIME),&(mfp->sysFile->fileDir.DIR_WRI_DATE));
  return finish;
}

int seek_file(MYFILE* mfp,size_t offset,int fromWhere){
  if(fromWhere+offset>_file_size(mfp)) mfp->posi=_file_size(mfp);
  else mfp->posi=fromWhere+offset;
}

int _file_posi(MYFILE* mfp){
  return mfp->posi;
}
