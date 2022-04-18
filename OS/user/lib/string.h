#ifndef STRING_H
#define STRING_H

#define NULL (void*)0x0

void memcpy(char* target,char* source,int rangeLen){
  for(int i=0;i<rangeLen;i++) target[i]=source[i];
}

void memset(char* target,char source,int rangeLen){
  for(int i=0;i<rangeLen;i++) target[i]=source;
}

int strlen(char* target){
  if(target==NULL) return 0;
  int strLen=0;
  while(target[strLen]!='\0') strLen++;
  return strLen;
}

int strcmp(char* s1,char* s2){
  int i=0;
  while(1){
  	if(s1[i]=='\0'&&s2[i]=='\0') return 0;
  	if(s1[i]=='\0'||s1[i]<s2[i]) return -1;
  	if(s2[i]=='\0'||s1[i]>s2[i]) return 1;
  	i++;
  }
  return 0;
}

int itoa(char* str){
  int res=0;
  int base=1;
  for(int i=strlen(str)-1;i>=0;i--){
    res+=(str[i]-'0')*base;
    base*=10;
  }
  return res;
}

#endif