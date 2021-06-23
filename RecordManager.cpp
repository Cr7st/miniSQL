#include "RecordManager.h"

Tuple::Tuple(){}

Tuple::Tuple(TableInfo &info)
{
    tuple_size = info.CalTupleSize();
    DataClass ptr;
    std::string str("");
    for (int i = 0; i < info.n_columns(); i++)
    {
        switch (info.columns[i].type){
            case DataType::INT:
                ptr.type = DataType::INT;
                ptr.bytes = 4;
                data_list.push_back(ptr);
                break;
            case DataType::FLOAT:
                ptr.type = DataType::FLOAT;
                ptr.bytes = 8;
                data_list.push_back(ptr);
                break;
            case DataType::CHAR:
                ptr.type = DataType::CHAR;
                ptr.bytes = info.columns[i].bytes;
                data_list.push_back(ptr);
                break;
            default: break;
        }
    }
}

Tuple::Tuple(TableInfo &info, std::vector<DataClass> &list)
{
    tuple_size = info.CalTupleSize();
    std::string str("");
    for (int i = 0; i < info.n_columns(); i++)
    {
        DataClass ptr;
        switch (info.columns[i].type){
            case DataType::INT:
                ptr.type = DataType::INT;
                ptr.bytes = 4;
                ptr.data.i = list[i].data.i;
                data_list.push_back(ptr);
                break;
            case DataType::FLOAT:
                ptr.type = DataType::FLOAT;
                ptr.bytes = 8;
                ptr.data.f = list[i].data.f;
                data_list.push_back(ptr);
                break;
            case DataType::CHAR:
                ptr.type = DataType::CHAR;
                ptr.bytes = info.columns[i].bytes;
                strcpy(ptr.data.str, list[i].data.str);
                data_list.push_back(ptr);
                break;
            default: break;
        }
    }
}

Tuple::~Tuple(){}

void* Tuple::GetWriteSource()
{
    void *source;
    int offset = 0;
    int temp_size;
    source = malloc(tuple_size);
    for (int i = 0; i < data_list.size(); i++){
        switch (data_list[i].type){
            case DataType::INT:
                temp_size = sizeof(int);
                memcpy((char*)source + offset, &(data_list[i].data.i), temp_size);
                break;
            case DataType::FLOAT:
                temp_size = sizeof(double);
                memcpy((char*)source + offset, &(data_list[i].data.f), temp_size);
                break;
            case DataType::CHAR:
                temp_size = data_list[i].bytes;
                memcpy((char*)source + offset, (data_list[i].data.str), temp_size);
                break;
            default: break;
        }
        offset += temp_size;
    }
    return source;
}

void Tuple::ReadFrom(const char *source)
{
    int offset = 0;
    int temp_size;
    for (int i = 0; i < data_list.size(); i++){
        switch (data_list[i].type){
            case DataType::INT:
                temp_size = sizeof(int);
                memcpy(&(data_list[i].data.i), (char*)source, temp_size);
                break;
            case DataType::FLOAT:
                temp_size = sizeof(double);
                memcpy(&(data_list[i].data.f), (char*)source + offset, temp_size);
                break;
            case DataType::CHAR:
                temp_size = data_list[i].bytes;
                memcpy((data_list[i].data.str), (char*)source + offset, temp_size);
                break;
            default: break;
        }
        offset += temp_size;
    }
}


void RM::SelectTuple(std::vector<SelectCondition> &conditions, const void* source, TableInfo &info)
{
    Tuple tuple(info);
    tuple.ReadFrom((char*)source);
    for (int i = 0; i < conditions.size(); i++)
    {
        if (!Satisfy(conditions[i], tuple, info))
            return;
    }
    select_rs.push_back(tuple);
}

void RM::GetSelectRS(std::vector<Tuple> &result)
{
    for (int i = 0; i < select_rs.size(); i++){
        result.push_back(select_rs[i]);
    }
    select_rs.clear();
}

bool RM::Satisfy(SelectCondition &condition, Tuple &tuple, const TableInfo &info)
{
    for (int i = 0; i < info.n_columns(); i++){
        if (condition.attr == info.columns[i].column_name){
            if (condition.op == "=="){
                return tuple.data_list[i] == condition.value;
            }
            else if (condition.op == "<"){
                return tuple.data_list[i] < condition.value;
            }
            else if (condition.op == "<="){
                return tuple.data_list[i] <= condition.value;
            }
            else if (condition.op == ">"){
                return tuple.data_list[i] > condition.value;
            }
            else if (condition.op == ">="){
                return tuple.data_list[i] >= condition.value;
            }
        }
    }
    std::string e("There is an error in the select condition!");
    throw SQLError::TABLE_ERROR(e);
    return false;
}

bool RM::InsertCheck(const void *source, TableInfo &table, std::vector<DataClass> &list)
{
    Tuple ex(table);
    ex.ReadFrom((char*)source);
    for (int i = 0; i < table.n_columns(); i++){
        if (ex.data_list[i].type != list[i].type){
            throw SQLError::KEY_INSERT_ERROR();
            return false;
        }
        else if (table.columns[i].is_unique || table.columns[i].is_PK){
            if (ex.data_list[i] == list[i]){
                throw SQLError::KEY_INSERT_ERROR();
                return false;
            }
        }
    }
    return true;
}

void RM::InsertTuple(void *destination, TableInfo &table, std::vector<DataClass> &list)
{
    Tuple t(table, list);
    void *p = t.GetWriteSource();
    memcpy(destination, p, table.CalTupleSize());
}

bool RM::DeleteCheck(std::vector<SelectCondition> &conditions, const void *source, TableInfo &info)
{
    Tuple tuple(info);
    tuple.ReadFrom((char*)source);
    for (int i = 0; i < conditions.size(); i++)
    {
        if (!Satisfy(conditions[i], tuple, info))
            return false;
    }
    return true;
}

void* RM::GetSource(Tuple *t)
{
    return t->GetWriteSource();
}