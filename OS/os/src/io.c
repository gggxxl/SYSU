#include "cglobal.h"
char STD_IN[nSec]={0};
int STD_IN_B=0;
int STD_IN_E=0;

void print_num(int number,int maxLen){
  char temp[16];
  int len=0;
  while((maxLen==0||len<maxLen)&&number){
    temp[len]=number%10+'0';
    number/=10;
    len++;
  }
  while(len<maxLen){
    temp[len]='0';
    len++;
  }
  while(len--){
    put_char(temp[len]);
  }
}

int print_str(char* str,int maxLen){
  int cnt=0;
  while(maxLen==0||cnt<maxLen){
    if(str[cnt]=='\0') break;
    put_char(str[cnt]);
    cnt++;
  }
  if(maxLen){
    while(cnt<maxLen){
      put_char(' ');
      cnt++;
    }
  }
  return cnt;
}

int get_str(char* str,int maxSize){
  int cnt=0;
  while(maxSize==0||cnt<maxSize-1){
    str[cnt]=get_char();
    if(str[cnt]=='\n'){
      str[cnt]='\0';
      break;
    }
    cnt++;
  }
  str[cnt]='\0';
  return cnt;
}

void put_char(char ch){
    if(ch=='\n'){
      get_xy();
      CUR_X=0;
      if(CUR_Y==CUR_D){
        CUR_MV=1;
        cur_up();
      }
      else CUR_Y=CUR_Y+1;
      set_xy();
    }
    else if(ch=='\b'){
      cur_last();
    }
    else{
      show_c(ch);
      cur_next();
    }
}

char get_char(){
  if(STD_IN_B==STD_IN_E){
    while(1){
      char ch=get_c();
      if(ch==CH_ENTER) ch='\n';//this is weird
      if(ch=='\b'){
        if(STD_IN_B==STD_IN_E) continue;
        else{
          cur_last();
          show_c(' ');
          STD_IN_E=(STD_IN_E-1+nSec)%nSec;
        }
      }
      else{
        if((STD_IN_E+2)%nSec==STD_IN_B&&ch!='\n'){
          continue;
        }
        STD_IN[STD_IN_E]=ch;
        STD_IN_E=(STD_IN_E+1)%nSec;
        if(ch=='\n'){
          put_char('\n');
          break;
        }
        else{
          show_c(ch);
          cur_next();
        }
      }
    }
  }
  char ch=STD_IN[STD_IN_B];
  STD_IN_B=(STD_IN_B+1)%nSec;
  return ch;
}

void cur_next(){
  get_xy();
  if(CUR_X==CUR_R){
    CUR_X=CUR_L;
    if(CUR_Y==CUR_D){
      CUR_MV=1;
      cur_up();
    }
    else CUR_Y=CUR_Y+1;
  }
  else CUR_X=CUR_X+1;
  set_xy();
}

void cur_last(){
  get_xy();
  if(CUR_X==CUR_L){
    CUR_X=CUR_R;
    if(CUR_Y==CUR_U){
      CUR_MV=1;
      cur_dw();
    }
    else CUR_Y=CUR_Y-1;
  }
  else CUR_X=CUR_X-1;
  set_xy();
}

void cur_cls(){
  get_xy();
  CUR_MV=CUR_Y+1;
  cur_up();
  CUR_X=0;
  CUR_Y=0;
  set_xy();
}

void cur_set_attr(char attr){
  CUR_ATTR=attr;
}

int io_load_sec(void* objSec,int secNum){
  OBJ_LBA=secNum-1;
  OBJ_PTR=objSec;
  WORD reg_AX=io_load();
  BYTE reg_AH=reg_AX>>8;
  int int_AH=reg_AH&0xFF;
  //int_AH=磁盘状态 为00H表示读取成功 为其它表示错误 输出错误码
  if(int_AH){
    if(isroot){
      print_str("# load sec failed, error=",0);
      print_num(int_AH,3);
      put_char('\n');
    }
    return false;
  }
  else{
    if(isroot) print_str("# load sec successfull!\n",0);
    return true;
  }
}

int io_save_sec(void* objSec,int secNum){
  OBJ_LBA=secNum-1;
  OBJ_PTR=objSec;
  WORD reg_AX=io_save();
  BYTE reg_AH=reg_AX>>8;
  //int_AH=磁盘状态 为00H表示写入成功 为其它表示错误 输出错误码
  int int_AH=reg_AH&0xFF;
  if(int_AH){
    if(isroot){
      print_str("# save sec failed, error=",0);
      print_num(int_AH,3);
      put_char('\n');
    }
    return false;
  }
  else{
    if(isroot) print_str("# save sec successfull!\n",0);
    return true;
  }
}
