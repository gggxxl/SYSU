#ifndef IO_H
#define IO_H
#define nSec 512
#define CH_ENTER 13
extern void cur_up();
extern void cur_dw();
extern void set_xy();
extern void get_xy();
extern void show_c(char ch);
extern char get_c();
extern void io_init();
extern WORD io_load();
extern WORD io_save();
extern char CUR_PAGE;
extern char CUR_ATTR;
extern char CUR_X;
extern char CUR_Y;
extern char CUR_CLR;
extern char CUR_L;
extern char CUR_U;
extern char CUR_R;
extern char CUR_D;
extern char CUR_MV;
extern void* OBJ_PTR;
extern int OBJ_LBA;

void print_num(int number,int maxLen);
//类似于puts
int print_str(char* str,int maxLen);
//类似于gets 字符串长度最多为maxSize-1
int get_str(char* str,int maxSize);

void put_char(char ch);

char get_char();

void cur_next();

void cur_last();

void cur_cls();

void cur_set_attr(char attr);

int io_load_sec(void* objSec,int secNum);

int io_save_sec(void* objSec,int secNum);

#endif

