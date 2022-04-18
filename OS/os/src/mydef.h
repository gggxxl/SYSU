#ifndef MYDEF_H
#define MYDEF_H

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

#define GET_BCD(x) (((x)&0xF)+((x)>>4)*10)

#define true 1
#define false 0
#define NULL (void*)0x0

#define WRITE 1
#define READ 0

#define TEX 0
#define BIN 1

#define MAXLEN 128

typedef int FLAG;
typedef unsigned int size_t;

//定义长度单位
typedef unsigned char BYTE;//1B
typedef unsigned short WORD;//2B
typedef unsigned long DWORD;//4B

#endif
