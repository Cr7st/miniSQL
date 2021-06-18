#include "CatalogManager.h"
#include "./ERROR/Error.h"
#include <string>
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


bool CM::NewInfoCheck(TableInfo &table)
{
    for (int i = 0; i < ex_tables.size(); i++){
        if (table.table_name == ex_tables[i].table_name){
            throw SQLError::TABLE_ERROR("There is already a table named " + table.table_name + "!");
            return false;
        }
    }
    for (int i = 0; i < table.n_columns(); i++){
        for (int j = 0; j < table.n_columns(); j++){
            if (i == j) continue;
            else{
                if (table.columns[i].column_name == table.columns[j].column_name){
                    throw SQLError::TABLE_ERROR("There is an name conflict between attributes!");
                    return false;
                }
            }
        }
    }
    return true;
}


TableInfo& CM::InitTableInfo(std::string table_name, std::vector<std::string> &column_names, 
                             std::vector<std::string> &data_types, int PK_index)
{
    TableInfo *table = new TableInfo;
    table->table_name = table_name;
    for (int i = 0; i < column_names.size(); i++){
        table->columns[i].has_index = false;
        table->columns[i].column_name = column_names[i];
        if (data_types[i].find("unique") != data_types[i].npos){
            table->columns[i].is_unique = true;
            // discard the prefix "unique"
            data_types[i] = data_types[i].substr(6, data_types[i].length() - 6);
        }
        else{
            table->columns[i].is_unique = false;
        }
        if (data_types[i] == "int"){
            table->columns[i].type = DataType::INT;
            table->columns[i].bytes = sizeof(int);
        }
        else if (data_types[i] == "float"){
            table->columns[i].type = DataType::FLOAT;
            table->columns[i].bytes = sizeof(double);
        }
        else if (data_types[i].substr(0, 4) == "char"){
            table->columns[i].type = DataType::CHAR;
            // get the length of char
            std::string str_size = data_types[i].substr(4, data_types[i].length() - 4);
            table->columns[i].bytes = atoi(str_size.c_str()) * sizeof(char);
        }
    }
    if (PK_index != -1) // this table has an primary key
        table->columns[PK_index].is_PK = true;
    return *table;
}
