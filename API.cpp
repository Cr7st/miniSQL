#include "API.h"

RM RecordManager;
CM CatalogManager;

void CreateTable(std::string table_name, std::vector<std::string> column_names,
                 std::vector<std::string> data_types, int PK_index)
{
    TableInfo info = CatalogManager.InitTableInfo(table_name, column_names, data_types, PK_index);
    std::string full_name = table_name + ".db";
    if (CatalogManager.NewInfoCheck(info))
    {
        try{
            GetGlobalFileBuffer().CreateFile(full_name.c_str());
        }
        catch(SQLError::TABLE_ERROR e){
            e.PrintError();
            return;
        }
        FileAddr addr;
        addr.SetFileAddr(0, sizeof(BlockHead) + sizeof(FileHeadInfo) - FILEHI_RESERVE_SPACE);
        void *p = GetGlobalFileBuffer()[full_name.c_str()]->ReadWriteRecord(&addr);
        CatalogManager.CreateTable(info, p);
        CreateIndex(table_name, table_name, column_names[PK_index]);
    }
}

bool OpenTable(std::string file_name)
{
    MemFile *file = GetGlobalFileBuffer()[file_name.c_str()];
    if (file != nullptr)
    {
        //注意看这一行！仅在OpenTable中有调用OpenTableFile，CM中可查表信息，所以确保调用OpenTable
        CatalogManager.OpenTableFile(GetGlobalClock()->GetMemAddr(file->fileID, 0)->GetFileHeadInfo()->reserve);
        return true;
    }
    else
        return false;
}

bool DropTable(std::string table_name)
{
    std::string dbf = table_name + ".db";
    if (OpenTable(dbf))
    { //郑博文修改与 06-23 14:23 CM.LookUpTable需要调用过OpenTable之后才能正常进行搜索
        TableInfo &info = CatalogManager.LookUpTableInfo(table_name);
        auto &buffer = GetGlobalFileBuffer();
        auto pClock = GetGlobalClock();
        //去除主键和unique键的index
        //建议以下删除索引的操作可以调用DropIndex，代码重用有利于测试
        for (int i = 0; i < info.n_columns(); i++)
        {
            std::string idx;
            if (info[i].has_index)
            {
                idx = info.GetIndexName(i);
                DropIndex(idx);
            }
        }
        CatalogManager.DropTable(table_name);
        //郑博文添加于 06-22 14:32 CM::DropTable仅删除内存中表信息，需要实际继续实际删除该文件
        MemFile *fileDb = GetGlobalFileBuffer()[dbf.c_str()];
        if (fileDb != nullptr)
        {
            for (int i = 1; i <= MEM_BLOCKAMOUNT; i++)
            {
                if (pClock->MemBlocks[i] && pClock->MemBlocks[i]->fileID == fileDb->fileID)
                {
                    pClock->MemBlocks[i]->fileID = 0;
                    pClock->MemBlocks[i]->isDirty = 0;
                }
            }
        }
        close(fileDb->fileID);
        remove(dbf.c_str());
        return true;
    }
    else
        return false;
}

bool InsertTuple(std::string table_name, std::vector<DataClass> &list)
{
    std::string table_file_name = table_name + ".db";
    if (OpenTable(table_file_name))
    {
        MemFile *file = GetGlobalFileBuffer()[table_file_name.c_str()];
        TableInfo &info = CatalogManager.LookUpTableInfo(table_name);
        for (int i = 0; i < info.n_columns(); i++)
        {
            if (info[i].is_unique || info[i].is_PK)
            {
                // if the column is unique all a primary key, should check duplication
                if (info[i].is_PK)
                {
                    BPTree tree(info.GetIndexName(i));
                    if (*(tree.Search(list[i])) != FileAddr{0, 0})
                        throw SQLError::KEY_INSERT_ERROR();
                }
                // if the unique attribute has an index, use it to search
                else if (info[i].has_index)
                {
                    BPTree tree(info.GetIndexName(i));
                    if (*(tree.Search(list[i])) != FileAddr{0, 0})
                        throw SQLError::KEY_INSERT_ERROR();
                }
                // if it doesn't have an index, linear scan all the records
                else
                {
                    BPTree tree(table_name);
                    std::vector<FileAddr *> addr_list = tree.AllSearch();
                    for (int j = 0; j < addr_list.size(); j++)
                    {
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
        // update the BPTrees of all the columns which has index
        for (int i = 0; i < info.n_columns(); i++)
        {
            if (info[i].has_index)
            {
                BPTree tree(info.GetIndexName(i));
                tree.Insert(list.at(i), addr);
            }
        }
        free(p); // called malloc in Tuple::GetWriteSource()
        return true;
    }
    else
    {
        try{
            throw SQLError::TABLE_ERROR(std::string("There is no such table!  "));
        }
        catch(SQLError::TABLE_ERROR e){
            e.PrintError();
        }
        return false;
    }
}

std::vector<Tuple> SelectTuples(std::vector<SelectCondition> &conditions, std::string table_name)
{
    std::string full_name = table_name + ".db";
    if (OpenTable(full_name))
    {
        MemFile *file = GetGlobalFileBuffer()[full_name.c_str()];
        bool found_index = false;
        int idx_cond = 0;
        int idx_i = 0;
        BPTree tree(table_name);
        TableInfo &table_info = CatalogManager.LookUpTableInfo(table_name);
        std::vector<Tuple> result_set;
        for (int i = 0; i < conditions.size(); i++)
        {
            for (int j = 0; j < table_info.n_columns(); j++)
            {
                if (table_info[j].column_name == conditions[i].attr)
                {
                    if (table_info[j].has_index)
                    {
                        tree = BPTree(table_info.GetIndexName(j));
                        found_index = true;
                        idx_i = j;
                        break;
                    }
                    else if (table_info[j].is_PK)
                    {
                        found_index = true;
                        idx_i = j;
                        break;
                    }
                }
            }
            if (found_index)
            {
                idx_cond = i;
                break;
            }
        }
        std::vector<FileAddr *> addr_list;
        const void *cmp_src;
        if (conditions.size() == 0)
        {
            addr_list = tree.AllSearch();
        }
        else
        {
            if (found_index)
            {
                if (conditions[idx_cond].op == "=")
                {
                    addr_list.push_back(tree.Search(conditions[idx_cond].value));
                }
                else if (conditions[idx_cond].op == "<" && conditions[idx_cond].op == "<=")
                {
                    addr_list = tree.LeftSearch(conditions[idx_cond].value);
                }
                else if (conditions[idx_cond].op == ">" && conditions[idx_cond].op == ">=")
                {
                    addr_list = tree.RightSearch(conditions[idx_cond].value);
                }
            }
            else
            {
                addr_list = tree.AllSearch();
            }
        }
        for (int i = 0; i < addr_list.size(); i++)
        {
            cmp_src = file->ReadRecord(addr_list[i]);
            RecordManager.SelectTuple(conditions, cmp_src, table_info);
        }
        RecordManager.GetSelectRS(result_set);
        return result_set;
    }
    else
    {
        std::cout<<"noo";
        try{
            throw SQLError::TABLE_ERROR();
        }
        catch(SQLError::TABLE_ERROR e){
            e.PrintError();
        }
    }
}

//待修改
bool DeleteTuples(std::vector<SelectCondition> &conditions, std::string table_name)
{
    int deleteNumber = 0;
    std::string full_name = table_name + ".db";
    if (OpenTable(full_name))
    {
        MemFile *file = GetGlobalFileBuffer()[full_name.c_str()];
        bool found_index = false;
        int idx_cond = 0;
        int idx_i = 0;
        BPTree tree(table_name);
        TableInfo &table_info = CatalogManager.LookUpTableInfo(table_name);
        std::vector<Tuple> result_set;
        for (int i = 0; i < conditions.size(); i++)
        {
            for (int j = 0; j < table_info.n_columns(); j++)
            {
                if (table_info[j].column_name == conditions[i].attr)
                {
                    if (table_info[j].has_index)
                    {
                        tree = BPTree(table_info.GetIndexName(j));
                        found_index = true;
                        idx_i = j;
                        break;
                    }
                }
            }
            if (found_index)
            {
                idx_cond = i;
                break;
            }
        }
        std::vector<FileAddr *> addr_list;
        FileAddr tmp_addr;
        const void *cmp_src;
        //郑博文修改于 06-22 14:41 需要提前对conditions数目做判断，为0直接进行AllSearch，否则else中用 [] 访问一个空的容器会报错
        if (conditions.size() == 0)
        {
            addr_list = tree.AllSearch();
        }
        else
        {
            if (found_index)
            {
                if (conditions[idx_cond].op == "=")
                {
                    addr_list.push_back(tree.Search(conditions[idx_cond].value));
                }
                else if (conditions[idx_cond].op == "<" && conditions[idx_cond].op == "<=")
                {
                    addr_list = tree.LeftSearch(conditions[idx_cond].value);
                }
                else if (conditions[idx_cond].op == ">" && conditions[idx_cond].op == ">=")
                {
                    addr_list = tree.LeftSearch(conditions[idx_cond].value);
                }
            }
            else
            {
                addr_list = tree.AllSearch();
            }
        }
        for (int i = 0; i < addr_list.size(); i++)
        {
            if (RecordManager.DeleteCheck(conditions, file->ReadRecord(addr_list[i]), table_info)){
                Tuple t(table_info);
                tmp_addr = *addr_list[i];
                t.ReadFrom((char*)file->ReadRecord(addr_list[i]));
                for (int j = 0; j < table_info.n_columns(); j++){
                    if (table_info[j].has_index){
                        tree = BPTree(table_info.GetIndexName(j));
                        tree.Delete(t.data_list[j]);
                        if (i < addr_list.size() - 1){
                            i--;//addr_list中存的是指针，tree中删除后，addr_list中也会少
                                                        //若删除的是最后一个，则没有影响
                            addr_list.pop_back();
                        }
                    }
                }
                file->DeleteRecord(&tmp_addr, table_info.CalTupleSize());
                deleteNumber++;
            }
        }
        std::cout<<"成功删除"<< deleteNumber <<"条记录！";
        //RecordManager.GetSelectRS(result_set);
        return true;
    }
    else
    {
        try {
            throw SQLError::TABLE_ERROR();
        }
        catch(SQLError::TABLE_ERROR e){
            e.PrintError();
        }
    }
}

bool CreateIndex(std::string table_name, std::string index_name, std::string column_name)
{
    std::string tb_full_name = table_name + std::string(".db");
    if (OpenTable(tb_full_name))
    {
        TableInfo &table_info = CatalogManager.LookUpTableInfo(table_name);
        int indexNum = -1;

        for (int i = 0; i < table_info.n_columns(); i++)
        {
            if (table_info[i].column_name == column_name && table_info[i].is_unique) //不需要判断是否为主键，因为主键建立时必定建立索引
            {
                indexNum = i;
            }
        }
        if (indexNum == -1)
        {
            std::logic_error e("error: attribute name is not found!");
            throw std::exception(e);
        }
        //BPTree tree(index_name);
        CatalogManager.SetIdxOn(table_info, indexNum, index_name);
        std::string idx = index_name + ".idx";

        //************************************************************************
        int KeyTypeIndex = 0;
        for (int j = 0; j < table_info.columns.size(); j++)
        {
            if (table_info.columns[j].is_PK)
            {
                KeyTypeIndex = j;
                break;
            }
        }

        // 字段信息
        char RecordTypeInfo[RecordColumnCount];          // 记录字段类型信息
        char *ptype = RecordTypeInfo;
        char RecordColumnName[RecordColumnCount / 4 * ColumnNameLength];
        char *pname = RecordColumnName;

        const auto &column_info_ref = table_info.columns;
        for (int i = 0; i < column_info_ref.size(); i++)
        {
            // 类型信息
            switch (column_info_ref[i].type)
            {
                case DataType::INT:
                    *ptype++ = 'I';
                    break;

                case  DataType::FLOAT:
                    *ptype++ = 'D';
                    break;

                case DataType::CHAR:
                    *ptype++ = 'C';
                    *ptype++ = column_info_ref[i].bytes / 100 + '0';
                    *ptype++ = (column_info_ref[i].bytes % 100) / 10 + '0';
                    *ptype++ = column_info_ref[i].bytes % 10 + '0';
                default:
                    break;
            }
            // 名称信息
            strcpy(pname, column_info_ref[i].column_name.c_str());
            pname += ColumnNameLength;
        }
        *ptype = '\0';
        //************************************************************************

        BPTree tree(index_name, table_name, KeyTypeIndex, RecordTypeInfo, RecordColumnName);
        FileAddr addr;
        addr.SetFileAddr(0, sizeof(BlockHead) + sizeof(FileHeadInfo) - FILEHI_RESERVE_SPACE);
        void *p = GetGlobalFileBuffer()[tb_full_name.c_str()]->ReadWriteRecord(&addr);
        CatalogManager.WriteTo(table_info, p);
        return true;
    }
    else
        return false;
}

//存疑
bool DropIndex(std::string index_name)
{
    std::string idx = index_name + ".idx";

    BPTree tree(index_name);
    std::string table_name = tree.GetPtrIndexHeadNode()->TableName;
    //消除表中信息
    if (OpenTable(table_name + ".db"))
    {
        TableInfo &table_info = CatalogManager.LookUpTableInfo(table_name);
        CatalogManager.DropIndex(table_info, index_name);
        FileAddr addr;
        addr.SetFileAddr(0, sizeof(BlockHead) + sizeof(FileHeadInfo) - FILEHI_RESERVE_SPACE);
        std::string tb_full_name = table_name + ".db";
        void *p = GetGlobalFileBuffer()[tb_full_name.c_str()]->ReadWriteRecord(&addr);
        CatalogManager.WriteTo(table_info, p);
    }
    //删除文件
    auto pClock = GetGlobalClock();
    auto &buffer = GetGlobalFileBuffer();
    if (_access(idx.c_str(), 0 == -1))
    {
        return false;
    }
    else
    {
        //如果文件已经打开，需要丢弃在内存中的文件页
        MemFile *fileidx = GetGlobalFileBuffer()[idx.c_str()];
        if (fileidx != nullptr)
        {
            for (int i = 1; i <= MEM_BLOCKAMOUNT; i++)
            {
                if (pClock->MemBlocks[i] && pClock->MemBlocks[i]->fileID == fileidx->fileID)
                {
                    pClock->MemBlocks[i]->fileID = 0; //丢弃该页
                    pClock->MemBlocks[i]->isDirty = false;
                }
            }
        }
        //删除文件
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
    if(OpenTable(table_name))
    {
        TableInfo &table_info = CatalogManager.LookUpTableInfo(table_name);
        for (int i = 0; i < table_info.n_columns(); i++)
        {
            cout << table_info[i].column_name << " ";
            if (table_info[i].type == DataType::INT)
                cout << "int ";
            else if (table_info[i].type == DataType::FLOAT)
                cout << "float ";
            else
                cout << "char(" << table_info[i].bytes << ") ";
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
    return false;
}
