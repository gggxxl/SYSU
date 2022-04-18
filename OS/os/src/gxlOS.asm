[BITS 16]

extern cmain
extern LBA2CHS
extern int_set
extern int_clr

global _start
global load_com
global exec_com
global create_psp

global USER_SAVE
global USER_RESTART
global CORE_SAVE
global CORE_RESTART

global COM_PTR
global COM_LBA
extern COM_SEG
extern COM_PSP
extern PCB_LIST
extern CURRENTPID
global RETURN

CORE_STACK_SEG equ 0x1000
CORE_STACK_BASE equ 0xFFF0

[SECTION .text]
_start:
  mov ax,cs
  mov ds,ax
  mov es,ax

  mov ax,CORE_STACK_SEG
  mov ss,ax
  mov bp,CORE_STACK_BASE
  mov sp,CORE_STACK_BASE

  call dword cmain
  jmp $

;-----------------------------------------------
load_com:
  push dx
  push bx
  push ax
  push es

  mov ax,[COM_SEG]
  mov es,ax
  mov ax,[COM_LBA]
  call dword LBA2CHS
  mov ah,02h
  mov al,1
  mov bx,[COM_PTR]
  mov dl,0
  int 13H

  pop es
  pop ax
  pop bx
  pop dx
  o32 ret
;-----------------------------------------------
create_psp:
  push es
  push ax
  mov ax,[COM_SEG]
  mov es,ax
                            ;int 20H code
  mov word[es:0],0x20cd
  mov word[es:0xFFF0],0x0000
  pop ax
  pop es
  o32 ret

;-----------------------------------------------
exec_com:
                            ;保存内核状态并开启时钟中断（进程轮转）
  call dword CORE_SAVE
                            ;开始执行用户程序
  call dword USER_RESTART

  RETURN:
    call dword CORE_RESTART
    o32 ret
;-----------------------------------------------
CORE_SAVE:
  CLI
  pop dword[CS:TEMP_ADDR]

  pushad
  pushf
  call dword int_set

  mov word[CORE_SP],sp
  mov word[CORE_BP],bp

  STI
  jmp [CS:TEMP_ADDR]
;-----------------------------------------------
CORE_RESTART:
    CLI
    pop dword[CS:TEMP_ADDR]
                            ;set ds before use data
    mov ax,cs
    mov ds,ax
    mov es,ax
                            ;->core stack
    mov ax,CORE_STACK_SEG
    mov ss,ax
    mov sp,word[CORE_SP]
    mov bp,word[CORE_BP]

    call dword int_clr
    popf
    popad

    STI
    jmp [CS:TEMP_ADDR]

;-----------------------------------------------
USER_SAVE:
  push ax
  push dx           ;注意会被乘法改变
  push di
                    ;获得当前PCB块的地址
  mov di,PCB_LIST
  mov ax,0x2b
  mul word[CS:CURRENTPID]  ;PCB的size
  add di,ax

  pop ax            ;user di
  mov [CS:di+8],ax
  pop dx
  pop ax            ;user ax

                    ;use CS because DS is not set here
  mov [CS:di],ax
  mov [CS:di+2],bx
  mov [CS:di+4],cx
  mov [CS:di+6],dx
                    ;addr for ret
  pop dword[CS:TEMP_ADDR]
                    ;USER's EFLAGS,CS,IP in INT stack
  pop ax
  mov [CS:di+28],ax
  pop ax
  mov [CS:di+26],ax
  pop ax
  mov [CS:di+30],ax
                    ;save sp,bp after pop EFLAGS,CS,IP
  mov [CS:di+12],sp
  mov [CS:di+14],bp
  mov [CS:di+16],SS

  mov [CS:di+10],si
  mov [CS:di+18],DS
  mov [CS:di+20],ES
  mov [CS:di+22],FS
  mov [CS:di+24],GS
                  ;save() shouldn't change regs or user stacks
  mov ax,[CS:di]
  mov di,[CS:di+8]
                  ;use ret addr to return
  jmp [CS:TEMP_ADDR]


;-----------------------------------------------
USER_RESTART:
                    ;获得当前PCB块的地址
  mov di,PCB_LIST
  mov ax,0x2b
  mul word[CS:CURRENTPID]  ;PCB的size
  add di,ax
                    ;use CS because DS is not set here
  mov ax,[CS:di]
  mov bx,[CS:di+2]
  mov cx,[CS:di+4]
  mov dx,[CS:di+6]
  mov si,[CS:di+10]
  mov ds,[CS:di+18]
  mov es,[CS:di+20]
  mov fs,[CS:di+22]
  mov gs,[CS:di+24]
                    ;restart USER's stack
  mov sp,[CS:di+12]
  mov bp,[CS:di+14]
  mov ss,[CS:di+16]
                    ;push EFLAGS,CS,IP for iret
  push word[CS:di+30]
  push word[CS:di+26]
  push word[CS:di+28]
  mov di,[CS:di+8]
                    ;use iret to restart EFLAGS,CS,IP
  iret



[SECTION .data]
COM_PTR dd 0      ;加载地址
COM_LBA dd 0
CORE_BP dw 0      ;内核堆栈
CORE_SP dw 0


TEMP_ADDR dw 0
