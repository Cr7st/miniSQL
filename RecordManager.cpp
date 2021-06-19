#include "RecordManager.h"
#include <string>

Tuple::Tuple(){}

Tuple::Tuple(TableInfo &info)
{
    tuple_size = info.CalTupleSize();
    DataBaseClass *ptr;
    std::string str("");
    for (int i = 0; i < info.n_columns(); i++)
    {
        switch (info.columns[i].type){
            case DataType::INT:
                ptr = new IntData;
                data_list.push_back(ptr);
                break;
            case DataType::FLOAT:
                ptr = new FloatData;
                data_list.push_back(ptr);
                break;
            case DataType::CHAR:
                ptr = new CharData(str.c_str(), info.columns[i].bytes / sizeof(char));
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
    for (int i = 0; i < data_list.size(); i++){
        switch (data_list[i]->type){
            case DataType::INT:
                temp_size = sizeof(int);
                memcpy((char*)source + offset, &(data_list[i]->data.i), temp_size);
                break;
            case DataType::FLOAT:
                temp_size = sizeof(double);
                memcpy((char*)source + offset, &(data_list[i]->data.f), temp_size);
                break;
            case DataType::CHAR:
                temp_size = sizeof(char) * ((CharData*)data_list[i])->get_length();
                memcpy((char*)source + offset, (data_list[i]->data.str), temp_size);
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
        switch (data_list[i]->type){
            case DataType::INT:
                temp_size = sizeof(int);
                memcpy(&(data_list[i]->data.i), (char*)source, temp_size);
                break;
            case DataType::FLOAT:
                temp_size = sizeof(double);
                memcpy(&(data_list[i]->data.f), (char*)source + offset, temp_size);
                break;
            case DataType::CHAR:
                temp_size = sizeof(char) * ((CharData*)data_list[i])->get_length();
                memcpy((data_list[i]->data.str), (char*)source + offset, temp_size);
                break;
            default: break;
        }
        offset += temp_size;
    }
}


void RM::SelectTuple(std::vector<SelectCondition> &conditions, void* source, TableInfo info)
{
    Tuple tuple;
    tuple.ReadFrom(source);
    for (int i = 0; i < conditions.size(); i++)
    {
        if (!Satisfy(conditions[i], tuple))
            return;
    }
    select_rs.push_back(tuple);
}

void RM::GetSelectRS(std::vector<Tuple> result)
{
    for (int i = 0; i < select_rs.size(); i++){
        result.push_back(select_rs[i]);
    }
    select_rs.clear();
}

bool RM::Satisfy(SelectCondition &condition, Tuple &tuple)
{
    return true;
}