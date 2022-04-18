#include "cglobal.h"

char cntPath[MAXLEN]="";//暂不考虑超出

//目录管理
char* CMD_CD="cd";//打开目录 支持路径
char* CMD_LS="ls";//列目录 支持路径
char* CMD_TREE="tree";//列目录树 支持路径
char* CMD_MD="md";//新建目录 支持路径
char* CMD_RD="rd";//删除目录 支持路径
//文件管理
char* CMD_RM="rm";//删除文件 支持路径
char* CMD_NEW="new";//新建文件 支持路径
char* CMD_READ="read";//打开文件 支持路径
char* CMD_COPY="copy";//复制文件 支持路径
//退出系统
char* CMD_CLOSE="close";//关闭系统
//其他
char* CMD_HELP="help";
char* CMD_ECHO="echo";//回音
char* CMD_SU="su";//切换用户
char* CMD_CLS="cls";

char* CMD_EXEC="exec";

void run(){
  while(true){
    cur_set_attr(0x03);
    print_str("[@",0);
    print_str(userName,0);
    print_str("] ",0);
    print_str(cntFlp,0);
    print_str(cntPath,0);
    print_str("> ",0);

    get_str(cmdLine,sizeof(cmdLine));//获取命令行
    cur_set_attr(0x06);
    argsNum=div_str(cmdArgs,argsRec,cmdLine);//获取参数个数，将命令划分为若干参数
    if(argsNum){
      put_char('\n');
      int argLen=len_str(cmdArgs+argsRec[0],0);
      if(check_str(cmdArgs+argsRec[0],max(argLen,len_str(CMD_ECHO,0)),CMD_ECHO)){
        for(int i=1;i<argsNum;i++){
          print_str(cmdArgs+argsRec[i],0);
        }
      }
      else if(check_str(cmdArgs+argsRec[0],max(argLen,len_str(CMD_LS,0)),CMD_LS)){
        cmd_ls();
      }
      else if(check_str(cmdArgs+argsRec[0],max(argLen,len_str(CMD_TREE,0)),CMD_TREE)){
        cmd_tree();
      }
      else if(check_str(cmdArgs+argsRec[0],max(argLen,len_str(CMD_CD,0)),CMD_CD)){
        cmd_cd();
      }
      else if(check_str(cmdArgs+argsRec[0],max(argLen,len_str(CMD_MD,0)),CMD_MD)){
        cmd_md();
      }
      else if(check_str(cmdArgs+argsRec[0],max(argLen,len_str(CMD_RD,0)),CMD_RD)){
        cmd_rd();
      }
      else if(check_str(cmdArgs+argsRec[0],max(argLen,len_str(CMD_READ,0)),CMD_READ)){
        cmd_read();
      }
      else if(check_str(cmdArgs+argsRec[0],max(argLen,len_str(CMD_NEW,0)),CMD_NEW)){
        cmd_new();
      }
      else if(check_str(cmdArgs+argsRec[0],max(argLen,len_str(CMD_RM,0)),CMD_RM)){
        cmd_rm();
      }
      else if(check_str(cmdArgs+argsRec[0],max(argLen,len_str(CMD_COPY,0)),CMD_COPY)){
        cmd_copy();
      }
      else if(check_str(cmdArgs+argsRec[0],max(argLen,len_str(CMD_CLS,0)),CMD_CLS)){
        cmd_cls();
      }
      else if(check_str(cmdArgs+argsRec[0],max(argLen,len_str(CMD_SU,0)),CMD_SU)){
        cmd_su();
      }
      else if(check_str(cmdArgs+argsRec[0],max(argLen,len_str(CMD_EXEC,0)),CMD_EXEC)){
        cmd_exec();
      }
      else if(check_str(cmdArgs+argsRec[0],max(argLen,len_str(CMD_HELP,0)),CMD_HELP)){
        if(check_arg(1,1)){
          mem_cpy(cmdArgs+argsRec[1],len_str("\\help.syc",0),"\\help.syc");
          argsNum=2;
          cmd_read();
        }
      }
      else if(check_str(cmdArgs+argsRec[0],max(argLen,len_str(CMD_CLOSE,0)),CMD_CLOSE)){
        if(check_arg(1,1)){
          print_str("# CLOSE?[y/n]y\b",0);
          if(!check_char(my_getc(),'n')){
            print_str("# CLOSING...\n",0);
            return;
          }
        }
      }
      else{
        print_str("#BAD COMMAND!\n",0);
        print_str("#SEE: help\n",0);
      }
      put_char('\n');
    }
  }
}

int check_arg(int least,int most){
  if(argsNum>most){
    print_str("# TOO MANY PARAMETERS!\n",0);
    return false;
  }
  if(argsNum<least){
    print_str("# TOO FEW PARAMETERS!\n",0);
    return false;
  }
  return true;
}

void cmd_ls(){
  if(!check_arg(1,2)) return;

  cover_cat(NULL,NULL);//备份当前目录
  dirItem* dirPtr;
  int dirNum=0;

  if(argsNum==1) dirPtr=&_cntDir;
  else dirPtr=path2ptr(cmdArgs+argsRec[1],false);//获取路径指向的目录文件

  if(dirPtr==NULL){
    recover_cat(NULL,NULL);
    return;
  }
  else if(dirPtr!=ROOT_DIR&&dirPtr->DIR_ATTR!=DIR_ITEM){
    print_str("# FILE NOT DIRECTOR!\n",0);
    recover_cat(NULL,NULL);
    return;
  }
  open_cat(dirPtr,false);//打开目录文件
	//遍历目录文件并打印目录中文件项信息
  dirPtr=(dirItem*)&cntSec;
  print_str(" NAME ",8);
  print_str(" EX ",5);
  print_str(" TYPE ",7);
  print_str(" SIZE ",8);
  print_str(" DATE ",10);
  print_str(" TIME ",5);
  put_char('\n');
  while(dirPtr!=NULL){
    for(int i=0;i<nSec/sizeof(dirItem);i++){
      if(all_zero((char*)(dirPtr+i),sizeof(dirItem))){
        dirPtr=NULL;
        break;
      }
      if(*(BYTE*)(dirPtr+i)==FREE_ITEM) continue;
      if(!isroot&&dirPtr[i].DIR_ATTR==SYS_ITEM) continue;
      dirNum++;
      display_item(dirPtr+i);
    }
    if(dirPtr) dirPtr=(dirItem*)next_sec(READ);
  }
  print_num(dirNum,0);
  print_str(" files\n",0);

  recover_cat(NULL,NULL);//恢复目录
}

void cmd_cd(){
  if(!check_arg(1,2)) return;

  cover_cat(NULL,NULL);
  dirItem* dirPtr=path2ptr(cmdArgs+argsRec[1],false);
  if(dirPtr==NULL){
    recover_cat(NULL,NULL);
    return;
  }
  else if(dirPtr!=ROOT_DIR&&dirPtr->DIR_ATTR!=DIR_ITEM){
    print_str("# NOT DIR!\n",0);
    recover_cat(NULL,NULL);
    return;
  }
  else{
    recover_cat(NULL,NULL);
    dirPtr=path2ptr(cmdArgs+argsRec[1],true);
    open_cat(dirPtr,true);
  }

}

void cmd_tree(){
  if(!check_arg(1,2)) return;

  dirItem oldDir;
  int oldSec;
  cover_cat(&oldDir,&oldSec);
  dirItem* dirPtr;

  if(argsNum==1) dirPtr=&_cntDir;
  else dirPtr=path2ptr(cmdArgs+argsRec[1],false);

  if(dirPtr==NULL){
    recover_cat(&oldDir,&oldSec);
    return;
  }
  if(dirPtr!=ROOT_DIR&&dirPtr->DIR_ATTR!=DIR_ITEM){
    print_str("# NOT DIR!\n",0);
    recover_cat(&oldDir,&oldSec);
    return;
  }

  _cmd_tree(dirPtr,0);

  recover_cat(&oldDir,&oldSec);
}

void _cmd_tree(dirItem* targetDir,int depth){
  if(targetDir==NULL) return;
  if(depth==0){
    if(targetDir==ROOT_DIR||targetDir->DIR_FST_CLUS==num2clus(ROOT_SEC)){
      print_str(cntFlp,0);
      print_str(cntPath,0);
      put_char('\n');
    }
    else{
      print_str(cntFlp,0);
      print_str("..\\",0);
      print_str(targetDir->DIR_NAME,len_str(targetDir->DIR_NAME,8));
      print_str("\\\n",0);
    }
    _cmd_tree(targetDir,1);
  }
  else{
    if(targetDir!=ROOT_DIR&&targetDir->DIR_ATTR!=DIR_ITEM&&depth>1){
      if(((targetDir->DIR_ATTR==SYS_ITEM)&&0)||targetDir->DIR_ATTR==NORMAL_ITEM||targetDir->DIR_ATTR==OTHER_ITEM){
        for(int i=0;i<depth-2;i++) print_str("|   ",0);
        print_str("|--",0);
        print_str(targetDir->DIR_NAME,len_str(targetDir->DIR_NAME,8));
        int exLen=len_str(targetDir->DIR_EX_NAME,3);
        if(exLen){
          put_char('.');
          print_str(targetDir->DIR_EX_NAME,exLen);
          put_char('\n');
        }
      }
    }
    else{
      if(targetDir!=ROOT_DIR&&targetDir->DIR_FST_CLUS!=num2clus(ROOT_SEC)&&depth>1){
        for(int i=0;i<depth-2;i++) print_str("|   ",0);
        print_str("|--",0);
        print_str(targetDir->DIR_NAME,len_str(targetDir->DIR_NAME,8));
        print_str("\\\n",0);
      }
      dirItem oldDir;
      int oldSec;
      cover_cat(&oldDir,&oldSec);
      open_cat(targetDir,false);
      dirItem* dirPtr=(dirItem*)&cntSec;
      while(dirPtr){
        for(int i=0;i<nSec/sizeof(dirItem);i++){
          if(all_zero((char*)(dirPtr+i),sizeof(dirItem))){
            dirPtr=NULL;
            break;
          }
          if(*(BYTE*)(dirPtr+i)==FREE_ITEM) continue;
          if(check_str(dirPtr[i].DIR_NAME,max(len_str(dirPtr[i].DIR_NAME,8),len_str(SELF_DIR,0)),SELF_DIR)) continue;
          if(check_str(dirPtr[i].DIR_NAME,max(len_str(dirPtr[i].DIR_NAME,8),len_str(UP_DIR,0)),UP_DIR)) continue;
          _cmd_tree(dirPtr+i,depth+1);
        }
        if(dirPtr) dirPtr=(dirItem*)next_sec(READ);
      }
      recover_cat(&oldDir,&oldSec);
    }
  }
}

void cmd_md(){
  if(!check_arg(2,2)) return;

  cover_cat(NULL,NULL);

  char newItemName[8];
  mem_set(newItemName,8,' ');//DOS
  if(!path2name(cmdArgs+argsRec[1],newItemName,NULL)){
    print_str("# BAD NAME!\n",0);
    recover_cat(NULL,NULL);
    return;
  }

  dirItem* dirPtr=path2ptr(cmdArgs+argsRec[1],false);
  if(dirPtr==NULL){
    recover_cat(NULL,NULL);
    return;
  }
  if(dirPtr!=ROOT_DIR&&dirPtr->DIR_ATTR!=DIR_ITEM){
    print_str("# NOT DIR!\n",0);
    recover_cat(NULL,NULL);
    return;
  }

  open_cat(dirPtr,false);
  dirItem newItem;
  mem_cpy(newItem.DIR_NAME,8,newItemName);
  mem_set(newItem.DIR_EX_NAME,3,' ');
  newItem.DIR_ATTR=DIR_ITEM;
  time2num(get_sys_time(),&(newItem.DIR_WRI_TIME),&(newItem.DIR_WRI_DATE));
  newItem.DIR_SIZE=0;
  creat_item(&newItem,&_cntDir);

  recover_cat(NULL,NULL);
}

void cmd_rd(){
  if(!check_arg(2,2)) return;

  cover_cat(NULL,NULL);
  dirItem* dirPtr;

  if(argsNum==1) dirPtr=&_cntDir;
  else dirPtr=path2ptr(cmdArgs+argsRec[1],false);
  if(dirPtr==NULL){
    recover_cat(NULL,NULL);
    return;
  }
  else if(dirPtr==ROOT_DIR){
    print_str("# CAN'T!\n",0);
    recover_cat(NULL,NULL);
    return;
  }
  else if(dirPtr->DIR_ATTR!=DIR_ITEM){
    print_str("# NOT DIR!\n",0);
    recover_cat(NULL,NULL);
    return;
  }

  open_cat(dirPtr,false);
  dirPtr=(dirItem*)&cntSec;
  int dirNum=0;//记录是否有有效文件
  while(dirPtr){
    for(int i=0;i<nSec/sizeof(dirItem);i++){
      if(all_zero((char*)(dirPtr+i),sizeof(dirItem))){
        dirPtr=NULL;
        break;
      }
      if(*(BYTE*)(dirPtr+i)==FREE_ITEM) continue;
      if(check_str(dirPtr[i].DIR_NAME,max(len_str(dirPtr[i].DIR_NAME,8),len_str(SELF_DIR,0)),SELF_DIR)) continue;
      if(check_str(dirPtr[i].DIR_NAME,max(len_str(dirPtr[i].DIR_NAME,8),len_str(UP_DIR,0)),UP_DIR)) continue;
      dirNum++;
      dirPtr=NULL;
      break;
    }
    if(dirPtr) dirPtr=(dirItem*)next_sec(READ);
  }//目录是否为空
  if(dirNum!=0){
    print_str("# DIR NOT EMPTY!\n",0);
  }//目录不空
  else{
    open_cat(&_cntDir,false);
    dirPtr=(dirItem*)&cntSec;
    while(dirPtr){
      for(int i=0;i<nSec/sizeof(dirItem);i++){
        if(all_zero((char*)(dirPtr+2),sizeof(dirItem))){
          dirPtr=NULL;
          break;
        }
        if(*(BYTE*)(dirPtr+i)==FREE_ITEM){
          push_free_clus(dirPtr[i].DIR_FST_CLUS);
          mem_set((char*)(dirPtr+i),sizeof(dirItem),0);
          continue;
        }
      }
      if(dirPtr) dirPtr=(dirItem*)next_sec(READ);
    }//回收目录下空闲文件

    dirItem* upDir=get_up_dir();//回到上级目录并获得当前目录文件位置
    if(upDir!=NULL){
      *(BYTE*)upDir=FREE_ITEM;
      //save_sec(_cntSecNum);
      upd_cat_dir();
    }
  }

  recover_cat(NULL,NULL);
}

void cmd_rm(){
  if(!check_arg(2,2)) return;
  cover_cat(NULL,NULL);
  dirItem* dirPtr=path2ptr(cmdArgs+argsRec[1],false);
  if(dirPtr==NULL){
    recover_cat(NULL,NULL);
    return;
  }
  else if(dirPtr==ROOT_DIR){
    print_str("# CAN'T!\n",0);
    recover_cat(NULL,NULL);
    return;
  }
  else if(dirPtr->DIR_ATTR==DIR_ITEM){
    print_str("# DEL DIR:rd!\n",0);
    recover_cat(NULL,NULL);
    return;
  }

  //文件已经打开 无法删除
  if(check_sys_file(dirPtr)){
    print_str("# OPENED!\n",0);
  }
  else{
    *(BYTE*)dirPtr=FREE_ITEM;//打上删除标志
    //save_sec(_cntSecNum);//保存当前扇区，即保存修改
    upd_cat_dir();//更新目录信息（修改时间）
  }

  recover_cat(NULL,NULL);
}

//直接修改md函数
void cmd_new(){
  if(!check_arg(2,2)) return;

  cover_cat(NULL,NULL);

  char newItemName[8];
  char newExName[3];
  mem_set(newItemName,8,' ');//DOS
  mem_set(newExName,3,' ');
  if(!path2name(cmdArgs+argsRec[1],newItemName,newExName)){
    print_str("# BAD NAME!\n",0);
    recover_cat(NULL,NULL);
    return;
  }

  dirItem* dirPtr=path2ptr(cmdArgs+argsRec[1],false);//拿到的是未打开文件夹

  if(dirPtr==NULL){
    recover_cat(NULL,NULL);
    return;
  }
  if(dirPtr!=ROOT_DIR&&dirPtr->DIR_ATTR!=DIR_ITEM){
    print_str("# NOT DIR!\n",0);
    recover_cat(NULL,NULL);
    return;
  }

  open_cat(dirPtr,false);
  dirItem newItem;
  mem_cpy(newItem.DIR_NAME,8,newItemName);
  mem_cpy(newItem.DIR_EX_NAME,3,newExName);
  if(!isroot) newItem.DIR_ATTR=NORMAL_ITEM;
  else newItem.DIR_ATTR=SYS_ITEM;
  time2num(get_sys_time(),&(newItem.DIR_WRI_TIME),&(newItem.DIR_WRI_DATE));
  newItem.DIR_SIZE=0;
  creat_item(&newItem,&_cntDir);

  recover_cat(NULL,NULL);
}

void cmd_read(){
  if(!check_arg(2,4)) return;
  cover_cat(NULL,NULL);

  dirItem* dirPtr=path2ptr(cmdArgs+argsRec[1],false);
  if(dirPtr==NULL){
    recover_cat(NULL,NULL);
    return;
  }
  else if(dirPtr==ROOT_DIR){
    print_str("# READ DIR:ls!\n",0);
    recover_cat(NULL,NULL);
    return;
  }
  else if(dirPtr->DIR_ATTR==DIR_ITEM){
    print_str("# READ DIR:ls!\n",0);
    recover_cat(NULL,NULL);
    return;
  }

  FLAG MODE=TEX;
  FLAG KEEP_OPEN=0;
  if(argsNum>2){
    if(check_str(cmdArgs+argsRec[2],max(len_str(cmdArgs+argsRec[2],0),len_str("-b",0)),"-b")){
      MODE=BIN;
    }
    else if(check_str(cmdArgs+argsRec[2],max(len_str(cmdArgs+argsRec[2],0),len_str("-keep",0)),"-keep")){
      KEEP_OPEN=1;
    }
  }
  if(argsNum>3){
    if(check_str(cmdArgs+argsRec[3],max(len_str(cmdArgs+argsRec[3],0),len_str("-keep",0)),"-keep")){
      KEEP_OPEN=1;
    }
    if(check_str(cmdArgs+argsRec[3],max(len_str(cmdArgs+argsRec[3],0),len_str("-b",0)),"-b")){
      MODE=BIN;
    }
  }

  MYFILE* mfp=open_file(&_cntDir,dirPtr,READ);
  if(mfp==NULL) return;
  BYTE data[nSec];
  int total=0;
  int validSize=read_file(data,1,nSec,mfp);
  while(validSize){
    total+=validSize;
    print_data(data,validSize,MODE);
    validSize=read_file(data,1,nSec,mfp);
  }
  print_str("\n# FILE SIZE:",0);
  print_num(total,10);
  put_char('\n');
  if(!KEEP_OPEN) close_file(mfp);

  recover_cat(NULL,NULL);
}

void cmd_copy(){
  if(!check_arg(3,7)) return;

  cover_cat(NULL,NULL);

  dirItem* dirPtr;
  dirItem newItem;
  dirItem srcItem[5];
  dirItem srcCat[5];

  for(int i=1;i<argsNum-1;i++){
    char* srcPath=cmdArgs+argsRec[i];
    dirPtr=path2ptr(srcPath,false);
    if(dirPtr==NULL){
      recover_cat(NULL,NULL);
      return;
    }
    else if(dirPtr==ROOT_DIR){
      print_str("# CAN'T!\n",0);
      recover_cat(NULL,NULL);
      return;
    }
    else if(dirPtr->DIR_ATTR==DIR_ITEM){
      print_str("# CAN'T!\n",0);
      recover_cat(NULL,NULL);
      return;
    }
    srcItem[i-1]=*dirPtr;
    srcCat[i-1]=_cntDir;
    newItem=srcItem[i-1];//如果不改名 以最后一个文件名为名
    recover_cat(NULL,NULL);//返回原目录 避免对第二个路径造成影响
  }

  char* tarPath=cmdArgs+argsRec[argsNum-1];
  char newItemName[8];
  char newExName[3];
  mem_set(newItemName,8,' ');//DOS
  mem_set(newExName,3,' ');
  if(path2name(tarPath,newItemName,newExName)){
    mem_cpy(newItem.DIR_NAME,8,newItemName);
    mem_cpy(newItem.DIR_EX_NAME,3,newExName);
  }//改名
  time2num(get_sys_time(),&(newItem.DIR_WRI_TIME),&(newItem.DIR_WRI_DATE));


  dirPtr=path2ptr(tarPath,false);//目标目录
  if(dirPtr==NULL){
    recover_cat(NULL,NULL);
    return;
  }
  else if(dirPtr!=ROOT_DIR&&dirPtr->DIR_ATTR!=DIR_ITEM){
    print_str("# WRONG PATH!\n",0);
    recover_cat(NULL,NULL);
    return;
  }

  dirItem* newPtr=creat_item(&newItem,dirPtr);
  dirPtr=&_cntDir;
  if(newPtr!=NULL){
    for(int i=1;i<argsNum-1;i++){
      if(!copy_item_data(dirPtr,newPtr,&srcCat[i-1],&srcItem[i-1])){
        *(BYTE*)(newPtr)=FREE_ITEM;
        //save_sec(_cntSecNum);
        upd_cat_dir();//复制失败 删除半成品
        break;
      }
    }
  }//支持同一文件重复合并

  recover_cat(NULL,NULL);
  return;
}

void cmd_su(){
  if(!check_arg(1,1)) return;
  isroot=!isroot;
  if(check_str(userName,len_str("ROOT",0),"ROOT")){
    mem_cpy(userName,len_str("GXL",0)+1,"GXL");
    print_str("# NORMAL USER!\n",0);
  }
  else{
    mem_cpy(userName,len_str("ROOT",0)+1,"ROOT");
    print_str("# SUPER USER!\n",0);
  }
}

void cmd_exec(){
  cur_cls();
  int execnum=0;
  int patch=min(argsNum-1-execnum,MAX_PROCESS-1);
  dirItem* dirPtr[MAX_PROCESS]={0};
  for(int i=0;i<patch;i++){
    cover_cat(NULL,NULL);
    //根据文件路径获取可执行文件执行地址（首扇区号）
    int canExec=true;
    dirPtr[i]=path2ptr(cmdArgs+argsRec[i+1+execnum],false);
    if(dirPtr[i]==NULL||dirPtr[i]==ROOT_DIR||dirPtr[i]->DIR_ATTR==DIR_ITEM){
      canExec=false;
    }
    else if(!check_str(dirPtr[i]->DIR_EX_NAME,max(len_str(dirPtr[i]->DIR_EX_NAME,0),len_str(EXE_FILE,0)),EXE_FILE)){
      canExec=false;
    }
    //如果可执行 加载程序
    if(canExec) _load(dirPtr[i]->DIR_FST_CLUS,dirPtr[i]->DIR_NAME);
    else{
      print_str("# SKIP WRONG FILE ",0);
      print_str(cmdArgs+argsRec[i+1+execnum],0);
      print_str(" !\n",0);
    }

    recover_cat(NULL,NULL);
  }
  //执行第一批程序
  _exec();
  //执行剩余程序（每执行完一个就添加一个）
  execnum+=patch;
  for(int i=0;i<argsNum-1-patch;i++){
    cover_cat(NULL,NULL);
    //根据文件路径获取可执行文件执行地址（首扇区号）
    int canExec=true;
    dirPtr[i]=path2ptr(cmdArgs+argsRec[i+1+execnum],false);
    if(dirPtr[i]==NULL||dirPtr[i]==ROOT_DIR||dirPtr[i]->DIR_ATTR==DIR_ITEM){
      canExec=false;
    }
    else if(!check_str(dirPtr[i]->DIR_EX_NAME,max(len_str(dirPtr[i]->DIR_EX_NAME,0),len_str(EXE_FILE,0)),EXE_FILE)){
      canExec=false;
    }
    //如果可执行 加载程序
    if(canExec){
      _load(dirPtr[i]->DIR_FST_CLUS,dirPtr[i]->DIR_NAME);
      _exec();
    }
    else{
      print_str("# SKIP WRONG FILE ",0);
      print_str(cmdArgs+argsRec[i+1+execnum],0);
      print_str(" !\n",0);
    }

    recover_cat(NULL,NULL);
  }
  //执行到所有程序结束
  while(_exec()!=0) continue;
}

void cmd_cls(){
  if(!check_arg(1,1)) return;
  cur_cls();
}
