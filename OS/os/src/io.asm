[BITS 16]
                  ;extern from c
extern SYS_TIME
                  ;func share for c
global set_xy
global get_xy
global show_c
global get_c
global cur_dw
global cur_up
global io_load
global io_save
global io_init
global upd_sys_time
                  ;func share for asm
global LBA2CHS
                  ;variables share for c
global CUR_PAGE
global CUR_ATTR
global CUR_X
global CUR_Y
global CUR_CLR
global CUR_L
global CUR_U
global CUR_R
global CUR_D
global CUR_MV
global OBJ_PTR
global OBJ_LBA

[SECTION .text]

upd_sys_time:
  push ax
                        ;year
  mov al,9
  out 70h,al
  in al,71h
  mov byte[SYS_TIME],al
                        ;month
  mov al,8
  out 70h,al
  in al,71h
  mov byte[SYS_TIME+2],al
                        ;day
  mov al,7
  out 70h,al
  in al,71h
  mov byte[SYS_TIME+4],al
                        ;hour
  mov al,4
  out 70h,al
  in al,71h
  mov byte[SYS_TIME+6],al
                        ;minute
  mov al,2
  out 70h,al
  in al,71h
  mov byte[SYS_TIME+8],al
                        ;second
  mov al,0
  out 70h,al
  in al,71h
  mov byte[SYS_TIME+10],al
  
  pop ax
  o32 ret

;输入 列x 行y
;输出/效果 光标位置设置
set_xy:
  push ax
  push bx
  push dx
  mov ah,02h
  mov bh,byte[CUR_PAGE]
  mov dh,byte[CUR_Y]  ;行=y
  mov dl,byte[CUR_X]  ;列=x
  int 10h
  pop dx
  pop bx
  pop ax
  o32 ret

get_xy:
  push ax
  push bx
  mov ah,03h
  mov bh,byte[CUR_PAGE]
  int 10h
  mov byte[CUR_X],dl
  mov byte[CUR_Y],dh
  pop bx
  pop ax
  o32 ret

show_c:
  push bp
  mov bp,sp
  push ax
  push bx
  push cx
  mov ah,09h
  mov al,byte[ss:bp+8-2]
  mov bh,byte[CUR_PAGE]
  mov bl,byte[CUR_ATTR]
  mov cx,1
  int 10h
  pop cx
  pop bx
  pop ax
  pop bp
  o32 ret

get_c:
  mov ah,00h
  int 16h
  o32 ret

cur_up:
  push ax
  push bx
  push cx
  push dx
  mov ah,06h
  mov al,[CUR_MV]
  mov bh,byte[CUR_CLR]
  mov ch,byte[CUR_U]
  mov cl,byte[CUR_L]
  mov dh,byte[CUR_D]
  mov dl,byte[CUR_R]
  int 10h
  pop dx
  pop cx
  pop bx
  pop ax
  o32 ret

cur_dw:
  push ax
  push bx
  push cx
  push dx
  mov ah,07h
  mov al,[CUR_MV]
  mov bh,byte[CUR_CLR]
  mov ch,byte[CUR_U]
  mov cl,byte[CUR_L]
  mov dh,byte[CUR_D]
  mov dl,byte[CUR_R]
  int 10h
  pop dx
  pop cx
  pop bx
  pop ax
  o32 ret

io_init:
  push ax
  mov ah,00h
  int 13h
  pop ax
  o32 ret

io_load:
  push bx
  push dx
  mov ax,[OBJ_LBA]
  call dword LBA2CHS
  mov ah,02h
  mov al,1
  mov bx,[OBJ_PTR]
  mov dl,0
  int 13H
  pop dx 
  pop bx
  o32 ret

io_save:
  push bx
  push dx
  mov ax,[OBJ_LBA]
  call dword LBA2CHS
  mov ah,03h
  mov al,1
  mov bx,[OBJ_PTR]
  mov dl,0
  int 13H
  pop dx 
  pop bx
  o32 ret

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
  o32 ret


[SECTION .data]
BPB_SEC_PER_TRK   dw 0x12             ;每磁道扇区数:NS
BPB_NUM_HEADS     dw 0x02             ;磁头数:NH

CUR_PAGE db 0
CUR_X db 0
CUR_Y db 0
CUR_ATTR db 70H
CUR_CLR db 07H
CUR_L db 0
CUR_U db 0
CUR_R db 79
CUR_D db 24
CUR_MV db 1
OBJ_PTR dd 0x0e00
OBJ_LBA dd 14

