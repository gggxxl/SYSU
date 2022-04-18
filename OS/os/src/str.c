#include "cglobal.h"

char my_getc(){
  char ch=get_char();
  if(ch!='\n'){
    while(get_char()!='\n') continue;
  }
  return ch;
}

int check_char(char target,char source){
  if(target==source) return true;
  if(target>='a'&&target<='z'&&target-'a'==source-'A') return true;
  if(source>='a'&&source<='z'&&source-'a'==target-'A') return true;
  return false;
}

int all_zero(char* target,int rangeLen){
  for(int i=0;i<rangeLen;i++){
    if(target[i]!=0) return false;
  }
  return true;
}

int check_str(char* target,int rangeLen,char* source){
  for(int i=0;i<rangeLen;i++){
    if(!check_char(target[i],source[i])) return false;
  }
  return true;
}

int itoa(char* str){
  int res=0;
  int base=1;
  for(int i=len_str(str,0)-1;i>=0;i--){
    res+=(str[i]-'0')*base;
    base*=10;
  }
  return res;
}

int save_str(char* target,int rangeLen,char* source){
  for(int i=0;i<rangeLen;i++){
      target[i]=source[i];
      if(source[i]=='\0') return true;
  }
  return true;
}

int div_str(char* target,int* rangeRec,char* source){
  if(source==NULL||source[0]=='\0'||source[0]=='\n') return 0;
  int recNum=0;
  int len=0;
  for(int i=0;true;i++){
    len++;
    if(source[i]==' '||source[i]=='\n') target[i]='\0';
    else target[i]=source[i];
    if(source[i]=='\0') break;
  }
  for(int i=0;i<len;i++){
    while(target[i]=='\0'&&i<len) i++;
    if(i<len){
      rangeRec[recNum]=i;
      recNum++;
    }
    while(target[i]!='\0'&&i<len) i++;
  }
  return recNum;
}

void mem_cpy(char* target,int rangeLen,char* source){
  for(int i=0;i<rangeLen;i++) target[i]=source[i];
}

void mem_set(char* target,int rangeLen,char source){
  for(int i=0;i<rangeLen;i++) target[i]=source;
}

int len_str(char* target,int maxLen){
  if(target==NULL) return 0;
  int strLen=0;
  while(target[strLen]!='\0'&&target[strLen]!=' '&&(!maxLen||strLen<maxLen)) strLen++;
  return strLen;
}

void apd_path_str(char* target,int rangeLen,char* source,int maxLen){
  if(source==NULL) return;
  int end=0;
  while(source[end]!='\0'&&source[end]!=' ') end++;
  int i=0;
  if(end==0){
    for(;i<rangeLen&&i<maxLen;i++){
      source[i]=target[i];
    }
    source[i]='\0';
  }
  else{
    source[end]='\\';
    for(;i<rangeLen&&end+i+1<maxLen-1;i++){
      source[end+i+1]=target[i];
    }
    source[end+i+1]='\0';
  }
}

void cut_path_str(char* source){
  if(source==NULL) return;
  int end=0;
  while(source[end]!='\0'&&source[end]!=' ') end++;
  while(end>=0&&source[end]!='\\') end--;
  if(end<0) source[0]='\0';
  else source[end]='\0';
}

//从path中提取最后一个文件/文件夹名
//如果成功 path也会被去掉最后一个文件
//返回合法与否
int path2name(char* srcPath,char* newItemName,char* newExName){
  //获得位置和检查合法性
  int begin=0;
  int exBegin=0;
  while(srcPath[begin]!='\0') begin++;
  while(begin>0&&srcPath[begin-1]!='\\'){
    if(srcPath[begin-1]=='.'){
      if(exBegin!=0){
        print_str("# CHARACTER \'.\'&\'\\\' NOT ALLOWED!\n",0);
        return false;
      }//名字出现了'.'
      else if(newExName==NULL){
        print_str("# EXTEND NAME NOT ALLOWED!\n",0);
        return false;
      }//与要求的无后缀不符
      else exBegin=begin;
    }
    begin--;
  }
  if(srcPath[begin]=='\0'){
    //printf("# EMPTY NAME NOT ALLOWED!\n");
    return false;//
  }
  if(exBegin!=0){
    if(len_str(srcPath+exBegin,0)>3){
      print_str("# FAILED: FILE EXTEND NAME MORE THAN 3 CHARACTERS!\n",0);
      return false;
    }
    mem_cpy(newExName,len_str(srcPath+exBegin,3),srcPath+exBegin);
    srcPath[exBegin-1]='\0';//'.'->'\0'
  }

  if(len_str(srcPath+begin,0)>8){
    print_str("# FAILED: FILE NAME MORE THAN 8 CHARACTERS!\n",0);
    return false;
  }
  mem_cpy(newItemName,len_str(srcPath+begin,8),srcPath+begin);
  srcPath[begin]='\0';

  return true;
}
