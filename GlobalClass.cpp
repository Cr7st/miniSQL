#include "GlobalClass.h"
#include <cstring>

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
        out << std::string(obj.data.str);
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
        return false;
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
            return false;
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