#include "cglobal.h"

char SELF_DIR[]=".";
char UP_DIR[]="..";
char EXE_FILE[]="com";

void upd_cat_dir(){
  if(clus2num(_cntDir.DIR_FST_CLUS)!=ROOT_SEC){
    dirItem oldDir;
    int oldSec;
    cover_cat(&oldDir,&oldSec);

    int upDirSec=_cntDir.DIR_FST_CLUS;//识别同一文件 通过首目录块号
    dirItem* dirPtr=(dirItem*)&cntSec+1;//指向UP_DIR
    open_cat(dirPtr,false);
    dirPtr=(dirItem*)&cntSec;
    while(dirPtr){
      for(int i=0;i<nSec/sizeof(dirItem);i++){
        if(all_zero((char*)(dirPtr+i),sizeof(dirItem))){
          dirPtr=NULL;
          break;
        }
        if(*(BYTE*)(dirPtr+i)==FREE_ITEM) continue;
        if(dirPtr[i].DIR_FST_CLUS==upDirSec){
          time2num(get_sys_time(),&(dirPtr[i].DIR_WRI_TIME),&(dirPtr[i].DIR_WRI_DATE));
          //save_sec(_cntSecNum);
          dirPtr=NULL;
          break;
        }
      }
      if(dirPtr) dirPtr=(dirItem*)next_sec(READ);
    }
    recover_cat(&oldDir,&oldSec);
  }
}

//
dirItem* get_up_dir(){
  if(clus2num(_cntDir.DIR_FST_CLUS)==ROOT_SEC) return NULL;
  else{
    open_cat(&_cntDir,false);//回到目录第一块
    dirItem* dirPtr=(dirItem*)cntSec+1;
    int upDirSec=_cntDir.DIR_FST_CLUS;//识别同一文件 通过首目录块号
    open_cat(dirPtr,false);//回到上层 通过“..”
    while(dirPtr){
      for(int i=0;i<nSec/sizeof(dirItem);i++){
        if(all_zero((char*)(dirPtr+i),sizeof(dirItem))){
          dirPtr=NULL;
          break;
        }
        if(*(BYTE*)(dirPtr+i)==FREE_ITEM) continue;
        if(dirPtr[i].DIR_FST_CLUS==upDirSec){
          return dirPtr+i;
        }
      }
      if(dirPtr) dirPtr=(dirItem*)next_sec(READ);
    }
  }
}

dirItem* path2ptr(char* targetPath,FLAG CHANGE){
  if(targetPath==NULL) return NULL;
  if(targetPath[0]=='\0') return &_cntDir;
  dirItem* target=NULL;
  //绝对路径 允许. 和..
  if(check_str(targetPath,len_str(cntFlp,0),cntFlp)){
    if(clus2num(_cntDir.DIR_FST_CLUS)!=ROOT_SEC) open_cat(ROOT_DIR,CHANGE);
    target=_path2ptr(targetPath,len_str(cntFlp,0),CHANGE);
  }
  else if(targetPath[0]=='\\'){
    if(clus2num(_cntDir.DIR_FST_CLUS)!=ROOT_SEC) open_cat(ROOT_DIR,CHANGE);
    if(targetPath[1]=='\0') target=ROOT_DIR;
    else target=_path2ptr(targetPath,1,CHANGE);
  }
  else{
    //相对路径
    target=_path2ptr(targetPath,0,CHANGE);
  }

  if(target==NULL){
    print_str("# WRONG PATH: ",0);
    print_str(targetPath,0);
    put_char('\n');
  }
  
  return target;
}

dirItem* _path2ptr(char* targetPath,int posi,FLAG CHANGE){
  if(targetPath==NULL) return NULL;
  if(targetPath[posi]=='\0') return &_cntDir;//这种情况应该不会出现
  if(targetPath[posi]=='\\') return NULL;//不允许多余的'\'
  //根目录下.和..
  if(clus2num(_cntDir.DIR_FST_CLUS)==ROOT_SEC&&targetPath[posi]=='.'){
    if(targetPath[posi+1]=='\\'){
      return _path2ptr(targetPath,posi+2,CHANGE);//跳过'\'
    }
    else if(targetPath[posi+1]=='\0'){
      return ROOT_DIR;
    }
    else return NULL;
  }

  int len=0;
  int exLen=0;
  while(targetPath[posi+len]!='\0'&&targetPath[posi+len]!='\\'){
    if((len>=1&&targetPath[posi+len-1]!='.')&&targetPath[posi+len]=='.') break;//*.
    len++;
  }
  if(targetPath[posi+len]=='.'){
    while(targetPath[posi+len+1+exLen]!='\0'&&targetPath[posi+len+1+exLen]!='\\'){
      exLen++;
    }
    if(targetPath[posi+len+1+exLen]=='\\'){
      return NULL;
    }
  }

  dirItem* dirPtr=(dirItem*)&cntSec;
  while(dirPtr){
    for(int i=0;i<nSec/sizeof(dirItem);i++){
      if(all_zero((char*)(dirPtr+i),sizeof(dirItem))){
        dirPtr=NULL;
        break;
      }
      if(*(BYTE*)(dirPtr+i)==FREE_ITEM) continue;
      if(check_str(targetPath+posi,max(len,len_str(dirPtr[i].DIR_NAME,8)),dirPtr[i].DIR_NAME)){
        if(dirPtr[i].DIR_ATTR!=DIR_ITEM){
          if(check_str(targetPath+posi+len+1,max(exLen,len_str(dirPtr[i].DIR_EX_NAME,3)),dirPtr[i].DIR_EX_NAME)){
            return dirPtr+i;
          }//扩展名匹配 扩展名可以为空
          else continue;//扩展名不匹配
        }//普通文件类型匹配

        if(exLen!=0&&dirPtr[i].DIR_ATTR==DIR_ITEM) continue;//文件类型不匹配


        else{
          if(targetPath[posi+len]=='\0'&&dirPtr[i].DIR_ATTR==DIR_ITEM){
            return dirPtr+i;
          }//最后一级目录
          else if(targetPath[posi+len]=='\\'&&targetPath[posi+len+1]=='\0'&&dirPtr[i].DIR_ATTR==DIR_ITEM){
            return dirPtr+i;
          }//最后一级目录
          else{
            open_cat(dirPtr+i,CHANGE);
            return _path2ptr(targetPath,posi+len+1,CHANGE);//前面已判断过'\0'，到这里肯定是'\'
          }//下一级目录
        }//目录类型匹配
      }
    }
    if(dirPtr) dirPtr=(dirItem*)next_sec(READ);
  }
  return NULL;
}

void ini_cat(dirItem* targetDir){
  int upDirClus=_cntDir.DIR_FST_CLUS;
  dirItem oldDir;
  int oldSec;
  cover_cat(&oldDir,&oldSec);
  open_cat(targetDir,false);
  dirItem* dirPtr=(dirItem*)&cntSec;
  //"."
  mem_cpy(dirPtr[0].DIR_NAME,len_str(SELF_DIR,7)+1,SELF_DIR);
  dirPtr[0].DIR_ATTR=DIR_ITEM;
  time2num(get_sys_time(),&(dirPtr[0].DIR_WRI_TIME),&(dirPtr[0].DIR_WRI_DATE));
  dirPtr[0].DIR_FST_CLUS=_cntDir.DIR_FST_CLUS;
  //".."
  mem_cpy(dirPtr[1].DIR_NAME,len_str(UP_DIR,7)+1,UP_DIR);
  dirPtr[1].DIR_ATTR=DIR_ITEM;
  time2num(get_sys_time(),&(dirPtr[1].DIR_WRI_TIME),&(dirPtr[1].DIR_WRI_DATE));
  dirPtr[1].DIR_FST_CLUS=upDirClus;

  //save_sec(_cntSecNum);

  recover_cat(&oldDir,&oldSec);
}

void cover_cat(dirItem* targetDir,int* targetSec){
  if(targetDir==NULL){
    _backDir=_cntDir;
    _backSec=_cntSecNum;
  }
  else{
    *targetDir=_cntDir;
    *targetSec=_cntSecNum;
  }
}

void recover_cat(dirItem* targetDir,int* targetSec){
  if(targetDir==NULL){
    open_cat(&_backDir,false);
    load_sec(_backSec);
  }
  else{
    open_cat(targetDir,false);
    load_sec(*targetSec);
  }
}

void open_cat(dirItem* targetDir,FLAG CHANGE){
  if(targetDir==NULL) return;
  int target=(targetDir==ROOT_DIR)?ROOT_SEC:clus2num(targetDir->DIR_FST_CLUS);
  char* tarName=(targetDir==ROOT_DIR)?NULL:targetDir->DIR_NAME;
  int catFstSec=clus2num(_cntDir.DIR_FST_CLUS);
  int readSec;
  if(target==catFstSec){
    if(readSec!=catFstSec){
      readSec=catFstSec;
    }
    else return;
  }
  else if(target==ROOT_SEC||target==0){
    readSec=ROOT_SEC;
    if(CHANGE){
      cntPath[0]='\0';
    }
  }
  else if(target>=DATA_SEC&&target<=BPB_TOT_SEC16){
    readSec=target;
    if(CHANGE){
      if(!check_str(tarName,max(len_str(tarName,8),len_str(SELF_DIR,0)),SELF_DIR)){
        if(check_str(tarName,max(len_str(tarName,8),len_str(UP_DIR,0)),UP_DIR)){
          cut_path_str(cntPath);
        }
        else{
          apd_path_str(tarName,len_str(tarName,8),cntPath,sizeof(cntPath));
        }
      }
    }
  }
  else{
    return;
  }

  if(targetDir==ROOT_DIR){
    mem_cpy(_cntDir.DIR_NAME,len_str("ROOT",8)+1,"ROOT");
    _cntDir.DIR_FST_CLUS=num2clus(ROOT_SEC);
    _cntDir.DIR_ATTR=DIR_ITEM;
  }
  else{
    _cntDir=*targetDir;
  }
  load_sec(readSec);
}

//在目标目录新建文件项 内容由tarItem确定（除了分配空间） 返回块内文件指针
//会检查同名文件
//副作用：改变目录 必要时注意恢复
dirItem* creat_item(dirItem* tarItem,dirItem* tarCat){
  open_cat(tarCat,false);
  dirItem* dirPtr=(dirItem*)&cntSec;
  //检查同名
  while(dirPtr){
    for(int i=0;i<nSec/sizeof(dirItem);i++){
      if(all_zero((char*)(dirPtr+i),sizeof(dirItem))){
        dirPtr=NULL;
        break;
      }
      if(*(BYTE*)(dirPtr+i)==FREE_ITEM) continue;

      if(tarItem->DIR_ATTR==DIR_ITEM&&dirPtr[i].DIR_ATTR==DIR_ITEM){
        if(check_str(dirPtr[i].DIR_NAME,max(len_str(dirPtr[i].DIR_NAME,8),len_str(tarItem->DIR_NAME,8)),tarItem->DIR_NAME)){
          print_str("# FILE ",0);
          print_str(tarItem->DIR_NAME,len_str(tarItem->DIR_NAME,8));
          print_str(" EXISTED!\n",0);
          return NULL;
        }
      }
      else if(tarItem->DIR_ATTR!=DIR_ITEM&&dirPtr[i].DIR_ATTR!=DIR_ITEM){
        if(check_str(dirPtr[i].DIR_NAME,max(len_str(dirPtr[i].DIR_NAME,8),len_str(tarItem->DIR_NAME,8)),tarItem->DIR_NAME)){
          if(check_str(dirPtr[i].DIR_EX_NAME,max(len_str(dirPtr[i].DIR_EX_NAME,3),len_str(tarItem->DIR_EX_NAME,3)),tarItem->DIR_EX_NAME)){
            print_str("# FILE ",0);
            print_str(tarItem->DIR_NAME,len_str(tarItem->DIR_NAME,8));
            put_char('.');
            print_str(tarItem->DIR_EX_NAME,len_str(tarItem->DIR_EX_NAME,3));
            print_str(" EXISTED!\n",0);
            return NULL;
          }
        }
      }

    }
    if(dirPtr) dirPtr=(dirItem*)next_sec(READ);
  }
  //新建
  open_cat(&_cntDir,false);//返回当前目录第一块
  dirPtr=(dirItem*)&cntSec;
  while(dirPtr){
    for(int i=0;i<nSec/sizeof(dirItem);i++){
      if(all_zero((char*)(dirPtr+i),sizeof(dirItem))){
        dirPtr[i]=*tarItem;//除了空间分配 其他信息由外部决定
        dirPtr[i].DIR_SIZE=0;
        dirPtr[i].DIR_FST_CLUS=pop_free_clus();
        //save_sec(_cntSecNum);
        if(tarItem->DIR_ATTR==DIR_ITEM) ini_cat(dirPtr+i);
        upd_cat_dir();
        *tarItem=dirPtr[i];//返回更改
        return dirPtr+i;
      }
      if(*(BYTE*)(dirPtr+i)==FREE_ITEM){
        push_free_clus(dirPtr[i].DIR_FST_CLUS);//先push！不然丢失首扇区信息
        dirPtr[i]=*tarItem;
        dirPtr[i].DIR_SIZE=0;
        dirPtr[i].DIR_FST_CLUS=pop_free_clus();
        //save_sec(_cntSecNum);
        if(tarItem->DIR_ATTR==DIR_ITEM) ini_cat(dirPtr+i);
        upd_cat_dir();
        *tarItem=dirPtr[i];//返回更改
        return dirPtr+i;
      }
    }
    if(dirPtr){
      dirPtr=(dirItem*)next_sec(WRITE);//不是根目录需要申请新的块
      if(dirPtr==NULL){
        print_str("#FAILED:CATALOG FULL OR SOMETHING WRONG!\n",0);
        return NULL;
      }
    }
  }

  return NULL;
}

//targetItem是已加载到内存的文件项信息 不在加载扇区中
void save_item(dirItem* tarItem,dirItem* tarCat){
  dirItem oldDir;
  int oldSec;
  cover_cat(&oldDir,&oldSec);

  open_cat(tarCat,false);
  dirItem* dirPtr=(dirItem*)&cntSec;
  while(dirPtr){
    for(int i=0;i<nSec/sizeof(dirItem);i++){
      if(all_zero((char*)(dirPtr+i),sizeof(dirItem))){
        dirPtr=NULL;
        break;
      }
      if(*(BYTE*)(dirPtr+i)==FREE_ITEM) continue;
      if(dirPtr[i].DIR_FST_CLUS==tarItem->DIR_FST_CLUS){
        dirPtr[i]=*tarItem;
        //save_sec(_cntSecNum);
        upd_cat_dir();
        dirPtr=NULL;
        break;
      }
    }
    if(dirPtr) dirPtr=(dirItem*)next_sec(READ);
  }

  recover_cat(&oldDir,&oldSec);
}

//复制：将原文件拼接到目标文件
//输入：tarCat:目标文件所在目录 newItem:目标文件
//输入：srcCat:源文件所在目录 srcItem:源文件
//输出成功与否
int copy_item_data(dirItem* tarCat,dirItem* newItem,dirItem* srcCat,dirItem* srcItem){
  MYFILE* srcFp=open_file(srcCat,srcItem,READ);
  if(srcFp==NULL) return false;
  MYFILE* tarFp=open_file(tarCat,newItem,WRITE);
  if(tarFp==NULL){
    close_file(srcFp);
    return false;
  }
  seek_file(tarFp,_file_size(tarFp),0);
  BYTE data[nSec];
  int validSize=read_file(data,1,nSec,srcFp);
  while(validSize!=0){
    if(write_file(data,1,validSize,tarFp)!=validSize){
      print_str("# FAILED IN COPYING, COPY WILL BE REMOVED!\n",0);
      return false;//复制失败 需要回收
    }
    validSize=read_file(data,1,nSec,srcFp);
  }
  close_file(tarFp);
  close_file(srcFp);
  return true;
}

void display_item(dirItem* dirPtr){
  int i=0;
  print_str(dirPtr[i].DIR_NAME,8);
  print_str(dirPtr[i].DIR_EX_NAME,3);
  if(dirPtr[i].DIR_ATTR!=DIR_ITEM){
    print_str("       ",0);
    print_num(dirPtr[i].DIR_SIZE,7);
  }
  else{
    print_str(" <DIR> ",0);
    print_str("       ",0);
  }

  TIME_DATE timeDate;
  num2time(&timeDate,dirPtr[i].DIR_WRI_TIME,dirPtr[i].DIR_WRI_DATE);
  put_char(' ');
  print_num(timeDate.year,4);
  put_char('-');
  print_num(timeDate.month,2);
  put_char('-');
  print_num(timeDate.day,2);
  put_char(' ');
  print_num(timeDate.hour,2);
  put_char(':');
  print_num(timeDate.minute,2);
  put_char(':');
  print_num(timeDate.sec,2);
  put_char(' ');
  put_char('\n');
}
