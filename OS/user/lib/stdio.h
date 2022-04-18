#ifndef STDIO_H
#define STDIO_H
#include<stdarg.h>
#include "string.h"

#define stdin (void*)0x0
#define stdout (void*)0x1
#define stderr (void*)0x2
#define MAXSIZE 128 //path str should not larger
typedef char FILE;

#define max(x,y) ({\
int _x=x;\
int _y=y;\
_x>_y?_x:_y;\
})
#define min(x,y) ({\
int _x=x;\
int _y=y;\
_x>_y?_y:_x;\
})

//文件打开
FILE* fopen(char* path,char* mode);
//文件关闭
void fclose(FILE* fp);
//文件（设备）读
int read(char* buffer,int size,FILE* fp);
//文件（设备）写
int write(char* buffer,int size,FILE* fp);
//文件读
int fwrite(char* buffer,int size,int count,FILE* fp);
//文件写
int fread(char* buffer,int size,int count,FILE* fp);

//使用文件接口实现的输入输出
//putchar与getchar
int putchar(char ch);
char getchar();
//puts与gets
int puts(char* str);
int gets(char* str);
//printf与scanf
int printf(char * str,...);
int scanf(char* str,...);


/////////////////////////////////////////////////////
FILE* fopen(char* path,char* mode){
	char imode;
	if(!strcmp(mode,"rb")||!strcmp(mode,"r")){
		imode=0;
	}
	else if(!strcmp(mode,"wb")||!strcmp(mode,"w")){
		imode=1;
	}
	else return NULL;
	int len=strlen(path);
	FILE* fp;
	__asm__(
		"movb $0x3D,%%ah;"
		"movb %1,%%al;"
		"int $0x21;"
		:"=a"(fp)
		:"r"(imode),"c"(len+1),"d"(path)
		:
	);
	return fp;
}

void fclose(FILE* fp){
	__asm__(
		"movb $0x3E,%%ah;"
		"int $0x21;"
		:
		:"b"(fp)
		:"eax"
	);
}

//size<=MAXSIZE
int read(char* buffer,int size,FILE* fp){
	int num;
	__asm__(
		"movb $0x3F,%%ah;"
		"int $0x21;"
		:"=a"(num)
		:"b"(fp),"c"(size),"d"(buffer)
		:
	);
	return num;
}

//size<=MAXSIZE
int write(char* buffer,int size,FILE* fp){
	int num;
	__asm__(
		"movb $0x40,%%ah;"
		"int $0x21;"
		:"=a"(num)
		:"b"(fp),"c"(size),"d"(buffer)
		:
	);
	return num;
}

int fwrite(char* buffer,int size,int count,FILE* fp){
	int total=0;
	int maxsize=size*count;
	while(total<maxsize){
		total+=write(buffer+total,min(MAXSIZE,maxsize-total),fp);
		if(total==0) break;
	}

	return total;
}



int fread(char* buffer,int size,int count,FILE* fp){
	int total=0;
	int maxsize=size*count;
	while(total<maxsize){
		int nsize=min(MAXSIZE,maxsize-total);
		int size=read(buffer+total,nsize,fp);
		total+=size;
		if(size<nsize) break;
	}
	return total;
}

//int fprintf(MYFILE* fp,char* str,...);

//int fscanf(MYFILE* fp,char* str,...);

int putchar(char ch){
	write(&ch,1,stdout);
	return ch;
}

char getchar(){
	char ch;
	read(&ch,1,stdin);
	return ch;
}

int puts(char* str){
	int total=0;
	int maxsize=strlen(str);
	while(total<maxsize){
		total+=write(str+total,min(MAXSIZE,maxsize-total),stdout);
	}

	return total;
}


int gets(char* str){
	int total=0;
	while(1){
		int size=read(str+total,MAXSIZE,stdin);
		total+=size;
		if(size<MAXSIZE) break;
	}
	return total;
}

int printf(char * str,...){
	va_list args;
	va_start(args,str);
	int cnt=0;
	int ttp=0;
	while(str[cnt]!='\0'){
		if(str[cnt]=='%'){
			cnt++;
			int zero=0;
			int r_sft=1;
			int pstv=0;
			int les_n=0;
			int max_n=-1;
			int i_typ=0;
			if(str[cnt]=='0'){
				zero=1;
				cnt++;
			}
			if(str[cnt]=='-'){
				r_sft=0;
				cnt++;
			}
			if(str[cnt]=='+'){
				zero=0;
				pstv=1;
				cnt++;
			}
			if(str[cnt]>='0'&&str[cnt]<='9'){
				int cur[50]={-1};
				int c_cnt=0;
				do{
					cur[c_cnt]=str[cnt]-'0';
					c_cnt++;
					cnt++;
				}while(str[cnt]>='0'&&str[cnt]<='9');
				int i=0;
				while(i<c_cnt){
					int temp=1,time=i;
					while(time--){
						temp*=10;
					}
					les_n+=cur[c_cnt-1-i]*temp;
					i++;
				}
			}
			if(str[cnt]=='.'){
				zero=0;
				cnt++;
			}
			if(str[cnt]>='0'&&str[cnt]<='9'){
				max_n=0;
				int cur[50]={-1};
				int c_cnt=0;
				do{
					cur[c_cnt]=str[cnt]-'0';
					c_cnt++;
					cnt++;
				}while(str[cnt]>='0'&&str[cnt]<='9');
				int i=0;
				while(i<c_cnt){
					int temp=1,time=i;
					while(time--){
						temp*=10;
					}
					max_n+=cur[c_cnt-1-i]*temp;
					i++;
				}
			}
			if(str[cnt]=='l'){
				i_typ++;
				cnt++;
				if(str[cnt]=='l'){
					i_typ++;
					cnt++;
				}
			}
			else if(str[cnt]=='h'){
				i_typ--;
				cnt++;
			}
			char typ=str[cnt];
			switch(typ){
				case 'd':
				case 'i':
						{
							long cur;
							if(i_typ==0) cur=va_arg(args,int);
							else if(i_typ==-1) cur=va_arg(args,int);
							else if(i_typ==1) cur=va_arg(args,long);
							else if(i_typ==2) cur=va_arg(args,long);
							else return -1;
							int dec[50]={-1};
							int i=0;
							int ngtv=0;
							if(cur<0){
								pstv=0;
								ngtv=1;
								cur=-cur;
								if(zero){
									ttp++;
									putchar('-');
									les_n--;
									ngtv=0;
								}
							}
							while(cur>9){
								dec[i]=cur%10;
								cur/=10;
								i++;
							}
							dec[i]=cur;
							int j=pstv||ngtv?i+2:i+1;
							if(pstv||ngtv) max_n++;
							int m=j;
							if(j<max_n){
								m=max_n;
								if(r_sft&&les_n>max_n){
									int k=les_n-max_n;
									ttp+=k;
									if(zero) while(k--) putchar('0');
									else while(k--) putchar(' ');
								}
								if(pstv){putchar('+');ttp++;}
								if(ngtv){putchar('-');ttp++;}
								int k=max_n-j;
								ttp+=k;
								while(k--) putchar('0');
							}
							else if(r_sft&&les_n>j){
								int k=les_n-j;
								ttp+=k;
								if(zero) while(k--) putchar('0');
								else while(k--) putchar(' ');
							}
							if(j>=max_n&&pstv){putchar('+');ttp++;}
							if(j>=max_n&&ngtv){putchar('-');ttp++;}
							while(i>=0){
								putchar(dec[i]+'0');
								ttp++;
								i--;
							}
							if(!r_sft&&les_n>m){
								int k=les_n-m;
								ttp+=k;
								while(k--) putchar(' ');
							}
							break;
						}
				case 'u':
						{
							unsigned long cur;
							if(i_typ==0) cur=va_arg(args,unsigned int);
							else if(i_typ==-1) cur=va_arg(args,unsigned int);
							else if(i_typ==1) cur=va_arg(args,unsigned long);
							else if(i_typ==2) cur=va_arg(args,unsigned long);
							else return -1;
							int dec[50]={-1};
							int i=0;
							while(cur>9){
								dec[i]=cur%10;
								cur/=10;
								i++;
							}
							dec[i]=cur;
							int j=pstv?i+2:i+1;
							if(pstv) max_n++;
							int m=j;
							if(j<max_n){
								m=max_n;
								if(r_sft&&les_n>max_n){
									int k=les_n-max_n;
									ttp+=k;
									if(zero) while(k--) putchar('0');
									else while(k--) putchar(' ');
								}
								if(pstv){
									putchar('+');
									ttp++;
								}
								int k=max_n-j;
								ttp+=k;
								while(k--) putchar('0');
							}
							else if(r_sft&&les_n>j){
								int k=les_n-j;
								ttp+=k;
								if(zero) while(k--) putchar('0');
								else while(k--) putchar(' ');
							}
							if(j>=max_n&&pstv){putchar('+');ttp++;}
							while(i>=0){
								ttp++;
								putchar(dec[i]+'0');
								i--;
							}
							if(!r_sft&&les_n>m){
								int k=les_n-m;
								ttp+=k;
								while(k--) putchar(' ');
							}
							break;
						}
				case 'f':
						{
							double cur0=va_arg(args,double);
							double deccur;
							int ngtv=0;
							if(cur0<0){
								pstv=0;
								ngtv=1;
								cur0=-cur0;
								if(zero){
									ttp++;
									putchar('-');
									les_n--;
									ngtv=0;
								}
							}
							long temp=1;
							max_n=(max_n==-1)?6:max_n;
							int time=max_n;
							while(time--) temp*=10;
							deccur=cur0*temp;
							if(deccur-(long)deccur>=0.5) deccur++;
							long cur=(long)deccur;
							int dec[50]={-1};
							int i=0;
							while(cur>9){
								dec[i]=cur%10;
								cur/=10;
								i++;
							}
							dec[i]=cur;
							int j=pstv||ngtv?i+3:i+2;
							if(r_sft&&les_n>j){
								int k=les_n-j;
								ttp+=k;
								if(zero) while(k--) putchar('0');
								else while(k--) putchar(' ');
							}
							if(pstv){putchar('+');ttp++;}
							if(ngtv){putchar('-');ttp++;}
							while(i>max_n-1){
								ttp++;
								putchar(dec[i]+'0');
								i--;
							}
							if(i>=0){putchar('.');ttp++;}
							while(i>=0){
								ttp++;
								putchar(dec[i]+'0');
								i--;
							}
							if(!r_sft&&les_n>j){
								int k=les_n-j;
								ttp+=k;
								while(k--) putchar(' ');
							}
							break;
						}
				case 'e':
						{
							double cur0=va_arg(args,double);
							double deccur;
							int ngtv=0;
							if(cur0<0){
								pstv=0;
								ngtv=1;
								cur0=-cur0;
								if(zero){
									ttp++;
									putchar('-');
									les_n--;
									ngtv=0;
								}
							}
							int p=0;
							if(cur0<1){
								while(cur0<1){
									cur0*=10;
									p--;
								}
							}
							else{
								while(cur0>=10){
									cur0/=10;
									p++;
								}
							}
							les_n-=5;
							long temp=1;
							max_n=(max_n==-1)?6:max_n;
							int time=max_n;
							while(time--) temp*=10;
							deccur=cur0*temp;
							if(deccur-(long)deccur>=0.5) deccur++;
							long cur=(long)deccur;
							int dec[50]={-1};
							int i=0;
							while(cur>9){
								dec[i]=cur%10;
								cur/=10;
								i++;
							}
							dec[i]=cur;
							int j=pstv||ngtv?i+3:i+2;
							if(r_sft&&les_n>j){
								int k=les_n-j;
								ttp+=k;
								if(zero) while(k--) putchar('0');
								else while(k--) putchar(' ');
							}
							if(pstv){putchar('+');ttp++;}
							if(ngtv){putchar('-');ttp++;}
							while(i>max_n-1){
								ttp++;
								putchar(dec[i]+'0');
								i--;
							}
							if(i>=0){putchar('.');ttp++;}
							while(i>=0){
								ttp++;
								putchar(dec[i]+'0');
								i--;
							}
							ttp+=5;
							putchar('e');
							putchar(p<0?'-':'+');
							putchar(p/100+'0');
							putchar(p/10+'0');
							putchar((p>0?p:-p)%10+'0');
							if(!r_sft&&les_n>j){
								int k=les_n-j;
								ttp+=k;
								while(k--) putchar(' ');
							}
							break;
						}
				case 'E':
						{
							double cur0=va_arg(args,double);
							double deccur;
							int ngtv=0;
							if(cur0<0){
								pstv=0;
								ngtv=1;
								cur0=-cur0;
								if(zero){
									ttp++;
									putchar('-');
									les_n--;
									ngtv=0;
								}
							}
							int p=0;
							if(cur0<1){
								while(cur0<1){
									cur0*=10;
									p--;
								}
							}
							else{
								while(cur0>=10){
									cur0/=10;
									p++;
								}
							}
							les_n-=5;
							long temp=1;
							max_n=(max_n==-1)?6:max_n;
							long time=max_n;
							while(time--) temp*=10;
							deccur=cur0*temp;
							if(deccur-(long)deccur>=0.5) deccur++;
							long cur=(long)deccur;
							int dec[50]={-1};
							int i=0;
							while(cur>9){
								dec[i]=cur%10;
								cur/=10;
								i++;
							}
							dec[i]=cur;
							int j=pstv||ngtv?i+3:i+2;
							if(r_sft&&les_n>j){
								int k=les_n-j;
								ttp+=k;
								if(zero) while(k--) putchar('0');
								else while(k--) putchar(' ');
							}
							if(pstv){putchar('+');ttp++;}
							if(ngtv){putchar('-');ttp++;}
							while(i>max_n-1){
								ttp++;
								putchar(dec[i]+'0');
								i--;
							}
							if(i>=0){putchar('.');ttp++;}
							while(i>=0){
								ttp++;
								putchar(dec[i]+'0');
								i--;
							}
							ttp+=5;
							putchar('E');
							putchar(p<0?'-':'+');
							putchar(p/100+'0');
							putchar(p/10+'0');
							putchar((p>0?p:-p)%10+'0');
							if(!r_sft&&les_n>j){
								int k=les_n-j;
								ttp+=k;
								while(k--) putchar(' ');
							}
							break;
						}
				case 'g':
						{
							double cur0=va_arg(args,double);
							double deccur;
							int ngtv=0;
							if(cur0<0){
								pstv=0;
								ngtv=1;
								cur0=-cur0;
								if(zero){
									ttp++;
									putchar('-');
									les_n--;
									ngtv=0;
								}
							}
							int p=0;
							double cur1=cur0;
							if(cur1<1){
								while(cur1<1){
									cur1*=10;
									p--;
								}
							}
							else{
								while(cur1>=10){
									cur1/=10;
									p++;
								}
							}
							max_n=(max_n==-1)?6:max_n;
							int ig=0;
							if(p>=max_n||p<-4) ig=1;
							if(ig){
								les_n-=5;
								cur0=cur1;
							}
							long temp=1;
							int time=max_n;
							while(time--) temp*=10;
							deccur=cur0*temp;
							if(deccur-(long)deccur>=0.5) deccur++;
							long cur=(long)deccur;
							int dzro=0;
							long look=cur;
							while(look%10==0&&dzro<max_n){
								dzro++;
								look/=10;
							}
							les_n+=dzro;
							int dec[50]={-1};
							int i=0;
							while(cur>9){
								dec[i]=cur%10;
								cur/=10;
								i++;
							}
							dec[i]=cur;
							int j=pstv||ngtv?i+3:i+2;
							if(r_sft&&les_n>j){
								int k=les_n-j;
								ttp+=k;
								if(zero) while(k--) putchar('0');
								else while(k--) putchar(' ');
							}
							if(pstv){putchar('+');ttp++;}
							if(ngtv){putchar('-');ttp++;}
							while(i>max_n-1){
								ttp++;
								putchar(dec[i]+'0');
								i--;
							}
							if(i>=dzro){putchar('.');ttp++;}
							while(i>=dzro){
								ttp++;
								putchar(dec[i]+'0');
								i--;
							}
							if(ig){
								ttp+=5;
								putchar('e');
								putchar(p<0?'-':'+');
								putchar(p/100+'0');
								putchar(p/10+'0');
								putchar((p>0?p:-p)%10+'0');
							}
							if(!r_sft&&les_n>j){
								int k=les_n-j;
								ttp+=k;
								while(k--) putchar(' ');
							}
							break;
						}
				case 'G':
						{
							double cur0=va_arg(args,double);
							double deccur;
							int ngtv=0;
							if(cur0<0){
								pstv=0;
								ngtv=1;
								cur0=-cur0;
								if(zero){
									putchar('-');
									ttp++;
									les_n--;
									ngtv=0;
								}
							}
							int p=0;
							double cur1=cur0;
							if(cur1<1){
								while(cur1<1){
									cur1*=10;
									p--;
								}
							}
							else{
								while(cur1>=10){
									cur1/=10;
									p++;
								}
							}
							max_n=(max_n==-1)?6:max_n;
							int ig=0;
							if(p>=max_n||p<-4) ig=1;
							if(ig){
								les_n-=5;
								cur0=cur1;
							}
							long temp=1;
							max_n=(max_n==-1)?6:max_n;
							int time=max_n;
							while(time--) temp*=10;
							deccur=cur0*temp;
							if(deccur-(long)deccur>=0.5) deccur++;
							long cur=(long)deccur;
							int dzro=0;
							long look=cur;
							while(look%10==0&&dzro<max_n){
								dzro++;
								look/=10;
							}
							les_n+=dzro;
							int dec[50]={-1};
							int i=0;
							while(cur>9){
								dec[i]=cur%10;
								cur/=10;
								i++;
							}
							dec[i]=cur;
							int j=pstv||ngtv?i+3:i+2;
							if(r_sft&&les_n>j){
								int k=les_n-j;
								ttp+=k;
								if(zero) while(k--) putchar('0');
								else while(k--) putchar(' ');
							}
							if(pstv){putchar('+');ttp++;}
							if(ngtv){putchar('-');ttp++;}
							while(i>max_n-1){
								putchar(dec[i]+'0');
								ttp++;
								i--;
							}
							if(i>=dzro){putchar('.');ttp++;}
							while(i>=dzro){
								putchar(dec[i]+'0');
								ttp++;
								i--;
							}
							if(ig){
								putchar('E');
								putchar(p<0?'-':'+');
								putchar(p/100+'0');
								putchar(p/10+'0');
								putchar((p>0?p:-p)%10+'0');
								ttp+=5;
							}
							if(!r_sft&&les_n>j){
								int k=les_n-j;
								ttp+=k;
								while(k--) putchar(' ');
							}
							break;
						}
				case 'c':
						{
							int cur=va_arg(args,int);
							if(r_sft&&les_n){
								int k=les_n-1;
								while(k--) putchar(' ');
							}
							putchar(cur);
							if(!r_sft&&les_n){
								int k=les_n-1;
								while(k--) putchar(' ');
							}
							ttp+=les_n>1?les_n:1;
							break;
						}
				case 's':
						{
							char* cur=va_arg(args,char*);
							int lenth=0;
							while(cur[lenth]!='\0') lenth++;\
							if(lenth>max_n&&max_n!=-1) lenth=max_n;
							if(lenth<les_n&&r_sft){
								int k=les_n-lenth;
								while(k--) putchar(' ');
							}
							int hp=0;
							while(hp<lenth){
								putchar(cur[hp]);
								hp++;
							}
							if(lenth<les_n&&!r_sft){
								int k=les_n-lenth;
								while(k--) putchar(' ');
							}
							ttp+=les_n>lenth?les_n:lenth;
							break;
						}
				case 'p':
						{
							long unsigned cur=va_arg(args,long unsigned);
							int dec[50]={-1};
							int i=0;
							while(cur>15){
								dec[i]=cur%16;
								cur/=16;
								i++;
							}
							int j=i+1;
							dec[i]=cur;
							int m=j;
							les_n-=2;
							if(j<max_n){
								m=max_n;
								if(r_sft&&les_n>max_n){
									int k=les_n-max_n;
									while(k--) putchar(' ');
								}
								putchar('0');
								putchar('x');
								int k=max_n-j;
								while(k--) putchar('0');
							}
							else if(r_sft&&les_n>j){
								int k=les_n-j;
								while(k--) putchar(' ');
							}
							if(j>=max_n){
								putchar('0');
								putchar('x');
							}
							while(i>=0){
								if(dec[i]<10) putchar(dec[i]+'0');
								else putchar('a'+dec[i]-10);
								i--;
							}
							if(!r_sft&&les_n>m){
								int k=les_n-m;
								while(k--) putchar(' ');
							}
							ttp+=2;
							ttp+=les_n>m?les_n:m;
							break;
						}
				case 'x':
						{
							unsigned long cur;
							if(i_typ==0) cur=va_arg(args,unsigned int);
							else if(i_typ==-1) cur=va_arg(args,unsigned int);
							else if(i_typ==1) cur=va_arg(args,unsigned long);
							else if(i_typ==2) cur=va_arg(args,unsigned long );
							else return -1;
							int dec[50]={-1};
							int i=0;

							while(cur>15){
								dec[i]=cur%16;
								cur/=16;
								i++;
							}
							int j=i+1;
							dec[i]=cur;
							int m=j;
							if(j<max_n){
								m=max_n;
								if(r_sft&&les_n>max_n){
									int k=les_n-max_n;
									if(zero) while(k--) putchar('0');
									else while(k--) putchar(' ');
								}
								int k=max_n-j;
								while(k--) putchar('0');
							}
							else if(r_sft&&les_n>j){
								int k=les_n-j;
								if(zero) while(k--) putchar('0');
								else while(k--) putchar(' ');
							}
							while(i>=0){
								if(dec[i]<10) putchar(dec[i]+'0');
								else putchar('a'+dec[i]-10);
								i--;
							}
							if(!r_sft&&les_n>m){
								int k=les_n-m;
								while(k--) putchar(' ');
							}
							ttp+=les_n>m?les_n:m;
							break;
						}
				case 'X':
						{
							unsigned long cur;
							if(i_typ==0) cur=va_arg(args,unsigned int);
							else if(i_typ==-1) cur=va_arg(args,unsigned int);
							else if(i_typ==1) cur=va_arg(args,unsigned long);
							else if(i_typ==2) cur=va_arg(args,unsigned long);
							else return -1;
							int dec[50]={-1};
							int i=0;
							while(cur>15){
								dec[i]=cur%16;
								cur/=16;
								i++;
							}
							int j=i+1;
							dec[i]=cur;
							int m=j;
							if(j<max_n){
								m=max_n;
								if(r_sft&&les_n>max_n){
									int k=les_n-max_n;
									if(zero) while(k--) putchar('0');
									else while(k--) putchar(' ');
								}
								int k=max_n-j;
								while(k--) putchar('0');
							}
							else if(r_sft&&les_n>j){
								int k=les_n-j;
								if(zero) while(k--) putchar('0');
								else while(k--) putchar(' ');
							}
							while(i>=0){
								if(dec[i]<10) putchar(dec[i]+'0');
								else putchar('A'+dec[i]-10);
								i--;
							}
							if(!r_sft&&les_n>m){
								int k=les_n-m;
								while(k--) putchar(' ');
							}
							ttp+=les_n>m?les_n:m;
							break;
						}
				case 'o':
						{
							unsigned long cur;
							if(i_typ==0) cur=va_arg(args,unsigned int);
							else if(i_typ==-1) cur=va_arg(args,unsigned int);
							else if(i_typ==1) cur=va_arg(args,unsigned long);
							else if(i_typ==2) cur=va_arg(args,unsigned long);
							else return -1;
							int dec[50]={-1};
							int i=0;
							while(cur>7){
								dec[i]=cur%8;
								cur/=8;
								i++;
							}
							int j=i+1;
							dec[i]=cur;
							int m=j;
							if(j<max_n){
								m=max_n;
								if(r_sft&&les_n>max_n){
									int k=les_n-max_n;
									if(zero) while(k--) putchar('0');
									else while(k--) putchar(' ');
								}
								int k=max_n-j;
								while(k--) putchar('0');
							}
							else if(r_sft&&les_n>j){
								int k=les_n-j;
								if(zero) while(k--) putchar('0');
								else while(k--) putchar(' ');
							}
							while(i>=0){
								if(dec[i]<10) putchar(dec[i]+'0');
								else putchar('a'+dec[i]-10);
								i--;
							}
							if(!r_sft&&les_n>m){
								int k=les_n-m;
								while(k--) putchar(' ');
							}
							ttp+=les_n>m?les_n:m;
							break;
						}
				default:
						{
							putchar('%');
							if(str[cnt]!='\0'){putchar(str[cnt]);ttp++;}
							ttp++;
							break;
						}
			}
		}
		else{
			putchar(str[cnt]);
			ttp++;
		}
		cnt++;
	}
	va_end(args);
	return ttp;
}

//%d %s %c
int scanf(char* str,...){
	va_list args;
	va_start(args,str);
	int cur=0;
	int success=0;
	while(1){
		success++;
		while(str[cur]!='\0'&&str[cur]!='%') cur++;
		if(str[cur]=='\0') break;
		cur++;
		switch(str[cur]){
			case 'd':
				{
					int* xi=va_arg(args,int*);
					int l=0;
					char temp[128];				
					while((temp[l]=getchar())==' ') continue;
					l++;
					while((temp[l]=getchar())!=' '&&temp[l]!='\n') l++;
					temp[l]='\0';
					*xi=itoa(temp);
					break;
				}

			case 's':
				{
					char* xs=va_arg(args,char*);
					int i=0;
					while((xs[i]=getchar())==' ') continue;
					i++;
					while((xs[i]=getchar())!=' '&&xs[i]!='\n') i++;
					xs[i]='\0';
					break;
				}

			case 'c':
				{
					char* xc=va_arg(args,char*);
					*xc=getchar();
					break;
				}

			default:
				{
					success--;
					break;
				}
		}
	}
	va_end(args);
	return success;
}


#endif
