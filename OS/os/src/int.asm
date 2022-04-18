[BITS 16]
global int_set
global int_clr

extern user_open
extern user_read
extern user_write
extern user_close
extern sched_com

extern CORE_SAVE
extern CORE_RESTART
extern RETURN
extern PCB_LIST
extern CURRENTPID
extern USER_SAVE
extern USER_RESTART

[SECTION .text]
int_set:
                                ;设置中断向量前关中断 防止中断过程地址设置被打断
  CLI

  push es
  push ax

  mov ax,0
  mov es,ax
                                ;备份旧的中断过程地址
  mov ax,word[es:0x08*0x4+0x2]
  mov word[int_08_cs],ax
  mov ax,word[es:0x08*0x4]
  mov word[int_08_ip],ax
                                ;设置自定义的中断过程地址
  mov word[es:0x08*0x4],TIME
  mov word[es:0x20*0x4],INT20H
  mov word[es:0x21*0x4],INT21H
  mov word[es:0x22*0x4],INT22H
  mov ax,cs
  mov word[es:0x08*0x4+0x2],ax
  mov word[es:0x20*0x4+0x2],ax
  mov word[es:0x21*0x4+0x2],ax
  mov word[es:0x22*0x4+0x2],ax

  pop ax
  pop es

  STI                           ;开中断

  o32 ret

int_clr:
  CLI                           ;恢复中断表也是设置中断 也要关中断

  push es
  push ax

  mov ax,0
  mov es,ax
                                ;从数据段中恢复原中断功能
  mov ax,word[int_08_cs]
  mov word[es:0x08*0x4+0x2],ax
  mov ax,word[int_08_ip]
  mov word[es:0x08*0x4],ax

  pop ax
  pop es

  STI                           ;开中断

  o32 ret

                                ;中断处理过程 修改原中断功能
TIME:
  call dword USER_SAVE

  mov ax,cs
  mov ds,ax
  mov ax,0xb800
  mov gs,ax
  mov di,flash_data
  add di,word[flash_cur]
  mov ah,0x03
  mov al,byte[di]               ;di记录当前帧
  mov word[gs:(80*25-1)*2],ax   ;将显示的字符及其属性放到最后一行最后一列的显存位置

  cmp word[flash_cur],flash_len-1
  jnz FLASH_DONE
  mov word[flash_cur],-1        ;di超过循环节 下一次回到第0帧

  FLASH_DONE:
                                ;执行原中断功能
    pushf
    push cs
    push I08_DONE
    jmp far [int_08_ip]
                                ;原中断功能执行完毕
    I08_DONE:
      inc word[flash_cur]
                                  ;轮转调度算法
      call dword sched_com

      mov al,0x20                 ;al=EOI end of interrupt
      out 0x20,al                 ;发送EOI到主8529A端口
      out 0xA0,al                 ;发送EOI到从8529A端口

      call dword USER_RESTART

                                   ;新中断号 新功能
INT20H:
  jmp RETURN                   ;当前进程结束 返回内核执行下一个


INT21H:
  cmp ah,0x00
  jz INT20H

  call dword USER_SAVE
  call dword CORE_RESTART
  pusha

  ;获得当前PCB块的地址
  mov di,PCB_LIST
  mov ax,0x2b
  mul word[CS:CURRENTPID]  ;PCB的size
  add di,ax

  mov ax,[CS:di]
  mov bx,[CS:di+2]
  mov cx,[CS:di+4]
  mov dx,[CS:di+6]

  cmp ah,0x3D
  jz INT21H_3D
  cmp ah,0x3E
  jz INT21H_3E
  cmp ah,0x3F
  jz INT21H_3F
  cmp ah,0x40
  jz INT21H_40

;文件打开
INT21H_3D:
  call dword GET_USER
  and eax,0xFF
  push eax
  push dword user_data
  call dword user_open
  add sp,8                    ;flatten the stack!
  mov word[CS:di],ax
  jmp INT21H_END

;文件关闭
INT21H_3E:
  push ebx
  call dword user_close
  add sp,4                    ;flatten the stack!
  mov word[CS:di],ax
  jmp INT21H_END

;文件(含设备)读
INT21H_3F:
  push ebx
  push ecx
  push dword user_data
  call dword user_read
  add sp,12                   ;flatten the stack!
  call dword GIVE_USER
  mov word[CS:di],ax
  jmp INT21H_END

;文件（含设备）写
INT21H_40:
  call dword GET_USER
  push ebx
  push ecx
  push dword user_data
  call dword user_write
  add sp,12                   ;flatten the stack!
  mov word[CS:di],ax
  jmp INT21H_END

INT21H_END:
  popa
  call dword CORE_SAVE
  call dword USER_RESTART

;-----------------------------------------------
GET_USER:
  pusha
  push ds
  push es

  ;获得当前PCB块的地址
  mov di,PCB_LIST
  mov ax,0x2b
  mul word[CS:CURRENTPID]  ;PCB的size
  add di,ax

  cld
  mov cx,word[CS:di+18]
  mov ds,cx
  mov cx,cs
  mov es,cx
  mov si,word[CS:di+6]           ;DS:SI
  mov cx,word[CS:di+4]
  cmp cx,128
  js GETNOTFULL
  mov cx,128
  GETNOTFULL:
    mov di,user_data             ;ES:DI
    rep movsb

  pop es
  pop ds
  popa
  o32 ret

;-----------------------------------------------
GIVE_USER:
  pusha
  push ds
  push es

  ;获得当前PCB块的地址
  mov di,PCB_LIST
  mov ax,0x2b
  mul word[CS:CURRENTPID]  ;PCB的size
  add di,ax

  cld
  mov cx,word[CS:di+18]
  mov es,cx
  mov cx,cs
  mov ds,cx
  mov si,user_data                ;tar ES:DI
  mov cx,word[CS:di+4]
  cmp cx,128
  js GIVENOTFULL
  mov cx,128
  GIVENOTFULL:
    mov di,word[CS:di+6]            ;src DS:SI
    rep movsb

  pop es
  pop ds
  popa
  o32 ret

INT22H:
  call dword USER_SAVE
                                    ;设置段寄存器 gs为显存地址
  mov ax,cs
  mov ds,ax
  mov ax,0xb800
  mov gs,ax

  mov si,(80*13+36)*2               ;显示位置13行36列
  mov di,INT22H_color
  add di,word[INT22H_color_cur]     ;di=当前颜色属性的地址
  mov ah,byte[di]                   ;ah=当前颜色属性
                                    ;使用循环显示OUCH字符串
  mov di,INT22H_msg
  INT22H_LOOP:
    mov al,byte[di]
    mov word[gs:si],ax
    inc si
    inc si
    inc di
    cmp di,INT22H_msg+INT22H_msg_len
    jnz INT22H_LOOP
                                  ;下一颜色属性地址重置
  cmp word[INT22H_color_cur],INT22H_color_len-1
  jnz INT22H_DONE
  mov word[INT22H_color_cur],-1
                                  ;新功能执行完毕
  INT22H_DONE:
                                  ;更新下一颜色属性的地址
      inc word[INT22H_color_cur]
                                  ;将保存的寄存器恢复
      call dword USER_RESTART



[SECTION .data]
flash_data db '|','/','\'       ;字符动画内容
flash_len equ $-flash_data      ;字符动画循环节
flash_cur dw 0

user_data times 128 db 0        ;内核-用户 数据缓冲

INT22H_msg db "INT22H"               ;INT22H字符串
INT22H_msg_len equ $-INT22H_msg      ;INT22H字符串长度
INT22H_color db 0x03,0x06,0x07,0x70  ;字符属性（轮流）
INT22H_color_len equ $-INT22H_color  ;字符属性个数
INT22H_color_cur dw 0                ;当前字符属性的地址

int_08_ip dw 0
int_08_cs dw 0                ;08H原功能地址
