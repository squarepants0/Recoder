# Buffer overflow 
常见的缓冲区溢出，都不是很难，最后一个Nitroglycerin在进入进入testn前会获取安排在栈上的offset来调整栈顶
```assembly
   0x8048f3d <launcher+1>     mov    ebp, esp
   0x8048f3f <launcher+3>     sub    esp, 0x28
   0x8048f42 <launcher+6>     mov    eax, dword ptr [ebp + 8]
   0x8048f45 <launcher+9>     mov    dword ptr [global_nitro], eax <0x804d0f8>
   0x8048f4a <launcher+14>    mov    eax, dword ptr [ebp + 0xc]
 ► 0x8048f4d <launcher+17>    mov    dword ptr [global_offset], eax <0x804d0f4>
   0x8048f52 <launcher+22>    mov    dword ptr [esp + 0x14], 0
   0x8048f5a <launcher+30>    mov    dword ptr [esp + 0x10], 0
   0x8048f62 <launcher+38>    mov    dword ptr [esp + 0xc], 0x132
   0x8048f6a <launcher+46>    mov    dword ptr [esp + 8], 7
   0x8048f72 <launcher+54>    mov    dword ptr [esp + 4], 0x100000

08:0020│      0xffffcf20 ◂— 0x4
09:0024│      0xffffcf24 —▸ 0xf7e2e49d (random+13) ◂— add    ebx, 0x183b63
0a:0028│ ebp  0xffffcf28 —▸ 0xffffcf68 ◂— 0x0
0b:002c│      0xffffcf2c —▸ 0x80491dd (main+471) ◂— add    ebx, 1
0c:0030│      0xffffcf30 ◂— 0x1
0d:0034│      0xffffcf34 ◂— 0x1b0   <=============
0e:0038│      0xffffcf38 ◂— 0x2ca564b9
```
这个offset是在调用launcher之前依靠random函数获得的。

然后由launcher进入launch
```assembly
   0x8048fd9 <launcher+157>    mov    edx, dword ptr [global_offset] <0x804d0f4>
   0x8048fdf <launcher+163>    mov    eax, dword ptr [global_nitro] <0x804d0f8>
 ► 0x8048fe4 <launcher+168>    call   launch <launch>
        arg[0]: 0x0
        arg[1]: 0x0
        arg[2]: 0x0
        arg[3]: 0x11
```

在launch中调整esp
```assembly
 EAX  0x0
 EBX  0x1
*ECX  0x1fa0
 EDX  0x1b0
 EDI  0x5

   0x8048eb9 <launch+23>    and    ecx, 0x3ff0
 ► 0x8048ebf <launch+29>    lea    eax, [ecx + edx + 0x1e] <====
   0x8048ec3 <launch+33>    and    eax, 0xfffffff0
   0x8048ec6 <launch+36>    sub    esp, eax       <======
   0x8048ec8 <launch+38>    lea    eax, [esp + 0x1b]
```
最后在getbufn中发现Gets的目标地址最大是：0x55683be8，最小为：0x55683b68。所以最后覆盖返回地址为0x55683be8，前面用128个nop覆盖即可。

然后就是由于开启了canary类型的保护需要保护原ebp的值，我们可以通过esp和原ebp固定偏移来获得原ebp，然后pop ebp即可

