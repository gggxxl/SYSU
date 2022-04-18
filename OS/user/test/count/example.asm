[BITS 16]                               ;在16位环境执行

[extern _count_char]					          ;引用外部函数符号名

[section .text]							            ;代码段
global _start							              ;将_start入口声明为global
_start:
  ;--------------------------------------设置cs=ds=es 用户栈已由内核设置
  mov ax,cs
  mov ds,ax
  mov es,ax
  ;--------------------------------------调用c函数 获取字符串中CHARX字符的出现次数
  push dword CHARX                      ;传入第二个参数 栈顶移动4字节
  push dword str1                       ;传入第一个参数 栈顶移动4字节
  call dword _count_char                ;调用c函数 返回值在ax中
  add sp,8                              ;平栈 栈顶恢复8字节
  ;--------------------------------------将返回值从最低位开始入栈
  mov dx,0                              ;32位除法 dx:ax=返回值
  mov bx,10								              ;模10
  mov cx,0							 	              ;用于保存位数
  PUSH_RES:
    div bx
    push dx
    inc cx                            	;至少显示0 位数cx>=1
    xor dx,dx
    cmp ax,0
    jnz PUSH_RES
  push cx								                ;将位数入栈
  ;--------------------------------------显示提示信息
  mov ax,1301H                        	;功能号
  mov bp,msg                          	;串地址
  mov cx,total_len                    	;串长
  mov bx,0x0070                       	;页号、颜色
  mov dx,0x1400                       	;行20列0
  int 10H                             	;BIOS调用
  ;--------------------------------------按位显示结果
  pop cx                             	  ;将位数恢复到cx中
  PRINT_RES:
    pop ax
    push cx
    mov cx,1                          	;显示次数
    mov ah,09H
    add al,'0'
    int 10H                           	;显示一位
    mov ah,03h
    int 10H                           	;当前光标位置
    cmp dl,79
    jz EOL
      inc dl
      jmp NEOL
    EOL:
      inc dh
      xor dl,dl                       	;更新光标位置 如果在行末 移到下一行
    NEOL:
    mov ah,02H
    int 10H                           	;移动光标到下一位置
    pop cx
    dec cx
    jnz PRINT_RES
  ret

[section .data]							           ;数据段
msg db "the number of character:"
db CHARX
db " in string:"
str1 db "abbcccccccccccccccccccddddeeeffg"
db 0									                 ;目标字符串结束标志 用于判断结束
db " is:"

total_len equ ($-msg)
CHARX equ 'd'							             ;目标字符
