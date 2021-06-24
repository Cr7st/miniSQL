#include "GlobalClass.h"
#include <cstring>

std::ostream& operator<<(std::ostream &out, const DataClass &obj){
    if (obj.type == DataType::INT){
        int i;
        memcpy(&i, &obj.data, 4);
        out << i;
        return out;
    }
    else if (obj.type == DataType::FLOAT){
        double f;
        memcpy(&f, &obj.data, 8);
        out << f;
        return out;
    }
    else{
        char s[30];
        memcpy(s, &obj.data, 30);
        std::string str(s);
        out << str;
        return out;
    }
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
    return !(*this <= rhs);
}