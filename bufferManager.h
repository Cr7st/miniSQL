/***********************************
* name: Buffer.h
*
* Copyright (c)
*
* creator: 陈云奇
*
* date: 2021-06-16
*
* description: all the classes and structures of the  MiniSQL buffer module
*			   write and delete physical files with this module
*
* version:1.00
***********************************/

#ifndef _BUFFER_MANAGER_H_
#define _BUFFER_MANAGER_H_
#include <vector>
#include <cassert>
#include "ERROR/Error.h"
extern "C" {
#include<io.h>
#include<fcntl.h>
}

#define FileAddrSize (sizeof(FileAddr))

constexpr int FILE_BLOCKSIZE = 8192;	                    // 内存block(==文件页)大小
constexpr int MEM_BLOCKAMOUNT = 4096;                    // 内存block数量
constexpr int MAX_FILENAME_LEN = 256;                   // 文件名（包含路径）最大长度
const unsigned int FILEHI_RESERVE_SPACE = 512;			// 文件头预留空间

class Clock;
class BUFFER;

Clock* GetGlobalClock();
BUFFER& GetGlobalFileBuffer();



/**************************
*  Block Head
**************************/
class BlockHead
{
public:
	void Initialize();		//初始化为第一个block
	unsigned long blockID;	//block id
	bool isFixed;			//是否为常驻内存
};

/**************************
*  File Address
**************************/
class FileAddr
{
	friend class FileHeadInfo;
public:
	void SetFileAddr(const unsigned long _fileBlockID, const unsigned int _offset);
	void ShiftOffset(const int Offset);

	unsigned long fileBlockID; // 文件blockID
	unsigned int offset;	  // 页内偏移量

	bool operator==(const FileAddr& rhs) const
	{
		return (this->fileBlockID == rhs.fileBlockID && this->offset == rhs.offset);
	}

	bool operator!=(const FileAddr& rhs) const
	{
		return !(this->fileBlockID == rhs.fileBlockID && this->offset == rhs.offset);
	}
	bool operator<(const FileAddr& rhs) const
	{
		return (this->fileBlockID < rhs.fileBlockID) || (this->fileBlockID == rhs.fileBlockID) && (this->offset < rhs.offset);
	}
};

/**************************
*  File header information
**************************/
class FileHeadInfo
{
public:
	void Initialize();							//初始化
	FileAddr firstDel;							//第一条被删除地址
	FileAddr lastDel;							//最后一条被删除地址
	FileAddr NewInsert;							//末尾可插入新数据地址
	unsigned long totalBlock;					//目前文件中共有block数
	char reserve[FILEHI_RESERVE_SPACE];			//预留空间
};

/**************************
*  name: Memory block
*  function: provide memory space to save file and relevant info
*  the size of memory block is fixed
**************************/
class MemBlock
{
	friend class MemFile;
	friend class Clock;
	friend class BUFFER;
public:
	MemBlock();
	~MemBlock();
public:
	unsigned long fileID;						//文件指针, fileID 为 0 时为被抛弃的页
	unsigned long fileBlockID;					//文件页码

	mutable bool isLastUsed;					//最近一次访问内存是否被使用，用于Clock算法
	mutable bool isDirty;					//是否为脏页
	mutable bool isPinned;						//是否被锁定
	void* PtrtoBlockBegin;						//实际保存物理文件数据的地址
	BlockHead* blockHead;						//页头指针
	FileHeadInfo* GetFileHeadInfo() const;		//文件头指针
private:
	//把缓存中的block写回到文件中
	void BacktoFile() const;
	//设置为修改过，即脏页
	void SetModified();
	//设置为被锁定
	void SetPinned();
};

/*内存block管理类，管理内存block资源。
将需要读写的文件block读入内存block缓存，将不再需要使用的block在适当时候释放
所谓释放，指的是将不再使用的内存block标记为空闲（需要先写回）以备后续使用
*/
/**************************
*  name: 内存页管理
*  function: 物理页面在内存中的缓存，加速读写
*  不变：调用者保证被载入的物理文件都存在，且加载的页面不越界
**************************/
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

class Clock
{
	friend class MemFile;
	friend class BUFFER;
	friend class BTree;
	//缺如
	friend bool InsertRecord(TB_Insert_Info tb_insert_info, std::string path /*= std::string("./")*/);
	//缺如
	friend bool DropTable(std::string table_name, std::string path);

public:
	Clock();
	~Clock();
private:
	//返回磁盘文件内存地址
	MemBlock* GetMemAddr(unsigned long fileID, unsigned long filePageID);	
	//创造新的Block
	MemBlock* CreateNewBlock(unsigned long fileID, unsigned long fileBlockID);	

private:
	//返回一个可替换的内存页索引
	//原页面内容需先写回
	unsigned int GetReplaceableBlock();

	//如果目标文件Block存在内存缓存则返回地址，否则返回 nullptr
	MemBlock* GetExistBlock(unsigned long fileID, unsigned long filePageID);
	// 从磁盘加载文件Block
	MemBlock* LoadFromFile(unsigned long fileID, unsigned long filePageID);

	//Clock内存替换算法
	unsigned long ClockSwap();

private:
	MemBlock* MemBlocks[MEM_BLOCKAMOUNT + 1];			//内存页对象数组

};
/*
内存文件，实例对应磁盘文件映射，调用内存替换算法clock进行读写
*/
/**************************
*  name: Memory file
*  function:  通过物理文件在内存中的映射文件操作进行物理操作
*  假设所有被操作的文件都存在且已经打开
*  record地址+record数据
**************************/
class MemFile
{
	friend class BUFFER;
	friend class BTree;
	friend bool DropTable(std::string table_name, std::string path);

public:
	/**
	**关于Record 的读取，添加和删除，还有更新
	**/
	//读取Record，返回 Record 指针,包括 Record 地址数据
	const void* ReadRecord(FileAddr* address) const;		
	//读取Record，返回指针,可写入
	void* ReadWriteRecord(FileAddr* address);	
	//新增Record，返回新添加 Record 的位置
	FileAddr AddRecord(const void* address, size_t sz_record);
	//删除Record，返回删除位置
	FileAddr DeleteRecord(FileAddr* address, size_t);	
	//更新记录，返回布尔值
	bool UpdateRecord(FileAddr* address, void* record_data, size_t record_sz);

private:
	//初始化
	MemFile(const char* file_name, unsigned long file_id);
	//读取内存文件，返回读取位置指针
	void* MemRead(FileAddr* memtoRead);				
	//在可写入地址写入数据，若空间不足则申请新的页，返回数据写入地址
	FileAddr MemWrite(const void* source, size_t length);		
	//任意位置写入任意数据，返回写入后的下一个地址位置，若写入失败则返回原地址
	FileAddr MemWrite(const void* source, size_t length, FileAddr* dest);
	//
	void MemWipe(void* source, size_t sz_wipe, FileAddr* fdtoWipe);
	//添加一页空间
	MemBlock* AddExtraBlock();							
	//得到文件首页
	MemBlock* GetFirstBlock();									

private:
	char fileName[MAX_FILENAME_LEN];
	unsigned long fileID;										//文件指针
	unsigned long totalBlock;									//文件中共有页数
};

/*
	Buffer, 通过调用 MemFile，实现对所有磁盘文件的读写
*/
class BUFFER
{
	friend bool DropTable(std::string table_name, std::string path);
public:
	BUFFER() = default;
	~BUFFER();
	//打开文件，打开失败返回 nullptr
	MemFile* operator[](const char* fileName);					

	void CreateFile(const char* fileName);
	void CloseFile(const char* fileName);
	void CloseAllFile();

private:
	MemFile* GetMemFile(const char* fileName);
private:
	std::vector<MemFile*> memFiles;								//已经打开的文件列表
};
#endif // !_BUFFER_MANAGER_H

