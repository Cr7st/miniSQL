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
                memcpy(source + offset, &(data_list[i]->data.i), temp_size);
                break;
            case DataType::FLOAT:
                temp_size = sizeof(double);
                memcpy(source + offset, &(data_list[i]->data.f), temp_size);
                break;
            case DataType::CHAR:
                temp_size = sizeof(char) * ((CharData*)data_list[i])->get_length();
                memcpy(source + offset, (data_list[i]->data.str), temp_size);
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
                memcpy(&(data_list[i]->data.i), source, temp_size);
                break;
            case DataType::FLOAT:
                temp_size = sizeof(double);
                memcpy(&(data_list[i]->data.f), source + offset, temp_size);
                break;
            case DataType::CHAR:
                temp_size = sizeof(char) * ((CharData*)data_list[i])->get_length();
                memcpy((data_list[i]->data.str), source + offset, temp_size);
                break;
            default: break;
        }
        offset += temp_size;
    }
}