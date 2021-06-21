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
                bpt tree(table_name);
                if (!tree.Search(list[i]))
                    throw SQLError::KEY_INSERT_ERROR();
            }
            else if (info[i].has_index){
                bpt tree(table_name + info[i].column_name);
                if (!tree.Search(list[i]))
                    throw SQLError::KEY_INSERT_ERROR();
            }            
            else{
                bpt tree(table_name);
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