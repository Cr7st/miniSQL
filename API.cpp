#include "API.h"

RM RecordManager;
CM CatalogManager;
BufferManager bufferManager;

void CreateTable(std::string table_name, std::vector<std::string> &column_names, 
                             std::vector<std::string> &data_types, int PK_index)
{
    TableInfo info = CatalogManager.InitTableInfo(table_name, column_names, data_types, PK_index);
    std::string full_name = table_name + ".db";
    if (CatalogManager.NewInfoCheck(info)){
        bufferManager.CreateFile(full_name.c_str(), info) ;
    }
}

bool OpenTable(std::string file_name)
{
    MemFile *file = bufferManager[file_name.c_str()];
    if (file != nullptr){
        CatalogManager.OpenTableFile(GetGlobalClock()->GetMemAddr(file->fileID, 0)->GetFileHeadInfo()->reserve);
        return true;
    }
    else
        return false;
}

bool InsertTuple(std::string table_name, std::vector<DataClass> &list)
{
    std::string table_file_name = table_name + ".db";
    MemFile *file = bufferManager[table_file_name.c_str()];
    TableInfo &info = CatalogManager.LookUpTableInfo(table_name);
    for (int i = 0; i < info.n_columns(); i++){
        if (info[i].is_unique || info[i].is_PK){
            if (info[i].is_PK){
                BPTree tree(table_name);
                if (!tree.Search(list[i]))
                    throw SQLError::KEY_INSERT_ERROR();
            }
            else if (info[i].has_index){
                BPTree tree(table_name + info[i].column_name);
                if (!tree.Search(list[i]))
                    throw SQLError::KEY_INSERT_ERROR();
            }            
            else{
                BPTree tree(table_name);
                std::vector<FileAddr*> addr_list = tree.AllSearch();
                for (int j = 0; j < addr_list.size(); j++){
                    const void *cmp_src = file->ReadRecord(addr_list[j]);
                    if (!RecordManager.InsertCheck(cmp_src, info, list))
                        throw SQLError::KEY_INSERT_ERROR();
                }
            }
        }
    }
    Tuple t(info, list);
    void *p = RecordManager.GetSource(&t);
    file->AddRecord(p, info.CalTupleSize());
    free(p);  // called malloc in Tuple::GetWriteSource()
    return true;
}

bool DropTable(std::string table_name)
{
    std::string dbf = table_name + ".db";
    TableInfo& info = CatalogManager.LookUpTableInfo(table_name);
    auto& buffer = bufferManager;
    auto pClock = GetGlobalClock();
    //去除主键和unique键的index
    for (int i = 0; i < info.n_columns(); i++)
    {
        std::string idx;
        if (info[i].is_unique || info[i].is_PK)
        {
            //primary key
            if (info[i].is_PK)
            {
                idx = table_name + ".idx";
            }
            //unique
            else
                idx = table_name + info[i].column_name + ".idx";
        }
        if (_access(idx.c_str(), 0 == -1))
        {
            return false;
        }
        else
        {
            MemFile* fileidx = buffer.GetMemFile(idx.c_str());
            if (fileidx)
            {
                for (int i = 1; i <= MEM_BLOCKAMOUNT; i++)
                {
                    if (pClock->MemBlocks[i] && pClock->MemBlocks[i]->fileID == fileidx->fileID)
                    {
                        pClock->MemBlocks[i]->fileID = 0;
                        pClock->MemBlocks[i]->isDirty = 0;
                    }
                }
            }
            close(fileidx->fileID);
        }
        remove(idx.c_str());
    }
   /* if ((_access(dbf.c_str(), 0) == -1))
    {
        return false;
    }
    
    else
    {
        MemFile* filedbf = bufferManager[dbf.c_str()];
        if (filedbf)
        {
            for (int i = 1; i <= MEM_BLOCKAMOUNT; i++)
            {
                if (pClock->MemBlocks[i] && pClock->MemBlocks[i]->fileID == filedbf->fileID)
                {
                    pClock->MemBlocks[i]->fileID = 0;
                    pClock->MemBlocks[i]->isDirty = false;
                }
            }
        }
        close(filedbf->fileID);
        remove(dbf.c_str());
    }*/
    CatalogManager.DropTable(table_name);
    return true;
}


//暂时搁置一边
std::vector<std::string> ShowAllTable()
{
    std::vector<std::string>dbs;
    _finddata_t FileInfo;
    std::string path = "./";
    path += "*.*";
    decltype(_findfirst(path.c_str(), &FileInfo)) k;
    decltype(_findfirst(path.c_str(), &FileInfo)) HANDLE;
    k = HANDLE = _findfirst(path.c_str(), &FileInfo);
    while (-1 != k)
    {
        if (!(FileInfo.attrib & _A_SUBDIR) && strcmp(FileInfo.name, ".") != 0 && strcmp(FileInfo.name, "..") != 0)
        {
            dbs.push_back(FileInfo.name);
        }
        k = _findnext(HANDLE, &FileInfo);
    }
    _findclose(HANDLE);
    return dbs;
}

std::vector<std::string> SelectRecord(std::string table_name, std::vector<std::string> attribute , std::vector<SelectCondition> condition)
{
    std::string table_file_name = table_name + ".db";
    MemFile* file = bufferManager[table_file_name.c_str()];
    TableInfo& info = CatalogManager.LookUpTableInfo(table_name);
    for (int i = 0; i < condition.size(); i++)
    {
        int flag = 0;
        for (int j = 0; j < info.n_columns(); j++)
        {
            if (info[j].column_name == condition[i].attr)
            {
                flag = 1;
                if(info[j].type ==  condition[i].value )
            }
        }
    }
    Tuple tuple(info);
    if (info.CalTupleSize() == 0)
    {
        RecordManager.SelectTuple(condition, file, info);
    }
    else
    {
        RecordManager.GetSelectRS(tuple);
    }
    if (condition.empty())
    {
        RecordManager.GetSelectRS(tuple);
        
        
    }
    return 
    //std::vector<std::pair<DataClass, FileAddr>> res;
    //std::vector<std::pair<DataClass, FileAddr>> fds;
    //TableInfo& info = CatalogManager.LookUpTableInfo(table_name);
    //
    //if (condition.empty())//查找所有记录
    //{   
    //    for (int i = 0; i < info.n_columns(); i++)
    //    {
    //        info[i]
    //    }
    //    //index file name
    //    std::string fileidx = table_name + ".idx";

    //    //read index info
    //    BPTree tree(fileidx);
    //    auto phead = tree.GetPtrIndexHeadNode();
    //    //first node
    //    auto node_fd = phead->MostLeftNode;
    //    
    //    
    //    while (node_fd.offset != 0)
    //    {
    //        //get record from the node
    //        auto pNode = tree.FileAddrToMemPtr(node_fd);
    //        for (int i = 0; i < pNode->count_valid_key; i++)
    //        {
    //            res.push_back({ pNode->key[i] , pNode->children[i]
    //        });
    //        }
    //        // next node
    //        node_fd = tree.FileAddrToMemPtr(node_fd)->next;
    //    }
    //}
    //else
    //{
    //    for (int i = 0; i < condition.size(); i++)
    //    {
    //        typedef struct {
    //            std::string attr;
    //            std::string op;   //1:<, 2:=, 3:>, 4:!=, 5:<=, 6:==, 7:>=
    //            DataClass value;
    //            friend class RM;
    //        }SelectCondition;
    //        //find records which satisfy single query
    //        table_name->
    //        fds = Search(table_name, condition[i]);
    //        if (res.empty())
    //        {
    //            res = fds;
    //        }
    //        else
    //        {
    //            std::vector<std::pair<DataClass, FileAddr>> v;
    //                sort(fds.begin(), fds.end());
    //                sort(res.begin(), res.end());
    //                set_intersection(fds.begin(), fds.end(), res.begin(), res.end(), std::back_inserter(v));
    //                res = v;
    //        }
    //    }
    //}
    //TableInfo 
    //SelectPrintInfo spi;
    //spi.table_name = table_name;
    //spi.key_fd = res;
    //return spi;
}

bool UpdateTable(TB_Update_Info tb_update_info,std::string table_name)
{
    std::string fileidx = table_name + ".idx";
    std::string filedbf = table_name + ".db";
    BTree tree(fileidx);
    TableIn
}

std::vector<std::pair<DataClass, FileAddr>>Search(SelectCondition condition, std::string table_name)
{
    //query result
    std::vector<std::pair<DataType, FileAddr>>res;
    //index file name
    std::string fileidx = table_name + ".idx";

    //index info
    
}