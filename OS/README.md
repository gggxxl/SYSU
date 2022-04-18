[TOC]

## 文件说明

```

C:.
│  README.md
│
├─floppy //软盘
│      OS.img	//可运行软盘 含内核与用户程序
│
├─lead	//引导程序
│      lead.asm
│      lead.bin	
│
├─os	//内核
│  │  make.sh	//内核编译脚本
│  │  os.syc	//内核可执行程序
│  │
│  └─src
│          cglobal.h
│          cmain.c
│          dir.c
│          dir.h
│          fat.c
│          fat.h
│          file.c
│          file.h
│          gxlOS.asm
│          int.asm
│          io.asm
│          io.c
│          io.h
│          mydef.h
│          mysys.c
│          mysys.h
│          run.c
│          run.h
│          str.c
│          str.h
│
└─user					//用户程序、C库及编译工具
    │  edit.com			//文件编辑器可执行程序（演示） 
    │  mygcc.sh			//C库程序编译脚本 
    					//使用样例：bash ./mygcc.sh test/editor.c -o edit.com
    │
    ├─lib				//C库
    │      starter.asm	//汇编入口程序
    │      stdio.h		//文件与IO库
    │      string.h		//字符串库
    │
    └─test
        │  editor.c		//文件编辑器源码 使用C库（include）
        │
        ├─count			//计数程序
        │      count.com
        │      example.asm
        │      example.c
        │	
        └─shoot			//字符动画程序
                rucolor.asm
                shoot.com
                shoot_f.com
```

## 系统功能说明

```
[BASIC COMMANDS]:("[a]" means parameter "a" can be ignored)  
(PATH EXAMPLES: "C:\USER\HOULAI.TXT", "COMMAND.COM", ".\USER\..\README.MD", "", ...)	
"ls [PATH]" : show catalog of a  director
"cd [PATH]" : open a director
"tree [PATH]": draw the catalog tree of target director
"md PATH\NAME": create a director with a NAME in target PATH
"rd PATH": delete a director
"new PATH\NAME": create a file with a NAME in target PATH
"rm PATH": delete a file
"read PATH -t": open a file and show in text way
"read PATH -b": open a file and show in binary
"copy PATH1 PATH2[\NAME]": copy file from PATH1 to PATH2 and new NAME is avaliable 
"copy PATH PATH [PATH] [PATH] [PATH] PATH[\NAME]": combine several file to one new file and new NAME is avaliable
"close": close the system
"cls": clear the window (pseudo)
"su": switch user between GXL and ROOT, the later can manage system file
"echo WORDS": just repeat your WORDS
"help": tell you something useless
[BY GXL18340047]
```

注：该帮助在系统中可用help命令查看
