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
        bufferManager.CreateFile(full_name.c_str());
        FileAddr addr;
        addr.SetFileAddr(0, sizeof(BlockHead) + sizeof(FileHeadInfo) - FILEHI_RESERVE_SPACE);
        void *p = bufferManager[full_name.c_str()]->ReadWriteRecord(&addr);
        CatalogManager.CreateTable(info, p);
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
    if (OpenTable(table_file_name)){
        MemFile *file = bufferManager[table_file_name.c_str()];
        TableInfo &info = CatalogManager.LookUpTableInfo(table_name);
        for (int i = 0; i < info.n_columns(); i++){
            if (info[i].is_unique || info[i].is_PK){
                if (info[i].is_PK){
                    BPTree tree(table_name);
                    if (tree.Search(list[i]))
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
        FileAddr addr;
        addr = file->AddRecord(p, info.CalTupleSize());
        for (int i =0; i < info.n_columns(); i++){
            if (info[i].is_PK){
                BPTree tree(table_name);
                tree.Insert(list.at(i), addr);
            }    
            else if (info[i].has_index){
                BPTree tree(table_name + info[i].column_name);
                tree.Insert(list.at(i), addr);
            }
        }
        free(p);  // called malloc in Tuple::GetWriteSource()
        return true;
    }
    else{
        throw SQLError::TABLE_ERROR();
        return false;
    }
}

std::vector<Tuple> SelectTuples(std::vector<SelectCondition> &conditions, std::string table_name)
{
    std::string full_name = table_name + ".db";
    if (OpenTable(full_name)){
        MemFile *file = bufferManager[full_name.c_str()];
        bool found_index = false;
        int idx_cond = 0;
        int idx_i = 0;
        BPTree tree(table_name);
        TableInfo &table_info = CatalogManager.LookUpTableInfo(table_name);
        std::vector<Tuple> result_set;
        for (int i = 0; i < conditions.size(); i++){
            for (int j = 0; j < table_info.n_columns(); j++){
                if (table_info[j].column_name == conditions[i].attr){
                    if (table_info[j].has_index){
                        tree = BPTree(table_name + table_info[j].column_name);
                        found_index = true;
                        idx_i = j;
                        break;
                    }
                    else if (table_info[j].is_PK){
                        found_index = true;
                        idx_i = j;
                        break;
                    }
                }
            }
            if (found_index){
                idx_cond = i;
                break;
            }
        }
        std::vector<FileAddr*> addr_list;
        const void *cmp_src;
        if (conditions.size() == 0){
            addr_list = tree.AllSearch();
        }
        else{
            if (found_index){
                if (conditions[idx_cond].op == "="){
                    addr_list = tree.Search(conditions[idx_cond].value);
                }
                else if (conditions[idx_cond].op == "<" && conditions[idx_cond].op == "<="){
                    addr_list = tree.LeftSearch(conditions[idx_cond].value);
                }
                else if (conditions[idx_cond].op == ">" && conditions[idx_cond].op == ">="){
                    addr_list = tree.RightSearch(conditions[idx_cond].value);
                }
            }
            else{
                addr_list = tree.AllSearch();
            }
        }
        for (int i = 0; i < addr_list.size(); i++){
            cmp_src = file->ReadRecord(addr_list[i]);
            RecordManager.SelectTuple(conditions, cmp_src, table_info);
        }
        RecordManager.GetSelectRS(result_set);
        return result_set;
    }
    else{
        throw SQLError::TABLE_ERROR();
    }
}