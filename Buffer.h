/***********************************
* name: Buffer.cpp
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

#ifndef _BUFFER_H
#define _BUFFER_H
#include<vector>
#include<cassert>

extern "C" {
#include<io.h>
#include<fcntl.h>
}

#define FileAddrSize (sizeof(FileAddr))

class Clock;
class Buffer;

Clock* GetGlobalClock();
Buffer& GetGlobalBuffer();

const unsigned int FILEHI_RESERVE_SPACE = 512; // 文件头预留空间

/**************************
*  Page Head
**************************/
class PageHead
{
public:
	void Initialize();		//初始化为第一页
	unsigned long pageId;	//page id
	bool isFixed;			//是否为常驻内存
};

/**************************
*  File Address
**************************/
class FileAddr
{
	friend class FileHI;
public:
	void SetFileAddr(const unsigned long _filePageID, const unsigned int _offset);
	void ShiftOffset(const int Offset);

	unsigned long filePageID; // 文件页ID
	unsigned int Offset;	  // 页内偏移量

	bool operator==(const FileAddr& rhs) const
	{
		return (this->filePageID == rhs.filePageID && this->Offset == rhs.Offset);
	}

	bool operator!=(const FileAddr& rhs) const
	{
		return !(this->filePageID == rhs.filePageID && this->Offset == rhs.Offset);
	}
	bool operator<(const FileAddr& rhs) const
	{
		return (this->filePageID < rhs.filePageID) || (this->filePageID == rhs.filePageID) && (this->Offset < rhs.Offset);
	}
};

/**************************
*  File header information
**************************/
class FILEHI
{
public:
	void Initialize();						//
	FileAddr DelFirst;						//第一条被删除地址
	FileAddr DelLast;						//最后一天被删除地址
	FileAddr NewInsert;						//末尾可插入新数据地址
	unsigned long totalPage;				//目前文件中共有页数
	char reserve[FILEHI_RESERVE_SPACE];		
};

/**************************
*  name: Memory page
*  function: provide memory space to save file and relevant info
*  the size of memory page is fixed
**************************/
class MemPage
{
	friend class MemFile;
	friend class Clock;
	friend class Buffer;
public:
	MemPage();
	~MemPage();
private:
	void BacktoFile() const;				//把内存中页写到文件中
	bool SetModified();						//设置为修改过

public:
	unsigned long fileID;					//文件指针
	unsigned long filePageID;				//文件页码

	mutable bool isLastUsed;				//最近一次访问内存是否被使用，用于Clock算法
	mutable bool isModified;				//是否被修改

	void* PtrtoPageBegin;					//实际保存物理文件数据的地址
	PageHead* pageHead;						//页头指针
	FILEHI* GetFileHI() const;				//文件头指针
};

/**************************
*  name: 内存页替换
*  function: provide memory space to save file and relevant info
*  the size of memory page is fixed
**************************/
struct TB_Insert_Info;
class Clock
{
	friend class MemFile;
	friend class Buffer;
	friend class BTree;
	//缺如
	friend bool InsertRecord(TB_Insert_Info tb_insert_info, std::string path /*= std::string("./")*/);
	friend bool DropTable(std::string table_name, std::string path);

public:
	Clock();
	~Clock();
private:
	//返回磁盘文件内存地址
	MemPage* GetMemAddr(unsigned long fileID, unsigned long filePageID);
	//创造新页
	MemPage* CreateNewPage(unsigned long fileID, unsigned long filePageID);

private:
	//返回一个可替换的内存页索引
	//原页面内容需先写回
	unsigned int GetReplaceable();

	//如果目标文件页存在内存缓存则返回地址，否则返回 nullptr
	MemPage* GetExistPage(unsigned long fileID, unsigned long filePageID);
	MemPage* LoadFromFile(unsigned long fileID, unsigned long filePageID);

	//clock swap
	unsigned long ClockSwap();

private:
	MemPage* MemPages[MEM_PAGEAMOUNT + 1];		//内存页对象数组

};

/**************************
*  name: Memory file
*  function:  通过物理文件在内存中的映射文件操作进行物理操作
*  
*  the size of memory page is fixed
**************************/
class MemFile
{
	friend class Buffer;
	friend class BTree;
	friend bool DropTable(std::string table_name, std::string path);

public:
	const void* ReadRecord(FileAddr* address) const;		//读取记录，返回指针
	void* ReadWriteRecord(FileAddr* address);			//读取记录，返回指针
	FileAddr AddRecord(const void* address, size_t sz_record);
	FileAddr DeleteRecord(FileAddr* address, size_t);	//返回删除位置
	bool UpdateRecord(FileAddr* address, void* record_data, size_t record_sz);

private:
	MemFile(const char* file_name, unsigned long file_id);

	void* MemRead(FileAddr* memtoRead);							//读取内存文件，返回读取位置指针
	FileAddr MemWrite(const void* source, size_t length);		//写入数据
	FileAddr MemWrite(const void* source, size_t length, FileAddr* dest);

	void MemWipe(void* source, size_t sz_wipe, FileAddr* fdtoWipe);

	MemPage* AddExtraPage();									//添加一页空间
	MemPage* GetFirstPage();									//得到首页

private:
	char fileName[/*MAX_FILENAME_LEN*/1000];
	unsigned long fileID;										//文件指针
	unsigned long totalPage;									//文件中共有页数
};

class Buffer
{
	friend bool DropTable(std::string table_name, std::string path);
public:
	Buffer() = default;
	~Buffer();
	MemFile* operator[](const char* fileName);					//打开文件，打开失败返回 nullptr

	void CreateFile(const char* fileName);
	void CloseFile(const char* fileName);
	void CloseAllFile();

private:
	MemFile* GetMemFile(const char* fileName);
private:
	std::vector<MemFile*> memFiles;								//已经打开的文件列表
};
#endif // !_BUFFER_H

