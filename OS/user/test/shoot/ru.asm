;-----------------------------define常量
define:
	PSP_STA equ 00h					;PSP段内地址
	CODE_STA equ 100h				;指令开始地址

	t equ 256								;移动次数
	t1 equ 2000							;移动时间因子1
	t2 equ 100							;移动时间因子2
	l_up_x equ 0						;显示范围左上x坐标
	l_up_y equ 40						;显示范围左上y坐标
	r_down_x equ 12					;显示范围右下x坐标
	r_down_y equ 79					;显示范围右下y坐标
	tail_attr equ 6Fh				;显示尾迹属性 青色

	N equ locx-mystr-1			;字符串末字符相对首字符偏移地址
;-----------------------------告诉编译器程序会被加载到那个位置 修正地址
	org CODE_STA

start:
;-----------------------------ds=cs，使数据段和代码段一致，在同512B内
	mov ax,cs
	mov ds,ax
;-----------------------------es=0xb800，显存起始位置
	mov ax,0xb800
	mov es,ax
;-----------------------------隐藏光标
;	mov cx,2000h
;	mov ah,01h
;	int 10h
;-----------------------------字符弹射程序 可结束
	mov cx,t
	sloop:
		push cx
		call show
		call delay
		call update
		pop cx
		loop sloop

		ret

;-----------------------------子程序：显示字符
show:
	mov cx,N+1							;循环次数：字符串长度
	mov si,N*2							;坐标数据偏移地址（字）
	mov di,N								;字符数据偏移地址（字节）
;-----------------------------循环显示各个字符
	hloop:
;-----------------------------计算显示位置：显存地址
		mov ax,[si+locx]
		mov bx,80
		mul bx
		add ax,[si+locy]
		mov bx,2							;每个字符两字节
		mul bx
		mov bx,ax
;-----------------------------如是尾字符则显示颜色，可以形成尾迹
		cmp si,N*2
		jnz notail
		mov ah,tail_attr
		jz tail
notail:
		mov ah,0Fh
tail:
		mov al,byte[di+mystr]	;显示对应字符
		mov [es:bx],ax
		dec si
		dec si
		dec di								;更新偏移地址
		loop hloop
	ret

;-----------------------------产生延迟 此处执行t2*t1级别条指令
delay:
	dloop:
		dec word[cnt]
		jnz dloop
		mov word[cnt],t1
		dec word[dcnt]
		jnz dloop
		mov word[cnt],t1
		mov word[dcnt],t2
	ret

;-----------------------------更新显示位置
update:
;-----------------------------更新除头部数据 每个坐标更新为前一字符坐标
	mov ax,es
	mov bx,ds
	mov es,bx
	std											;反向传送（从后往前处理）
;-----------------------------更新x坐标
	mov si,locx
	add si,2*N-2
	mov di,si
	inc di
	inc di
	mov cx,N
	rep movsw
;-----------------------------更新y坐标
	mov si,locy
	add si,2*N-2
	mov di,si
	inc di
	inc di
	mov cx,N
	rep movsw
	mov es,ax
;-----------------------------在边缘更新方向（头部坐标增量）
	cmp word[locx],l_up_x
	jz turnX
	cmp word[locx],r_down_x
	jz turnX
	jnz noTurnX
turnX:
	xor ax,ax
	sub ax,[tx]
	mov [tx],ax
noTurnX:
	cmp word[locy],l_up_y
	jz turnY
	cmp word[locy],r_down_y
	jz turnY
	cmp word[locy],l_up_y+1
	jz turnY
	cmp word[locy],r_down_y-1
	jz turnY
	jnz noTurnY
turnY:
	xor ax,ax
	sub ax,[ty]
	mov [ty],ax
noTurnY:
;-----------------------------更新头部坐标
	mov ax,[locx]
	add ax,[tx]
	mov [locx],ax
	mov ax,[locy]
	add ax,[ty]
	mov [locy],ax
	ret

;-----------------------------数据
data:
data:
	tcnt dw t
	cnt dw t1										;延迟数
	dcnt dw t2
	tx dw 1										;方向（头部坐标增量）
	ty dw 2
	mystr db "18340047-GXL "		;显示的字符串
	locx times N+1 dw r_down_x		;各字符坐标数据
	locy times N+1 dw r_down_y
