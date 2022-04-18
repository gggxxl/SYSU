__asm__(".code16gcc\r\n");
//输入：字符串地址(非空）,目标字符
//输出：目标字符在字符中出现的次数
int count_char(char* str,char ch){
  int cnt=0;
  for(int i=0;str[i]!='\0';i++){
    if(str[i]==ch) cnt++;
  }
  return cnt;
}
