/***********************************
* name: Buffer.cpp
*
* Copyright (c)
*
* creator: 陈云奇
*
* date: 2021-06-16
*
* descripti: define all the classes and structures of the  MiniSQL buffer module
*
* version:1.00
***********************************/
#include "BufferManager.h"
#include <iostream>
#include <string>

//全局内存缓存页
Clock* GetGlobalClock()
{
	static Clock MemClock;
	return &MemClock;
}

BufferManager& GetGlobalFileBuffer()
{
	static BufferManager buffer;
	return buffer;
}

//BlockHead

void BlockHead::Initialize()
{
	blockID = 0;
}

//FileAddr

void FileAddr::SetFileAddr(const unsigned long _fileBlockID, const unsigned int _offset)
{
	fileBlockID = _fileBlockID;
	offset = _offset;
}

void FileAddr::ShiftOffset(const int OFFSET)
{
	this->offset += OFFSET;
}

//FileHeadInfo

void FileHeadInfo::Initialize()
{
	totalBlock = 1;
	FileAddr tmpFH;
	tmpFH.SetFileAddr(0, 0);
	firstDel = tmpFH;
	lastDel = tmpFH;
	tmpFH.offset = sizeof(BlockHead) + sizeof(FileHeadInfo);
	NewInsert = tmpFH;
	memset(reserve, 0, FILEHI_RESERVE_SPACE);
}

//MemBlock

MemBlock* MemFile::AddExtraBlock()
{
	Clock* pMemClock = GetGlobalClock();
	//获取首个block
	MemBlock* firstBlock = pMemClock->GetMemAddr(this->fileID, 0);
	this->totalBlock = firstBlock->GetFileHeadInfo()->totalBlock + 1;
	firstBlock->GetFileHeadInfo()->totalBlock += 1;
	firstBlock->SetModified();
	firstBlock->isLastUsed = true;

	//创造新内存block
	MemBlock* newMemBlock = pMemClock->CreateNewBlock(this->fileID, firstBlock->GetFileHeadInfo()->totalBlock - 1);
	newMemBlock->isDirty = true;
	newMemBlock->isLastUsed = true;

	return newMemBlock;
}

MemBlock* MemFile::GetFirstBlock()
{
	return GetGlobalClock()->GetMemAddr(this->fileID, 0); //文件首页
}

MemBlock::MemBlock()
{
	PtrtoBlockBegin = malloc(FILE_BLOCKSIZE);
	blockHead = (BlockHead*)PtrtoBlockBegin;
	fileID = 0;
	isDirty = false;
	isLastUsed = false;
}

MemBlock::~MemBlock()
{
	if (this->isDirty && this->fileID > 0)
	{
		this->BacktoFile();
	}
	delete PtrtoBlockBegin;
}

FileHeadInfo* MemBlock::GetFileHeadInfo() const
{
	return (FileHeadInfo*)((char*)PtrtoBlockBegin + sizeof(BlockHead));
}

void MemBlock::BacktoFile() const
{
	//脏页需要写回
	if (this->isDirty && this->fileID > 0)
	{
		int tmp = 0;
		tmp = lseek(this->fileID, this->fileBlockID * FILE_BLOCKSIZE, SEEK_SET);
		//
		if (tmp == -1)throw SQLError::LSEEK_ERROR();
		//printf("SQLError::LSEEK_ERROR()");
		tmp = write(this->fileID, this->PtrtoBlockBegin, FILE_BLOCKSIZE);//写回文件
		if (tmp != FILE_BLOCKSIZE)throw SQLError::WRITE_ERROR();
		//printf("SQLError::WRITE_ERROR()");
		isDirty = false;
		isLastUsed = true;
	}
}

void MemBlock::SetModified()
{
	isDirty = true;
}

//Clock

Clock::Clock()
{
	for (int i = 0; i <= MEM_BLOCKAMOUNT; i++)
	{
		MemBlocks[i] = nullptr;
	}
}

Clock::~Clock()
{
	for (int i = 0; i <= MEM_BLOCKAMOUNT; i++)
	{
		if (MemBlocks[i] != nullptr)
			delete MemBlocks[i];
	}
}

MemBlock* Clock::GetMemAddr(unsigned long fileID, unsigned long fileBlockID)
{
	//先查找是否存在内存中
	MemBlock* pMemBlock = GetExistBlock(fileID, fileBlockID);
	if (pMemBlock != nullptr)
		return pMemBlock;

	// 否则，从磁盘换入
	return LoadFromFile(fileID, fileBlockID);
}

MemBlock* Clock::CreateNewBlock(unsigned long fileID, unsigned long fileBlockID)
{
	// 初始化内存block对象
	auto i = GetReplaceableBlock();
	memset(MemBlocks[i]->PtrtoBlockBegin, 0, FILE_BLOCKSIZE);
	MemBlocks[i]->fileID = fileID;
	MemBlocks[i]->fileBlockID = fileBlockID;
	MemBlocks[i]->isDirty = true;  //设置为脏页，需要写回

	// 初始化新block的block头信息
	MemBlocks[i]->blockHead->blockID = fileBlockID;
	if (fileBlockID != 0)
	{
		MemBlocks[i]->blockHead->isPinned = 0;
	}
	else
	{
		MemBlocks[i]->blockHead->isPinned = 1;
		MemBlocks[i]->blockHead->Initialize();
	}
	return MemBlocks[i];
}

unsigned int Clock::GetReplaceableBlock()
{
	// 查找未分配的内存页
	for (int i = 1; i <= MEM_BLOCKAMOUNT; i++)
	{
		if (MemBlocks[i] == nullptr)
		{
			MemBlocks[i] = new MemBlock();
			return 1;
		}
	}

	// 查找被抛弃的页
	for (int i = 1; i <= MEM_BLOCKAMOUNT; i++)
	{
		if (MemBlocks[i]->fileID == 0)
			return i;
	}

	// clock算法
	unsigned int i = ClockSwap();
	// unsigned int i = rand() % MEM_PAGEAMOUNT;
	if (i == 0)i++;
	MemBlocks[i]->BacktoFile();
	return i;
}

MemBlock* Clock::GetExistBlock(unsigned long fileID, unsigned long fileBlockID)
{
	// 寻找在memBlock List 的 Block
	for (int i = i; i <= MEM_BLOCKAMOUNT; i++)
	{
		if (MemBlocks[i] && MemBlocks[i]->fileID == fileID && MemBlocks[i]->fileBlockID == fileBlockID)
			return MemBlocks[i];
	}
	return nullptr;
}

// 从磁盘加载文件Block
MemBlock* Clock::LoadFromFile(unsigned long fileID, unsigned long fileBlockID)
{
	unsigned int freeBlock = GetReplaceableBlock();
	MemBlocks[freeBlock]->fileID = fileID;
	MemBlocks[freeBlock]->fileBlockID = fileBlockID;
	MemBlocks[freeBlock]->isDirty = false;
	MemBlocks[freeBlock]->isLastUsed = true;

	try
	{
		assert(fileID > 0);
		assert(fileBlockID >= 0);
		long offset_t = lseek(fileID, fileBlockID * FILE_BLOCKSIZE, SEEK_SET);					// 定位到将要取出的文件页的首地址
		if (offset_t == -1) throw SQLError::LSEEK_ERROR();
		//printf("throw SQLError::LSEEK_ERROR();");
		long byte_count = read(fileID, MemBlocks[freeBlock]->PtrtoBlockBegin, FILE_BLOCKSIZE);	// 读入内存
		if (byte_count == 0)throw SQLError::READ_ERROR();
		//printf("throw SQLError::READ_ERROR();");
	}
	catch (const SQLError::BaseError& e)
	{
		DispatchError(e);
	}


	return MemBlocks[freeBlock];
}

unsigned long Clock::ClockSwap()
{
	static unsigned long index = 1;
	assert(MemBlocks[index] != nullptr);

	while (MemBlocks[index]->isLastUsed)		//最近被使用过
	{
		MemBlocks[index]->isLastUsed = 0;
		index = (index + 1) % MEM_BLOCKAMOUNT;
		if (index == 0)index++;
	}

	auto res = index;
	MemBlocks[index]->isLastUsed = 1;
	index = (index + 1) % MEM_BLOCKAMOUNT;
	if (index == 0)index++;
	return res;
}

//MemFile

const void* MemFile::ReadRecord(FileAddr* address) const
{
	auto pMemBlock = GetGlobalClock()->GetMemAddr(this->fileID, address->fileBlockID);
	return (char*)(pMemBlock->PtrtoBlockBegin) + address->offset;
}

void* MemFile::ReadWriteRecord(FileAddr* address)
{
	auto pMemBlock = GetGlobalClock()->GetMemAddr(this->fileID, address->fileBlockID);
	pMemBlock->isDirty = true;
	return (char*)(pMemBlock->PtrtoBlockBegin) + address->offset;
}

FileAddr MemFile::AddRecord(const void* const source, size_t sz_record)
{
	auto pMemBlock = GetGlobalClock()->GetMemAddr(this->fileID, 0);
	auto pFileHI = pMemBlock->GetFileHeadInfo();
	FileAddr fh;	//写入位置
	void* tmpSource;
	if (pFileHI->firstDel.offset == 0 && pFileHI->lastDel.offset == 0)
	{
		//	无被删除过的空余空间，直接在文件尾部插入数据
		//	将添加的新地址作为记录数据的一部分写入
		tmpSource = malloc(sz_record + sizeof(FileAddr));
		memcpy(tmpSource, &pFileHI->NewInsert, sizeof(FileAddr));
		memcpy((char*)tmpSource + sizeof(FileAddr), source, sz_record);
		auto realPos = MemWrite(tmpSource, sz_record + sizeof(FileAddr));
		MemWrite(&realPos, sizeof(FileAddr), &realPos);
		fh = realPos;
	}
	else if (pFileHI->firstDel == pFileHI->lastDel)
	{
		//	在第一个被删除的数据处，添加新数据
		tmpSource = malloc(sz_record + sizeof(FileAddr));
		memcpy(tmpSource, &pFileHI->NewInsert, sizeof(FileAddr));
		memcpy((char*)tmpSource + sizeof(FileAddr), source, sz_record);
		MemWrite(tmpSource, sz_record + sizeof(FileAddr), &pFileHI->firstDel);
		fh = pFileHI->firstDel;
		pFileHI->firstDel.offset = pFileHI->lastDel.offset = 0;
	}
	else
	{
		auto firstDelPos = pFileHI->firstDel;
		fh = pFileHI->firstDel;
		pFileHI->firstDel = *(FileAddr*)MemRead(&pFileHI->firstDel);

		tmpSource = malloc(sz_record + sizeof(FileAddr));
		memcpy(tmpSource, &firstDelPos, sizeof(FileAddr));
		memcpy((char*)tmpSource + sizeof(FileAddr), source, sz_record);
		MemWrite(tmpSource, sz_record + sizeof(FileAddr), &firstDelPos);
	}
	delete tmpSource;
	pMemBlock->SetModified();	//设置为脏页
	return fh;
}

FileAddr MemFile::DeleteRecord(FileAddr* address, size_t)
{
	auto pMemBlock = GetGlobalClock()->GetMemAddr(this->fileID, 0);
	auto pFileHI = pMemBlock->GetFileHeadInfo();


	FileAddr fh = *(FileAddr*)MemRead(address);
	// 如果删除数据地址的地址标识和本身地址不等，说明数据已经删除过
	if (fh != *address)
	{
		return FileAddr{ 0,0 };
	}
	else if (pFileHI->firstDel.offset == 0 && pFileHI->lastDel.offset == 0)
	{
		pFileHI->firstDel = pFileHI->lastDel = *address;
		FileAddr tmp{ 0,0 };
		MemWrite(&tmp, sizeof(FileAddr), &pFileHI->lastDel);
	}
	else
	{
		//删除记录
		MemWrite(address, sizeof(FileAddr), &pFileHI->lastDel);
		pFileHI->lastDel = *address;
		FileAddr tmp{ 0, 0 };
		MemWrite(&tmp, sizeof(FileAddr), &pFileHI->lastDel);
	}

	pMemBlock->SetModified();
	return *address;
}

bool MemFile::UpdateRecord(FileAddr* address, void* record_data, size_t record_sz)
{
	auto pMemBlock = GetGlobalClock()->GetMemAddr(this->fileID, address->fileBlockID);
	auto pDest = (char*)pMemBlock->PtrtoBlockBegin + address->offset + sizeof(FileAddr);
	memcpy(pDest, record_data, record_sz);
	pMemBlock->isDirty = true;
	return true;
}

void* MemFile::MemRead(FileAddr* memtoRead)
{
	auto pMemBlock = GetGlobalClock()->GetMemAddr(this->fileID, memtoRead->fileBlockID);
	pMemBlock->isLastUsed = true;
	return (char*)pMemBlock->PtrtoBlockBegin + memtoRead->offset;
}

FileAddr MemFile::MemWrite(const void* source, size_t length)
{
	//获取可写入地址
	FileAddr insertPos = GetGlobalClock()->GetMemAddr(this->fileID, 0)->GetFileHeadInfo()->NewInsert;

	//写入
	FileAddr writeRes = MemWrite(source, length, &insertPos);
	if (writeRes.fileBlockID == insertPos.fileBlockID && writeRes.offset == insertPos.offset)\
	{
		//开辟新的空间
		AddExtraBlock();
		insertPos.SetFileAddr(insertPos.fileBlockID + 1, sizeof(BlockHead));
		//重新写入
		writeRes = MemWrite(source, length, &insertPos);
	}

	//更新可写入位置
	GetGlobalClock()->GetMemAddr(this->fileID, 0)->GetFileHeadInfo()->NewInsert = writeRes;
	GetGlobalClock()->GetMemAddr(this->fileID, 0)->SetModified();
	GetGlobalClock()->GetMemAddr(this->fileID, 0)->isLastUsed = true;
	return insertPos;
}

FileAddr MemFile::MemWrite(const void* source, size_t length, FileAddr* dest)
{
	auto pMemBlock = GetGlobalClock()->GetMemAddr(this->fileID, dest->fileBlockID);
	//如果剩余空间不足，写入失败
	if ((FILE_BLOCKSIZE - dest->offset) < length)
	{
		return *dest;
	}
	memcpy((void*)((char*)pMemBlock->PtrtoBlockBegin + dest->offset), source, length);
	pMemBlock->isDirty = true;
	pMemBlock->isLastUsed = true;

	//dest->Offser += length
	FileAddr fh;
	fh.SetFileAddr(dest->fileBlockID, dest->offset + length);
	return fh;
}

void MemFile::MemWipe(void* source, size_t sz_wipe, FileAddr* fhtoWipe)
{
	auto pMemBlock = GetGlobalClock()->GetMemAddr(this->fileID, fhtoWipe->fileBlockID);
	// wipe
	memcpy((char*)pMemBlock->PtrtoBlockBegin + fhtoWipe->offset, source, sz_wipe);
	pMemBlock->isDirty = true;
	pMemBlock->isLastUsed = true;
}

MemFile::MemFile(const char* fileName, unsigned long fileID)
{
	strcpy(this->fileName, fileName);
	this->fileID = fileID;
	this->totalBlock = GetGlobalClock()->GetMemAddr(this->fileID, 0)->GetFileHeadInfo()->totalBlock;
}

//BufferManager

BufferManager::~BufferManager()
{
	//closeallfile();
	for (auto e : memFiles)
		delete e;
}

MemFile* BufferManager::GetMemFile(const char* fileName)
{
	//若文件已经打开
	for (size_t i = 0; i < memFiles.size(); i++)
	{
		if (strcmp(memFiles[i]->fileName, fileName) == 0)
			return memFiles[i];
	}

	//文件存在但是没打开
	int ptrtoFile = open(fileName, _O_BINARY | O_RDWR, 0664);
	if (ptrtoFile != -1)
	{
		MemFile* newFile = new MemFile(fileName, ptrtoFile);
		memFiles.push_back(newFile);
		return newFile;
	}

	// 文件不存在
	return nullptr;
}

void BufferManager::CreateFile(const char* fileName)
{
	// 文件存在 创建失败
	int ptrtoFile = open(fileName, _O_BINARY | O_RDWR, 0664);
	if (ptrtoFile != 01)
	{
		close(ptrtoFile);
		return;
	}

	//创建文件
	int  newFile = open(fileName, _O_BINARY | O_RDWR | O_CREAT, 0664);
	void* ptr = malloc(FILE_BLOCKSIZE);
	memset(ptr, 0, FILE_BLOCKSIZE);
	BlockHead* pPageHead = (BlockHead*)ptr;
	FileHeadInfo* pFileHI = (FileHeadInfo*)((char*)ptr + sizeof(BlockHead));
	pPageHead->blockID = 0;
	pPageHead->isPinned = 1;
	pFileHI->Initialize();
	//写回
	write(newFile, ptr, FILE_BLOCKSIZE);
	close(newFile);
	delete ptr;
	return;
}

void BufferManager::CloseAllFile()
{
	while (!memFiles.empty())
	{
		CloseFile((*memFiles.begin())->fileName);
	}
}

void BufferManager::CloseFile(const char* fileName)
{
	auto pMemBlock = GetMemFile(fileName);
	// 内存中所有保存该文件的页全部写回
	auto pClock = GetGlobalClock();
	for (int i = 1; i <= MEM_BLOCKAMOUNT; i++)
	{
		if (pClock->MemBlocks[i] && pClock->MemBlocks[i]->fileID == pMemBlock->fileID)
		{
			assert(pClock->MemBlocks[i]);
			pClock->MemBlocks[i]->BacktoFile();
			pClock->MemBlocks[i]->isLastUsed = false;
			pClock->MemBlocks[i]->isDirty = false;
			pClock->MemBlocks[i]->fileID = 0;
		}
	}

	for (auto it = memFiles.begin(); it != memFiles.end();)
	{
		if (strcmp((*it)->fileName, fileName) == 0)
		{
			close((*it)->fileID);
			delete (*it);
			memFiles.erase(it);
			break;
		}
		it++;
	}
}

MemFile* BufferManager::operator[](const char* fileName)
{
	return GetMemFile(fileName);
}

