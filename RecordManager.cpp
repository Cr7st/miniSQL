#include "RecordManager.h"
#include <string>

std::ostream& operator<<(std::ostream &out, const DataClass &obj){
    switch (obj.type)
    {
    case DataType::INT:
        out << obj.data.i;
        break;
    case DataType::FLOAT:
        out << obj.data.f;
        break;
    case DataType::CHAR:
        out << obj.data.str;
        break;
    default:
        break;
    }
    return out;
}


//IntData::IntData() : DataType(DataType::INT){};
DataClass::DataClass(){}

DataClass::DataClass(int i){
    type = DataType::INT;
    data.i = i;
    bytes = 4;
}


DataClass::DataClass(double f){
    type = DataType::FLOAT;
    data.f = f;
    bytes = 8;
}


DataClass::DataClass(std::string str){
    type = DataType::CHAR;
    bytes = str.length();
    data.str = new char[bytes+1];
    strcpy(data.str, str.c_str());
}

bool DataClass::operator==(const DataClass &rhs)
{
    switch (this->type)
    {
    case DataType::INT:
        return this->data.i == rhs.data.i;
        break;
    case DataType::FLOAT:
        return this->data.f == rhs.data.f;
        break;
    case DataType::CHAR:
        if (strcmp(this->data.str, rhs.data.str) == 0)
            return true;
        else return false;
        break;
    default:
        break;
    }
}

bool DataClass::operator<(const DataClass &rhs)
{
    switch (this->type)
    {
    case DataType::INT:
        return this->data.i < rhs.data.i;
        break;
    case DataType::FLOAT:
        return this->data.f < rhs.data.f;
        break;
    case DataType::CHAR:
        if (strcmp(this->data.str, rhs.data.str) < 0)
            return true;
        else return false;
        break;
    default:
        break;
    }
}
bool DataClass::operator<=(const DataClass &rhs)
{
    return *this < rhs || *this == rhs;
}
bool DataClass::operator>=(const DataClass &rhs)
{
    return !(*this < rhs);
}
bool DataClass::operator>(const DataClass &rhs)
{
    return !(*this >= rhs);
}


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
                data_list.push_back(ptr);
                ptr.data.i = list[i].data.i;
                break;
            case DataType::FLOAT:
                ptr.type = DataType::FLOAT;
                ptr.bytes = 8;
                data_list.push_back(ptr);
                ptr.data.f = list[i].data.f;
                break;
            case DataType::CHAR:
                ptr.type = DataType::CHAR;
                ptr.bytes = info.columns[i].bytes;
                data_list.push_back(ptr);
                ptr.data.str = (char*)malloc(ptr.bytes);
                strcpy(ptr.data.str, list[i].data.str);
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

void Tuple::ReadFrom(void *source)
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


void RM::SelectTuple(std::vector<SelectCondition> &conditions, void* source, TableInfo &info)
{
    Tuple tuple(info);
    tuple.ReadFrom(source);
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
        if (condition.on_attr == info.columns[i].column_name){
            if (condition.op == "=="){
                return tuple.data_list[i] == condition.cmp_value;
            }
            else if (condition.op == "<"){
                return tuple.data_list[i] < condition.cmp_value;
            }
            else if (condition.op == "<="){
                return tuple.data_list[i] <= condition.cmp_value;
            }
            else if (condition.op == ">"){
                return tuple.data_list[i] > condition.cmp_value;
            }
            else if (condition.op == ">="){
                return tuple.data_list[i] >= condition.cmp_value;
            }
        }
    }
    std::string e("There is an error in the select condition!");
    throw SQLError::TABLE_ERROR(e);
    return false;
}

bool RM::InsertCheck(void *source, TableInfo &table, std::vector<DataClass> &list)
{
    Tuple ex(table);
    ex.ReadFrom(source);
    if (ex.data_list[table.PK_index] == list[table.PK_index]){
        std::string e("Primary key conflict!");
        throw SQLError::TABLE_ERROR(e);
        return false;
    }
    else
        return true;
}

void RM::InsertTuple(void *destination, TableInfo &table, std::vector<DataClass> &list)
{
    Tuple t(table, list);
    void *p = t.GetWriteSource();
    memcpy(destination, p, table.CalTupleSize());
}

bool RM::DeleteCheck(std::vector<SelectCondition> &conditions, void *source, TableInfo &info)
{
    Tuple tuple(info);
    tuple.ReadFrom(source);
    for (int i = 0; i < conditions.size(); i++)
    {
        if (!Satisfy(conditions[i], tuple, info))
            return false;
    }
    return true;
}