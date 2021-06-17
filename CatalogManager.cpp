#include "CatalogManager.h"
#include <string.h>
#include <vector>
#include <iterator>

std::ostream& operator<<(std::ostream &out, const DataBaseClass &obj){
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
IntData::IntData(int i){
    type = DataType::INT;
    data.i = i;
};

FloatData::FloatData(double f){
    type = DataType::FLOAT;
    data.f = f;
};

CharData::CharData(const char str[], int n){
    type = DataType::CHAR;
    length = n;
    data.str = new char[n+1];
    strcpy(data.str, str);
};

CharData::~CharData(){
    delete[] data.str;
}

CharData::CharData(){};

int CharData::get_length() const{
    return length;
}

int TableInfo::CalTupleSize() const
{
    int size = 0;
    for (auto vi = columns.begin(); vi != columns.end(); vi++){
        size += vi->bytes;
    }
    return size;
}

int TableInfo::n_columns() const
{
    return columns.size();
}