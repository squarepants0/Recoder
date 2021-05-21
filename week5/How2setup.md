# Using qemu-user
```bash
sudo apt install gdb-multiarch
sudo apt-get install qemu-user
sudo apt install qemu-user-static   #for staticall linked
sudo apt-get install qemu-use-binfmt qemu-user-binfmt:i386
```

## For dynamically linked
对于动态链接的程序需要提前安装对应的库
```bash
➜  ~ apt-cache search "libc6-" | grep "arm"
libc6-arm64-cross - GNU C Library: Shared libraries (for cross-compiling)
libc6-armhf-cross - GNU C Library: Shared libraries (for cross-compiling)  #for 32
```
安装完成后在/usr/下有对应文件

## Gdb
远程调试：
启动脚本：
```bash
qemu-arm -g 1234 \
        -L /usr/arm-linux-gnueabihf/ \
        ./vuln
```
连接脚本：
```bash
gdb-multiarch -q ./vuln \
        -ex 'file /usr/arm-linux-gnueabihf/lib/libc-2.27.so' \
        -ex 'file ./vuln'       \
        -ex 'target remote localhost:1235' \
        -ex 'b*$rebase(0x00000F8C)'     \
        -ex 'b*$rebase(0x00000874)'     \
        -ex 'b*$rebase(0x0000100A)'     \
        -ex 'set solib-search-path /usr/arm-linux-gnueabihf/lib/'
```
远程不能加载对方的lib库，需要自己指定：`set solib-search-path /usr/arm-linux-gnueabihf/lib/`

但是我在远程调试的时候总是遇到环境问题


# Using qemu-system
用qemu-system模拟一个操作系统
这里我直接安装一个树梅派(Debian8 jessie)
raspbian 镜像下载地址： https://www.raspberrypi.org/downloads/raspbian/
树莓派内核下载地址： https://github.com/dhruvvyas90/qemu-rpi-kernel

具体安装教程：
+ https://bbs.pediy.com/thread-220907.htm
+ https://azeria-labs.com/emulate-raspberry-pi-with-qemu/

## My settings
安装一个linux系统少不了换源：
```bash
# 编辑 `/etc/apt/sources.list` 文件，删除原文件所有内容，用以下内容取代：
deb http://mirrors.tuna.tsinghua.edu.cn/raspbian/raspbian/ jessie main non-free contrib rpi
deb-src http://mirrors.tuna.tsinghua.edu.cn/raspbian/raspbian/ jessie main non-free contrib rpi

# 编辑 `/etc/apt/sources.list.d/raspi.list` 文件，删除原文件所有内容，用以下内容取代：
deb http://mirrors.tuna.tsinghua.edu.cn/raspberrypi/ jessie main ui
```

然后最好扩大树梅派的磁盘

启动脚本：
```bash
sudo qemu-system-arm \
-kernel ./kernel-qemu-4.4.34-jessie \
-cpu arm1176	\
-m 256	\
-M versatilepb	\
-serial	stdio	\
-append "root=/dev/sda2 panic=1 rootfstype=ext4 rw" \
-drive "file=2017_jessie.img,index=0,media=disk,format=raw" \
-net user,hostfwd=tcp::5555-:22,hostfwd=tcp::6666-:666	\
-net nic -net tap,ifname=vnet0,script=no,downscript=no \
-no-reboot 
```
+ kernel：指定内核
+ cpu：指定cpu
+ m：指定内存大小
+ M：模拟的机器，versatilepb：`ARM Versatile/PB`
+ serial：重定位虚拟串口到主机的stdio
+ append：表明文件系统的位置和类型
+ drive：attach文件系统镜像
+ net：设置网卡，这里开放啦两个端口5555用于ssh连接，6666用于gdbserver连接

启动后开启ssh服务：
```bash
 sudo service ssh start
 sudo update-rc.d ssh enable
```
在主机上用ssh连接后使用命令`scp -P 端口 本地path pi@127.0.0.1:/tmp`将本地文件上传到树梅派的/tmp中

### gdbserver
从`http://ftp.gnu.org/gnu/gdb/`找到对应主机(gdb版本)的gdbserver的源码(好像版本不对会出bug)，使用scp上传到树梅派进行编译：
```bash
$ CC="arm-linux-gnueabihf-gcc" CXX="arm-linux-gnueabihf-g++" ./configure --target=arm-linux-gnueabi --host="arm-linux-gnueabi" --prefix="setup-directory"
$ make install
```
因为我的主机是ubuntu2004所以没有低版本的包来编译，在树梅派中无法运行，所以干脆让树梅派自己编译
效果：
```bash
i@raspberrypi:~/workspace/shellcode/shell1 $ sudo gdbserver 0.0.0.0:666 ./execve3
Process ./execve3 created; pid = 904
Listening on port 666
Remote debugging from host 10.0.2.2

#Host
➜  ~ gdb-multiarch 
pwndbg> set architecture arm
The target architecture is assumed to be arm
pwndbg> target remote 0.0.0.0:6666
Remote debugging using 0.0.0.0:6666
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
0x00010054 in ?? ()
Downloading '/home/pi/workspace/shellcode/shell1/execve3' from the remote server: OK
add-symbol-file /tmp/tmpqa1wk1xg/execve3 0x10000 -s .text 0x20054

#remote
pi@raspberrypi:~/workspace/shellcode/shell1 $ sudo cat /proc/904/maps 
00010000-00011000 rwxp 00000000 08:02 261310     /home/pi/workspace/shellcode/shell1/execve3
b6fff000-b7000000 r-xp 00000000 00:00 0          [sigpage]
befdf000-bf000000 rwxp 00000000 00:00 0          [stack]
ffff0000-ffff1000 r-xp 00000000 00:00 0          [vectors]

#Host vmmap
pwndbg> vmmap
LEGEND: STACK | HEAP | CODE | DATA | RWX | RODATA
   0x10000    0x11000 rwxp     1000 0      /home/pi/workspace/shellcode/shell1/execve3
0xb6fff000 0xb7000000 r-xp     1000 0      [sigpage]
0xbefdf000 0xbf000000 rwxp    21000 0      [stack]
0xffff0000 0xffff1000 r-xp     1000 0      [vectors]
```
可以看到用gdbserver远程调试环境是比较准确的