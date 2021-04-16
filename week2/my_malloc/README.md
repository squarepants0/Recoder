# 拙劣模仿了一下malloc.c，一个简略的显示链表分配器

chunk模型

```bash
	chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		    |             Size of previous chunk                            |
		    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	`head:' |             Size of chunk, in bytes                         |P|
	  mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		    |             Forward pointer to next chunk in list             |
		    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		    |             Back pointer to previous chunk in list            |
		    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		    |             Unused space (may be 0 bytes long)                .
		    .                                                               .
		    .                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	`foot:' |             Size of chunk, in bytes                           |
		    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

其中判断一个p位是判断，本chunk是否处于inuse状态的标志之一(偷个懒 : ) )

内存管理结构体：mstate

```c
struct mm_malloc_state{
	fchunkptr fastbinsY[NFASTBINS];
	schunkptr smallbins[NSMALLBINS];
	unsortedbinptr ub[2];
	chunkptr top;
}mstate;
```

采用fastbinY和smallbins来满足exactly fit。ub存放大chunk或者切割的剩余部分，主要来进行切割。

和malloc.c一样fastbin chunk使用fd字段单链表，smallbin chunk， UB chunk使用双链表。

## Functions

### mm_malloc

首先通过top检查初始化情况，然后根据nb分别从fastbin，smallbin，UB中查找，最后选择切割top。



### mm_malloc_init

先通过mem_init获取堆区0x10000大小作为top，然后初始化链表的值。



### consolidate

遍历每一个fastbin链表对fastbin chunk进行上下合并操作，合并后的chunk根据大小放入smallbin或者UB



### mm_free

如果chunk是fastbin范围就直接放入对应fastbin链表，否则进行合并操作，放入UB