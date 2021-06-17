一个文件分成很多个block，每个block固定大小

举例：新增一条记录，从BufferManager取一个block的地址，

具体应该是哪个block由BufferManager控制（文件头当中应该记录了上次写到第几个block的哪个位置了）

> 取到的这个block里同样也告诉我上次写到这个block的哪个位置（应该可以由block的首地址加上已写字节数算出来）以及这个block还剩多少字节。
>
> 我拿到这个block之后，先把他pin锁住，确保我在操作的时候不会同时被其他人操作，导致我要写的位置变了（虽然我们应该不存在并发，走个形式），
>
> 然后比较一下block剩下的空间够不够写下我这一条新记录，
>
> 如果够的话就memcpy把我这条记录的对象的内容copy到这个block可写位置的内存地址，然后更新可写位置和剩余空间，并且把这个block标记为dirty，解锁；

这部分存在于

```c++
FileAddr MemFile::MemWrite(const void* source, size_t length)；
FileAddr MemFile::MemWrite(const void* source, size_t length, FileAddr* dest)；
```

> 如果写不下，解锁，要一个新的block，写进去，标记为dirty。

这部分存在于

```c++
MemBlock* MemFile::AddExtraBlock();
```

> 然后比如要查找的时候，因为我们的所有tuple都是定长的，所以我就可以从直接从每一个block里面memcpy(curPosition, sizeof(tuple))到一个tuple对象里，就可以拿到数据了。

这部分存在于：

```c++
const void* MemFile::ReadRecord(FileAddr* address) const;
void* MemFile::ReadWriteRecord(FileAddr* address)
```

> 表的定义信息应该是写在文件头里的，包括每个属性的名字，属性的类型，哪个是主键，哪些上面有索引，文件头怎么组织看你吧。我也是直接memcpy进去。

插入信息定义如下：

```c++
struct TB_Insert_Info;

//插入信息定义，应该由APILIB定义
struct TB_Insert_Info
{
	using InsertInfo = struct {
		std::string column_name;                        // 插入的字段
		std::string column_value;                       // 插入的值
	};

	std::string table_name;                             // 插入的表名
	std::vector<InsertInfo> insert_info;                // 需要插入的字段集合
};
```

然后存索引文件应该也是一个道理，就是不知道zzh那边他叶节点的指针是怎么指到记录那里去的



