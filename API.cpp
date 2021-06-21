#include "API.h"

RM RecordManager;
CM CatalogManager;
BufferManager bufferManager;

void CreateTable(std::string table_name, std::vector<std::string>& column_names,
    std::vector<std::string>& data_types, int PK_index)
{
    TableInfo info = CatalogManager.InitTableInfo(table_name, column_names, data_types, PK_index);
    std::string full_name = table_name + ".db";
    if (CatalogManager.NewInfoCheck(info)) {
        bufferManager.CreateFile(full_name.c_str(), info);
    }
}

bool OpenTable(std::string file_name)
{
    MemFile* file = bufferManager[file_name.c_str()];
    if (file != nullptr) {
        CatalogManager.OpenTableFile(GetGlobalClock()->GetMemAddr(file->fileID, 0)->GetFileHeadInfo()->reserve);
        return true;
    }
    else
        return false;
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
                //idx = index_name+".idx"; 待改
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
    CatalogManager.DropTable(table_name);
    return true;
}

bool InsertTuple(std::string table_name, std::vector<DataClass>& list)
{
    std::string table_file_name = table_name + ".db";
    if (OpenTable(table_file_name)) {
        MemFile* file = bufferManager[table_file_name.c_str()];
        TableInfo& info = CatalogManager.LookUpTableInfo(table_name);
        for (int i = 0; i < info.n_columns(); i++) {
            if (info[i].is_unique || info[i].is_PK) {
                if (info[i].is_PK) {
                    BPTree tree(table_name);
                    if (tree.Search(list[i]))
                        throw SQLError::KEY_INSERT_ERROR();
                }
                else if (info[i].has_index) {
                    BPTree tree(table_name + info[i].column_name);
                    if (!tree.Search(list[i]))
                        throw SQLError::KEY_INSERT_ERROR();
                }
                else {
                    BPTree tree(table_name);
                    std::vector<FileAddr*> addr_list = tree.AllSearch();
                    for (int j = 0; j < addr_list.size(); j++) {
                        const void* cmp_src = file->ReadRecord(addr_list[j]);
                        if (!RecordManager.InsertCheck(cmp_src, info, list))
                            throw SQLError::KEY_INSERT_ERROR();
                    }
                }
            }
        }
        Tuple t(info, list);
        void* p = RecordManager.GetSource(&t);
        FileAddr addr;
        addr = file->AddRecord(p, info.CalTupleSize());
        for (int i = 0; i < info.n_columns(); i++) {
            if (info[i].is_PK) {
                BPTree tree(table_name);
                tree.Insert(list.at(i), addr);
            }
            else if (info[i].has_index) {
                BPTree tree(table_name + info[i].column_name);
                tree.Insert(list.at(i), addr);
            }
        }
        free(p);  // called malloc in Tuple::GetWriteSource()
        return true;
    }
    else {
        throw SQLError::TABLE_ERROR();
        return false;
    }
}

std::vector<Tuple> SelectTuples(std::vector<SelectCondition>& conditions, std::string table_name)
{
    std::string full_name = table_name + ".db";
    if (OpenTable(full_name)) {
        MemFile* file = bufferManager[full_name.c_str()];
        bool found_index = false;
        int idx_cond = 0;
        int idx_i;
        BPTree tree(table_name);
        TableInfo& table_info = CatalogManager.LookUpTableInfo(table_name);
        std::vector<Tuple> result_set;
        for (int i = 0; i < conditions.size(); i++) {
            for (int j = 0; j < table_info.n_columns(); j++) {
                if (table_info[j].column_name == conditions[i].on_attr) {
                    if (table_info[j].has_index) {
                        tree = BPTree(table_name + table_info[j].column_name);
                        found_index = true;
                        idx_i = j;
                        break;
                    }
                    else if (table_info[j].is_PK) {
                        found_index = true;
                        idx_i = j;
                        break;
                    }
                }
            }
            if (found_index) {
                idx_cond = i;
                break;
            }
        }
        std::vector<FileAddr*> addr_list;
        const void* cmp_src;
        if (found_index) {
            if (conditions[idx_cond].op == "=") {
                addr_list = tree.Search(conditions[idx_cond].cmp_value);
            }
            else if (conditions[idx_cond].op == "<" && conditions[idx_cond].op == "<=") {
                addr_list = tree.LeftSearch(conditions[idx_cond].cmp_value);
            }
            else if (conditions[idx_cond].op == ">" && conditions[idx_cond].op == ">=") {
                addr_list = tree.LeftSearch(conditions[idx_cond].cmp_value);
            }
        }
        else {
            addr_list = tree.AllSearch();
        }
        for (int i = 0; i < addr_list.size(); i++) {
            cmp_src = file->ReadRecord(addr_list[i]);
            RecordManager.SelectTuple(conditions, cmp_src, table_info);
        }
        RecordManager.GetSelectRS(result_set);
        return result_set;
    }
    else {
        throw SQLError::TABLE_ERROR();
    }
}

//待修改
bool DeleteTuples(std::vector<SelectCondition>& conditions, std::string table_name)
{
    int deleteNumber = 0;
    std::string full_name = table_name + ".db";
    if (OpenTable(full_name)) {
        MemFile* file = bufferManager[full_name.c_str()];
        bool found_index = false;
        int idx_cond = 0;
        int idx_i;
        BPTree tree(table_name);
        TableInfo& table_info = CatalogManager.LookUpTableInfo(table_name);
        std::vector<Tuple> result_set;
        for (int i = 0; i < conditions.size(); i++) {
            for (int j = 0; j < table_info.n_columns(); j++) {
                if (table_info[j].column_name == conditions[i].on_attr) {
                    if (table_info[j].has_index) {
                        tree = BPTree(table_name + table_info[j].column_name);
                        found_index = true;
                        idx_i = j;
                        break;
                        
                    }
                    else if (table_info[j].is_PK) {
                        throw SQLError::TABLE_ERROR();
                        break;
                    }
                    
                }
            }
            if (found_index) {
                idx_cond = i;
                break;
            }
        }
        std::vector<FileAddr*> addr_list;
        const void* cmp_src;
        if (found_index) {
            if (conditions[idx_cond].op == "=") {
                addr_list = tree.Search(conditions[idx_cond].cmp_value);
            }
            else if (conditions[idx_cond].op == "<" && conditions[idx_cond].op == "<=") {
                addr_list = tree.LeftSearch(conditions[idx_cond].cmp_value);
            }
            else if (conditions[idx_cond].op == ">" && conditions[idx_cond].op == ">=") {
                addr_list = tree.LeftSearch(conditions[idx_cond].cmp_value);
            }
        }
        else {
            addr_list = tree.AllSearch();
        }
        for (int i = 0; i < addr_list.size(); i++) {
            file->DeleteRecord(addr_list[i], addr_list[i]->offset);//存疑
            //删除索引，缺如
            //cmp_src = file->ReadRecord(addr_list[i]);
            //RecordManager.SelectTuple(conditions, cmp_src, table_info)
            //DropIndex()
            deleteNumber++;
        }
        //RecordManager.GetSelectRS(result_set);
        return true;
    }
    else {
        throw SQLError::TABLE_ERROR();
    }
}

bool CreateIndex(std::string table_name, std::string index_name,std::string column_name)
{
    TableInfo& table_info = CatalogManager.LookUpTableInfo(table_name);
    int indexNum = -1;
     
    for (int i = 0; i < table_info.n_columns(); i++)
    {
        if (table_info[i].column_name == column_name && table_info[i].is_unique)//不需要判断是否为主键，因为主键建立时必定建立索引
        {
            indexNum = i;
        }
    }
    if (indexNum == -1)
    {
        std::logic_error e("error: attribute name is not found!");
        throw std::exception(e);
    }
    BPTree tree(table_name + table_info[indexNum].column_name);
    CatalogManager;
    std::string idx = index_name + ".idx";
    BPTree tree(idx);
    /**/
    return true;

}

//存疑
bool DropIndex(std::string index_name)
{
    auto pClock = GetGlobalClock();
    std::string idx = index_name + ".idx";
    auto& buffer = bufferManager;
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
     return true;
}

bool ShowTable(std::string table_name)
{
    using std::cout;
    using std::endl;
    cout << "---" << table_name << "---" << endl;
    TableInfo& table_info = CatalogManager.LookUpTableInfo(table_name);
    for (int i = 0; i < table_info.n_columns(); i++)
    {
        cout << table_info[i].column_name << " ";
        if (table_info[i].type == DataType::INT)
            cout << "int";
        else if (table_info[i].type == DataType::FLOAT)
            cout << "float";
        else
            cout << "char(" << table_info[i].bytes << ")";
        if (table_info[i].is_unique)
            cout << "unique";
        if (table_info[i].is_PK)
            cout << "primary key";
        if (table_info[i].has_index)
            cout << "has index";
        cout << endl;
    }
    return true;
}


