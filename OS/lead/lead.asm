[BITS 16]
;-------------------------------------------------------------------------------
;常量定义与偏移量设置
;-------------------------------------------------------------------------------
  org 7c00h                             ;地址偏移量为7c00h
  ROOT_ADDR equ 7e00h                   ;程序表加载位置
  ROOT_LBA equ 19                       ;程序表所在扇区LBA
  ROOT_SIZE equ 1                       ;程序表扇区数
  ENTRY_CLUS equ 26                     ;程序表项中程序首簇号信息位置
  ENTRY_SIZE equ 28                     ;程序表项中程序大小信息位置
  CORE_SEG equ 1000h                    ;内核程序加载位置
  CLUS_LBA equ 31                       ;簇号到LBA
  PSP_ADDR equ 100h                     ;程序段前缀偏移

;-------------------------------------------------------------------------------
;MBR信息1
;-------------------------------------------------------------------------------
BS_JMPBOOT        jmp short START       ;跳转指令 跳转到运行入口
WTF               db 0x90               ;把跳转指令填充到3字节
BS_OEMNAM         db "18340047"         ;8位 厂商名（名称参考自老师的PPT 下同）
BPB_BYTS_PER_SEC  dw 512                ;每扇区字节数
BPB_SEC_PER_CLUS  db 1                  ;每簇扇区数
BPB_RSVD_SEC_CNT  dw 1                  ;BOOT记录占多少扇区
BPB_NUM_FATS      db 2                  ;共有多少FAT表
BPB_ROOT_ENT_CNT  dw 224                ;根目录文件数最大值
BPB_TOT_SEC16     dw 2880               ;扇区总数
BPB_MEDIA         db 0xF0               ;介质描述符
BPB_FAT_Sz16      dw 0x09               ;每FAT扇区数
BPB_SEC_PER_TRK   dw 0x12               ;每磁道扇区数:NS
BPB_NUM_HEADS     dw 0x02               ;磁头数:NH
BPB_HIDD_SEC      dd 0                  ;隐藏扇区数
BPB_TOT_SEC32     dd 2880               ;值记录扇区数
BS_DRV_NUM        db 0                  ;中断13的驱动器号
BS_RESERVED1      db 0                  ;未使用
BS_BOOT_SIG       db 0x29               ;扩展引导标记
BS_VOL_D          dd 0                  ;卷序列号
BS_VOL_LAB        db "18340047GXL"      ;11位 卷标
BS_FILE_SYS_TYPE  db "FAT12   "         ;8位 文件系统类型

;-------------------------------------------------------------------------------
;运行入口
;-------------------------------------------------------------------------------
START:
  ;1.初始化段寄存器为同一段
  mov ax,cs
  mov ds,ax
  mov es,ax

  ;2.BIOS调用读取软盘根目录第一块

  mov ax,ROOT_LBA                       ;子过程参数：LBA
  call LBA2CHS                          ;LBA转换子过程 CHS存储到相应位置
  mov ah,02H                            ;读扇区 功能号
  mov al,ROOT_SIZE                      ;扇区数
  mov bx,ROOT_ADDR                      ;es:bx加载到内存的位置 es已经设置过
  mov dl,0                              ;软盘
  int 13H                               ;BIOS调用


  mov ax,[ROOT_ADDR+ENTRY_SIZE]         ;ax=内核程序大小
  xor dx,dx                             ;dx=0
  div word[BPB_BYTS_PER_SEC]            ;ax=ax/512
  cmp dx,0
  jz L1                                 ;ax=ax/512
    inc ax                              ;ax=ax/512+1
  L1:
  push es
  push ax                               ;将ax入栈
  mov ax,[ROOT_ADDR+ENTRY_CLUS]         ;ax=内核程序首簇号
  add ax,CLUS_LBA                       ;ax=LBA
  call LBA2CHS                          ;LBA转换子过程 CHS存储到相应位置
  mov ax,CORE_SEG
  mov es,ax
  pop ax                                ;ax出栈 al=扇区数
  mov ah,02H                            ;ah=功能号
  mov bx,PSP_ADDR                       ;1000h:0100h
  mov dl,0                              ;软盘
  int 13H                               ;BIOS调用 加载程序
  pop es


                                        ;使用系统堆栈作为内核堆栈
  push cs
  push RETURN                           ;把返回地址cs:ip入栈
  jmp CORE_SEG:PSP_ADDR                 ;跳转到内核程序

  RETURN:
    mov ax,cs
    mov ds,ax
    mov es,ax                           ;恢复段

  END_EXEC:
    jmp $                               ;后面可以改成关机

;-------------------------------------------------------------------------------
;子过程
;-------------------------------------------------------------------------------
;输入：ax：LBA
;输出：CHS：ch=柱面 cl=柱头 dh=磁头
LBA2CHS:
  xor dx,dx
  div WORD [BPB_SEC_PER_TRK]
  mov cl,dl
  inc cl                              ;柱头（扇区编号）=LBA%NS+1 保存到cl
  xor dx,dx
  div WORD [BPB_NUM_HEADS]
  mov ch,al                           ;柱面=LBA/NS/NH 保存到 ch
  mov dh,dl                           ;磁头=LBA/NS%NH 保存到 dh
  ret

;-------------------------------------------------------------------------------
;数据段
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;MBR信息2
;-------------------------------------------------------------------------------
times 510-($-$$)  db 0                  ;填充到510字节
END_MARK          dw 0xAA55             ;结束标志
